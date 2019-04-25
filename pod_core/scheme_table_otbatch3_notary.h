#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_table_otbatch3_protocol.h"

namespace scheme::table::otbatch3 {
bool VerifyProof(Receipt const& receipt, Secret const& secret);
}  // namespace scheme::table::otbatch3