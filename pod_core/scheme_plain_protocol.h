#pragma once

#include <string>
#include <vector>

#include "basic_types.h"
#include "ecc.h"
#include "vrf.h"

namespace scheme_misc::plain {
namespace range {
struct Request {
  uint64_t start;
  uint64_t count;
};

struct Response {
  std::vector<G1> k;
};

struct Challenge {
  mpz_class seed2;
};

struct Reply {
  std::vector<Fr> m;
};

struct Receipt {
  mpz_class seed2;
  h256_t k_mkl_root;
};

struct Secret {
  mpz_class seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace range
}  // namespace scheme_misc::plain
