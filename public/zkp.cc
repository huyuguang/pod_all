#include "zkp.h"
#include <fstream>
#include <iostream>

#include <libff/common/profiling.hpp>

namespace {
void DisableLibffLog() {
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
}
}  // namespace

void InitZkp() {
  libsnark::default_r1cs_gg_ppzksnark_pp::init_public_params();
  DisableLibffLog();
}

ZkFr ConvertToZkFr(Fr const& mcl_fr) {
  mpz_class m = mcl_fr.getMpz();
  return ZkFr(libff::bigint<ZkFr::num_limbs>(m.get_mpz_t()));
}

std::vector<ZkFr> ConvertToZkFr(std::vector<Fr> const& mcl_frs) {
  std::vector<ZkFr> zk_frs(mcl_frs.size());
  for (size_t i = 0; i < zk_frs.size(); ++i) {
    zk_frs[i] = ConvertToZkFr(mcl_frs[i]);
  }
  return zk_frs;
}

std::vector<ZkFr> ConvertToZkFr(std::vector<uint64_t> const& o) {
  std::vector<ZkFr> zk_frs(o.size());
  for (size_t i = 0; i < zk_frs.size(); ++i) {
    zk_frs[i] = ConvertToZkFr(Fr(o[i]));
  }
  return zk_frs;
}

ZkPkPtr LoadZkPk(std::string const& file) {
  try {
    ZkPkPtr ret(new ZkPk());
    std::ifstream ifs;
    ifs.open(file, std::ifstream::in | std::ifstream::binary);
    ifs >> (*ret);
    return ret;
  } catch (std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << "\n";
    return ZkPkPtr();
  }
}

ZkVkPtr LoadZkVk(std::string const& file) {
  try {
    ZkVkPtr ret(new ZkVk());
    std::ifstream ifs;
    ifs.open(file, std::ifstream::in | std::ifstream::binary);
    ifs >> (*ret);
    return ret;
  } catch (std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << "\n";
    return ZkVkPtr();
  }
}