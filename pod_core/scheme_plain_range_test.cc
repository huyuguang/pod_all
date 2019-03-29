#include "scheme_plain_range_test.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_b.h"
#include "scheme_plain_notary.h"
#include "scheme_plain_protocol.h"
#include "scheme_plain_range_client.h"
#include "scheme_plain_range_session.h"

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t{1};
const h256_t kDummyClientId = h256_t{2};
}  // namespace

namespace scheme::plain::range {

bool Test(std::string const& output_file, APtr a, BPtr b, Range const& demand,
          bool evil) {
  Tick _tick_(__FUNCTION__);

  Session session(a, kDummySessionId, kDummyClientId);
  Client client(b, kDummyClientId, kDummySessionId, demand);
  if (evil) session.TestSetEvil();

  Request request;
  client.GetRequest(request);

  Response response;
  if (!session.OnRequest(request, response)) {
    assert(false);
    return false;
  }

  Challenge challenge;
  if (!client.OnResponse(std::move(response), challenge)) {
    assert(false);
    return false;
  }

  Reply reply;
  if (!session.OnChallenge(challenge, reply)) {
    assert(false);
    return false;
  }

  Receipt receipt;
  if (!client.OnReply(std::move(reply), receipt)) {
    assert(false);
    return false;
  }

  Secret secret;
  if (!session.OnReceipt(receipt, secret)) {
    assert(false);
    return false;
  }

  if (!evil) {
    Claim claim;
    if (!client.OnSecret(secret, claim)) {
      assert(false);
      return false;
    }

    if (!client.SaveDecrypted(output_file)) {
      assert(false);
      return false;
    }
  } else {
    Claim claim;
    if (client.OnSecret(secret, claim)) {
      assert(false);
      return false;
    }
    std::cout << "claim: " << claim.i << "," << claim.j << "\n";
    if (!VerifyClaim(a->bulletin().s, receipt, secret, claim)) {
      assert(false);
      return false;
    }
  }

  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          Range const& demand) {
  try {
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);
    return Test(output_path, a, b, demand, false);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::plain::range