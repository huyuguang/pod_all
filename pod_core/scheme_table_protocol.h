#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "vrf.h"

namespace scheme_misc::table {
struct QueryReq {
  std::string key_name;
  std::string key_value;
};

struct QueryRsp {
  vrf::Psk<> psk_exp_r;
  G1 g_exp_r;
};

struct QueryReceipt {
  G1 g_exp_r;
};

struct QuerySecret {
  Fr r;
};
}  // namespace scheme_misc::table
