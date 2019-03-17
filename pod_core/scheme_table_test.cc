#include "scheme_table_test.h"
#include "scheme_table.h"
#include "scheme_table_a.h"
#include "scheme_table_b.h"
#include "scheme_table_client.h"
#include "scheme_table_protocol.h"
#include "scheme_table_session.h"

namespace scheme_misc::table {

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t { 1 };
const h256_t kDummyClientId = h256_t { 2 };
}

bool QueryOne(APtr a, BPtr b, std::string const& key_name,
              std::string const& key_value, std::vector<uint64_t>& positions) {
  Session session(a, kDummySessionId, kDummyClientId);
  Client client(b, kDummyClientId, kDummySessionId, key_name, key_value);

  QueryReq query_req;
  query_req.key_name = key_name;
  query_req.key_value = key_value;
  QueryRsp query_rsp;
  if (!session.OnQueryReq(query_req, query_rsp)) {
    assert(false);
    return false;
  }

  QueryReceipt query_receipt;
  if (!client.OnQueryRsp(query_rsp, query_receipt)) {
    assert(false);
    return false;
  }

  QuerySecret query_secret;
  if (!session.OnQueryReceipt(query_receipt, query_secret)) {
    assert(false);
    return false;
  }

  if (!client.OnSecretReveal(query_secret, positions)) {
    assert(false);
    return false;
  }
  return true;
}

bool QueryInternal(APtr a, BPtr b, std::string const& key_name,
                   std::string const& key_value) {
  std::vector<uint64_t> positions;
  if (!QueryOne(a, b, key_name, key_value, positions)) {
    std::cerr << "Query failed\n";
    return false;
  } else {
    if (positions.empty()) {
      std::cout << "Query " << key_name << " = " << key_value << ": not exist\n";
    } else {
      std::cout << "Query " << key_name << " = " << key_value << ", Positions: ";
      for (auto i : positions) {
        std::cout << i << ";";
      }
      std::cout << "\n";
    }
  }
  return true;
}

bool Query(APtr a, BPtr b, std::string const& key_name,
           std::string const& key_value) {
  auto vrf_key = GetKeyMetaByName(b->vrf_meta(), key_name);
  if (!vrf_key) return false;
  bool unique = vrf_key->unique;

  if (unique) {
    for (uint64_t i = 0;; ++i) {
      std::string key_value_with_suffix = key_value + "_" + std::to_string(i);
      QueryInternal(a, b, key_name, key_value_with_suffix);
    }
  } else {
    QueryInternal(a, b, key_name, key_value);
  }
  return true;
}

bool Test(std::string const& publish_path, std::string const& key_name,
          std::string const& key_value) {
  try {
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);

    return Query(a, b, key_name, key_value);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}
}  // namespace scheme_misc::table