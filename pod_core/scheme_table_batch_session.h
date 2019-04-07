#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"
#include "scheme_table_a.h"
#include "scheme_table_protocol.h"

namespace scheme::table::batch {

class Session {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Session(APtr a, h256_t const& self_id, h256_t const& peer_id);

 public:
  bool OnRequest(Request request, Response& response);
  bool OnReceipt(Receipt const& receipt, Secret& secret);

 public:
  void TestSetEvil() { evil_ = true; }

 private:
  void BuildMapping();

 private:
  APtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;

 private:
  std::vector<Range> demands_;
  h256_t seed2_seed_;
  h256_t seed2_;

 private:
  struct Mapping {
    uint64_t index_of_m;
  };
  uint64_t demands_count_ = 0;
  std::vector<Mapping> mappings_;

 private:
  h256_t seed0_;
  std::vector<Fr> v_;  // size() is count * s_
  std::vector<Fr> w_;  // size() is count
  h256_t k_mkl_root_;

 private:
  bool evil_ = false;
};

typedef std::shared_ptr<Session> SessionPtr;
}  // namespace scheme::table::batch