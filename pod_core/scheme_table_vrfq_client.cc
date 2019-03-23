#include "scheme_table_vrfq_client.h"
#include "public.h"
#include "scheme_table.h"
#include "scheme_table_b.h"

namespace scheme_misc::table::vrfq {

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               std::string const& key_name,
               std::vector<std::string> const& key_values)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      key_name_(key_name),
      key_values_(key_values) {
  vrf_key_ = GetKeyMetaByName(b_->vrf_meta(), key_name);
  if (!vrf_key_) throw std::runtime_error("invalid key_name");

  CryptoPP::SHA256 hash;
  key_digests_.resize(key_values_.size());
  for (size_t i = 0; i < key_values_.size(); ++i) {
    auto const& value = key_values_[i];
    hash.Update((uint8_t*)value.data(), value.size());
    hash.Final(key_digests_[i].data());
  }
  
  last_psk_exp_r_.resize(key_values.size());
  fsk_.resize(key_values.size());
}

void Client::GetRequest(Request& request) {
  request.key_name = key_name_;
  request.key_digests = key_digests_;
}

bool Client::OnResponse(Response const& response, Receipt& receipt) {
  if (response.psk_exp_r.size() != key_values_.size()) {
    assert(false);
    return false;
  }

  g_exp_r_ = response.g_exp_r;
  for (size_t i = 0; i < response.psk_exp_r.size(); ++i) {
    auto const& key_digest = key_digests_[i];
    auto const& psk_exp_r = response.psk_exp_r[i];
    if (!vrf::VerifyWithR(b_->vrf_pk(), key_digest.data(), psk_exp_r,
                          g_exp_r_)) {
      assert(false);
      return false;
    }

    last_psk_exp_r_[i] = psk_exp_r.back();
    receipt.g_exp_r = g_exp_r_;
  }

  return true;
}

bool Client::OnSecret(Secret const& query_secret,
                      std::vector<std::vector<uint64_t>>& positions) {
  auto const& ecc_pub = GetEccPub();
  if (ecc_pub.PowerG1(query_secret.r) != g_exp_r_) {
    assert(false);
    return false;
  }

  positions.resize(key_values_.size());
  for (size_t i = 0; i < last_psk_exp_r_.size(); ++i) {
    vrf::GetFskFromPskExpR(last_psk_exp_r_[i], query_secret.r, fsk_[i]);

    uint8_t fsk_bin[12 * 32];
    fsk_[i].serialize(fsk_bin, sizeof(fsk_bin), mcl::IoMode::IoSerialize);

    CryptoPP::SHA256 hash;
    h256_t digest;
    hash.Update(fsk_bin, sizeof(fsk_bin));
    hash.Final(digest.data());
    Fr fr_fsk = BinToFr31(digest.data(), digest.data() + 31);

    auto const& key_m = b_->key_m();
    auto const& km = key_m[vrf_key_->j];

    auto& position = positions[i];
    for (uint64_t i = 0; i < km.size(); ++i) {
      if (fr_fsk == km[i]) {
        position.push_back(i);
        if (vrf_key_->unique) break;
      }
    }
  }

  return true;
}
}  // namespace scheme_misc::table::vrfq