#pragma once

#include "basic_types_serialize.h"
#include "matrix_fr_serialize.h"
#include "misc.h"
#include "scheme_table_batch3_protocol.h"

namespace scheme::table::batch3 {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("stb3::Request", ("d", t.demands));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("stb3::Request", ("d", t.demands));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Commitment const &t) {
  ar &YAS_OBJECT_NVP("stb3::Commitment", ("uk", t.uk), ("ux0", t.ux0),
                     ("u0x", t.u0x), ("g2x0", t.g2x0), ("ud", t.ud),
                     ("g2d", t.g2d));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Commitment &t) {
  ar &YAS_OBJECT_NVP("stb3::Commitment", ("uk", t.uk), ("ux0", t.ux0),
                     ("u0x", t.u0x), ("g2x0", t.g2x0), ("ud", t.ud),
                     ("g2d", t.g2d));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Challenge const &t) {
  ar &YAS_OBJECT_NVP("stb3::Challenge", ("r", t.r));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Challenge &t) {
  ar &YAS_OBJECT_NVP("stb3::Challenge", ("r", t.r));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("stb3::Response", ("m", t.m), ("ek", t.ek), ("ex", t.ex));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("stb3::Response", ("m", t.m), ("ek", t.ek), ("ex", t.ex));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  ar &YAS_OBJECT_NVP("stb3::Receipt", ("u0_x0_lgs", t.u0_x0_lgs),
                     ("u0d", t.u0d));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("stb3::Receipt", ("u0_x0_lgs", t.u0_x0_lgs),
                     ("u0d", t.u0d));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {  
//#ifdef _DEBUG
//  ar &YAS_OBJECT_NVP("stb3::Secret", ("x0_lgs", t.x0_lgs), ("d", t.d),
//                     ("k", t.k), ("m", t.m), ("x", t.x));
//#else
  ar &YAS_OBJECT_NVP("stb3::Secret", ("x0_lgs", t.x0_lgs), ("d", t.d));
//#endif
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
//#ifdef _DEBUG
//  ar &YAS_OBJECT_NVP("stb3::Secret", ("x0_lgs", t.x0_lgs), ("d", t.d),
//                     ("k", t.k), ("m", t.m), ("x", t.x));
//#else
  ar &YAS_OBJECT_NVP("stb3::Secret", ("x0_lgs", t.x0_lgs), ("d", t.d));
//#endif
}
}  // namespace scheme::table::batch3
