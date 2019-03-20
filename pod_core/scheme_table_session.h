#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"
#include "scheme_table_protocol.h"

namespace scheme_misc::table {
class A;
typedef std::shared_ptr<A> APtr;

class Session {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Session(APtr a, h256_t const& self_id, h256_t const& peer_id);
  bool OnQueryRequest(VrfQueryRequest const& req, VrfQueryResponse& rsp);
  bool OnQueryReceipt(VrfQueryReceipt const& receipt, VrfQuerySecret& secret);

 private:
  APtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;

 private:
  Fr r_;
  G1 g_exp_r_;
};

typedef std::unique_ptr<Session> SessionUPtr;

}  // namespace scheme_misc::table