#include "scheme_table_test.h"
#include "scheme_table.h"
#include "scheme_table_a.h"
#include "scheme_table_b.h"
#include "scheme_table_protocol.h"
#include "scheme_table_vrfq_client.h"
#include "scheme_table_vrfq_session.h"

namespace scheme_misc::table {

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t{1};
const h256_t kDummyClientId = h256_t{2};
}  // namespace

namespace vrfq {
bool QueryInternal(APtr a, BPtr b, std::string const& key_name,
                   std::string const& key_value) {
  Session session(a, kDummySessionId, kDummyClientId);
  Client client(b, kDummyClientId, kDummySessionId, key_name, key_value);

  Request query_req;
  query_req.key_name = key_name;
  query_req.key_value = key_value;
  Response query_rsp;
  if (!session.OnRequest(query_req, query_rsp)) {
    assert(false);
    return false;
  }

  Receipt query_receipt;
  if (!client.OnResponse(query_rsp, query_receipt)) {
    assert(false);
    return false;
  }

  Secret query_secret;
  if (!session.OnReceipt(query_receipt, query_secret)) {
    assert(false);
    return false;
  }

  std::vector<uint64_t> positions;
  if (!client.OnSecret(query_secret, positions)) {
    assert(false);
    return false;
  }

  if (positions.empty()) {
    std::cout << "Query " << key_name << " = " << key_value << ": not exist\n";
  } else {
    std::cout << "Query " << key_name << " = " << key_value << ", Positions: ";
    for (auto i : positions) {
      std::cout << i << ";";
    }
    std::cout << "\n";
  }

  return true;
}

bool Test(APtr a, BPtr b, std::string const& key_name,
          std::string const& key_value) {
  auto vrf_key = GetKeyMetaByName(b->vrf_meta(), key_name);
  if (!vrf_key) return false;
  bool unique = vrf_key->unique;

  if (unique) {
    for (uint64_t i = 0;; ++i) {
      std::string key_value_with_suffix = key_value + "_" + std::to_string(i);
      if (!QueryInternal(a, b, key_name, key_value_with_suffix)) return false;
    }
    return true;
  } else {
    return QueryInternal(a, b, key_name, key_value);
  }
}
}  // namespace vrfq

bool Test(std::string const& publish_path, std::string const& key_name,
          std::string const& key_value) {
  try {
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);

    return vrfq::Test(a, b, key_name, key_value);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme_misc::table