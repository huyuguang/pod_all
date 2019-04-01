#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_table_b.h"
#include "scheme_table_protocol.h"

namespace scheme::table::otbatch {

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
  bool OnSecret(Secret const& secret, Claim& claim);
  bool SaveDecrypted(std::string const& file);

 private:
  void BuildMapping();
  bool CheckEncryptedM();
  bool CheckK(std::vector<Fr> const& v, Claim& claim);
  bool CheckKDirect(std::vector<Fr> const& v, Claim& claim);
  bool CheckKMultiExp(std::vector<Fr> const& v, Claim& claim);
  void DecryptM(std::vector<Fr> const& v);
  uint64_t FindMismatchI(uint64_t mismatch_j,
                         std::vector<G1 const*> const& k_col,
                         std::vector<Fr const*> const& v_col);
  void BuildClaim(uint64_t i, uint64_t j, Claim& claim);

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
  std::vector<G1> k_;      // sizeof() = L
  std::vector<G1> ot_ui_;  // sizeof() = K
  h256_t seed2_seed_;

 private:
  struct Mapping {
    uint64_t phantom_offset;
    uint64_t index_of_m;
  };
  std::vector<Mapping> mappings_;

 private:
  h256_t seed2_;
  std::vector<Fr> w_;  // size() is L
  h256_t k_mkl_root_;
  std::vector<Fr> decrypted_m_;
  std::vector<Fr> encrypted_m_;

 private:
  G1 ot_self_pk_;
  G2 ot_peer_pk_;
  G1 ot_sk_;
  Fr ot_beta_;
  Fr ot_rand_a_;
  Fr ot_rand_b_;
};
}  // namespace scheme::table::otbatch