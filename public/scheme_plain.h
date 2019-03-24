#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "ecc.h"

namespace scheme::plain {

uint64_t GetDataBlockCount(uint64_t size, uint64_t column_num);

bool DataToM(std::string const& pathname, uint64_t size, uint64_t n,
             uint64_t column_num, std::vector<Fr>& m);

bool MToFile(std::string const& file, uint64_t size, uint64_t s, uint64_t start,
             uint64_t count, std::vector<Fr> const& part_m);

void BuildK(std::vector<Fr> const& v, std::vector<G1>& k, uint64_t s);

h256_t CalcRootOfK(std::vector<G1> const& k);
}  // namespace scheme::plain