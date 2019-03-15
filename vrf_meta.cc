#include "vrf_meta.h"

#include "chain.h"
#include "ecc_pub.h"
#include "misc.h"
#include "mkl_tree.h"
#include "public.h"

namespace scheme_misc {
namespace table {
bool VrfKeyMeta::operator==(VrfKeyMeta const& v) const {
  return column_index == v.column_index && mj_mkl_root == v.mj_mkl_root &&
         bp_digest == bp_digest && j == v.j;
}

bool VrfKeyMeta::operator!=(VrfKeyMeta const& v) const {
  return !((*this) == v);
}

bool VrfMeta::valid() const {
  if (column_names.empty()) return false;
  if (keys.empty()) return false;
  for (uint64_t i = 0; i < keys.size(); ++i) {
    if (keys[i].column_index >= column_names.size()) return false;
    if (keys[i].j != i) return false;
  }
  return true;
}

bool VrfMeta::operator==(VrfMeta const& v) const {
  return pk_digest == v.pk_digest && column_names == v.column_names &&
         keys == v.keys;
}

bool VrfMeta::operator!=(VrfMeta const& v) const { return !((*this) == v); }

bool SaveVrfMeta(std::string const& output, VrfMeta const& data) {
  if (!data.valid()) {
    assert(false);
    return false;
  }

  try {
    pt::ptree tree;
    tree.put("pk_digest", misc::HexToStr(data.pk_digest));

    pt::ptree names_node;
    for (auto const& i : data.column_names) {
      pt::ptree name_node;
      name_node.put("", i);
      names_node.push_back(std::make_pair("", name_node));
    }
    tree.add_child("column_names", names_node);

    pt::ptree vrf_keys_node;
    for (auto const& i : data.keys) {
      pt::ptree vrf_key_node;
      vrf_key_node.put("j", i.j);
      vrf_key_node.put("column_index", i.column_index);
      vrf_key_node.put("mj_mkl_root", misc::HexToStr(i.mj_mkl_root));
      vrf_key_node.put("bp_digest", misc::HexToStr(i.bp_digest));
      vrf_keys_node.push_back(std::make_pair("", vrf_key_node));
    }
    tree.add_child("keys", vrf_keys_node);
    pt::write_json(output, tree);

    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadVrfMeta(std::string const& input, h256_t const& digest,
                 VrfMeta& data) {
  h256_t digest2;
  if (!misc::GetFileSha256(input, digest2)) {
    assert(false);
    return false;
  }

  if (digest2 != digest) {
    assert(false);
    return false;
  }

  try {
    pt::ptree tree;
    std::string str;
    pt::read_json(input, tree);

    misc::HexStrToH256(tree.get<std::string>("pk_digest"), data.pk_digest);

    for (pt::ptree::value_type& name_key : tree.get_child("column_names")) {
      data.column_names.push_back(name_key.second.data());
    }

    for (pt::ptree::value_type& pt_key : tree.get_child("keys")) {
      VrfKeyMeta vrf_key;
      vrf_key.j = pt_key.second.get<uint32_t>("j");
      vrf_key.column_index = pt_key.second.get<uint32_t>("column_index");
      str = pt_key.second.get<std::string>("mj_mkl_root");
      misc::HexStrToH256(str, vrf_key.mj_mkl_root);
      str = pt_key.second.get<std::string>("bp_digest");
      misc::HexStrToH256(str, vrf_key.bp_digest);
      data.keys.push_back(vrf_key);
    }
    return data.valid();
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveVrfPk(std::string const& output, vrf::Pk<> const& pk) {
  const uint64_t kG2BufSize = 64;
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = pk.size() * kG2BufSize;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < pk.size(); ++i) {
      G2ToBin(pk[i], start + i * kG2BufSize);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadVrfPk(std::string const& file, h256_t const& digest, vrf::Pk<>& pk) {
  h256_t digest2;
  if (!misc::GetFileSha256(file, digest2)) {
    assert(false);
    return false;
  }
  if (digest != digest2) {
    assert(false);
    return false;
  }

  const uint64_t kG2BufSize = 64;
  try {
    io::mapped_file_params params;
    params.path = file;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    if (view.size() != kG2BufSize * pk.size()) {
      assert(false);
      return false;
    }

    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < pk.size(); ++i) {
      if (!BinToG2(start + i * kG2BufSize, &pk[i])) {
        assert(false);
        return false;
      }
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveVrfSk(std::string const& output, vrf::Sk<> const& sk) {
  const uint64_t kFrBufSize = 32;
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = sk.size() * kFrBufSize;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < sk.size(); ++i) {
      FrToBin(sk[i], start + i * kFrBufSize);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadVrfSk(std::string const& file, vrf::Sk<>& sk) {
  const uint64_t kFrBufSize = 32;
  try {
    io::mapped_file_params params;
    params.path = file;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    if (view.size() != kFrBufSize * sk.size()) {
      assert(false);
      return false;
    }

    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < sk.size(); ++i) {
      if (!BinToFr32(start + i * kFrBufSize, &sk[i])) {
        assert(false);
        return false;
      }
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveBpP1Proof(std::string const& output, bp::P1Proof const& proof) {
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = proof.GetBufSize();
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    if (!proof.serialize(start, params.new_file_size)) {
      assert(false);
      return false;
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadBpP1Proof(std::string const& file, h256_t const& digest,
                   bp::P1Proof& proof) {
  h256_t digest2;
  if (!misc::GetFileSha256(file, digest2)) {
    assert(false);
    return false;
  }
  if (digest != digest2) {
    assert(false);
    return false;
  }

  try {
    io::mapped_file_params params;
    params.path = file;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    uint8_t* start = (uint8_t*)view.data();
    auto size = view.size();
    if (proof.deserialize(start, size) != size) {
      assert(false);
      return false;
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

void BuildKeyBp(uint64_t n, uint64_t s, std::vector<Fr> const& m,
                std::vector<G1> const& sigmas, h256_t const& sigma_mkl_root,
                uint64_t key_pos, h256_t keycol_mkl_root,
                bp::P1Proof& p1_proof) {
  assert(sigmas.size() == n);
  assert(m.size() == n * s);

  auto& ecc_pub = GetEccPub();
  auto bp_count = s - 1;

  uint8_t seed[64];
  memcpy(seed, sigma_mkl_root.data(), sigma_mkl_root.size());
  memcpy(seed + 32, keycol_mkl_root.data(), keycol_mkl_root.size());
  std::vector<Fr> v(n);
  for (uint64_t i = 0; i < n; ++i) {
    v[i] = Chain(seed, sizeof(seed), i);
  }

  std::vector<Fr> mv(bp_count);
  for (uint64_t j = 0; j < s; ++j) {
    if (j == key_pos) continue;
    auto jj = j < key_pos ? j : j - 1;
    mv[jj] = FrZero();
    for (uint64_t i = 0; i < n; ++i) {
      mv[jj] += v[i] * m[i * s + j];
    }
  }

  Fr f0 = FrZero();
  auto get_f = [&mv, &f0, bp_count](uint64_t j) -> Fr const& {
    if (j < bp_count)
      return mv[j];
    else
      return f0;
  };

  G1 g0 = G1Zero();
  auto get_g = [&ecc_pub, &g0, key_pos, bp_count](uint64_t j) -> G1 const& {
    if (j < bp_count) {
      auto const& u = ecc_pub.u1();
      if (j < key_pos) {
        return u[j];
      } else {
        return u[j + 1];
      }
    } else {
      return g0;
    }
  };

  p1_proof = bp::P1Prove(get_g, get_f, bp_count);

#ifdef _DEBUG
  std::vector<Fr> km(n);
  for (uint64_t i = 0; i < n; ++i) km[i] = m[i * s + key_pos];
  assert(VerifyKeyBp, n, s, km, sigmas, key_pos, sigma_mkl_root,
         keycol_mkl_root, p1_proof);
#endif
}

bool VerifyKeyBp(uint64_t n, uint64_t s, std::vector<Fr> const& km,
                 std::vector<G1> const& sigmas, uint64_t key_pos,
                 h256_t const& sigma_mkl_root, h256_t keycol_mkl_root,
                 bp::P1Proof const& p1_proof) {
  assert(sigmas.size() == n);
  assert(km.size() == n);

  auto& ecc_pub = GetEccPub();
  auto bp_count = s - 1;

  uint8_t seed[64];
  memcpy(seed, sigma_mkl_root.data(), sigma_mkl_root.size());
  memcpy(seed + 32, keycol_mkl_root.data(), keycol_mkl_root.size());
  std::vector<Fr> v(n);
  for (uint64_t i = 0; i < n; ++i) {
    v[i] = Chain(seed, sizeof(seed), i);
  }

  std::vector<G1> sigmas2(n);
  for (uint64_t i = 0; i < n; ++i) {
    G1 u_exp_mi_key = ecc_pub.PowerU1(key_pos, km[i]);
    sigmas2[i] = sigmas[i] - u_exp_mi_key;
  }
  if (MultiExpBdlo12(sigmas2, v) != p1_proof.committment.p) {
    assert(false);
    return false;
  }

  G1 g0 = G1Zero();
  auto get_g = [&ecc_pub, &g0, key_pos, bp_count](uint64_t j) -> G1 const& {
    if (j < bp_count) {
      auto const& u = ecc_pub.u1();
      if (j < key_pos) {
        return u[j];
      } else {
        return u[j + 1];
      }
    } else {
      return g0;
    }
  };

  return P1Verify(p1_proof, get_g, bp_count);
}
}  // namespace table
}  // namespace scheme_misc