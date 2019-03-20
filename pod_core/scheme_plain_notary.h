#pragma once

#include <basic_types.h>
#include <memory>
#include <string>
#include "scheme_plain_protocol.h"

namespace scheme_misc::plain {
bool VerifyRangeClaim(uint64_t count, uint64_t s, RangeReceipt const& receipt,
                      RangeSecret const& secret, RangeClaim const& claim);
}