#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "bp.h"
#include "ecc.h"
#include "vrf.h"

namespace scheme_misc {

enum Mode { kPlain, kTable };

std::istream& operator>>(std::istream& in, Mode& t);

std::ostream& operator<<(std::ostream& os, Mode const& t);

void LoadMij(uint8_t const* data_start, uint8_t const* data_end, uint64_t i,
             uint64_t j, uint64_t s, Fr& mij);

bool CopyData(std::string const& src, std::string const& dst);

bool SaveMkl(std::string const& output, std::vector<h256_t> const& mkl_tree);

bool LoadMkl(std::string const& input, uint64_t n,
             std::vector<h256_t>& mkl_tree);

bool SaveSigma(std::string const& output, std::vector<G1> const& sigma);

bool LoadSigma(std::string const& input, uint64_t n, std::vector<G1>& sigmas);

bool SaveMatrix(std::string const& output, std::vector<Fr> const& m);

bool LoadMatrix(std::string const& input, uint64_t ns, std::vector<Fr>& m);

std::vector<G1> CalcSigma(std::vector<Fr> const& m, uint64_t n, uint64_t s);

std::vector<h256_t> BuildSigmaMklTree(std::vector<G1> const& sigmas);

bool GetBulletinMode(std::string const& file, Mode& mode);
}  // namespace scheme_misc
