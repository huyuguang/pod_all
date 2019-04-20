#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_plain_b.h"
#include "scheme_plain_range_protocol.h"

namespace scheme::plain {

namespace range {
class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
         Range const& demand);

 public:
  void GetRequest(Request& request);
  bool OnResponse(Response response, Receipt& receipt);
  bool OnSecret(Secret const& secret);
  bool GenerateClaim(Claim& claim);
  bool SaveDecrypted(std::string const& file);

 private:
  bool CheckEncryptedM();
  bool CheckK(std::vector<Fr> const& v);
  bool CheckKDirect(std::vector<Fr> const& v);
  bool CheckKMultiExp(std::vector<Fr> const& v);
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
  Range const demand_;
  h256_t seed2_seed_;

 private:
  std::vector<G1> k_;

 private:
  h256_t seed2_;
  std::vector<Fr> w_;  // size() is count
  h256_t k_mkl_root_;
  std::vector<Fr> encrypted_m_;
  std::vector<Fr> decrypted_m_;
  int64_t claim_i_ = -1;
  int64_t claim_j_ = -1;
};

typedef std::shared_ptr<Client> ClientPtr;
}  // namespace range

}  // namespace scheme::plain