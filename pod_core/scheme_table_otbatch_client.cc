#include "scheme_table_otbatch_client.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_table.h"
#include "scheme_table_b.h"
#include "scheme_table_otbatch_protocol.h"
#include "vrf.h"

namespace {

void CheckDemandPhantoms(uint64_t n, std::vector<Range> const& demands,
                         std::vector<Range> const& phantoms) {
  if (demands.empty() || phantoms.empty()) throw std::invalid_argument("empty");

  for (auto const& demand : demands) {
    if (!demand.count || demand.start >= n || demand.count > n ||
        (demand.start + demand.count) > n)
      throw std::invalid_argument("demand");
  }

  for (auto const& phantom : phantoms) {
    if (!phantom.count || phantom.start >= n || phantom.count > n ||
        (phantom.start + phantom.count) > n)
      throw std::invalid_argument("phantom");
  }

  for (size_t i = 1; i < demands.size(); ++i) {
    if (demands[i].start <= demands[i - 1].start + demands[i - 1].count)
      throw std::invalid_argument("demand overlap");
  }

  for (size_t i = 1; i < phantoms.size(); ++i) {
    if (phantoms[i].start <= phantoms[i - 1].start + phantoms[i - 1].count)
      throw std::invalid_argument("phantoms overlap");
  }

  for (size_t i = 0; i < demands.size(); ++i) {
    auto d_start = demands[i].start;
    auto d_end = d_start + demands[i].count;
    bool find = false;
    for (size_t j = 0; j < phantoms.size(); ++j) {
      auto p_start = phantoms[j].start;
      auto p_end = p_start + phantoms[j].count;
      if (p_start <= d_start && p_end >= d_end) {
        find = true;
        break;
      }
    }
    if (!find) throw std::invalid_argument("phantoms must include demand");
  }
}

uint64_t GetRangesOffsetByIndexOfM(std::vector<Range> const& ranges,
                                   uint64_t index) {
  uint64_t offset = 0;
  for (auto const& range : ranges) {
    if (index >= range.start && index < (range.start + range.count)) {
      offset += index - range.start;
      return offset;
    }
    offset += range.count;
  }
  assert(false);
  throw std::runtime_error(__FUNCTION__);
}

}  // namespace

namespace scheme::table::otbatch {

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               std::vector<Range> demands, std::vector<Range> phantoms)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      demands_(std::move(demands)),
      phantoms_(std::move(phantoms)) {
  CheckDemandPhantoms(n_, demands_, phantoms_);
  for (auto const& i : demands_) demands_count_ += i.count;
  for (auto const& i : phantoms_) phantoms_count_ += i.count;
  BuildMapping();

  seed2_seed_ = misc::RandH256();

  ot_self_pk_ = G1Rand();
  ot_beta_ = FrRand();
  ot_rand_a_ = FrRand();
  ot_rand_b_ = FrRand();
}

void Client::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(demands_count_);
  size_t index = 0;
  for (auto const& d : demands_) {
    for (size_t i = d.start; i < (d.start + d.count); ++i) {
      auto& map = mappings_[index];
      map.global_index = i;
      map.phantom_offset = GetRangesOffsetByIndexOfM(phantoms_, i);
      ++index;
    }
  }
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
  request.seed2_seed = seed2_seed_;
  request.phantoms = phantoms_;

  request.ot_vi.reserve(demands_count_);
  for (auto const& i : demands_) {
    for (size_t j = i.start; j < i.start + i.count; ++j) {
      auto fr = MapToFr(j);
      request.ot_vi.push_back(ot_sk_ * (ot_rand_b_ * fr));
    }
  }
  request.ot_v = ot_self_pk_ * (ot_rand_a_ * ot_rand_b_);
}

bool Client::OnResponse(Response response, Receipt& receipt) {
  Tick _tick_(__FUNCTION__);
  if (response.k.size() != phantoms_count_ * s_) {
    assert(false);
    return false;
  }
  if (response.ot_ui.size() != demands_count_) {
    assert(false);
    return false;
  }
  if (response.m.size() != phantoms_count_ * s_) {
    assert(false);
    return false;
  }

  k_ = std::move(response.k);
  ot_ui_ = std::move(response.ot_ui);
  k_mkl_root_ = CalcRootOfK(k_);
  seed2_ = CalcSeed2(seed2_seed_, k_mkl_root_);

  H2(seed2_, phantoms_count_, w_);

  encrypted_m_.resize(demands_count_ * s_);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)ot_ui_.size(); ++i) {
    Fp12 e;
    G1 ui_exp_a = ot_ui_[i] * ot_rand_a_;
    mcl::bn256::pairing(e, ui_exp_a, ot_peer_pk_);
    uint8_t buf[32 * 12];
    auto ret_len = e.serialize(buf, sizeof(buf));
    if (ret_len != sizeof(buf)) {
      assert(false);
      throw std::runtime_error("oops");
    }
    Fr fr_e = MapToFr(buf, sizeof(buf));

    auto phantom_offset = mappings_[i].phantom_offset;
    for (size_t j = 0; j < s_; ++j) {
      encrypted_m_[i * s_ + j] = response.m[phantom_offset * s_ + j] - fr_e;
    }
  }

  if (!CheckEncryptedM()) {
    assert(false);
    return false;
  }

  receipt.count = phantoms_count_;
  receipt.k_mkl_root = k_mkl_root_;
  receipt.seed2 = seed2_;
  return true;
}

bool Client::CheckEncryptedM() {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  auto const& sigmas = b_->sigmas();

  // uint64_t phantom_offset = phantom_.start - demand_.start;
  int not_equal = 0;
#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    if (not_equal) continue;
    auto const& mapping = mappings_[i];
    G1 const& sigma = sigmas[mapping.global_index];
    G1 left = sigma * w_[mapping.phantom_offset];
    for (uint64_t j = 0; j < s_; ++j) {
      left += k_[mapping.phantom_offset * s_ + j];
    }
    G1 right = G1Zero();
    for (uint64_t j = 0; j < s_; ++j) {
      Fr const& m = encrypted_m_[i * s_ + j];
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

bool Client::OnSecret(Secret const& secret) {
  Tick _tick_(__FUNCTION__);

  // compute v
  std::vector<Fr> v;
  H2(secret.seed0, phantoms_count_ * s_, v);

  if (!CheckK(v)) {
    assert(claim_i_ >= 0 && claim_j_ >= 0);
    return false;
  } else {
    DecryptM(v);
    return true;
  }
}

bool Client::GenerateClaim(Claim& claim) {
  if (claim_i_ == -1 || claim_j_ == -1) {
    assert(false);
    return false;
  }
  BuildClaim(claim_i_, claim_j_, claim);
  return true;
}

bool Client::CheckK(std::vector<Fr> const& v) {
  if (v.size() > (1024 * 1024) && omp_get_max_threads() < 3) {
    return CheckKMultiExp(v);
  } else {
    return CheckKDirect(v);
  }
}

bool Client::CheckKDirect(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);

  // compute k
  std::vector<G1> k;
  BuildK(v, k, s_);

  // compare k
  for (uint64_t i = 0; i < phantoms_count_; ++i) {
    for (uint64_t j = 0; j < s_; ++j) {
      auto offset = i * s_ + j;
      if (k[offset] == k_[offset]) continue;
      claim_i_ = i;
      claim_j_ = j;
      return false;
    }
  }
  return true;
}

bool Client::CheckKMultiExp(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  uint64_t mismatch_j = (uint64_t)(-1);
  for (uint64_t j = 0; j < s_; ++j) {
    Fr sigma_vij = FrZero();
    std::vector<G1 const*> k(phantoms_count_);
    for (uint64_t i = 0; i < phantoms_count_; ++i) {
      sigma_vij += v[i * s_ + j] * w_[i];
      k[i] = &k_[i * s_ + j];
    }

    G1 check_sigma_kij = ecc_pub.PowerU1(j, sigma_vij);
    G1 sigma_kij = MultiExpBdlo12(k, w_, 0, phantoms_count_);
    if (check_sigma_kij != sigma_kij) {
      mismatch_j = j;
      break;
    }
  }

  if (mismatch_j == (uint64_t)(-1)) return true;

  std::vector<G1 const*> k_col(phantoms_count_);
  std::vector<Fr const*> v_col(phantoms_count_);
  for (uint64_t i = 0; i < phantoms_count_; ++i) {
    auto offset = i * s_ + mismatch_j;
    k_col[i] = &k_[offset];
    v_col[i] = &v[offset];
  }

  uint64_t mismatch_i = FindMismatchI(mismatch_j, k_col, v_col);
  if (mismatch_i == (uint64_t)(-1)) {
    assert(false);
    throw std::runtime_error("oops! FindMismatchI failed to find mismatch i");
  }

  claim_i_ = mismatch_i;
  claim_j_ = mismatch_j;

  return false;
}

void Client::BuildClaim(uint64_t i, uint64_t j, Claim& claim) {
  Tick _tick_(__FUNCTION__);
  claim.i = i;
  claim.j = j;
  auto ij = i * s_ + j;
  claim.kij = k_[ij];
  auto root = CalcPathOfK(k_, ij, claim.mkl_path);
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
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    auto const& mapping = mappings_[i];
    Fr inv_w = FrInv(w_[mapping.phantom_offset]);
    for (uint64_t j = 0; j < s_; ++j) {
      encrypted_m_[i * s_ + j] =
          (encrypted_m_[i * s_ + j] - v[mapping.phantom_offset * s_ + j]) *
          inv_w;
    }
  }

  decrypted_m_ = std::move(encrypted_m_);
}

bool Client::SaveDecrypted(std::string const& file) {
  Tick _tick_(__FUNCTION__);

  return DecryptedRangeMToFile(file, s_, b_->vrf_meta(), demands_,
                               decrypted_m_);
}
}  // namespace scheme::table::otbatch
