#pragma once

#include "basic_types_serialize.h"
#include "matrix_fr_serialize.h"
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
  ar &YAS_OBJECT_NVP("stv::Receipt", ("g", t.g_exp_r));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("stv::Receipt", ("g", t.g_exp_r));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("stv::Secret", ("r", t.r));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("stv::Secret", ("r", t.r));
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
  ar &YAS_OBJECT_NVP("stov::Receipt", ("g", t.g_exp_r));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("stov::Receipt", ("g", t.g_exp_r));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("stov::Secret", ("r", t.r));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("stov::Secret", ("r", t.r));
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
  ar &YAS_OBJECT_NVP("stb::Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("stb::Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("stb::Secret", ("s", t.seed0));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("stb::Secret", ("s", t.seed0));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Claim const &t) {
  ar &YAS_OBJECT_NVP("stb::Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  ar &YAS_OBJECT_NVP("stb::Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
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
  ar &YAS_OBJECT_NVP("stob::Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("stob::Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("stob::Secret", ("s", t.seed0));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("stob::Secret", ("s", t.seed0));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Claim const &t) {
  ar &YAS_OBJECT_NVP("stob::Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  ar &YAS_OBJECT_NVP("stob::Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
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
  ar &YAS_OBJECT_NVP("stb2::Receipt", ("s", t.seed2), ("vw", t.sigma_vw),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("stb2::Receipt", ("s", t.seed2), ("vw", t.sigma_vw),
                     ("c", t.count));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("stb2::Secret", ("s", t.seed0));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("stb2::Secret", ("s", t.seed0));
}
}  // namespace scheme::table::batch2

namespace scheme::table::batch3 {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("stb3::Request", ("s", t.seed2_seed), ("p", t.demands));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("stb3::Request", ("s", t.seed2_seed), ("p", t.demands));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("stb3::Response", ("k", t.k), ("m", t.m), ("vw", t.vw),
                     ("matrix", t.matrix));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("stb3::Response", ("k", t.k), ("m", t.m), ("vw", t.vw),
                     ("matrix", t.matrix));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  ar &YAS_OBJECT_NVP("stb3::Receipt", ("s", t.seed2), ("vw", t.sigma_vw),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("stb3::Receipt", ("s", t.seed2), ("vw", t.sigma_vw),
                     ("c", t.count));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("stb3::Secret", ("s", t.seed0));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("stb3::Secret", ("s", t.seed0));
}
}  // namespace scheme::table::batch3
