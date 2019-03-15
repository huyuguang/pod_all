#include "scheme_table_test.h"
#include "scheme_table_a.h"
#include "scheme_table_b.h"
#include "scheme_table_client.h"
#include "scheme_table_protocol.h"
#include "scheme_table_session.h"

namespace scheme_misc::table {

bool Query(APtr a, BPtr b, std::string const& key_name,
           std::string const& key_value) {
  for (uint64_t i = 0;; ++i) {
    std::string key_value_with_suffix = key_value + "_" + std::to_string(i);
    Session session(a);
    Client client(b, key_name, key_value_with_suffix);

    QueryReq query_req;
    query_req.key_name = key_name;
    query_req.key_value = key_value_with_suffix;
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

    Fr r;
    if (!session.OnQueryReceipt(query_receipt, r)) {
      assert(false);
      return false;
    }
    int64_t position;
    if (!client.OnR(r, position)) {
      assert(false);
      return false;
    }
    std::cout << "Query " << key_name << " = " << key_value_with_suffix
              << ", position: " << position << "\n";
    if (position == -1) break;
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