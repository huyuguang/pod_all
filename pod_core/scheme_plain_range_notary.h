#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_plain_range_protocol.h"

namespace scheme::plain::range {
bool VerifyClaim(uint64_t s, Receipt const& receipt, Secret const& secret,
                 Claim const& claim);
}
