#pragma once

#include <basic_types.h>
#include <memory>
#include <string>
#include "scheme_table_protocol.h"

namespace scheme::table::otbatch {
bool VerifyClaim(uint64_t s, Receipt const& receipt, Secret const& secret,
                 Claim const& claim);
}

namespace scheme::table::batch {
bool VerifyClaim(uint64_t s, Receipt const& receipt, Secret const& secret,
                 Claim const& claim);
}