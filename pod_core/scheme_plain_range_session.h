#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"
#include "scheme_plain_a.h"
#include "scheme_plain_protocol.h"

namespace scheme_misc::plain {

namespace range {
class Session {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Session(APtr a, h256_t const& self_id, h256_t const& peer_id);
  bool OnRequest(Request const& request, Response& response);
  bool OnChallenge(Challenge const& challenge, Reply& reply);
  bool OnReceipt(Receipt const& receipt, Secret& secret);

 public:
  void TestSetEvil() { evil_ = true; }

 private:
 private:
  APtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;

 private:
  Request request_;
  Challenge challenge_;

 private:
  mpz_class seed0_;
  std::vector<Fr> v_;  // size() is count * s_
  std::vector<Fr> w_;  // size() is count
  h256_t k_mkl_root_;

 private:
  bool evil_ = false;
};

}  // namespace range

}  // namespace scheme_misc::plain