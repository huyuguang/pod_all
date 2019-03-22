#include "scheme_table_otvrfq_session.h"
#include "public.h"
#include "scheme_table_a.h"
#include "vrf.h"

namespace scheme_misc::table::otvrfq {

Session::Session(APtr a, h256_t const& self_id, h256_t const& peer_id)
    : a_(a), self_id_(self_id), peer_id_(peer_id) {
  auto const& ecc_pub = GetEccPub();
  r_ = FrRand();
  g_exp_r_ = ecc_pub.PowerG1(r_);
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
  if (request.vi.empty() || request.mixed_key_digests.empty() ||
      request.key_name.empty()) {
    assert(false);
    return false;
  }

  if (request.mixed_key_digests.size() < request.vi.size()) {
    assert(false);
    return false;
  }

  auto key_meta = a_->GetKeyMetaByName(request.key_name);
  if (!key_meta) return false;

  response.g_exp_r = g_exp_r_;

  Fr c = FrRand();
  response.ui.resize(request.vi.size());

  for (size_t i = 0; i < response.ui.size(); ++i) {
    response.ui[i] = request.vi[i] * c;
  }

  response.psk_exp_r_mixed.resize(request.mixed_key_digests.size());
  for (size_t i = 0; i < request.mixed_key_digests.size(); ++i) {
    auto const& key_digest = request.mixed_key_digests[i];
    Fr key_fr = BinToFr31(key_digest.data(), key_digest.data() + 31);

    auto& psk_exp_r_mixed = response.psk_exp_r_mixed[i];
    vrf::ProveWithR(a_->vrf_sk(), key_digest.data(), r_, psk_exp_r_mixed);

#ifdef _DEBUG
    vrf::Fsk fsk2 = vrf::Vrf(a_->vrf_sk(), key_digest.data());
    vrf::VerifyWithR(a_->vrf_pk(), key_digest.data(), psk_exp_r_mixed,
                     response.g_exp_r);
    vrf::Fsk fsk1;
    vrf::GetFskFromPskExpR(psk_exp_r_mixed.back(), r_, fsk1);
    assert(fsk1 == fsk2);
#endif

    Fp12 e;
    G1 v_exp_key_c = request.v * (key_fr * c);
    mcl::bn256::pairing(e, v_exp_key_c, ot_sk_);
    uint8_t buf[32 * 12];
    auto ret_len = e.serialize(buf, sizeof(buf));
    assert(ret_len == sizeof(buf));
    G1 ge = MapToG1(buf, sizeof(buf));

    for (auto& j : psk_exp_r_mixed) {
      j += ge;
    }
  }

  return true;
}

bool Session::OnReceipt(Receipt const& receipt, Secret& secret) {
  if (receipt.g_exp_r != g_exp_r_) return false;
  secret.r = r_;
  return true;
}
}  // namespace scheme_misc::table::otvrfq
