#include "scheme_table_b.h"
#include "misc.h"
#include "scheme_table_protocol.h"

namespace scheme_misc::table {

B::B(Bulletin const& bulletin, std::string const& public_path)
    : bulletin_(bulletin), public_path_(public_path) {
  LoadAndVerifyData();
}

B::B(std::string const& bulletin_file, std::string const& public_path)
    : public_path_(public_path) {
  if (!LoadBulletin(bulletin_file, bulletin_))
    throw std::runtime_error("invalid bulletin file");
  LoadAndVerifyData();
}

void B::LoadAndVerifyData() {
  if (!bulletin_.n || !bulletin_.s)
    throw std::runtime_error("invalid bulletin");

  std::string sigma_file = public_path_ + "/sigma";
  std::string sigma_mkl_tree_file = public_path_ + "/sigma_mkl_tree";
  std::string vrf_pk_file = public_path_ + "/vrf_pk";
  std::string key_meta_file = public_path_ + "/vrf_meta";

  // vrf meta
  if (!LoadVrfMeta(key_meta_file, bulletin_.vrf_meta_digest, vrf_meta_)) {
    assert(false);
    throw std::runtime_error("invalid vrf meta file");
  }
  h256_t vrf_meta_digest;
  if (!misc::GetFileSha256(key_meta_file, vrf_meta_digest) ||
    vrf_meta_digest != bulletin_.vrf_meta_digest) {
    assert(false);
    throw std::runtime_error("vrf meta digest mismatch");
  }

  // key m
  key_m_.resize(vrf_meta_.keys.size());
  for (size_t j = 0; j < key_m_.size(); ++j) {
    auto& km = key_m_[j];
    auto key_m_file = public_path_ + "/key_m_" + std::to_string(j);
    if (!LoadMatrix(key_m_file, bulletin_.n, km)) {
      assert(false);
      throw std::runtime_error("invalid key m file");
    }

    if (vrf_meta_.keys[j].unique && !IsElementUnique(km)) {
      assert(false);
      throw std::runtime_error("km not unique");
    }

    auto get_item = [&km](uint64_t i) -> h256_t {
      if (i < km.size()) {
        h256_t h;
        FrToBin(km[i], h.data());
        return h;
      } else {
        return h256_t();
      }
    };
    auto root = mkl::CalcRoot(get_item, bulletin_.n);
    if (vrf_meta_.keys[j].mj_mkl_root != root) {
      assert(false);
      throw std::runtime_error("key m mkl root mismatch");
    }
  }

  // sigma
  if (!LoadSigma(sigma_file, bulletin_.n, sigmas_)) {
    assert(false);
    throw std::runtime_error("invalid sigma file");
  }
  auto get_sigma = [this](uint64_t i) -> h256_t {
    if (i >= sigmas_.size()) return h256_t();
    return G1ToBin(sigmas_[i]);
  };
  if (bulletin_.sigma_mkl_root !=
      mkl::CalcRoot(std::move(get_sigma), sigmas_.size())) {
    assert(false);
    throw std::runtime_error("sigma mkl root mismatch");
  }

  // vrf bp
  vrf_key_bp_proofs_.resize(vrf_meta_.keys.size());
  for (size_t j = 0; j < vrf_key_bp_proofs_.size(); ++j) {
    auto const& key = vrf_meta_.keys[j];
    auto key_bp_file = public_path_ + "/key_bp_" + std::to_string(j);
    if (!LoadBpP1Proof(key_bp_file, key.bp_digest, vrf_key_bp_proofs_[j])) {
      assert(false);
      throw std::runtime_error("invalid key bp file");
    }

    auto const& km = key_m_[j];
    if (!VerifyKeyBp(bulletin_.n, bulletin_.s, km, sigmas_, j,
                     bulletin_.sigma_mkl_root, vrf_meta_.keys[j].mj_mkl_root,
                     vrf_key_bp_proofs_[j])) {
      assert(false);
      throw std::runtime_error("key m bp proof mismatch");
    }
  }

  // vrf pk
  if (!LoadVrfPk(vrf_pk_file, vrf_meta_.pk_digest, vrf_pk_)) {
    assert(false);
    throw std::runtime_error("invalid vrf pk file");
  }
  h256_t pk_digest;
  if (!misc::GetFileSha256(vrf_pk_file, pk_digest) ||
      vrf_meta_.pk_digest != pk_digest) {
    assert(false);
    throw std::runtime_error("vrf pk digest mismatch");
  }  
}

}  // namespace scheme_misc::table