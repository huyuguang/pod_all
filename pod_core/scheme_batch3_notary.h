#pragma once

#include "scheme_otbatch3_notary.h"

namespace scheme::batch3 {
template <typename Receipt, typename Secret>
constexpr auto VerifyProof = scheme::otbatch3::VerifyProof<Receipt, Secret>;
}  // namespace scheme::batch3