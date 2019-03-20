#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_plain_protocol.h"

namespace scheme_misc::plain {
class B;
typedef std::shared_ptr<B> BPtr;

class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BPtr b, h256_t const& self_id, h256_t const& peer_id, uint64_t start,
         uint64_t count);
  bool OnRangeResponse(RangeResponse const& response,
                       RangeChallenge& challenge);
  bool OnRangeReply(RangeReply const& reply, RangeReceipt& receipt);
  bool OnRangeSecret(RangeSecret const& secret, RangeClaim& claim);
  bool SaveDecrypted(std::string const& file);

 private:
  bool CheckK(std::vector<Fr> const& v, RangeClaim& claim);
  void DecryptM(std::vector<Fr> const& v);
  uint64_t FindMismatchI(uint64_t offset_i, uint64_t mismatch_j,
                         std::vector<G1 const*> const& k_col,
                         std::vector<Fr const*> const& v_col);

 private:
  BPtr b_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;
  uint64_t const start_;
  uint64_t const count_;

 private:
  RangeResponse response_;
  RangeReply reply_;

 private:
  mpz_class seed2_;
  std::vector<Fr> w_;  // size() is count
  h256_t k_mkl_root_;
  std::vector<Fr> decrypted_m_;
};
}  // namespace scheme_misc::plain