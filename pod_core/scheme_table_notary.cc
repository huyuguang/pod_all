#include "scheme_table_notary.h"
#include "chain.h"
#include "mkl_tree.h"
#include "scheme_misc.h"

namespace scheme::table::otbatch {
bool VerifyClaim(uint64_t s, Receipt const& receipt, Secret const& secret,
                 Claim const& claim) {
  // same with the scheme::plain::range::VerifyClaim
  if (!VerifyPathOfK(claim.kij, claim.i * s + claim.j, receipt.count * s,
                     receipt.k_mkl_root, claim.mkl_path)) {
    assert(false);
    return false;  
  }

  // NOTE: Blockchain vm does not have ecc pub, must call u^v directly
  auto const& ecc_pub = GetEccPub();
  Fr v = Chain(secret.seed0, claim.i * s + claim.j);
  G1 check_k = ecc_pub.PowerU1(claim.j, v);
  if (check_k == claim.kij) {
    assert(false);
    return false;
  }

  return true;
}
}  // namespace scheme::table::otbatch


namespace scheme::table::batch {
bool VerifyClaim(uint64_t s, Receipt const& receipt, Secret const& secret,
                 Claim const& claim) {
  // same with the scheme::plain::range::VerifyClaim
  if (!VerifyPathOfK(claim.kij, claim.i * s + claim.j, receipt.count * s,
                     receipt.k_mkl_root, claim.mkl_path)) {
    assert(false);
    return false;  
  }

  // NOTE: Blockchain vm does not have ecc pub, must call u^v directly
  auto const& ecc_pub = GetEccPub();
  Fr v = Chain(secret.seed0, claim.i * s + claim.j);
  G1 check_k = ecc_pub.PowerU1(claim.j, v);
  if (check_k == claim.kij) {
    assert(false);
    return false;
  }

  return true;
}
}  // namespace scheme::plain::otrange

namespace scheme::table::batch2 {
bool VerifyProof(uint64_t s, Receipt const& receipt, Secret const& secret) {
  std::vector<Fr> v;
  H2(secret.seed0, (receipt.count + 1) * s, v);
  std::vector<Fr> w;
  H2(receipt.seed2, receipt.count, w);

  return VerifyProof(s, receipt.count, receipt.sigma_vw, v, w);
}

bool VerifyProof(uint64_t s, uint64_t count, Fr const& sigma_vw,
                 std::vector<Fr> const& v, std::vector<Fr> const& w) {
  assert(v.size() == (count + 1) * s);
  assert(w.size() == count);
  Fr check_sigma_vw = FrZero();

  size_t offset = count * s;
  for (size_t j = 0; j < s; ++j) {
    check_sigma_vw += v[offset + j];
  }
  for (size_t i = 0; i < count; ++i) {
    Fr sigma_v = FrZero();
    for (size_t j = 0; j < s; ++j) {
      sigma_v += v[i * s + j];
    }
    check_sigma_vw += sigma_v * w[i];
  }
  return check_sigma_vw == sigma_vw;
}
}  // namespace scheme::plain::otrange