#include "scheme_misc.h"

#include "chain.h"
#include "ecc_pub.h"
#include "misc.h"
#include "mkl_tree.h"
#include "multiexp.h"
#include "public.h"

namespace scheme_misc {
std::istream& operator>>(std::istream& in, Mode& t) {
  std::string token;
  in >> token;
  if (token == "plain") {
    t = Mode::kPlain;
  } else if (token == "table") {
    t = Mode::kTable;
  } else {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

std::ostream& operator<<(std::ostream& os, Mode const& t) {
  if (t == Mode::kPlain) {
    os << "plain";
  } else if (t == Mode::kTable) {
    os << "table";
  } else {
    os.setstate(std::ios_base::failbit);
  }
  return os;
}

bool GetBulletinMode(std::string const& file, Mode& mode) {
  try {
    pt::ptree tree;
    pt::read_json(file, tree);
    auto str = tree.get<std::string>("mode");
    if (str == "table") {
      mode = Mode::kTable;
    } else if (str == "plain") {
      mode = Mode::kPlain;
    } else {
      return false;
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

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

bool LoadMkl(std::string const& input, uint64_t n,
             std::vector<h256_t>& mkl_tree) {
  try {
    io::mapped_file_params params;
    params.path = input;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    auto tree_size = mkl::GetTreeSize(n);
    if (view.size() != tree_size * 32) {
      assert(false);
      return false;
    }
    mkl_tree.resize(tree_size);
    auto start = (uint8_t*)view.data();
    for (size_t i = 0; i < tree_size; ++i) {
      h256_t& h = mkl_tree[i];
      memcpy(h.data(), start + i * h.size(), h.size());
    }

    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

std::vector<G1> CalcSigma(std::vector<Fr> const& m, uint64_t n, uint64_t s) {
  assert(m.size() == n * s);

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

bool LoadSigma(std::string const& input, uint64_t n, std::vector<G1>& sigmas) {
  try {
    io::mapped_file_params params;
    params.path = input;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    if (view.size() != n * 32) return false;

    sigmas.resize(n);
    auto start = (uint8_t*)view.data();
    for (size_t i = 0; i < n; ++i) {
      sigmas[i] = BinToG1(start + i * 32);
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

bool LoadMatrix(std::string const& input, uint64_t ns, std::vector<Fr>& m) {
  try {
    io::mapped_file_params params;
    params.path = input;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    if (view.size() != 32 * ns) return false;

    auto start = (uint8_t*)view.data();
    m.resize(ns);
    for (uint64_t i = 0; i < m.size(); ++i) {
      if (!BinToFr32(start + i * 32, &m[i])) {
        assert(false);
        return false;
      }
    }
    return true;
  } catch (std::exception&) {
    return false;
  }
}

std::vector<h256_t> BuildSigmaMklTree(std::vector<G1> const& sigmas) {
  auto get_sigma = [&sigmas](uint64_t i) -> h256_t {
    return G1ToBin(sigmas[i]);
  };
  return mkl::BuildTree(sigmas.size(), get_sigma);
}

bool IsElementUnique(std::vector<Fr> const v) {
  std::vector<Fr const*> pv(v.size());
  for (size_t i = 0; i < v.size(); ++i) pv[i] = &v[i];

  auto compare = [](Fr const* a, Fr const* b) {
    return a->getMpz() < b->getMpz();
  };

  std::sort(pv.begin(), pv.end(), compare);

  return std::adjacent_find(pv.begin(), pv.end(), compare) == pv.end();
}
}  // namespace scheme_misc

