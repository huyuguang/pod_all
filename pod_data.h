#pragma once

#include "ecc.h"
#include "mkl_tree.h"
#include "mpz.h"
#include "public.h"

// load from file
class PodData {
 public:
  PodData(std::string const& root_path, h256_t const& name);  // load from file
  PodData(h256_t const& name, uint64_t n, uint64_t s);        // use rand data
  G2 const& pk() const { return pk_; }
  uint64_t size() const { return size_; }
  uint64_t n() const { return n_; }
  uint64_t s() const { return s_; }
  h256_t const& name() const { return name_; }
  std::vector<Fr> const& m() const { return m_; }
  std::vector<G1> const& sigma() const { return sigmas_; }

 public:
  std::vector<h256_t> const& mkl_tree() const { return sigma_mkl_tree_; }
  h256_t const& sigma_mkl_root() const { return sigma_mkl_tree_.back(); }

 private:
  std::string const root_path_;
  h256_t const name_;
  uint64_t n_;
  uint64_t s_;
  std::vector<Fr> m_;
  G2 pk_;
  std::vector<G1> sigmas_;
  uint64_t size_;
  std::vector<h256_t> sigma_mkl_tree_;
  h256_t sigma_mkl_root_;
};

inline uint64_t GetDataBlockCount(uint64_t size, uint64_t s) {
  uint64_t slice_count = (size + 31 - 1) / 31;
  return (slice_count + s - 1) / s;
}

struct PodDataPubInfo {
  h256_t name;
  G2 pk;
  h256_t sigma_mkl_root;
  uint64_t size;
  uint64_t s;
  uint64_t n() const { return GetDataBlockCount(size, s); }
};

bool BuildFileData(std::string const& src_file, std::string const& root_path,
                   h256_t const& name, uint64_t s, PodDataPubInfo* info);

PodDataPubInfo GetPodDataPubInfo(PodData const& data);

bool SaveFrToFile(std::string const& file, uint64_t size, uint64_t s,
                  uint64_t start, uint64_t count, std::vector<Fr> const& m);