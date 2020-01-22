#pragma once

#include <cryptopp/keccak.h>
#include <vector>

#include "ecc.h"

enum {
  kMimc3Round = 64,
  kMimc5Round = 110,
};

inline uint64_t constexpr ZkpMimcCount() {
  return 1024;
}

std::vector<Fr> const& Mimc3Const();

std::vector<Fr> const& Mimc5Const();

Fr Mimc3(Fr const& left, Fr const& right);

Fr Mimc3Circuit(Fr const& left, Fr const& right);

Fr Mimc5(Fr const& s);