#include "scheme_table_client.h"
#include "public.h"
#include "scheme_table.h"
#include "scheme_table_b.h"

namespace scheme_misc::table {

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               std::string const& key_name, std::string const& key_value)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      key_name_(key_name),
      key_value_(key_value) {
  vrf_key_ = GetKeyMetaByName(b_->vrf_meta(), key_name);
  if (!vrf_key_) throw std::runtime_error("invalid key_name");

  CryptoPP::SHA256 hash;
  hash.Update((uint8_t*)key_value.data(), key_value.size());
  hash.Final(key_digest_.data());
}

bool Client::OnQueryResponse(VrfQueryResponse const& response,
                             VrfQueryReceipt& receipt) {
  if (!vrf::VerifyWithR(b_->vrf_pk(), key_digest_.data(), response.psk_exp_r,
                        response.g_exp_r))
    return false;

  g_exp_r_ = response.g_exp_r;
  last_psk_exp_r_ = response.psk_exp_r.back();
  receipt.g_exp_r = response.g_exp_r;
  return true;
}

bool Client::OnQuerySecret(VrfQuerySecret const& query_secret,
                            std::vector<uint64_t>& positions) {
  auto const& ecc_pub = GetEccPub();
  if (ecc_pub.PowerG1(query_secret.r) != g_exp_r_) {
    assert(false);
    return false;
  }

  vrf::GetFskFromPskExpR(last_psk_exp_r_, query_secret.r, fsk_);

  uint8_t fsk_bin[12 * 32];
  fsk_.serialize(fsk_bin, sizeof(fsk_bin), mcl::IoMode::IoSerialize);

  CryptoPP::SHA256 hash;
  h256_t digest;
  hash.Update(fsk_bin, sizeof(fsk_bin));
  hash.Final(digest.data());
  Fr fr_fsk = BinToFr31(digest.data(), digest.data() + 31);

  auto const& key_m = b_->key_m();
  auto const& km = key_m[vrf_key_->j];

  for (uint64_t i = 0; i < km.size(); ++i) {
    if (fr_fsk == km[i]) {
      positions.push_back(i);
      if (vrf_key_->unique) break;
    }
  }

  return true;
}
}  // namespace scheme_misc::table