#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "vrf.h"

namespace scheme_misc::table {

namespace vrfq {
struct Request {
  std::string key_name;
  std::string key_value;
};

struct Response {
  vrf::Psk<> psk_exp_r;
  G1 g_exp_r;
};

struct Receipt {
  G1 g_exp_r;
};

struct Secret {
  Fr r;
};
}  // namespace vrfq

namespace otvrfq {
struct NegoARequest {
  G2 s;
};

struct NegoAResponse {
  G2 s_exp_beta;
};

struct NegoBRequest {
  G1 t;
};

struct NegoBResponse {
  G1 t_exp_alpha;
};

struct Request {
  std::string key_name;
  std::vector<h256_t> mixed_key_digests; // sizeof() = L
  std::vector<G1> vi; // sizeof() = K
  G1 v;
};

struct Response {
  std::vector<vrf::Psk<>> psk_exp_r_mixed; // sizeof() = L
  G1 g_exp_r;
  std::vector<G1> ui; // sizeof() = K
};

struct Receipt {
  G1 g_exp_r;
};

struct Secret {
  Fr r;
};
}  // namespace otvrfq

}  // namespace scheme_misc::table
