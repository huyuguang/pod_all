#pragma once

#include <string>
#include <vector>

#include "basic_types.h"
#include "ecc.h"
#include "vrf.h"

namespace scheme_misc::plain {
struct RangeRequest {
  uint64_t start;
  uint64_t count;
};

struct RangeResponse {
  std::vector<G1> k;
};

struct RangeChallenge {
  mpz_class seed2;
};

struct RangeReply {
  std::vector<Fr> m;
};

struct RangeReceipt {
  mpz_class seed2;
  h256_t k_mkl_root;
};

struct RangeSecret {
  mpz_class seed0;
};

struct RangeClaim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace scheme_misc::plain
