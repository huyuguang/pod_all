#include "scheme_table_client.h"
#include "public.h"
#include "scheme_table_b.h"
#include "scheme_table_protocol.h"
#include "vrf.h"
#include "scheme_table.h"

namespace scheme_misc::table {

Client::Client(BPtr b, std::string const& key_name,
               std::string const& key_value)
    : b_(b), key_name_(key_name), key_value_(key_value) {
  vrf_key_ = GetKeyMetaByName(b_->vrf_meta(), key_name);
  if (!vrf_key_) throw std::runtime_error("invalid key_name");

  CryptoPP::SHA256 hash;
  hash.Update((uint8_t*)key_value.data(), key_value.size());
  hash.Final(key_digest_.data());
}

bool Client::OnQueryRsp(QueryRsp const& rsp, QueryReceipt& receipt) {
  if (!vrf::VerifyWithR(b_->vrf_pk(), key_digest_.data(), rsp.psk_exp_r,
                        rsp.g_exp_r))
    return false;

  g_exp_r_ = rsp.g_exp_r;
  last_psk_exp_r_ = rsp.psk_exp_r.back();
  receipt.g_exp_r = rsp.g_exp_r;
  return true;
}

bool Client::OnR(Fr const& r, int64_t& position) {
  auto const& ecc_pub = GetEccPub();
  if (ecc_pub.PowerG1(r) != g_exp_r_) {
    assert(false);
    return false;
  }

  vrf::GetFskFromPskExpR(last_psk_exp_r_, r, fsk_);
  
  uint8_t fsk_bin[12 * 32];
  fsk_.serialize(fsk_bin, sizeof(fsk_bin), mcl::IoMode::IoSerialize);

  CryptoPP::SHA256 hash;
  h256_t digest;
  hash.Update(fsk_bin, sizeof(fsk_bin));
  hash.Final(digest.data());
  Fr fr_fsk = BinToFr31(digest.data(), digest.data() + 31);

  auto const& key_m = b_->key_m();
  auto const& km = key_m[vrf_key_->j];

  auto it = std::find(km.begin(), km.end(), fr_fsk);
  if (it == km.end()) {
    position = -1;
  } else {
    position = std::distance(km.begin(), it);
  }

  return true;
}
}  // namespace scheme_misc::table