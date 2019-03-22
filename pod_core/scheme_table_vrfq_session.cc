#include "scheme_table_vrfq_session.h"
#include "public.h"
#include "scheme_table_a.h"
#include "vrf.h"

namespace scheme_misc::table::vrfq {

Session::Session(APtr a, h256_t const& self_id, h256_t const& peer_id)
    : a_(a), self_id_(self_id), peer_id_(peer_id) {
  auto const& ecc_pub = GetEccPub();
  r_ = FrRand();
  g_exp_r_ = ecc_pub.PowerG1(r_);
}

bool Session::OnRequest(Request const& request, Response& response) {
  auto key_meta = a_->GetKeyMetaByName(request.key_name);
  if (!key_meta) return false;

  response.g_exp_r = g_exp_r_;

  h256_t digest;
  CryptoPP::SHA256 hash;
  hash.Update((uint8_t*)request.key_value.data(), request.key_value.size());
  hash.Final(digest.data());

  vrf::ProveWithR(a_->vrf_sk(), digest.data(), r_, response.psk_exp_r);

#ifdef _DEBUG
  vrf::Fsk fsk2 = vrf::Vrf(a_->vrf_sk(), digest.data());
  vrf::VerifyWithR(a_->vrf_pk(), digest.data(), response.psk_exp_r,
                   response.g_exp_r);
  vrf::Fsk fsk1;
  vrf::GetFskFromPskExpR(response.psk_exp_r.back(), r_, fsk1);
  assert(fsk1 == fsk2);
#endif
  return true;
}

bool Session::OnReceipt(Receipt const& receipt, Secret& secret) {
  if (receipt.g_exp_r != g_exp_r_) return false;
  secret.r = r_;
  return true;
}
}  // namespace scheme_misc::table::vrfq
