#include "scheme_plain_batch3_session.h"
#include "chain.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_batch3_misc.h"
#include "scheme_plain_batch3_protocol.h"
#include "tick.h"

namespace scheme::plain::batch3 {

Session::Session(APtr a, h256_t const& self_id, h256_t const& peer_id)
    : a_(a),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(a_->bulletin().n),
      s_(a_->bulletin().s),
      d_(FrRand()) {
  auto const& ecc_pub = GetEccPub();
  align_s_ = misc::Pow2UB(s_);
  log_s_ = misc::Log2UB(align_s_);
  if (align_s_ > ecc_pub.u1().size()) {
    assert(false);
    throw std::runtime_error("align_s too large");
  }
}

void Session::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(demands_count_);
  size_t index = 0;
  for (auto const& p : request_.demands) {
    for (size_t i = p.start; i < (p.start + p.count); ++i) {
      mappings_[index++].index_of_m = i;
    }
  }
}

bool Session::OnRequest(Request request, Response& response) {
  Tick _tick_(__FUNCTION__);

  if (!CheckDemands(n_, request.demands)) {
    assert(false);
    return false;
  }

  request_ = std::move(request);
  for (auto const& i : request_.demands) {
    demands_count_ += i.count;
  }

  BuildMapping();

  align_c_ = misc::Pow2UB(demands_count_);
  log_c_ = misc::Log2UB(align_c_);

  BuildK();
  BuildX();

  BuildUK(response.uk);
  BuildUX0(response.ux0);
  BuildU0X(response.u0x);
  BuildG2X0(response.g2x0);
  BuildCommitmentD(response.ud, response.g2d);

  auto challenge_seed =
      RomChallengeSeed(peer_id_, self_id_, a_->bulletin(), request_, response);
  ComputeChallenge(challenge_seed, challenge_);
  std::cout << "Session side: challenge_seed: "
            << misc::HexToStr(challenge_seed) << "\n";

  BuildM(response.m);
  BuildEK(response.ek);
  BuildEX(response.ex);

  return true;
}

bool Session::OnReceipt(Receipt const& receipt, Secret& secret) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  auto x0_p = GetX(0, log_s_);
  if (receipt.u0_x0_lgs != ecc_pub.PowerU1(0, x0_p)) {
    assert(false);
    return false;
  }
  if (receipt.u0d != u0d_) {
    assert(false);
    return false;
  }
  secret.x0_lgs = x0_p;
  secret.d = d_;
#ifdef _DEBUG
  secret.k = k_;
  secret.x = x_;
  secret.m.resize(demands_count_ * s_);
  for (size_t i = 0; i < mappings_.size(); ++i) {
    auto const& map = mappings_[i];
    for (uint64_t j = 0; j < s_; ++j) {
      auto m_ij = map.index_of_m * s_ + j;
      secret.m[i * s_ + j] = a_->m()[m_ij];
    }
  }
#endif
  return true;
}

void Session::BuildK() {
  Tick _tick_(__FUNCTION__);
  k_.resize(log_c_ + 1);

  for (size_t p = 0; p < k_.size(); ++p) {
    auto& kp = k_[p];
    size_t rows = align_c_ / (1ULL << p);
    size_t cols = align_s_;
    kp.resize(rows * cols);
    for (auto& i : kp) {
      i = FrRand();
    }
  }
}

void Session::BuildX() {
  Tick _tick_(__FUNCTION__);
  assert(k_.size() == (log_c_ + 1));
  x_.resize(log_s_ + 1);

  // x_0 == k_[log_c]
  assert(k_[log_c_].size() == align_s_);
  x_[0] = k_[log_c_];

  for (size_t p = 1; p < x_.size(); ++p) {
    auto& xp = x_[p];
    size_t cols = align_s_ / (1ULL << p);
    xp.resize(cols);
    for (auto& i : xp) {
      i = FrRand();
    }
  }
}

Fr const& Session::GetK(uint64_t i, uint64_t j, uint64_t p) {
  auto const& kp = k_[p];
  return kp[i * align_s_ + j];
}

Fr const& Session::GetX(uint64_t j, uint64_t p) {
  auto const& xp = x_[p];
  return xp[j];
}

void Session::BuildUK(std::vector<std::vector<G1>>& uk) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  uk.resize(log_c_ + 1);

  for (uint64_t p = 0; p < uk.size(); ++p) {
    auto& ukp = uk[p];
    size_t rows = align_c_ / (1ULL << p);
    ukp.resize(rows);
  }

  std::vector<G1> temp(align_s_);
  for (uint64_t p = 0; p < uk.size(); ++p) {
    auto& ukp = uk[p];
    for (uint64_t i = 0; i < ukp.size(); ++i) {
#pragma omp parallel for
      for (uint64_t j = 0; j < align_s_; ++j) {
        temp[j] = ecc_pub.PowerU1(j, GetK(i, j, p));
      }
      ukp[i] = std::accumulate(temp.begin(), temp.end(), G1Zero());
    }
  }
}

void Session::BuildUX0(std::vector<G1>& ux0) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  ux0.resize(align_s_);

#pragma omp parallel for
  for (uint64_t j = 0; j < ux0.size(); ++j) {
    ux0[j] = ecc_pub.PowerU1(j, GetK(0, j, k_.size() - 1));
  }
}

void Session::BuildU0X(std::vector<std::vector<G1>>& u0x) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  u0x.resize(log_s_ + 1);

  for (uint64_t p = 0; p < u0x.size(); ++p) {
    auto& u0xp = u0x[p];
    u0xp.resize(align_s_ / (1ULL << p));
#pragma omp parallel for
    for (uint64_t j = 0; j < u0xp.size(); ++j) {
      u0xp[j] = ecc_pub.PowerU1(0, GetX(j, p));
    }
  }
}

void Session::BuildG2X0(std::vector<G2>& g2x0) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  g2x0.resize(align_s_);
#pragma omp parallel for
  for (uint64_t j = 0; j < g2x0.size(); ++j) {
    auto const& x = GetX(j, 0);
    g2x0[j] = ecc_pub.PowerG2(x);
  }
}

void Session::BuildM(std::vector<Fr>& encrypted_m) {
  Tick _tick_(__FUNCTION__);
  // mij' = kij0 + d + c^i * mij
  auto const& m = a_->m();
  encrypted_m.resize(demands_count_ * align_s_);
  Fr zero = FrZero();

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    auto const& map = mappings_[i];
    for (uint64_t j = 0; j < align_s_; ++j) {
      Fr const& mij = j < s_ ? m[map.index_of_m * s_ + j] : zero;
      encrypted_m[i * align_s_ + j] = GetK(i, j, 0) + d_ + challenge_.c * mij;
    }
  }
}

void Session::BuildEK(std::vector<std::vector<Fr>>& ek) {
  Tick _tick_(__FUNCTION__);
  auto f1 = [this](uint64_t i, uint64_t j, uint64_t p) {
    auto const& a = GetK(i, j, p + 1);
    auto const& b = GetK(2 * i, j, p);
    auto const& c = GetK(2 * i + 1, j, p);
    return a + challenge_.e1 * b + challenge_.e1_square * c;
  };
  auto f2 = [this](uint64_t i, uint64_t j, uint64_t p) {
    auto const& a = GetK(i, j, p + 1);
    auto const& b = GetK(2 * i, j, p);
    auto const& c = GetK(2 * i + 1, j, p);
    return a + challenge_.e2 * b + challenge_.e2_square * c;
  };

  ek.resize(log_c_);
  for (size_t p = 0; p < log_c_; ++p) {
    auto& kep = ek[p];
    size_t rows = align_c_ / (1ULL << p);
    size_t cols = align_s_;
    kep.resize(rows * cols);
    for (size_t i = 0; i < rows / 2; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        size_t row = 2 * i;
        size_t col = j;
        kep[row * cols + col] = f1(i, j, p);
        ++row;
        kep[row * cols + col] = f2(i, j, p);
      }
    }
  }
}

void Session::BuildEX(std::vector<std::vector<Fr>>& ex) {
  Tick _tick_(__FUNCTION__);
  auto f1 = [this](uint64_t j, uint64_t p) {
    auto const& a = GetX(j, p + 1);
    auto const& b = GetX(2 * j, p);
    auto const& c = GetX(2 * j + 1, p);
    return a + challenge_.e1 * b + challenge_.e1_square * c;
  };
  auto f2 = [this](uint64_t j, uint64_t p) {
    auto const& a = GetX(j, p + 1);
    auto const& b = GetX(2 * j, p);
    auto const& c = GetX(2 * j + 1, p);
    return a + challenge_.e2 * b + challenge_.e2_square * c;
  };

  ex.resize(log_s_);
  for (size_t p = 0; p < log_s_; ++p) {
    auto& exp = ex[p];
    size_t cols = align_s_ / (1ULL << (p + 1));
    exp.resize(cols * 2);  // rows = 2
    for (size_t j = 0; j < cols; ++j) {
      exp[j] = f1(j, p);
      exp[cols + j] = f2(j, p);
    }
  }
}

void Session::BuildCommitmentD(std::vector<G1>& ud, G2& g2d) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  g2d = ecc_pub.PowerG2(d_);
  ud.resize(align_s_);

#pragma omp parallel for
  for (size_t j = 0; j < align_s_; ++j) {
    ud[j] = ecc_pub.PowerU1(j, d_);
  }
  u0d_ = ud[0];
}

}  // namespace scheme::plain::batch3
