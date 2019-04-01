#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "vrf.h"

namespace scheme::table {

namespace vrfq {
struct Request {
  std::string key_name;
  std::vector<h256_t> value_digests;
};

struct Response {
  std::vector<vrf::Psk<>> psk_exp_r;
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
  std::vector<h256_t> shuffled_value_digests;  // sizeof() = L
  std::vector<G1> ot_vi;                       // sizeof() = K
  G1 ot_v;
};

struct Response {
  std::vector<vrf::Psk<>> shuffled_psk_exp_r;  // sizeof() = L
  G1 g_exp_r;
  std::vector<G1> ot_ui;  // sizeof() = K
};

struct Receipt {
  G1 g_exp_r;
};

struct Secret {
  Fr r;
};
}  // namespace otvrfq

namespace batch {
struct Request {
  Fr seed2_seed;
  std::vector<Range> demands;  
};

struct Response {
  std::vector<G1> k;
  std::vector<Fr> m;  // sizeof() = L
};

struct Receipt {
  mpz_class seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  mpz_class seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace batch

namespace otbatch {
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
  Fr seed2_seed;
  std::vector<Range> phantoms;  // sizeof() = L
  std::vector<G1> ot_vi;        // sizeof() = K
  G1 ot_v;
};

struct Response {
  std::vector<G1> k;      // sizeof() = L
  std::vector<G1> ot_ui;  // sizeof() = K
  std::vector<Fr> m;  // sizeof() = L
};

struct Receipt {
  mpz_class seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  mpz_class seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace otbatch
}  // namespace scheme::table
