#include "scheme_plain_batch3_notary.h"
#include "chain.h"
#include "mkl_tree.h"
#include "scheme_misc.h"

namespace scheme::plain::batch3 {
bool VerifyProof(Receipt const& receipt, Secret const& secret) {
  // NOTE: Blockchain vm does not have ecc pub, must call u^v directly
  auto const& ecc_pub = GetEccPub();
  if (receipt.u0d != ecc_pub.PowerU1(0, secret.d)) {
    assert(false);
    return false;
  }
  if (receipt.u0_x0_lgs != ecc_pub.PowerU1(0, secret.x0_lgs)) {
    assert(false);
    return false;
  }
  return true;
}

}  // namespace scheme::plain::batch3