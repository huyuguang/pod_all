#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"
#include "scheme_plain_protocol.h"

namespace scheme_misc::plain {
class A;
typedef std::shared_ptr<A> APtr;

class Session {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Session(APtr a, h256_t const& self_id, h256_t const& peer_id);
  bool OnRangeRequest(RangeRequest const& request, RangeResponse& response);
  bool OnRangeChallenge(RangeChallenge const& challenge, RangeReply& reply);
  bool OnRangeReceipt(RangeReceipt const& receipt, RangeSecret& secret);

 private:

 private:
  APtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;

 private:
  RangeRequest request_;
  RangeChallenge challenge_;

 private:
  mpz_class seed0_;
  std::vector<Fr> v_;  // size() is count * s_
  std::vector<Fr> w_;  // size() is count
  h256_t k_mkl_root_;
};

typedef std::unique_ptr<Session> SessionUPtr;

}  // namespace scheme_misc::plain