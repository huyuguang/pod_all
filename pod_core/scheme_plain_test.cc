#include "scheme_plain_test.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_b.h"
#include "scheme_plain_client.h"
#include "scheme_plain_protocol.h"
#include "scheme_plain_session.h"
#include "scheme_plain_notary.h"

namespace scheme_misc::plain {

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t{1};
const h256_t kDummyClientId = h256_t{2};
}  // namespace

bool Range(std::string const& output_file, APtr a, BPtr b, uint64_t start,
           uint64_t count, bool evil) {
  Tick _tick_(__FUNCTION__);

  Session session(a, kDummySessionId, kDummyClientId);
  Client client(b, kDummyClientId, kDummySessionId, start, count);
  if (evil) session.TestSetEvil();

  RangeRequest request;
  request.start = start;
  request.count = count;
  RangeResponse response;
  if (!session.OnRangeRequest(request, response)) {
    assert(false);
    return false;
  }

  RangeChallenge challenge;
  if (!client.OnRangeResponse(std::move(response), challenge)) {
    assert(false);
    return false;
  }

  RangeReply reply;
  if (!session.OnRangeChallenge(challenge, reply)) {
    assert(false);
    return false;
  }

  RangeReceipt receipt;
  if (!client.OnRangeReply(std::move(reply), receipt)) {
    assert(false);
    return false;
  }

  RangeSecret secret;
  if (!session.OnRangeReceipt(receipt, secret)) {
    assert(false);
    return false;
  }

  if (!evil) {
    RangeClaim claim;
    if (!client.OnRangeSecret(secret, claim)) {
      assert(false);
      return false;
    }

    if (!client.SaveDecrypted(output_file)) {
      assert(false);
      return false;
    }
  } else {
    RangeClaim claim;
    if (client.OnRangeSecret(secret, claim)) {
      assert(false);
      return false;
    }
    std::cout << "claim: " << claim.i << "," << claim.j << "\n";
    if (!VerifyRangeClaim(count, a->bulletin().s, receipt, secret, claim)) {
      assert(false);
      return false;
    }
  }

  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          uint64_t start, uint64_t count) {
  try {
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);
    return Range(output_path, a, b, start, count, true);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme_misc::plain