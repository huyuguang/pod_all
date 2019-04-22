#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "matrix_fr.h"

namespace scheme::plain::batch3 {
struct Request {
  std::vector<Range> demands;
};

struct Commitment {
  std::vector<std::vector<G1>> uk;
  std::vector<G1> ux0;  // s
  std::vector<std::vector<G1>> u0x;
  std::vector<G2> g2x0;  // s
  std::vector<G1> ud;
  G2 g2d;
};

struct Challenge {
  h256_t r;
};

struct Response {
  std::vector<Fr> m;  // n*s
  std::vector<std::vector<Fr>> ek;
  std::vector<std::vector<Fr>> ex;
};

struct Receipt {
  G1 u0_x0_lgs;
  G1 u0d;
};

struct Secret {
  Fr x0_lgs;
  Fr d;
#ifdef _DEBUG
  std::vector<std::vector<Fr>> k;
  std::vector<std::vector<Fr>> x;
  std::vector<Fr> m;
#endif
};
}  // namespace scheme::plain::batch3
