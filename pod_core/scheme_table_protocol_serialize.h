#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_table_protocol.h"

namespace scheme::table::vrfq {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("stv::Request", ("k", t.key_name), ("v", t.value_digests));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("stv::Request", ("k", t.key_name), ("v", t.value_digests));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("stv::Response", ("p", t.psk_exp_r), ("g", t.g_exp_r));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("stv::Response", ("p", t.psk_exp_r), ("g", t.g_exp_r));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  auto g_exp_r_str = G1ToStr(t.g_exp_r);
  ar &YAS_OBJECT_NVP("stv::Receipt", ("g", g_exp_r_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  std::string g_exp_r_str;
  ar &YAS_OBJECT_NVP("stv::Receipt", ("s", g_exp_r_str));
  t.g_exp_r = StrToG1(g_exp_r_str);
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  auto r_str = FrToStr(t.r);
  ar &YAS_OBJECT_NVP("stv::Secret", ("r", r_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  std::string r_str;
  ar &YAS_OBJECT_NVP("stv::Secret", ("r", r_str));
  t.r = StrToFr(r_str);
}
}  // namespace scheme::table::vrfq

namespace scheme::table::otvrfq {
// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoARequest const &t) {
  ar &YAS_OBJECT_NVP("stov::NegoARequest", ("s", t.s));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoARequest &t) {
  ar &YAS_OBJECT_NVP("stov::NegoARequest", ("s", t.s));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoAResponse const &t) {
  ar &YAS_OBJECT_NVP("stov::NegoAResponse", ("s", t.s_exp_beta));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoAResponse &t) {
  ar &YAS_OBJECT_NVP("stov::NegoAResponse", ("s", t.s_exp_beta));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoBRequest const &t) {
  ar &YAS_OBJECT_NVP("stov::NegoBRequest", ("t", t.t));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoBRequest &t) {
  ar &YAS_OBJECT_NVP("stov::NegoBRequest", ("t", t.t));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoBResponse const &t) {
  ar &YAS_OBJECT_NVP("stov::NegoBResponse", ("t", t.t_exp_alpha));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoBResponse &t) {
  ar &YAS_OBJECT_NVP("stov::NegoBResponse", ("t", t.t_exp_alpha));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("stov::Request", ("k", t.key_name),
                     ("s", t.shuffled_value_digests), ("ot_vi", t.ot_vi),
                     ("ot_v", t.ot_v));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("stov::Request", ("k", t.key_name),
                     ("s", t.shuffled_value_digests), ("ot_vi", t.ot_vi),
                     ("ot_v", t.ot_v));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("stov::Response", ("s", t.shuffled_psk_exp_r),
                     ("g", t.g_exp_r), ("o", t.ot_ui));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("stov::Response", ("s", t.shuffled_psk_exp_r),
                     ("g", t.g_exp_r), ("o", t.ot_ui));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  auto g_exp_r_str = G1ToStr(t.g_exp_r);
  ar &YAS_OBJECT_NVP("stov::Receipt", ("g", g_exp_r_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  std::string g_exp_r_str;
  ar &YAS_OBJECT_NVP("stov::Receipt", ("s", g_exp_r_str));
  t.g_exp_r = StrToG1(g_exp_r_str);
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  auto r_str = FrToStr(t.r);
  ar &YAS_OBJECT_NVP("stov::Secret", ("r", r_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  std::string r_str;
  ar &YAS_OBJECT_NVP("stov::Secret", ("r", r_str));
  t.r = StrToFr(r_str);
}
}  // namespace scheme::table::otvrfq

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

namespace scheme::table::otbatch {
// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoARequest const &t) {
  ar &YAS_OBJECT_NVP("stob::NegoARequest", ("s", t.s));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoARequest &t) {
  ar &YAS_OBJECT_NVP("stob::NegoARequest", ("s", t.s));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoAResponse const &t) {
  ar &YAS_OBJECT_NVP("stob::NegoAResponse", ("s", t.s_exp_beta));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoAResponse &t) {
  ar &YAS_OBJECT_NVP("stob::NegoAResponse", ("s", t.s_exp_beta));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoBRequest const &t) {
  ar &YAS_OBJECT_NVP("stob::NegoBRequest", ("t", t.t));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoBRequest &t) {
  ar &YAS_OBJECT_NVP("stob::NegoBRequest", ("t", t.t));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoBResponse const &t) {
  ar &YAS_OBJECT_NVP("stob::NegoBResponse", ("t", t.t_exp_alpha));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoBResponse &t) {
  ar &YAS_OBJECT_NVP("stob::NegoBResponse", ("t", t.t_exp_alpha));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("stob::Request", ("s", t.seed2_seed), ("p", t.phantoms),
                     ("ot_vi", t.ot_vi), ("ot_v", t.ot_v));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("stob::Request", ("s", t.seed2_seed), ("p", t.phantoms),
                     ("ot_vi", t.ot_vi), ("ot_v", t.ot_v));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("stob::Response", ("k", t.k), ("o", t.ot_ui), ("m", t.m));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("stob::Response", ("k", t.k), ("o", t.ot_ui), ("m", t.m));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  auto seed2_str = misc::HexToStr(t.seed2);
  auto k_mkl_root_str = misc::HexToStr(t.k_mkl_root);
  ar &YAS_OBJECT_NVP("stob::Receipt", ("s", seed2_str), ("k", k_mkl_root_str),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  std::string seed2_str;
  std::string k_mkl_root_str;
  ar &YAS_OBJECT_NVP("stob::Receipt", ("s", seed2_str), ("k", k_mkl_root_str),
                     ("c", t.count));
  misc::HexStrToH256(seed2_str, t.seed2);
  misc::HexStrToH256(k_mkl_root_str, t.k_mkl_root);
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  auto seed0_str = misc::HexToStr(t.seed0);
  ar &YAS_OBJECT_NVP("stob::Secret", ("s", seed0_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  std::string seed0_str;
  ar &YAS_OBJECT_NVP("stob::Secret", ("s", seed0_str));
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
  ar &YAS_OBJECT_NVP("stob::Claim", ("i", t.i), ("j", t.j), ("k", kij_str),
                     ("m", mkl_path_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  std::string kij_str;
  std::vector<std::string> mkl_path_str;
  ar &YAS_OBJECT_NVP("stob::Claim", ("i", t.i), ("j", t.j), ("k", kij_str),
                     ("m", mkl_path_str));
  t.kij = StrToG1(kij_str);  // throw
  t.mkl_path.resize(mkl_path_str.size());
  for (size_t i = 0; i < mkl_path_str.size(); ++i) {
    misc::HexStrToH256(mkl_path_str[i], t.mkl_path[i]);
  }
}

}  // namespace scheme::table::otbatch

namespace scheme::table::batch2 {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("stb2::Request", ("s", t.seed2_seed), ("p", t.demands));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("stb2::Request", ("s", t.seed2_seed), ("p", t.demands));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("stb2::Response", ("k", t.k), ("m", t.m), ("vw", t.vw));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("stb2::Response", ("k", t.k), ("m", t.m), ("vw", t.vw));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  auto seed2_str = misc::HexToStr(t.seed2);
  auto sigma_vw_str = FrToStr(t.sigma_vw);
  ar &YAS_OBJECT_NVP("stb2::Receipt", ("s", seed2_str), ("vw", sigma_vw_str),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  std::string seed2_str;
  std::string sigma_vw_str;
  ar &YAS_OBJECT_NVP("stb2::Receipt", ("s", seed2_str), ("vw", sigma_vw_str),
                     ("c", t.count));
  misc::HexStrToH256(seed2_str, t.seed2);
  t.sigma_vw = StrToFr(sigma_vw_str);
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  auto seed0_str = misc::HexToStr(t.seed0);
  ar &YAS_OBJECT_NVP("stb2::Secret", ("s", seed0_str));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  std::string seed0_str;
  ar &YAS_OBJECT_NVP("stb2::Secret", ("s", seed0_str));
  misc::HexStrToH256(seed0_str, t.seed0);
}
}  // namespace scheme::table::batch2
