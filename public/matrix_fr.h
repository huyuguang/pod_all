#pragma once

#include <Eigen/Dense>
#include "ecc.h"

namespace Eigen {
template <>
struct NumTraits<Fr> : NumTraits<mpz_class> {
  typedef Fr Real;
  typedef Fr NonInteger;
  typedef Fr Nested;
  enum {
    IsComplex = 0,
    IsInteger = 0,
    IsSigned = 1,
    RequireInitialization = 1,
    ReadCost = 1,
    AddCost = 3,
    MulCost = 3
  };
};
}  // namespace Eigen
