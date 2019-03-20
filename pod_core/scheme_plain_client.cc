#include "scheme_plain_client.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain.h"
#include "scheme_plain_b.h"
#include "scheme_plain_protocol.h"
#include "vrf.h"

namespace scheme_misc::plain {

namespace {

void BuildK(std::vector<Fr> const& v, std::vector<G1>& k) {
  k.resize(v.size());
  auto const& ecc_pub = GetEccPub();

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)v.size(); ++i) {
    k[i] = ecc_pub.PowerG1(v[i]);
  }
}

}  // namespace

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               uint64_t start, uint64_t count)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      start_(start),
      count_(count) {
  seed2_ = misc::RandMpz32();
}

bool Client::OnRangeResponse(RangeResponse const& response,
                             RangeChallenge& challenge) {
  Tick _tick_(__FUNCTION__);

  if (response.k.size() != count_ * s_) {
    assert(false);
    return false;
  }

  response_ = response;
  challenge.seed2 = seed2_;
  k_mkl_root_ = CalcRootOfK(response_.k);
  return true;
}

bool Client::OnRangeReply(RangeReply const& reply, RangeReceipt& receipt) {
  Tick _tick_(__FUNCTION__);

  if (reply.m.size() != count_ * s_) {
    assert(false);
    return false;
  }

  reply_ = reply;

  H2(seed2_, count_, w_);

  auto const& ecc_pub = GetEccPub();
  auto const& sigmas = b_->sigmas();

  int not_equal_times = 0;
#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)count_; ++i) {
    G1 const& sigma = sigmas[start_ + i];
    G1 left = sigma * w_[i];
    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      left += response_.k[is + j];
    }
    G1 right = G1Zero();
    for (uint64_t j = 0; j < s_; ++j) {
      Fr const& m = reply_.m[is + j];
      right += ecc_pub.PowerU1(j, m);
    }
    if (left != right) {
#pragma omp atomic
      ++not_equal_times;
    }
  }

  if (not_equal_times) {
    assert(false);
    return false;
  }

  receipt.k_mkl_root = k_mkl_root_;
  receipt.seed2 = seed2_;

  return true;
}

bool Client::OnRangeSecret(RangeSecret const& secret, RangeClaim& claim) {
  Tick _tick_(__FUNCTION__);

  // compute v
  std::vector<Fr> v;
  H2(secret.seed0, count_ * s_, v);

  if (!CheckK(v, claim)) return false;

  DecryptM(v);

  return true;
}

bool Client::CheckK(std::vector<Fr> const& v, RangeClaim& claim) {
  Tick _tick_(__FUNCTION__);

#if 0
  // compute k
  std::vector<G1> k;
  BuildK(v, k, s_);

  // compare k
  for (uint64_t i = 0; i < count_; ++i) {
    for (uint64_t j = 0; j < s_; ++j) {
      auto offset = i * s_ + j;
      if (k[offset] == response_.k[offset]) continue;

      claim.i = i;
      claim.j = j;
      claim.kij = response_.k[offset];
      auto root = mkl::CalcPath(
          [this](uint64_t i) -> h256_t {
            assert(i < response_.k.size());
            return G1ToBin(response_.k[i]);
          },
          count_* s_, offset, &claim.mkl_path);
      assert(root == k_mkl_root_);
      return false;
    }
  }
#endif
  auto const& ecc_pub = GetEccPub();
  uint64_t mismatch_j = (uint64_t)(-1);
  for (uint64_t j = 0; j < s_; ++j) {
    Fr sigma_vij = FrZero();
    for (uint64_t i = 0; i < count_; ++i) {
      sigma_vij += v[i * s_ + j];
    }
    G1 check_sigma_kij = ecc_pub.PowerU1(j, sigma_vij);
    G1 sigma_kij = G1Zero();
    for (uint64_t i = 0; i < count_; ++i) {
      sigma_kij += response_.k[i * s_ + j];
    }

    if (check_sigma_kij != sigma_kij) {
      mismatch_j = j;
      break;
    }
  }

  if (mismatch_j == (uint64_t)(-1)) return true;

  std::vector<G1 const*> k_col(count_);
  std::vector<Fr const*> v_col(count_);
  for (uint64_t i = 0; i < count_; ++i) {
    auto offset = i * s_ + mismatch_j;
    k_col[i] = &response_.k[offset];
    v_col[i] = &v[offset];
  }

  claim.i = FindMismatchI(0, mismatch_j, k_col, v_col);
  if (claim.i == (uint64_t)(-1)) {
    assert(false);
    throw std::runtime_error("oops! FindMismatchI failed to find mismatch i");
  }

  claim.j = mismatch_j;
  auto ij = claim.i * s_ + claim.j;
  claim.kij = response_.k[ij];
  auto root = mkl::CalcPath(
      [this](uint64_t i) -> h256_t {
        assert(i < response_.k.size());
        return G1ToBin(response_.k[i]);
      },
      count_* s_, ij, &claim.mkl_path);
  assert(root == k_mkl_root_);

  return false;
}

// recursive find mismatch i
uint64_t Client::FindMismatchI(uint64_t offset_i, uint64_t mismatch_j,
                               std::vector<G1 const*> const& k_col,
                               std::vector<Fr const*> const& v_col) {
  assert(k_col.size() == v_col.size() && !k_col.empty());

  auto const& ecc_pub = GetEccPub();
  if (k_col.size() == 1) {
    auto check_k = ecc_pub.PowerU1(mismatch_j, *v_col[0]);
    return (check_k == *k_col[0]) ? (uint64_t)(-1) : offset_i;
  }

  uint64_t half_len = k_col.size() / 2;
  std::vector<G1 const*> half_k_col(k_col.begin(), k_col.begin() + half_len);
  std::vector<Fr const*> half_v_col(v_col.begin(), v_col.begin() + half_len);
  auto mismatch_i = FindMismatchI(offset_i, mismatch_j, half_k_col, half_v_col);
  if (mismatch_i != (uint64_t)(-1)) return mismatch_i;

  half_k_col.assign(k_col.begin() + half_len, k_col.end());
  half_v_col.assign(v_col.begin() + half_len, v_col.end());
  mismatch_i =
      FindMismatchI(offset_i + half_len, mismatch_j, half_k_col, half_v_col);
  if (mismatch_i != (uint64_t)(-1)) return mismatch_i;

  return (uint64_t)(-1);
}

void Client::DecryptM(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);

  std::vector<Fr> inv_w(count_);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)count_; ++i) {
    inv_w[i] = FrInv(w_[i]);
  }

  auto& m = reply_.m;

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)count_; ++i) {
    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      m[ij] = (m[ij] - v[ij]) * inv_w[i];
    }
  }

  decrypted_m_ = std::move(reply_.m);
}

bool Client::SaveDecrypted(std::string const& file) {
  Tick _tick_(__FUNCTION__);

  return MToFile(file, b_->bulletin().size, s_, start_, count_, decrypted_m_);
}
}  // namespace scheme_misc::plain