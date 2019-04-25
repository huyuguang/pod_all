#include "scheme_table_batch2_client.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_table.h"
#include "scheme_table_b.h"
#include "scheme_table_batch2_notary.h"
#include "scheme_table_batch2_protocol.h"
#include "vrf.h"

namespace {

void CheckDemands(uint64_t n, std::vector<Range> const& demands) {
  if (demands.empty()) throw std::invalid_argument("demands empty");

  for (auto const& demand : demands) {
    if (!demand.count || demand.start >= n || demand.count > n ||
        (demand.start + demand.count) > n)
      throw std::invalid_argument("demand");
  }

  for (size_t i = 1; i < demands.size(); ++i) {
    if (demands[i].start <= demands[i - 1].start + demands[i - 1].count)
      throw std::invalid_argument("demand overlap, must combine them");
  }
}
}  // namespace

namespace scheme::table::batch2 {

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               std::vector<Range> demands)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      demands_(std::move(demands)) {
  CheckDemands(n_, demands_);
  for (auto const& i : demands_) demands_count_ += i.count;
  BuildMapping();

  seed2_seed_ = misc::RandH256();
}

void Client::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(demands_count_);
  size_t index = 0;
  for (auto const& d : demands_) {
    for (size_t i = d.start; i < (d.start + d.count); ++i) {
      auto& map = mappings_[index];
      map.global_index = i;
      ++index;
    }
  }
}

void Client::GetRequest(Request& request) {
  request.seed2_seed = seed2_seed_;
  request.demands = demands_;
}

bool Client::OnResponse(Response response, Receipt& receipt) {
  Tick _tick_(__FUNCTION__);
  if (response.k.size() != (demands_count_ + 1) * s_) {
    assert(false);
    return false;
  }
  if (response.m.size() != demands_count_ * s_) {
    assert(false);
    return false;
  }
  if (response.vw.size() != s_) {
    assert(false);
    return false;
  }

  k_ = std::move(response.k);

  vw_ = std::move(response.vw);

  seed2_ = CalcSeed2(seed2_seed_, CalcRootOfK(k_));

  H2(seed2_, demands_count_, w_);

  encrypted_m_ = std::move(response.m);

  if (!CheckEncryptedM()) {
    assert(false);
    return false;
  }

  if (!CheckKVW()) {
    assert(false);
    return false;
  }

  sigma_vw_ = FrZero();
  for (auto const& i : vw_) {
    sigma_vw_ += i;
  }

  receipt.count = demands_count_;
  receipt.seed2 = seed2_;
  receipt.sigma_vw = sigma_vw_;

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

bool Client::CheckKVW() {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  G1 right = G1Zero();
  for (size_t j = 0; j < s_; ++j) {
    right += ecc_pub.PowerU1(j, vw_[j]);
  }

  G1 left = G1Zero();
  size_t offset = demands_count_ * s_;
  for (size_t i = offset; i < offset + s_; ++i) {
    left += k_[i];
  }

  std::vector<G1> sigma_k(demands_count_);
  for (size_t i = 0; i < demands_count_; ++i) {
    sigma_k[i] = G1Zero();
    for (size_t j = 0; j < s_; ++j) {
      sigma_k[i] += k_[i * s_ + j];
    }
  }

  left += MultiExpBdlo12(sigma_k, w_);

  bool ret = right == left;
  assert(ret);
  return ret;
}

bool Client::OnSecret(Secret const& secret) {
  Tick _tick_(__FUNCTION__);

  // compute v
  std::vector<Fr> v;
  H2(secret.seed0, (demands_count_ + 1) * s_, v);

  if (!VerifyProof(s_, demands_count_, sigma_vw_, v, w_)) {
    // assert(false);
    return false;
  }

  DecryptM(v);

  return true;
}

void Client::DecryptM(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
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

  return DecryptedRangeMToFile(file, s_, b_->vrf_meta(), demands_,
                               decrypted_m_);
}
}  // namespace scheme::table::batch2
