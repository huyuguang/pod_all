#include "c_api.h"

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../scheme_table_a.h"
#include "../scheme_table_b.h"
#include "../scheme_table_protocol_serialize.h"
#include "../scheme_table_batch_client.h"
#include "../scheme_table_batch_session.h"
#include "../scheme_table_batch2_client.h"
#include "../scheme_table_batch2_session.h"
#include "../scheme_table_otbatch_client.h"
#include "../scheme_table_otbatch_session.h"
#include "../scheme_table_otvrfq_client.h"
#include "../scheme_table_otvrfq_session.h"
#include "../scheme_table_vrfq_client.h"
#include "../scheme_table_vrfq_session.h"
#include "ecc.h"
#include "ecc_pub.h"

#if 0
namespace scheme::table {
std::mutex a_set_mutex;
std::unordered_map<void*, APtr> a_set;
std::mutex b_set_mutex;
std::unordered_map<void*, BPtr> b_set;

APtr GetAPtr(handle_t h) {
  APtr ret;
  std::scoped_lock<std::mutex> lock(a_set_mutex);
  auto it = a_set.find(h);
  if (it != a_set.end()) ret = it->second;
  return ret;
}

scheme::table::BPtr GetBPtr(handle_t h) {
  scheme::table::BPtr ret;
  std::scoped_lock<std::mutex> lock(b_set_mutex);
  auto it = b_set.find(h);
  if (it != b_set.end()) ret = it->second;
  return ret;
}

void AddA(A* p) {
  APtr ptr(p);
  std::scoped_lock<std::mutex> lock(a_set_mutex);
  a_set.insert(std::make_pair(p, ptr));
}

void AddB(B* p) {
  BPtr ptr(p);
  std::scoped_lock<std::mutex> lock(b_set_mutex);
  b_set.insert(std::make_pair(p, ptr));
}

bool DelA(A* p) {
  std::scoped_lock<std::mutex> lock(a_set_mutex);
  return a_set.erase(p) != 0;
}

bool DelB(B* p) {
  std::scoped_lock<std::mutex> lock(b_set_mutex);
  return b_set.erase(p) != 0;
}
}  // namespace scheme::table

namespace scheme::table::batch {
std::mutex session_set_mutex;
std::unordered_map<void*, SessionPtr> session_set;
std::mutex client_set_mutex;
std::unordered_map<void*, ClientPtr> client_set;

void AddSession(Session* p) {
  SessionPtr ptr(p);
  std::scoped_lock<std::mutex> lock(session_set_mutex);
  session_set.insert(std::make_pair(p, ptr));
}

SessionPtr GetSessionPtr(handle_t h) {
  SessionPtr ret;
  std::scoped_lock<std::mutex> lock(session_set_mutex);
  auto it = session_set.find(h);
  if (it != session_set.end()) ret = it->second;
  return ret;
}

bool DelSession(Session* p) {
  std::scoped_lock<std::mutex> lock(session_set_mutex);
  return session_set.erase(p) != 0;
}

void AddClient(Client* p) {
  ClientPtr ptr(p);
  std::scoped_lock<std::mutex> lock(client_set_mutex);
  client_set.insert(std::make_pair(p, ptr));
}

ClientPtr GetClientPtr(handle_t h) {
  ClientPtr ret;
  std::scoped_lock<std::mutex> lock(client_set_mutex);
  auto it = client_set.find(h);
  if (it != client_set.end()) ret = it->second;
  return ret;
}

bool DelClient(Client* p) {
  std::scoped_lock<std::mutex> lock(client_set_mutex);
  return client_set.erase(p) != 0;
}

}  // namespace scheme::table::range

extern "C" {

EXPORT handle_t E_TableANew(char const* publish_path) {
  using namespace scheme::table;
  try {
    auto p = new A(publish_path);
    AddA(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT handle_t E_TableBNew(char const* bulletin_file,
                            char const* public_path) {
  using namespace scheme::table;
  try {
    auto p = new B(bulletin_file, public_path);
    AddB(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableABulletin(handle_t h, plain_bulletin_t* bulletin) {
  using namespace scheme::table;
  APtr a = GetAPtr(h);
  if (!a) return false;
  Bulletin const& v = a->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  bulletin->size = v.size;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  return true;
}

EXPORT bool E_TableBBulletin(handle_t h, plain_bulletin_t* bulletin) {
  using namespace scheme::table;
  BPtr b = GetBPtr(h);
  if (!b) return false;
  Bulletin const& v = b->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  bulletin->size = v.size;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  return true;
}

EXPORT bool E_TableAFree(handle_t h) {
  using namespace scheme::table;
  return DelA((A*)h);
}

EXPORT bool E_TableBFree(handle_t h) {
  using namespace scheme::table;
  return DelB((B*)h);
}

EXPORT handle_t E_TableBatchSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  APtr a = GetAPtr(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, std::tuple_size<h256_t>::value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, std::tuple_size<h256_t>::value);

  try {
    auto p = new Session(a, self_id, peer_id);
    AddSession(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableBatchSessionOnRequest(handle_t c_session,
                                         char const* request_file,
                                         char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream> ia(is);
    ia.serialize(request);

    Response response;
    if (!session->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableBatchSessionOnReceipt(handle_t c_session,
                                         char const* receipt_file,
                                         char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!session->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableBatchSessionSetEvil(handle_t c_session) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_TableBatchSessionFree(handle_t h) {
  using namespace scheme::table::batch;
  return DelSession((Session*)h);
}

EXPORT handle_t E_TableBatchClientNew(handle_t c_b, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id,
                                      range_t c_demand) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  BPtr b = GetBPtr(c_b);
  if (!b) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, std::tuple_size<h256_t>::value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, std::tuple_size<h256_t>::value);
  Range demand(c_demand.start, c_demand.count);

  try {
    auto p = new Client(b, self_id, peer_id, demand);
    AddClient(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableBatchClientGetRequest(handle_t c_client,
                                         char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Request request;
    client->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableBatchClientOnResponse(handle_t c_client,
                                         char const* response_file,
                                         char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(response, receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableBatchClientOnSecret(handle_t c_client,
                                       char const* secret_file,
                                       char const* claim_file) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);

    Claim claim;   
    if (!client->OnSecret(secret, claim)) {
      yas::file_ostream os(claim_file);
      yas::json_oarchive<yas::file_ostream> oa(os);
      oa.serialize(claim);
      return false;
    }
    return true;
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableBatchClientSaveDecrypted(handle_t c_client,
                                            char const* file) {
  using namespace scheme::table;
  using namespace scheme::table::batch;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableBatchClientFree(handle_t h) {
  using namespace scheme::table::batch;
  return DelClient((Client*)h);
}
}  // extern "C"
#endif 