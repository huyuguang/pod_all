#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_table_b.h"
#include "scheme_table_otbatch3_protocol.h"
#include "scheme_table_otbatch3_misc.h"

namespace scheme::table::otbatch3 {

class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
         std::vector<Range> demands, std::vector<Range> phantoms);

 public:
  void GetNegoReqeust(NegoBRequest& request);
  bool OnNegoRequest(NegoARequest const& request, NegoAResponse& response);
  bool OnNegoResponse(NegoBResponse const& response);

 public:
  void GetRequest(Request& request);
  bool OnResponse(Response response, Receipt& receipt);
  bool OnSecret(Secret secret);
  bool SaveDecrypted(std::string const& file);

 private:
  void BuildMapping();
  bool CheckEncryptedM();
  bool CheckUX0();
  bool CheckEK();
  bool CheckEX();  
  bool CheckCommitmentOfD();
  void DecryptK();
  void DecryptX();
  void DecryptM();
  void ExtractM();
  Fr GetOtFrE(G1 const& ot_ui);
 private:
  BPtr b_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;
  std::vector<Range> const demands_;
  std::vector<Range> const phantoms_;
  uint64_t demands_count_ = 0;
  uint64_t phantoms_count_ = 0;

 private:
  uint64_t align_c_;
  uint64_t align_s_;
  uint64_t log_c_;
  uint64_t log_s_;
  Request request_;
  Response response_;
  Receipt receipt_;
  Secret secret_;
  std::vector<std::vector<Fr>> k_;
  std::vector<std::vector<Fr>> x_;
  RomChallenge challenge_;

 private:
  G1 ot_self_pk_;
  G2 ot_peer_pk_;
  G1 ot_sk_;
  Fr ot_beta_;
  Fr ot_rand_a_;
  Fr ot_rand_b_;

 private:
  struct Mapping {
    uint64_t phantom_offset;
    uint64_t global_index;
  };
  std::vector<Mapping> mappings_;

 private:
  std::vector<Fr> decrypted_m_;
  std::vector<Fr> encrypted_m_;
};

typedef std::shared_ptr<Client> ClientPtr;
}  // namespace scheme::table::otbatch3