#include "scheme_plain_notary.h"
#include "chain.h"
#include "mkl_tree.h"

namespace scheme_misc::plain::range {
bool VerifyClaim(uint64_t count, uint64_t s, Receipt const& receipt,
                 Secret const& secret, Claim const& claim) {
  h256_t k_bin = G1ToBin(claim.kij);
  if (!mkl::VerifyPath(claim.i * s + claim.j, k_bin, count * s,
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
}  // namespace scheme_misc::plain::range