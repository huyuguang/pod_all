#pragma once

#include <basic_types.h>
#include <memory>
#include <string>
#include "scheme_plain_protocol.h"

namespace scheme_misc::plain::range {
bool VerifyClaim(uint64_t count, uint64_t s, Receipt const& receipt,
                 Secret const& secret, Claim const& claim);
}