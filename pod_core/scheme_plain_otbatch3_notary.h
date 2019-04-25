#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_plain_otbatch3_protocol.h"

namespace scheme::plain::otbatch3 {
bool VerifyProof(Receipt const& receipt, Secret const& secret);
}  // namespace scheme::plain::otbatch3