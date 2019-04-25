#pragma once

#include "bulletin_plain.h"
#include "scheme_plain_otbatch3_protocol.h"

namespace scheme::plain::otbatch3 {

struct RomChallenge {
  Fr c;
  Fr e1;
  Fr e2;
  Fr e1_square;
  Fr e2_square;
  Fr e1_e2_inverse;
};

void ComputeChallenge(RomChallenge& challenge, h256_t const& client_id,
                      h256_t const& session_id, Bulletin const& bulletin,
                      Request const& request, Response const& response);
}  // namespace scheme::plain::otbatch3