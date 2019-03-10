#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "ecc.h"

void LoadMij(uint8_t const* data_start, uint8_t const* data_end, uint64_t i,
             uint64_t j, uint64_t s, Fr& mij);

bool CopyData(std::string const& src, std::string const& dst);

bool SaveMkl(std::string const& output, std::vector<h256_t> const& mkl_tree);

bool SaveSigma(std::string const& output, std::vector<G1> const& sigma);

bool SaveMatrix(std::string const& output, std::vector<Fr> const& m);

std::vector<G1> CalcSigma(std::vector<Fr> const& m, uint64_t n, uint64_t s);

bool GetFileSha256(std::string const& file, h256_t& h);