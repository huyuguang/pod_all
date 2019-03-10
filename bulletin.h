#pragma once

#include <stdint.h>
#include <cassert>
#include <string>
#include <vector>

#include "basic_types.h"

namespace bulletin {

struct Plain {
  uint64_t size;
  uint64_t s;
  h256_t sigma_mkl_root;
};

struct Table {
  uint64_t n;
  uint64_t s;
  h256_t sigma_mkl_root;
  h256_t key_meta_hash;
};

bool IsValid(Plain const& data);
bool IsValid(Table const& data);

bool Save(std::string const& output, Plain const& data);
bool Load(std::string const& input, Plain& data);

bool Save(std::string const& output, Table const& data);
bool Load(std::string const& input, Table& data);

}  // namespace bulletin
