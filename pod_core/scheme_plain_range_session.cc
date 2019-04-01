#include "scheme_plain_range_session.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_protocol.h"
#include "tick.h"

namespace scheme::plain::range {

Session::Session(APtr a, h256_t const& self_id, h256_t const& peer_id)
    : a_(a),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(a_->bulletin().n),
      s_(a_->bulletin().s) {
  seed0_ = misc::RandMpz32();
}

bool Session::OnRequest(Request const& request, Response& response) {
  Tick _tick_(__FUNCTION__);

  seed2_seed_ = request.seed2_seed_;
  demand_ = request.demand;

  if (!demand_.count || demand_.start >= n_ || demand_.count > n_ ||
      (demand_.start + demand_.count) > n_)
    return false;  

  H2(seed0_, demand_.count * s_, v_);

  if (evil_) {
    uint64_t evil_i = rand() % demand_.count;
    uint64_t evil_j = s_ - 1;  // last col
    v_[evil_i * s_ + evil_j] = FrRand();
    std::cout << "evil: " << evil_i << "," << evil_j << "\n";
  }

  BuildK(v_, response.k, s_);

  k_mkl_root_ = CalcRootOfK(response.k);

  seed2_ = CalcSeed2(seed2_seed_, k_mkl_root_);

  H2(seed2_, demand_.count, w_);

  // compute mij' = vij + wi * mij
  auto const& m = a_->m();
  response.m.resize(demand_.count * s_);
  auto offset = demand_.start * s_;

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)demand_.count; ++i) {
    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      response.m[ij] = v_[ij] + w_[i] * m[offset + ij];
    }
  }

  return true;
}

bool Session::OnReceipt(Receipt const& receipt, Secret& secret) {
  if (receipt.seed2 != seed2_) {
    assert(false);
    return false;
  }
  if (receipt.k_mkl_root != k_mkl_root_) {
    assert(false);
    return false;
  }
  if (receipt.count != demand_.count) {
    assert(false);
    return false;
  }
  secret.seed0 = seed0_;
  return true;
}

}  // namespace scheme::plain::range
