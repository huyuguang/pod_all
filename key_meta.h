#pragma once

#include <stdint.h>
#include <cassert>
#include <string>
#include <vector>

#include "basic_types.h"

namespace key_meta {
struct VrfKey {
  uint64_t column_index;
  uint64_t j;
  h256_t mj_mkl_root;
  h256_t bp_hash;
  bool operator==(VrfKey const& v) const {
    return column_index == v.column_index && mj_mkl_root == v.mj_mkl_root &&
           bp_hash == bp_hash && j == v.j;
  }
  bool operator!=(VrfKey const& v) const { return !((*this) == v); }
};

struct Vrf {
  h256_t pk_hash;
  std::vector<std::string> column_names;
  std::vector<VrfKey> keys;
  bool valid() const {
    if (column_names.empty()) return false;
    if (keys.empty()) return false;
    for (uint64_t i = 0; i<keys.size(); ++i) {
      if (keys[i].column_index >= column_names.size()) return false;
      if (keys[i].j != i) return false;
    }
    return true;
  }
  bool operator==(Vrf const& v) const {
    return pk_hash == v.pk_hash && column_names == v.column_names &&
           keys == v.keys;
  }
  bool operator!=(Vrf const& v) const { return !((*this) == v); }
};

bool SaveVrf(std::string const& output, Vrf const& data);
}  // namespace key_meta
