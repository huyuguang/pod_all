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
                   std::vector<std::string> const& key_values) {
  Session session(a, kDummySessionId, kDummyClientId);
  Client client(b, kDummyClientId, kDummySessionId, key_name, key_values);

  Request query_req;
  client.GetRequest(query_req);

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

  std::vector<std::vector<uint64_t>> positions;
  if (!client.OnSecret(query_secret, positions)) {
    assert(false);
    return false;
  }

  assert(positions.size() == key_values.size());
  for (size_t i = 0; i < positions.size(); ++i) {
    auto const& position = positions[i];
    if (position.empty()) {
      std::cout << "Query " << key_name << " = " << key_values[i]
                << ": not exist\n";
    } else {
      std::cout << "Query " << key_name << " = " << key_values[i]
                << ", Positions: ";
      for (auto p : position) {
        std::cout << p << ";";
      }
      std::cout << "\n";
    }
  }

  return true;
}

bool Test(APtr a, BPtr b, std::string const& key_name,
          std::vector<std::string> const& key_values) {
  auto vrf_key = GetKeyMetaByName(b->vrf_meta(), key_name);
  if (!vrf_key) return false;
  bool unique = vrf_key->unique;

  if (unique) {
    for (auto const& key_value : key_values) {
      for (uint64_t i = 0;; ++i) {
        std::string key_value_with_suffix = key_value + "_" + std::to_string(i);
        std::vector<std::string> values{key_value_with_suffix};
        if (!QueryInternal(a, b, key_name, values)) return false;
      }
    }
    return true;
  } else {
    return QueryInternal(a, b, key_name, key_values);
  }
}
}  // namespace vrfq

bool Test(std::string const& publish_path, std::string const& key_name,
          std::vector<std::string> const& key_values) {
  try {
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);

    return vrfq::Test(a, b, key_name, key_values);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme_misc::table