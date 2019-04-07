#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_table_protocol.h"

namespace scheme::table::batch {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("stb::Request", ("s", t.seed2_seed), ("d", t.demands));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("stb::Request", ("s", t.seed2_seed), ("d", t.demands));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("stb::Response", ("k", t.k), ("m", t.m));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("stb::Response", ("k", t.k), ("m", t.m));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  auto seed2_str = misc::HexToStr(t.seed2);
  auto k_mkl_root_str = misc::HexToStr(t.k_mkl_root);
  ar &YAS_OBJECT_NVP("stb::Receipt", ("s", seed2_str), ("k", k_mkl_root_str),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  std::string seed2_str;
  std::string k_mkl_root_str;
  ar &YAS_OBJECT_NVP("stb::Receipt", ("s", seed2_str), ("k", k_mkl_root_str),
                     ("c", t.count));
  misc::HexStrToH256(seed2_str, t.seed2);
  misc::HexStrToH256(k_mkl_root_str, t.k_mkl_root);
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  auto seed0_str = misc::HexToStr(t.seed0);
  ar &YAS_OBJECT_NVP("stb::Secret", ("s", seed0_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  std::string seed0_str;
  ar &YAS_OBJECT_NVP("stb::Secret", ("s", seed0_str));
  misc::HexStrToH256(seed0_str, t.seed0);
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Claim const &t) {
  std::string kij_str = G1ToStr(t.kij);
  std::vector<std::string> mkl_path_str(t.mkl_path.size());
  for (size_t i = 0; i < t.mkl_path.size(); ++i) {
    mkl_path_str[i] = misc::HexToStr(t.mkl_path[i]);
  }
  ar &YAS_OBJECT_NVP("stb::Claim", ("i", t.i), ("j", t.j), ("k", kij_str),
                     ("m", mkl_path_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  std::string kij_str;
  std::vector<std::string> mkl_path_str;
  ar &YAS_OBJECT_NVP("stb::Claim", ("i", t.i), ("j", t.j), ("k", kij_str),
                     ("m", mkl_path_str));
  t.kij = StrToG1(kij_str);  // throw
  t.mkl_path.resize(mkl_path_str.size());
  for (size_t i = 0; i < mkl_path_str.size(); ++i) {
    misc::HexStrToH256(mkl_path_str[i], t.mkl_path[i]);
  }
}

}  // namespace scheme::table::batch
