#include "scheme_plain_otrange_session.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_protocol.h"
#include "tick.h"

namespace scheme::plain::otrange {

Session::Session(APtr a, h256_t const& self_id, h256_t const& peer_id)
    : a_(a),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(a_->bulletin().n),
      s_(a_->bulletin().s) {
  seed0_ = misc::RandMpz32();
  ot_self_pk_ = G2Rand();
  ot_alpha_ = FrRand();
}

void Session::GetNegoReqeust(NegoARequest& request) { request.s = ot_self_pk_; }

bool Session::OnNegoRequest(NegoBRequest const& request,
                            NegoBResponse& response) {
  ot_peer_pk_ = request.t;
  response.t_exp_alpha = ot_peer_pk_ * ot_alpha_;
  return true;
}

bool Session::OnNegoResponse(NegoAResponse const& response) {
  ot_sk_ = response.s_exp_beta * ot_alpha_;
  return true;
}

bool Session::OnRequest(Request const& request, Response& response) {
  Tick _tick_(__FUNCTION__);

  if (!request.count || request.start >= n_ || request.count > n_ ||
      (request.start + request.count) > n_) {
    assert(false);
    return false;
  }
    
  if (request.ot_vi.size() >= request.count) {
    assert(false);
    return false;
  }

  request_ = request;

  H2(seed0_, request_.count * s_, v_);

  if (evil_) {
    uint64_t evil_i = rand() % request_.count;
    uint64_t evil_j = s_ - 1;  // last col
    v_[evil_i * s_ + evil_j] = FrRand();
    std::cout << "evil: " << evil_i << "," << evil_j << "\n";
  }

  BuildK(v_, response.k, s_);

  k_mkl_root_ = CalcRootOfK(response.k);

  ot_rand_c_ = FrRand();
  response.ot_ui.resize(request_.ot_vi.size());

#pragma omp parallel for
  for (int64_t j = 0; j < (int64_t)response.ot_ui.size(); ++j) {
    response.ot_ui[j] = request_.ot_vi[j] * ot_rand_c_;
  }
  return true;
}

bool Session::OnChallenge(Challenge const& challenge, Reply& reply) {
  Tick _tick_(__FUNCTION__);

  challenge_ = challenge;
  H2(challenge_.seed2, request_.count, w_);

  // compute mij' = vij + wi * mij
  auto const& m = a_->m();
  reply.m.resize(request_.count * s_);
  auto offset = request_.start * s_;

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)request_.count; ++i) {
    auto fr_i = MapToFr(i + request_.start);
    Fp12 e;    
    G1 v_exp_fr_c = request_.ot_v * (fr_i * ot_rand_c_);
    mcl::bn256::pairing(e, v_exp_fr_c, ot_sk_);
    uint8_t buf[32 * 12];
    auto ret_len = e.serialize(buf, sizeof(buf));
    if (ret_len != sizeof(buf)) {
      assert(false);
      throw std::runtime_error("oops");
    }
    Fr fr_e = MapToFr(buf, sizeof(buf));

    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      reply.m[ij] = v_[ij] + w_[i] * m[offset + ij];
      reply.m[ij] += fr_e;
    }
  }

  return true;
}

bool Session::OnReceipt(Receipt const& receipt, Secret& secret) {
  if (receipt.seed2 != challenge_.seed2) {
    assert(false);
    return false;
  }
  if (receipt.k_mkl_root != k_mkl_root_) {
    assert(false);
    return false;
  }
  if (receipt.count != request_.count) {
    assert(false);
    return false;
  }
  secret.seed0 = seed0_;
  return true;
}

}  // namespace scheme::plain::range
