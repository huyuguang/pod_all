#include "scheme_plain_range_client.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain.h"
#include "scheme_plain_b.h"
#include "scheme_plain_protocol.h"
#include "vrf.h"

namespace scheme::plain::range {

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               Range const& demand)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      demand_(demand) {
  seed2_seed_ = misc::RandH256();
  if (!demand_.count) throw std::invalid_argument("empty range");
}

void Client::GetRequest(Request& request) {
  request.demand = demand_;
  request.seed2_seed = seed2_seed_;
}

bool Client::OnResponse(Response response, Receipt& receipt) {
  Tick _tick_(__FUNCTION__);

  if (response.k.size() != demand_.count * s_) {
    assert(false);
    return false;
  }

  if (response.m.size() != demand_.count * s_) {
    assert(false);
    return false;
  }

  k_ = std::move(response.k);
  k_mkl_root_ = CalcRootOfK(k_);
  seed2_ = CalcSeed2(seed2_seed_, k_mkl_root_);

  H2(seed2_, demand_.count, w_);

  encrypted_m_ = std::move(response.m);

  if (!CheckEncryptedM()) {
    assert(false);
    return false;
  }

  receipt.count = demand_.count;
  receipt.k_mkl_root = k_mkl_root_;
  receipt.seed2 = seed2_;

  return true;
}

bool Client::CheckEncryptedM() {
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
      left += k_[is + j];
    }
    G1 right = G1Zero();
    for (uint64_t j = 0; j < s_; ++j) {
      Fr const& m = encrypted_m_[is + j];
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
      if (k[offset] == k_[offset]) continue;
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
      k[i] = &k_[i * s_ + j];
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
    k_col[i] = &k_[offset];
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
  claim.kij = k_[ij];
  auto root = mkl::CalcPath(
      [this](uint64_t i) -> h256_t {
        assert(i < k_.size());
        return G1ToBin(k_[i]);
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

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)demand_.count; ++i) {
    Fr inv_w = FrInv(w_[i]);
    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      encrypted_m_[ij] = (encrypted_m_[ij] - v[ij]) * inv_w;
    }
  }

  decrypted_m_ = std::move(encrypted_m_);
}

bool Client::SaveDecrypted(std::string const& file) {
  Tick _tick_(__FUNCTION__);

  return DecryptedMToFile(file, b_->bulletin().size, s_, demand_.start,
                          demand_.count, decrypted_m_);
}
}  // namespace scheme::plain::range
