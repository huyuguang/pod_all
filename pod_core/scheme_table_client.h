#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "vrf.h"
namespace scheme_misc::table {
class B;
typedef std::shared_ptr<B> BPtr;

struct QueryReq;
struct QueryRsp;
struct QueryReceipt;
struct QuerySecret;
struct VrfKeyMeta;

class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
         std::string const& key_name, std::string const& key_value);
  bool OnQueryRsp(QueryRsp const& rsp, QueryReceipt& receipt);
  bool OnSecretReveal(QuerySecret const& secret,
                      std::vector<uint64_t>& positions);

 private:
  BPtr b_;
  h256_t self_id_;
  h256_t peer_id_;
  std::string key_name_;
  std::string key_value_;
  VrfKeyMeta const* vrf_key_;
  h256_t key_digest_;
  G1 last_psk_exp_r_;
  G1 g_exp_r_;
  vrf::Fsk fsk_;

 private:
};
}  // namespace scheme_misc::table