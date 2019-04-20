#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_table_batch3_protocol.h"

namespace scheme::table::batch3 {
bool VerifyProof(Receipt const& receipt, Secret const& secret);
}  // namespace scheme::table::batch3