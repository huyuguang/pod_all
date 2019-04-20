#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_table_batch_protocol.h"

namespace scheme::table::batch {
bool VerifyClaim(uint64_t s, Receipt const& receipt, Secret const& secret,
                 Claim const& claim);
}
