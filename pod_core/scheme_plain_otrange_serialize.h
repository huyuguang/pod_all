#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_plain_otrange_protocol.h"

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
  ar &YAS_OBJECT_NVP("spor::Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("spor::Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("spor::Secret", ("s", t.seed0));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("spor::Secret", ("s", t.seed0));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Claim const &t) {
  ar &YAS_OBJECT_NVP("spor::Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  ar &YAS_OBJECT_NVP("spor::Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
}

}  // namespace scheme::plain::otrange