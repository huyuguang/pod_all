#pragma once

#include <memory>
#include <string>
#include "ecc.h"
#include "vrf.h"

namespace scheme_misc::table {
class B;
typedef std::shared_ptr<B> BPtr;

struct QueryReq;
struct QueryRsp;
struct QueryReceipt;
struct VrfKeyMeta;

class Client {
 public:
  Client(BPtr b, std::string const& key_name, std::string const& key_value);
  bool OnQueryRsp(QueryRsp const& rsp, QueryReceipt& receipt);
  bool OnR(Fr const& r, int64_t& position);

 private:
  BPtr b_;
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