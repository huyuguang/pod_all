#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"
#include "scheme_plain_a.h"
#include "scheme_plain_range_protocol.h"

namespace scheme::plain {

namespace range {
class Session {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Session(APtr a, h256_t const& self_id, h256_t const& peer_id);
  bool OnRequest(Request const& request, Response& response);
  bool OnReceipt(Receipt const& receipt, Secret& secret);

 public:
  void TestSetEvil() { evil_ = true; }

  private:
  APtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;

 private:
  Range demand_;
  h256_t seed2_seed_;
  h256_t seed2_;

 private:
  h256_t seed0_;
  std::vector<Fr> v_;  // size() is count * s_
  std::vector<Fr> w_;  // size() is count
  h256_t k_mkl_root_;

 private:
  bool evil_ = false;
};

typedef std::shared_ptr<Session> SessionPtr;
}  // namespace range

}  // namespace scheme::plain