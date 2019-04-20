#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "matrix_fr.h"

namespace scheme::table::batch3 {
struct Request {
  std::vector<Range> demands;
};

struct Commitment {
  std::vector<Eigen::ColVectorXG1> uk;
  Eigen::RowVectorXG1 ux0;  // s
  std::vector<Eigen::RowVectorXG1> u0x;
  Eigen::RowVectorXG2 g2x0;  // s
  Eigen::RowVectorXG1 ud;
  G2 g2d;
};

struct Challenge {
  h256_t r;
};

struct Response {
  std::vector<Fr> m;  // n*s
  std::vector<Eigen::MatrixXFr> ek;
  std::vector<Eigen::MatrixXFr> ex;
};

struct Receipt {
  G1 u0_x0_lgs;
  G1 u0d;
};

struct Secret {
  Fr x0_lgs;
  Fr d;
#ifdef _DEBUG
  std::vector<Eigen::MatrixXFr> k;
  std::vector<Eigen::RowVectorXFr> x;
  std::vector<Fr> m;
#endif
};
}  // namespace scheme::table::batch3
