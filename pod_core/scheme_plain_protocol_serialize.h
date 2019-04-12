#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_plain_protocol.h"
#include "matrix_fr_serialize.h"

namespace scheme::plain::range {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("spr::Request", ("s", t.seed2_seed), ("d", t.demand));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("spr::Request", ("s", t.seed2_seed), ("d", t.demand));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("spr::Response", ("k", t.k), ("m", t.m));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("spr::Response", ("k", t.k), ("m", t.m));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  auto seed2_str = misc::HexToStr(t.seed2);
  auto k_mkl_root_str = misc::HexToStr(t.k_mkl_root);
  ar &YAS_OBJECT_NVP("spr::Receipt", ("s", seed2_str), ("k", k_mkl_root_str),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  std::string seed2_str;
  std::string k_mkl_root_str;
  ar &YAS_OBJECT_NVP("spr::Receipt", ("s", seed2_str), ("k", k_mkl_root_str),
                     ("c", t.count));
  misc::HexStrToH256(seed2_str, t.seed2);
  misc::HexStrToH256(k_mkl_root_str, t.k_mkl_root);
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  auto seed0_str = misc::HexToStr(t.seed0);
  ar &YAS_OBJECT_NVP("spr::Secret", ("s", seed0_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  std::string seed0_str;
  ar &YAS_OBJECT_NVP("spr::Secret", ("s", seed0_str));
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
  ar &YAS_OBJECT_NVP("spr::Claim", ("i", t.i), ("j", t.j), ("k", kij_str),
                     ("m", mkl_path_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  std::string kij_str;
  std::vector<std::string> mkl_path_str;
  ar &YAS_OBJECT_NVP("spr::Claim", ("i", t.i), ("j", t.j), ("k", kij_str),
                     ("m", mkl_path_str));
  t.kij = StrToG1(kij_str);  // throw
  t.mkl_path.resize(mkl_path_str.size());
  for (size_t i = 0; i < mkl_path_str.size(); ++i) {
    misc::HexStrToH256(mkl_path_str[i], t.mkl_path[i]);
  }
}

}  // namespace scheme::plain::range

namespace scheme::plain::otrange {
// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoARequest const &t) {
  ar &YAS_OBJECT_NVP("spor::NegoARequest", ("s", t.s));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoARequest &t) {
  ar &YAS_OBJECT_NVP("spor::NegoARequest", ("s", t.s));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoAResponse const &t) {
  ar &YAS_OBJECT_NVP("spor::NegoAResponse", ("s", t.s_exp_beta));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoAResponse &t) {
  ar &YAS_OBJECT_NVP("spor::NegoAResponse", ("s", t.s_exp_beta));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoBRequest const &t) {
  ar &YAS_OBJECT_NVP("spor::NegoBRequest", ("t", t.t));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoBRequest &t) {
  ar &YAS_OBJECT_NVP("spor::NegoBRequest", ("t", t.t));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoBResponse const &t) {
  ar &YAS_OBJECT_NVP("spor::NegoBResponse", ("t", t.t_exp_alpha));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoBResponse &t) {
  ar &YAS_OBJECT_NVP("spor::NegoBResponse", ("t", t.t_exp_alpha));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("spor::Request", ("s", t.seed2_seed), ("p", t.phantom),
                     ("ot_vi", t.ot_vi), ("ot_v", t.ot_v));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("spor::Request", ("s", t.seed2_seed), ("p", t.phantom),
                     ("ot_vi", t.ot_vi), ("ot_v", t.ot_v));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("spor::Response", ("k", t.k), ("o", t.ot_ui), ("m", t.m));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("spor::Response", ("k", t.k), ("o", t.ot_ui), ("m", t.m));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  auto seed2_str = misc::HexToStr(t.seed2);
  auto k_mkl_root_str = misc::HexToStr(t.k_mkl_root);
  ar &YAS_OBJECT_NVP("spor::Receipt", ("s", seed2_str), ("k", k_mkl_root_str),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  std::string seed2_str;
  std::string k_mkl_root_str;
  ar &YAS_OBJECT_NVP("spor::Receipt", ("s", seed2_str), ("k", k_mkl_root_str),
                     ("c", t.count));
  misc::HexStrToH256(seed2_str, t.seed2);
  misc::HexStrToH256(k_mkl_root_str, t.k_mkl_root);
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  auto seed0_str = misc::HexToStr(t.seed0);
  ar &YAS_OBJECT_NVP("spor::Secret", ("s", seed0_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  std::string seed0_str;
  ar &YAS_OBJECT_NVP("spor::Secret", ("s", seed0_str));
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
  ar &YAS_OBJECT_NVP("spor::Claim", ("i", t.i), ("j", t.j), ("k", kij_str),
                     ("m", mkl_path_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  std::string kij_str;
  std::vector<std::string> mkl_path_str;
  ar &YAS_OBJECT_NVP("spor::Claim", ("i", t.i), ("j", t.j), ("k", kij_str),
                     ("m", mkl_path_str));
  t.kij = StrToG1(kij_str);  // throw
  t.mkl_path.resize(mkl_path_str.size());
  for (size_t i = 0; i < mkl_path_str.size(); ++i) {
    misc::HexStrToH256(mkl_path_str[i], t.mkl_path[i]);
  }
}

}  // namespace scheme::plain::range