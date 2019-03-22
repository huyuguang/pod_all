#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_table_b.h"
#include "scheme_table_protocol.h"
#include "vrf.h"

namespace scheme_misc::table {

struct VrfKeyMeta;

namespace vrfq {
class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
         std::string const& key_name, std::string const& key_value);

 public:
  void GetRequest(Request& request);
  bool OnResponse(Response const& request, Receipt& receipt);
  bool OnSecret(Secret const& secret, std::vector<uint64_t>& positions);

 private:
  BPtr b_;
  h256_t const self_id_;
  h256_t const peer_id_;
  std::string const key_name_;
  std::string const key_value_;

 private:
  VrfKeyMeta const* vrf_key_;
  h256_t key_digest_;
  G1 last_psk_exp_r_;
  G1 g_exp_r_;
  vrf::Fsk fsk_;
};
}  // namespace vrfq

}  // namespace scheme_misc::table