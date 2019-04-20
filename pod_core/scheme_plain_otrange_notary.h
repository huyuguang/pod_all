#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_plain_otrange_protocol.h"

namespace scheme::plain::otrange {
bool VerifyClaim(uint64_t s, Receipt const& receipt, Secret const& secret,
                 Claim const& claim);
}