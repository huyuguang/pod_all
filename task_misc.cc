#include "task_misc.h"

#include "public.h"
#include "mkl_tree.h"
#include "misc.h"
#include "ecc_pub.h"
#include "multiexp.h"

void LoadMij(uint8_t const* data_start, uint8_t const* data_end, uint64_t i,
             uint64_t j, uint64_t s, Fr& mij) {
  auto offset = i * s + j;
  uint8_t const* p = data_start + offset * 31;
  uint8_t const* q = p + 31;
  if (p >= data_end) {
    mij = FrZero();
  } else {
    if (q > data_end) q = data_end;
    mij = BinToFr31(p, q);
  }
}

bool CopyData(std::string const& src, std::string const& dst) {
  try {
    io::mapped_file_params src_params;
    src_params.path = src;
    src_params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source src_view(src_params);

    io::mapped_file_params dst_params;
    dst_params.path = dst;
    dst_params.flags = io::mapped_file_base::readwrite;
    dst_params.new_file_size = src_view.size();
    io::mapped_file dst_view(dst_params);

    memcpy(dst_view.data(), src_view.data(), src_view.size());

    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveMkl(std::string const& output, std::vector<h256_t> const& mkl_tree) {
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = mkl_tree.size() * 32;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < mkl_tree.size(); ++i) {
      h256_t const& h = mkl_tree[i];
      memcpy(start + i * 32, h.data(), 32);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

std::vector<G1> CalcSigma(std::vector<Fr> const& m, uint64_t n, uint64_t s) {
  assert(m.size() == n*s);

  auto const& ecc_pub = GetEccPub();

  auto const& u1 = ecc_pub.u1();
  std::vector<G1> sigmas(n);
#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)n; ++i) {
    G1& sigma = sigmas[i];
    auto is = i * s;
    if (s > 1024) {
      Fr const* mi0 = &m[is];
      sigma = MultiExpBdlo12(u1.data(), mi0, s);
    } else {
      sigma = G1Zero();
      for (uint64_t j = 0; j < s; ++j) {
        sigma += ecc_pub.PowerU1(j, m[is + j]);
      }
    }
  }
  return sigmas;
}

bool SaveSigma(std::string const& output, std::vector<G1> const& sigma) {
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = sigma.size() * 32;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < sigma.size(); ++i) {
      G1ToBin(sigma[i], start + i * 32);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveMatrix(std::string const& output, std::vector<Fr> const& m) {
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = m.size() * 32;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < m.size(); ++i) {
      FrToBin(m[i], start + i * 32);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool GetFileSha256(std::string const& file, h256_t& h) {
  try {
    io::mapped_file_params params;
    params.path = file;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);

    CryptoPP::SHA256 hash;
    hash.Update((uint8_t const*)view.data(), view.size());
    hash.Final(h.data());
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}