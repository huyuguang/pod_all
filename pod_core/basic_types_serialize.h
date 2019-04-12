#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4101)
#pragma warning(disable : 4702)
#endif
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>
#include <yas/types/std/array.hpp>
#include <yas/types/std/pair.hpp>
#include <yas/types/std/string.hpp>
#include <yas/types/std/vector.hpp>
#include <yas/mem_streams.hpp>
#include <yas/file_streams.hpp>
#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/json_iarchive.hpp>
#include <yas/json_oarchive.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "basic_types.h"
#include "ecc.h"

// save
template<typename Ar>
void serialize(Ar &ar, Range const& t) {
  ar &YAS_OBJECT_NVP("Range", ("", t.start), ("", t.count));
}

// load
template<typename Ar>
void serialize(Ar &ar, Range &t) {
  ar &YAS_OBJECT_NVP("Range", ("", t.start), ("", t.count));
}

namespace mcl {
// save
template<typename Ar>
void serialize(Ar &ar, G1 const& t) {
  if (yas::is_binary_archive<Ar>::value) {
    h256_t bin = G1ToBin(t);
    ar &YAS_OBJECT_NVP("G1", ("", bin));
  } else {
    assert(yas::is_json_archive<Ar>::value);
    std::string str = G1ToStr(t);
    ar &YAS_OBJECT_NVP("G1", ("", str));
  }
}

// load
template <typename Ar>
void serialize(Ar &ar, G1 &t) {
  if (ar.type() == yas::binary) {
    h256_t bin;
    ar &YAS_OBJECT_NVP("G1", ("", bin));
    t = BinToG1(bin.data());  // throw
  } else {
    assert(ar.type() == yas::json);
    std::string str;
    ar &YAS_OBJECT_NVP("G1", ("", str));
    t = StrToG1(str);  // throw
  }
}

// save
template<typename Ar>
void serialize(Ar &ar, G2 const& t) {
  if (yas::is_binary_archive<Ar>::value) {
    std::array<uint8_t, 64> bin;
    G2ToBin(t, bin.data());
    ar &YAS_OBJECT_NVP("G2", ("", bin));
  } else {
    assert(yas::is_json_archive<Ar>::value);
    std::string str = G2ToStr(t);
    ar &YAS_OBJECT_NVP("G2", ("", str));
  }
}

// load
template<typename Ar>
void serialize(Ar &ar, G2 &t) {
  if (ar.type() == yas::binary) {
    std::array<uint8_t, 64> bin;
    ar &YAS_OBJECT_NVP("G2", ("", bin));
    t = BinToG2(bin.data());  // throw
  }  else {
    assert(ar.type() == yas::json);
    std::string str;
    ar &YAS_OBJECT_NVP("G2", ("", str));
    t = StrToG2(str);  // throw
  }
}

// save
template<typename Ar>
void serialize(Ar &ar, Fr const& t) {
  if (yas::is_binary_archive<Ar>::value) {
    h256_t bin = FrToBin(t);
    ar &YAS_OBJECT_NVP("Fr", ("", bin));
  } else {
    assert(yas::is_json_archive<Ar>::value);
    std::string str = FrToStr(t);
    ar &YAS_OBJECT_NVP("Fr", ("", str));
  }
}

// load
template<typename Ar>
void serialize(Ar &ar, Fr &t) {
  if (ar.type() == yas::binary) {
    h256_t bin;
    ar &YAS_OBJECT_NVP("Fr", ("", bin));
    t = BinToFr32(bin.data());  // throw
  } else {
    assert(ar.type() == yas::json);
    std::string str;
    ar &YAS_OBJECT_NVP("Fr", ("", str));
    t = StrToFr(str);  // throw
  }
}
}
