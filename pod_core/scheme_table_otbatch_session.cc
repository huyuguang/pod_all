#include "scheme_table_otbatch_session.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_table.h"
#include "scheme_table_a.h"
#include "scheme_table_protocol.h"
#include "tick.h"

namespace {
bool CheckPhantoms(uint64_t n, std::vector<Range> const& phantoms) {
  for (auto const& phantom : phantoms) {
    if (!phantom.count || phantom.start >= n || phantom.count > n ||
        (phantom.start + phantom.count) > n)
      return false;
  }

  for (size_t i = 1; i < phantoms.size(); ++i) {
    if (phantoms[i].start <= phantoms[i - 1].start + phantoms[i - 1].count)
      return false;
  }
  return true;
}
}

namespace scheme::table::otbatch {

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

void Session::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(phantoms_count_);
  size_t index = 0;
  for (auto const& p : request_.phantoms) {
    for (size_t i = p.start; i < (p.start + p.count); ++i) {
      mappings_[index++].index_of_m = i;
    }
  }
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
  
  if (!CheckPhantoms(n_, request.phantoms)) {
    assert(false);
    return false;
  }

  for (auto const& i : request.phantoms) phantoms_count_ += i.count;

  if (request.ot_vi.size() >= phantoms_count_) {
    assert(false);
    return false;
  }

  request_ = request;

  BuildMapping();

  H2(seed0_, phantoms_count_ * s_, v_);

  if (evil_) {
    uint64_t evil_i = rand() % phantoms_count_;
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
  H2(challenge_.seed2, phantoms_count_, w_);

  // compute mij' = vij + wi * mij
  auto const& m = a_->m();
  reply.m.resize(phantoms_count_ * s_);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    auto const& map = mappings_[i];
    auto fr_i = MapToFr(map.index_of_m);
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
    auto m_is = map.index_of_m * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      auto m_ij = m_is + j;
      reply.m[ij] = v_[ij] + w_[i] * m[m_ij];
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
  if (receipt.count != phantoms_count_) {
    assert(false);
    return false;
  }
  secret.seed0 = seed0_;
  return true;
}

}  // namespace scheme::table::otbatch
