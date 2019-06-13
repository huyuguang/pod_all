#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_atomic_swap_vc_protocol.h"

namespace scheme::atomic_swap_vc {
bool VerifyProof(uint64_t s, Receipt const& receipt, Secret const& secret);
bool VerifyProof(uint64_t s, uint64_t count, Fr const& sigma_vw,
                 std::vector<Fr> const& v, std::vector<Fr> const& w);
}  // namespace scheme::atomic_swap_vc
