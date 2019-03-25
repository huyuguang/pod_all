#include "scheme_plain_otrange_client.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain.h"
#include "scheme_plain_b.h"
#include "scheme_plain_protocol.h"
#include "vrf.h"

namespace scheme::plain::otrange {

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               Range const& demand, Range const& phantom)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      demand_(demand),
      phantom_(phantom) {
  if (!demand_.count || demand_.start >= n_ || demand_.count > n_ ||
      (demand_.start + demand_.count) > n_)
    throw std::invalid_argument("demand");

  if (!phantom_.count || phantom_.start >= n_ || phantom_.count > n_ ||
      (phantom_.start + phantom_.count) > n_)
    throw std::invalid_argument("phantom");

  if (phantom_.start < demand_.start || phantom_.count < demand_.count)
    throw std::invalid_argument("phantom");

  seed2_ = misc::RandMpz32();

  ot_self_pk_ = G1Rand();
  ot_beta_ = FrRand();
  ot_rand_a_ = FrRand();
  ot_rand_b_ = FrRand();
}

void Client::GetNegoReqeust(NegoBRequest& request) { request.t = ot_self_pk_; }

bool Client::OnNegoRequest(NegoARequest const& request,
                           NegoAResponse& response) {
  ot_peer_pk_ = request.s;
  response.s_exp_beta = ot_peer_pk_ * ot_beta_;
  return true;
}

bool Client::OnNegoResponse(NegoBResponse const& response) {
  ot_sk_ = response.t_exp_alpha * ot_beta_;
  return true;
}

void Client::GetRequest(Request& request) {
  request.start = phantom_.start;
  request.count = phantom_.count;

  request.ot_vi.resize(demand_.count);
  for (size_t i = 0; i < request.ot_vi.size(); ++i) {
    auto fr = MapToFr(i + demand_.start);
    request.ot_vi[i] = ot_sk_ * (ot_rand_b_ * fr);
  }
  request.ot_v = ot_self_pk_ * (ot_rand_a_ * ot_rand_b_);
}

bool Client::OnResponse(Response response, Challenge& challenge) {
  Tick _tick_(__FUNCTION__);
  if (response.k.size() != phantom_.count * s_) {
    assert(false);
    return false;
  }
  if (response.ot_ui.size() != demand_.count) {
    assert(false);
    return false;
  }
  response_ = std::move(response);
  challenge.seed2 = seed2_;
  k_mkl_root_ = CalcRootOfK(response_.k);

  return true;
}

bool Client::OnReply(Reply reply, Receipt& receipt) {
  Tick _tick_(__FUNCTION__);

  if (reply.m.size() != phantom_.count * s_) {
    assert(false);
    return false;
  }

  encrypted_m_.resize(demand_.count * s_);
  uint64_t phantom_offset = phantom_.start - demand_.start;
  for (size_t i = 0; i < response_.ot_ui.size(); ++i) {
    Fp12 e;
    G1 ui_exp_a = response_.ot_ui[i] * ot_rand_a_;
    mcl::bn256::pairing(e, ui_exp_a, ot_peer_pk_);
    uint8_t buf[32 * 12];
    auto ret_len = e.serialize(buf, sizeof(buf));
    if (ret_len != sizeof(buf)) {
      assert(false);
      throw std::runtime_error("oops");
    }
    Fr fr_e = MapToFr(buf, sizeof(buf));
    for (size_t j = 0; j < s_; ++j) {
      encrypted_m_[i * s_ + j] = reply.m[(phantom_offset + i) * s_ + j] - fr_e;
    }    
  }

  H2(seed2_, demand_.count, w_);

  if (!CheckEncryptedM(reply.m)) {
    assert(false);
    return false;
  }

  receipt.k_mkl_root = k_mkl_root_;
  receipt.seed2 = seed2_;

  return true;
}

bool Client::CheckEncryptedM(std::vector<Fr> const& encrypted_m) {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  auto const& sigmas = b_->sigmas();

  int not_equal = 0;
#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)demand_.count; ++i) {
    if (not_equal) continue;
    G1 const& sigma = sigmas[demand_.start + i];
    G1 left = sigma * w_[i];
    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      left += response_.k[is + j];
    }
    G1 right = G1Zero();
    for (uint64_t j = 0; j < s_; ++j) {
      Fr const& m = encrypted_m[is + j];
      right += ecc_pub.PowerU1(j, m);
    }
    if (left != right) {
#pragma omp atomic
      ++not_equal;
    }
  }

  if (not_equal) {
    assert(false);
    return false;
  }
  return true;
}

bool Client::OnSecret(Secret const& secret, Claim& claim) {
  Tick _tick_(__FUNCTION__);

  // compute v
  std::vector<Fr> v;
  H2(secret.seed0, demand_.count * s_, v);

  if (!CheckK(v, claim)) return false;

  DecryptM(v);

  return true;
}

bool Client::CheckK(std::vector<Fr> const& v, Claim& claim) {
  if (v.size() > (1024 * 1024) && omp_get_max_threads() < 3) {
    return CheckKMultiExp(v, claim);
  } else {
    return CheckKDirect(v, claim);
  }
}

bool Client::CheckKDirect(std::vector<Fr> const& v, Claim& claim) {
  Tick _tick_(__FUNCTION__);

  // compute k
  std::vector<G1> k;
  BuildK(v, k, s_);

  // compare k
  for (uint64_t i = 0; i < demand_.count; ++i) {
    for (uint64_t j = 0; j < s_; ++j) {
      auto offset = i * s_ + j;
      if (k[offset] == response_.k[offset]) continue;
      BuildClaim(i, j, claim);
      return false;
    }
  }
  return true;
}

bool Client::CheckKMultiExp(std::vector<Fr> const& v, Claim& claim) {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  uint64_t mismatch_j = (uint64_t)(-1);
  for (uint64_t j = 0; j < s_; ++j) {
    Fr sigma_vij = FrZero();
    std::vector<G1 const*> k(demand_.count);
    for (uint64_t i = 0; i < demand_.count; ++i) {
      sigma_vij += v[i * s_ + j] * w_[i];
      k[i] = &response_.k[i * s_ + j];
    }

    G1 check_sigma_kij = ecc_pub.PowerU1(j, sigma_vij);
    G1 sigma_kij = MultiExpBdlo12(k, w_, 0, demand_.count);
    if (check_sigma_kij != sigma_kij) {
      mismatch_j = j;
      break;
    }
  }

  if (mismatch_j == (uint64_t)(-1)) return true;

  std::vector<G1 const*> k_col(demand_.count);
  std::vector<Fr const*> v_col(demand_.count);
  for (uint64_t i = 0; i < demand_.count; ++i) {
    auto offset = i * s_ + mismatch_j;
    k_col[i] = &response_.k[offset];
    v_col[i] = &v[offset];
  }

  uint64_t mismatch_i = FindMismatchI(mismatch_j, k_col, v_col);
  if (mismatch_i == (uint64_t)(-1)) {
    assert(false);
    throw std::runtime_error("oops! FindMismatchI failed to find mismatch i");
  }

  BuildClaim(mismatch_i, mismatch_j, claim);

  return false;
}

void Client::BuildClaim(uint64_t i, uint64_t j, Claim& claim) {
  Tick _tick_(__FUNCTION__);
  claim.i = i;
  claim.j = j;
  auto ij = i * s_ + j;
  claim.kij = response_.k[ij];
  auto root = mkl::CalcPath(
      [this](uint64_t i) -> h256_t {
        assert(i < response_.k.size());
        return G1ToBin(response_.k[i]);
      },
      demand_.count* s_, ij, &claim.mkl_path);
  if (root != k_mkl_root_) {
    assert(false);
    throw std::runtime_error("oops, mkl root mismatch");
  }
}

uint64_t Client::FindMismatchI(uint64_t mismatch_j,
                               std::vector<G1 const*> const& k_col,
                               std::vector<Fr const*> const& v_col) {
  Tick _tick_(__FUNCTION__);

  assert(k_col.size() == v_col.size() && !k_col.empty());

  auto const& ecc_pub = GetEccPub();
  uint64_t offset = 0;
  uint64_t count = k_col.size();

  for (;;) {
    if (count == 1) {
      auto check_k = ecc_pub.PowerU1(mismatch_j, *v_col[offset]);
      return (check_k == *k_col[offset]) ? (uint64_t)(-1) : offset;
    }

    uint64_t half_len = count / 2;
    Fr sigma_vij = FrZero();
    for (uint64_t i = 0; i < half_len; ++i) {
      sigma_vij += (*v_col[offset + i]) * w_[offset + i];
    }
    G1 check_sigma_kij = ecc_pub.PowerU1(mismatch_j, sigma_vij);
    G1 sigma_kij = MultiExpBdlo12(k_col, w_, offset, half_len);

    if (check_sigma_kij != sigma_kij) {
      count = half_len;
    } else {
      offset += half_len;
      count -= half_len;
    }
  }
}

void Client::DecryptM(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);

  std::vector<Fr> inv_w(demand_.count);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)demand_.count; ++i) {
    inv_w[i] = FrInv(w_[i]);
  }

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)demand_.count; ++i) {
    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      encrypted_m_[ij] = (encrypted_m_[ij] - v[ij]) * inv_w[i];
    }
  }

  decrypted_m_ = std::move(encrypted_m_);
}

bool Client::SaveDecrypted(std::string const& file) {
  Tick _tick_(__FUNCTION__);

  return MToFile(file, b_->bulletin().size, s_, demand_.start, demand_.count,
                 decrypted_m_);
}
}  // namespace scheme::plain::otrange
