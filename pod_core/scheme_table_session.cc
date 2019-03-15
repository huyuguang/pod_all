#include "scheme_table_session.h"
#include "public.h"
#include "scheme_table_a.h"
#include "scheme_table_protocol.h"
#include "vrf.h"

namespace scheme_misc::table {

Session::Session(APtr a) : a_(a) {
  auto const& ecc_pub = GetEccPub();
  r_ = FrRand();
  g_exp_r_ = ecc_pub.PowerG1(r_);
}

bool Session::OnQueryReq(QueryReq const& req, QueryRsp& rsp) {
  auto key_meta = a_->GetKeyMetaByName(req.key_name);
  if (!key_meta) return false;

  rsp.g_exp_r = g_exp_r_;

  h256_t digest;
  CryptoPP::SHA256 hash;
  hash.Update((uint8_t*)req.key_value.data(), req.key_value.size());
  hash.Final(digest.data());

  vrf::ProveWithR(a_->vrf_sk(), digest.data(), r_, rsp.psk_exp_r);

#ifdef _DEBUG  
  vrf::Fsk fsk2 = vrf::Vrf(a_->vrf_sk(), digest.data());
  vrf::VerifyWithR(a_->vrf_pk(), digest.data(), rsp.psk_exp_r, rsp.g_exp_r);
  vrf::Fsk fsk1;
  vrf::GetFskFromPskExpR(rsp.psk_exp_r.back(), r_, fsk1);
  assert(fsk1 == fsk2);
#endif
  return true;
}

bool Session::OnQueryReceipt(QueryReceipt const& receipt, Fr& r) {
  if (receipt.g_exp_r != g_exp_r_) return false;
  r = r_;
  return true;
}
}  // namespace scheme_misc::table
