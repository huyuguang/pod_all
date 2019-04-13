#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "vrf.h"
#include "matrix_fr.h"

namespace scheme::table::vrfq {
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
}  // namespace scheme::table::vrfq

namespace scheme::table::otvrfq {
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
}  // namespace scheme::table::otvrfq

namespace scheme::table::batch {
struct Request {
  h256_t seed2_seed;
  std::vector<Range> demands;
};

struct Response {
  std::vector<G1> k;
  std::vector<Fr> m;
};

struct Receipt {
  h256_t seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace scheme::table::batch

namespace scheme::table::otbatch {
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
  h256_t seed2_seed;
  std::vector<Range> phantoms;  // sizeof() = L
  std::vector<G1> ot_vi;        // sizeof() = K
  G1 ot_v;
};

struct Response {
  std::vector<G1> k;      // sizeof() = L
  std::vector<G1> ot_ui;  // sizeof() = K
  std::vector<Fr> m;      // sizeof() = L
  
};

struct Receipt {
  h256_t seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace scheme::table::otbatch

namespace scheme::table::batch2 {
struct Request {
  h256_t seed2_seed;
  std::vector<Range> demands;
};

struct Response {
  std::vector<G1> k; // (n+1)*s
  std::vector<Fr> m; // n*s
  std::vector<Fr> vw; // s
};

struct Receipt {
  h256_t seed2;
  Fr sigma_vw;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};
}  // namespace scheme::table::batch2


namespace scheme::table::batch3 {
struct Request {
  h256_t seed2_seed;
  std::vector<Range> demands;
};

struct Response {
  std::vector<G1> k; // (n+1)*s
  std::vector<Fr> m; // n*s
  std::vector<Fr> vw; // s
  Eigen::Matrix<Fr, Eigen::Dynamic, Eigen::Dynamic> matrix;
};

struct Receipt {
  h256_t seed2;
  Fr sigma_vw;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};
}  // namespace scheme::table::batch3
