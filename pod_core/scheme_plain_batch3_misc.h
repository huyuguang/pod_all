#pragma once

#include "bulletin_plain.h"
#include "scheme_plain_batch3_protocol.h"

namespace scheme::plain::batch3 {
bool CheckDemands(uint64_t n, std::vector<Range> const& demands);

h256_t RomChallengeSeed(h256_t const& client_id, h256_t const& session_id,
                        Bulletin const& bulletin, Request const& request,
                        Response const& response);

struct RomChallenge {
  Fr c;
  Fr e1;
  Fr e2;
  Fr e1_square;
  Fr e2_square;
  Fr e1_e2_inverse;
};

void ComputeChallenge(h256_t const& seed, RomChallenge& challenge);
}  // namespace scheme::plain::batch3