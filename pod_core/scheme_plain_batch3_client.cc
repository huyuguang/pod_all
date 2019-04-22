#include "scheme_plain_batch3_client.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain.h"
#include "scheme_plain_b.h"
#include "scheme_plain_batch3_notary.h"
#include "scheme_plain_batch3_protocol.h"

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

namespace scheme::plain::batch3 {

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               std::vector<Range> demands)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      demands_(std::move(demands)),
      r_(misc::RandH256()) {
  CheckDemands(n_, demands_);
  for (auto const& i : demands_) demands_count_ += i.count;
  BuildMapping();
  align_c_ = misc::Pow2UB(demands_count_);
  align_s_ = misc::Pow2UB(s_);
  log_c_ = misc::Log2UB(align_c_);
  log_s_ = misc::Log2UB(align_s_);
}

void Client::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(demands_count_);
  size_t index = 0;
  for (auto const& d : demands_) {
    for (size_t i = d.start; i < (d.start + d.count); ++i) {
      auto& map = mappings_[index];
      map.index_of_m = i;
      ++index;
    }
  }
}

void Client::GetRequest(Request& request) { request.demands = demands_; }

bool Client::OnCommitment(Commitment commitment, Challenge& challenge) {
  Tick _tick_(__FUNCTION__);
  commitment_ = std::move(commitment);

  // check data format
  if (commitment_.uk.size() != (log_c_ + 1)) {
    assert(false);
    return false;
  }

  for (uint64_t p = 0; p < commitment_.uk.size(); ++p) {
    auto& ukp = commitment_.uk[p];
    auto rows = align_c_ / (1LL << p);
    if (ukp.size() != rows) {
      assert(false);
      return false;
    }
  }

  if (commitment_.ux0.size() != align_s_) {
    assert(false);
    return false;
  }

  if (commitment_.u0x.size() != (log_s_ + 1)) {
    assert(false);
    return false;
  }

  for (uint64_t p = 0; p < commitment_.u0x.size(); ++p) {
    auto& u0xp = commitment_.u0x[p];
    auto cols = align_s_ / (1LL << p);
    if (u0xp.size() != cols) {
      assert(false);
      return false;
    }
  }

  if (commitment_.g2x0.size() != align_s_) {
    assert(false);
    return false;
  }

  if (commitment_.ud.size() != align_s_) {
    assert(false);
    return false;
  }

  // check commitment of d
  if (!CheckCommitmentOfD()) {
    assert(false);
    return false;
  }

  challenge.r = r_;
  ComputeChallenge(r_);
  return true;
}

bool Client::OnResponse(Response response, Receipt& receipt) {
  Tick _tick_(__FUNCTION__);
  response_ = std::move(response);

  // check format
  if (response_.m.size() != demands_count_ * align_s_) {
    assert(false);
    return false;
  }

  if (response_.ek.size() != log_c_) {
    assert(false);
    return false;
  }

  for (size_t p = 0; p < response_.ek.size(); ++p) {
    auto const& ekp = response_.ek[p];
    auto rows = align_c_ / (1LL << p);
    auto cols = align_s_;
    if (ekp.size() != rows * cols) {
      assert(false);
      return false;
    }
  }

  if (response_.ex.size() != log_s_) {
    assert(false);
    return false;
  }

  for (size_t p = 0; p < response_.ex.size(); ++p) {
    auto const& exp = response_.ex[p];
    auto cols = align_s_ / (1LL << (p + 1));
    if (exp.size() != 2 * cols) {
      assert(false);
      return false;
    }
  }

  encrypted_m_ = std::move(response_.m);

  if (!CheckEncryptedM()) {
    assert(false);
    return false;
  }

  if (!CheckUX0()) {
    assert(false);
    return false;
  }

  if (!CheckEK()) {
    assert(false);
    return false;
  }

  if (!CheckEX()) {
    assert(false);
    return false;
  }

  receipt.u0d = commitment_.ud[0];
  receipt.u0_x0_lgs = commitment_.u0x.back()[0];

  receipt_ = receipt;
  return true;
}

bool Client::CheckEncryptedM() {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  auto const& sigmas = b_->sigmas();
  
  auto const& ud = commitment_.ud;
  assert(ud.size() == align_s_);
  G1 sigma_ud = std::accumulate(ud.begin(), ud.end(), G1Zero());

  int not_equal = 0;
#pragma omp parallel for
  for (size_t i = 0; i < mappings_.size(); ++i) {
    if (not_equal) continue;
    auto const& mapping = mappings_[i];
    G1 const& sigma = sigmas[mapping.index_of_m];
    auto const& uk0 = commitment_.uk[0];
    G1 left = sigma * c_;
    left += uk0[i];
    left += sigma_ud;

    G1 right = G1Zero();
    for (uint64_t j = 0; j < align_s_; ++j) {
      Fr const& m = encrypted_m_[i * align_s_ + j];
      right += ecc_pub.PowerU1(j, m);
    }
    if (left != right) {
#pragma omp atomic
      ++not_equal;
      assert(false);
    }
  }

  if (not_equal) {
    assert(false);
    return false;
  }
  return true;
}

bool Client::OnSecret(Secret secret) {
  Tick _tick_(__FUNCTION__);

  secret_ = std::move(secret);

  if (!VerifyProof(receipt_, secret_)) {
    assert(false);
    return false;
  }

  DecryptX();
  DecryptK();
  DecryptM();

  return true;
}

void Client::DecryptM() {
  Tick _tick_(__FUNCTION__);
  
  Fr inv_c = FrInv(c_);

  decrypted_m_.resize(demands_count_ * s_);

  auto const& k0 = k_[0];
#pragma omp parallel for
  for (size_t i = 0; i < mappings_.size(); ++i) {
    for (uint64_t j = 0; j < s_; ++j) {
      auto index1 = i * s_ + j;
      auto index2 = i * align_s_ + j;
      decrypted_m_[index1] =
          (encrypted_m_[index2] - k0[index2] - secret_.d) * inv_c;
    }
  }

#ifdef _DEBUG
  if (!secret_.m.empty()) {
    assert(decrypted_m_ == secret_.m);
  }
#endif
}

bool Client::SaveDecrypted(std::string const& file) {
  Tick _tick_(__FUNCTION__);
  for (auto const& demand : demands_) {
    std::string range_file = file + "_" + std::to_string(demand.start) + "_" +
                             std::to_string(demand.count);
    if (!DecryptedMToFile(range_file, b_->bulletin().size, s_, demand.start,
                          demand.count, decrypted_m_)) {
      assert(false);
      return false;
    }
  }
  return true;
}

void Client::ComputeChallenge(h256_t const& r) {
  static const std::string suffix_c = "challenge_c";
  static const std::string suffix_e1 = "challenge_e1";
  static const std::string suffix_e2 = "challenge_e2";

  h256_t digest;
  CryptoPP::Keccak_256 hash;

  hash.Update(r.data(), r.size());
  hash.Update((uint8_t*)suffix_c.data(), suffix_c.size());
  hash.Final(digest.data());
  c_.setArrayMaskMod(digest.data(), digest.size());
  //std::cout << "client c_: " << c_ << "\n";

  hash.Update(r.data(), r.size());
  hash.Update((uint8_t*)suffix_e1.data(), suffix_e1.size());
  hash.Final(digest.data());
  e1_.setArrayMaskMod(digest.data(), digest.size());
  //std::cout << "client e1_: " << e1_ << "\n";

  hash.Update(r.data(), r.size());
  hash.Update((uint8_t*)suffix_e2.data(), suffix_e2.size());
  hash.Final(digest.data());
  e2_.setArrayMaskMod(digest.data(), digest.size());
  //std::cout << "client e2_: " << e2_ << "\n";

  e1_square_ = e1_ * e1_;
  e2_square_ = e2_ * e2_;
  e1_e2_inverse_ = FrInv(e1_ * e2_square_ - e2_ * e1_square_);
}

bool Client::CheckCommitmentOfD() {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();

  int failed = 0;
#pragma omp parallel for
  for (size_t j = 0; j < align_s_; ++j) {
    if (failed) continue;
    auto const& ujd = commitment_.ud[j];
    auto const& uj = ecc_pub.u1()[j];
    auto const& g2d = commitment_.g2d;
    if (!PairingMatch(ujd, uj, g2d)) {
#pragma omp atomic
      ++failed;
      assert(false);
    }
  }
  return failed == 0;
}

bool Client::CheckUX0() {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  auto const& ux0 = commitment_.ux0;
  auto const& u0x0 = commitment_.u0x[0];
  auto const& g2x0 = commitment_.g2x0;

  int failed = 0;
#pragma omp parallel for
  for (size_t j = 0; j < s_; ++j) {
    if (failed) continue;
    auto const& uj_xj_0 = ux0[j];
    auto const& uj = ecc_pub.u1()[j];
    auto const& u0 = ecc_pub.u1()[0];
    auto const& g2_xj_0 = g2x0[j];
    auto const& u0_xj_0 = u0x0[j];
    if (!PairingMatch(uj_xj_0, uj, g2_xj_0)) {
      assert(false);
#pragma omp atomic
      ++failed;
      continue;
    }
    if (!PairingMatch(u0_xj_0, u0, g2_xj_0)) {
      assert(false);
#pragma omp atomic
      ++failed;
      continue;
    }
  }
  return failed == 0;
}

bool Client::CheckEX() {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  auto const& ex = response_.ex;
  auto const& u0x = commitment_.u0x;

  int failed = 0;
  for (size_t p = 0; p < log_s_; ++p) {
    auto const& exp = ex[p];
    auto const& u0xp = u0x[p];
    auto const& u0xp_1 = u0x[p + 1];
    auto cols = align_s_ / (1ULL << (p + 1));
    assert(exp.size() ==  cols * 2);
    if (failed) break;

#pragma omp parallel for
    for (size_t j = 0; j < cols; ++j) {
      if (failed) continue;
      auto left = ecc_pub.PowerU1(0, exp[j]);
      auto right = u0xp_1[j];
      right += u0xp[2 * j] * e1_;
      right += u0xp[2 * j + 1] * e1_square_;
      if (left != right) {
        assert(false);
#pragma omp atomic
        ++failed;
        continue;
      }
      left = ecc_pub.PowerU1(0, exp[cols + j]);
      right = u0xp_1[j];
      right += u0xp[2 * j] * e2_;
      right += u0xp[2 * j + 1] * e2_square_;
      if (left != right) {
        assert(false);
#pragma omp atomic
        ++failed;
        continue;
      }
    }
  }
  return failed == 0;
}

bool Client::CheckEK() {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  auto const& ek = response_.ek;
  auto const& uk = commitment_.uk;

  int failed = 0;
  for (size_t p = 0; p < log_c_; ++p) {
    auto const& ukp = uk[p];
    auto const& ekp = ek[p];
    auto rows = align_c_ / (1ULL << p);
    auto cols = align_s_;
    assert(ekp.size() == rows * cols);
    if (failed) break;

#pragma omp parallel for
    for (size_t i = 0; i < rows / 2; ++i) {
      if (failed) continue;      
      std::vector<G1> temp(align_s_);
      for (size_t j = 0; j < align_s_; ++j) {
        temp[j] += ecc_pub.PowerU1(j, ekp[2 * i * cols + j]);
      }
      G1 left = std::accumulate(temp.begin(), temp.end(), G1Zero());

      auto const& ukp_1 = uk[p + 1];
      G1 right = ukp_1[i];
      right += ukp[2 * i] * e1_;
      right += ukp[2 * i + 1] * e1_square_;
      if (left != right) {
        assert(false);
        std::cout << "CheckEK " << p << " " << i << " " << __LINE__ << "\n";
#pragma omp atomic
        ++failed;
        continue;
      }

      for (size_t j = 0; j < align_s_; ++j) {
        temp[j] += ecc_pub.PowerU1(j, ekp[(2 * i + 1) * cols + j]);
      }
      left = std::accumulate(temp.begin(), temp.end(), G1Zero());
      right = ukp_1[i];
      right += ukp[2 * i] * e2_;
      right += ukp[2 * i + 1] * e2_square_;
      if (left != right) {
        assert(false);
        std::cout << "CheckEK " << p << " " << i << " " << __LINE__ << "\n";
#pragma omp atomic
        ++failed;
        continue;
      }
    }
  }
  return failed == 0;
}

void Client::DecryptK() {
  k_.resize(log_c_ + 1);
  for (size_t p = 0; p < k_.size(); ++p) {
    auto& kp = k_[p];
    size_t rows = align_c_ / (1ULL << p);
    size_t cols = align_s_;
    kp.resize(rows * cols);
  }

  // x_0 == k_[log_c]
  assert(x_[0].size() == k_[log_c_].size());
  k_[log_c_] = x_[0];

  // NOTE: can not parallel
  auto const& ek = response_.ek;
  for (int64_t p = (int64_t)log_c_ - 1; p >= 0; --p) {
    auto& kp = k_[p];
    auto& kp_1 = k_[p + 1];
    auto const& ekp = ek[p];
    auto kp_1_rows = align_c_ / (1ULL << (p + 1));
    auto cols = align_s_;
    for (size_t i = 0; i < kp_1_rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        kp[2 * i * cols + j] =
            e2_square_ * (ekp[2 * i * cols + j] - kp_1[i * cols + j]);
        kp[2 * i * cols + j] -=
            e1_square_ * (ekp[(2 * i + 1) * cols + j] - kp_1[i * cols + j]);
        kp[2 * i * cols + j] *= e1_e2_inverse_;

        kp[(2 * i + 1) * cols + j] =
            e1_ * (ekp[(2 * i + 1) * cols + j] - kp_1[i * cols + j]);
        kp[(2 * i + 1) * cols + j] -=
            e2_ * (ekp[2 * i * cols + j] - kp_1[i * cols + j]);
        kp[(2 * i + 1)*cols+ j] *= e1_e2_inverse_;
      }
    }
  }

#ifdef _DEBUG
  if (!secret_.k.empty()) {
    assert(k_ == secret_.k);
  }
#endif
}

void Client::DecryptX() {
  x_.resize(log_s_ + 1);
  for (size_t p = 0; p < x_.size(); ++p) {
    auto& xp = x_[p];
    size_t cols = align_s_ / (1ULL << p);
    xp.resize(cols);
  }
  assert(x_[log_s_].size() == 1);
  x_[log_s_][0] = secret_.x0_lgs;

  // NOTE: can not parallel
  auto const& ex = response_.ex;
  for (int64_t p = (int64_t)log_s_ - 1; p >= 0; --p) {
    auto& xp = x_[p];
    auto const& xp_1 = x_[p + 1];
    auto const& exp = ex[p];
    auto exp_cols = align_s_ / (1ULL << (p + 1));
    for (size_t j = 0; j < xp_1.size(); ++j) {
      xp[2 * j] = e2_square_ * (exp[j] - xp_1[j]);
      xp[2 * j] -= e1_square_ * (exp[exp_cols+ j] - xp_1[j]);
      xp[2 * j] *= e1_e2_inverse_;

      xp[2 * j + 1] = e1_ * (exp[exp_cols+ j] - xp_1[j]);
      xp[2 * j + 1] -= e2_ * (exp[j] - xp_1[j]);
      xp[2 * j + 1] *= e1_e2_inverse_;
    }
  }

#ifdef _DEBUG
  if (!secret_.x.empty()) {
    assert(x_ == secret_.x);
  }
#endif
}
}  // namespace scheme::plain::batch3
