#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "vrf.h"

namespace scheme_misc::table {
struct VrfQueryRequest {
  std::string key_name;
  std::string key_value;
};

struct VrfQueryResponse {
  vrf::Psk<> psk_exp_r;
  G1 g_exp_r;
};

struct VrfQueryReceipt {
  G1 g_exp_r;
};

struct VrfQuerySecret {
  Fr r;
};
}  // namespace scheme_misc::table
