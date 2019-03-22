#include "scheme_table_otvrfq_client.h"
#include "misc.h"
#include "public.h"
#include "scheme_table.h"
#include "scheme_table_b.h"

namespace scheme_misc::table::otvrfq {

Client::Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
               std::string const& key_name,
               std::vector<std::string> const& key_values,
               std::vector<std::string> const& phantoms)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      key_name_(key_name),
      key_values_(key_values),
      phantoms_count_(phantoms.size()) {
  assert(!phantoms.empty());
  vrf_key_ = GetKeyMetaByName(b_->vrf_meta(), key_name);
  if (!vrf_key_) throw std::runtime_error("invalid key_name");

  CryptoPP::SHA256 hash;
  mixed_key_digests_.resize(key_digests_.size() + phantoms.size());
  for (size_t i = 0; i < key_values.size(); ++i) {
    hash.Update((uint8_t*)key_values[i].data(), key_values[i].size());
    hash.Final(key_digests_[i].data());
    mixed_key_digests_[i] = key_digests_[i];
  }

  // do not need to unique
  for (size_t i = 0; i < phantoms.size(); ++i) {
    hash.Update((uint8_t*)phantoms[i].data(), phantoms[i].size());
    hash.Final(mixed_key_digests_[i + key_digests_.size()].data());
  }

  uint64_t seed;
  misc::RandomBytes((uint8_t*)&seed, sizeof(seed));
  std::shuffle(mixed_key_digests_.begin(), mixed_key_digests_.end(),
               std::default_random_engine(seed));
  mix_index_.resize(key_digests_.size());
  for (size_t i = 0; i < mix_index_.size(); ++i) {
    auto it = std::find(mixed_key_digests_.begin(), mixed_key_digests_.end(),
                        key_digests_[i]);
    assert(it != mixed_key_digests_.end());
    mix_index_[i] = std::distance(mixed_key_digests_.begin(), it);
  }

  ot_self_pk_ = G1Rand();
  ot_beta_ = FrRand();
  ot_rand_a_ = FrRand();
  ot_rand_b_ = FrRand();
}

void Client::GetNegoReqeust(NegoBRequest& request) { request.t = ot_self_pk_; }

bool Client::OnNegoRequest(NegoARequest const& request,
                           NegoAResponse& response) {
  ot_peer_pk_ = request.s;
  response.s_exp_beta = ot_peer_pk_ * ot_beta_;
  return true;
}

bool Client::OnNegoResponse(NegoBResponse const& response) {
  ot_sk_ = response.t_exp_alpha * ot_beta_;
  return true;
}

void Client::GetRequest(Request& request) {
  request.key_name = key_name_;
  request.mixed_key_digests = std::move(mixed_key_digests_);

  request.vi.resize(key_digests_.size());
  for (size_t i = 0; i < request.vi.size(); ++i) {
    h256_t const& key_digest = key_digests_[i];
    Fr key_fr = BinToFr31(key_digest.data(), key_digest.data() + 31);
    request.vi[i] = ot_sk_ * (ot_rand_b_ * key_fr);
  }
  request.v = ot_self_pk_ * (ot_rand_a_ * ot_rand_b_);
}

bool Client::OnResponse(Response const& response, Receipt& receipt) {
  if (response.ui.size() != key_digests_.size()) {
    assert(false);
    return false;
  }
  if (response.psk_exp_r_mixed.size() !=
      (phantoms_count_ + key_digests_.size())) {
    assert(false);
    return false;
  }

  std::vector<vrf::Psk<>> psk_exp_r(key_digests_.size());
  for (size_t i = 0; i < mix_index_.size(); ++i) {
    psk_exp_r[i] = response.psk_exp_r_mixed[mix_index_[i]];
  }

  last_psk_exp_r_.resize(psk_exp_r.size());
  for (size_t i = 0; i < psk_exp_r.size(); ++i) {
    Fp12 e;
    G1 ui_exp_a = response.ui[i] * ot_rand_a_;
    mcl::bn256::pairing(e, ui_exp_a, ot_peer_pk_);
    uint8_t buf[32 * 12];
    auto ret_len = e.serialize(buf, sizeof(buf));
    assert(ret_len == sizeof(buf));
    G1 ge = MapToG1(buf, sizeof(buf));
    for (auto& j : psk_exp_r[i]) {
      j -= ge;
    }

    if (!vrf::VerifyWithR(b_->vrf_pk(), key_digests_[i].data(), psk_exp_r[i],
                          response.g_exp_r)) {
      assert(false);
      return false;
    }
    last_psk_exp_r_[i] = psk_exp_r[i].back();
  }

  g_exp_r_ = response.g_exp_r;
  receipt.g_exp_r = response.g_exp_r;
  return true;
}

bool Client::OnSecret(Secret const& query_secret,
                      std::vector<std::vector<uint64_t>>& positions) {
  auto const& ecc_pub = GetEccPub();
  if (ecc_pub.PowerG1(query_secret.r) != g_exp_r_) {
    assert(false);
    return false;
  }

  positions.resize(last_psk_exp_r_.size());
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

    auto& postion = positions[i];
    for (uint64_t i = 0; i < km.size(); ++i) {
      if (fr_fsk == km[i]) {
        postion.push_back(i);
        if (vrf_key_->unique) break;
      }
    }
  }

  return true;
}
}  // namespace scheme_misc::table::otvrfq