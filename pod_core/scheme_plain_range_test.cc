#include "scheme_plain_range_test.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_b.h"
#include "scheme_plain_range_client.h"
#include "scheme_plain_range_notary.h"
#include "scheme_plain_range_protocol.h"
#include "scheme_plain_range_session.h"

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t{1};
const h256_t kDummyClientId = h256_t{2};
}  // namespace

namespace scheme::plain::range {

bool Test(std::string const& output_path, APtr a, BPtr b, Range const& demand,
          bool evil) {
  Tick _tick_(__FUNCTION__);

  auto output_file = output_path + "/decrypted_data";

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

  Receipt receipt;
  if (!client.OnResponse(std::move(response), receipt)) {
    assert(false);
    return false;
  }

  Secret secret;
  if (!session.OnReceipt(receipt, secret)) {
    assert(false);
    return false;
  }

  if (!evil) {
    if (!client.OnSecret(secret)) {
      assert(false);
      return false;
    }

    if (!client.SaveDecrypted(output_file)) {
      assert(false);
      return false;
    }
  } else {
    if (client.OnSecret(secret)) {
      assert(false);
      return false;
    }
    Claim claim;
    if (!client.GenerateClaim(claim)) {
      assert(false);
      return false;
    }
    std::cout << "claim: " << claim.i << "," << claim.j << "\n";
    if (!VerifyClaim(a->bulletin().s, receipt, secret, claim)) {
      assert(false);
      return false;
    }
    std::cout << "verify claim success\n";
  }

  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          Range const& demand, bool test_evil) {
  try {
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);
    return Test(output_path, a, b, demand, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::plain::range