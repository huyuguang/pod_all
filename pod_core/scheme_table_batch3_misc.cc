#include "scheme_table_batch3_misc.h"
#include "misc.h"
#include "tick.h"

namespace scheme::table::batch3 {
bool CheckDemands(uint64_t n, std::vector<Range> const& demands) {
  if (demands.empty()) return false;

  for (auto const& demand : demands) {
    if (!demand.count || demand.start >= n || demand.count > n ||
        (demand.start + demand.count) > n) {
      return false;
    }
  }

  for (size_t i = 1; i < demands.size(); ++i) {
    if (demands[i].start <= demands[i - 1].start + demands[i - 1].count)
      return false;
  }
  return true;
}

h256_t RomChallengeSeed(h256_t const& client_id, h256_t const& session_id,
                        Bulletin const& bulletin, Request const& request,
                        Response const& response) {
  Tick _tick_(__FUNCTION__);
  CryptoPP::Keccak_256 hash;

  // client_id and session_id
  {
    hash.Update(client_id.data(), client_id.size());
    hash.Update(session_id.data(), session_id.size());
  }

  // bulletin
  {
    uint64_t big = bulletin.n;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));

    big = bulletin.s;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));

    hash.Update(bulletin.sigma_mkl_root.data(), bulletin.sigma_mkl_root.size());
  }

  // request
  for (auto const& i : request.demands) {
    uint64_t big = i.start;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));
    big = i.count;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));
  }

  // commitment of response
  for (auto const& i : response.uk) {
    for (auto const& j : i) {
      auto bin = G1ToBin(j);
      hash.Update(bin.data(), bin.size());
    }
  }
  for (auto const& i : response.ux0) {
    auto bin = G1ToBin(i);
    hash.Update(bin.data(), bin.size());
  }
  for (auto const& i : response.u0x) {
    for (auto const& j : i) {
      auto bin = G1ToBin(j);
      hash.Update(bin.data(), bin.size());
    }
  }
  for (auto const& i : response.g2x0) {
    std::array<uint8_t, 64> bin;
    G2ToBin(i, bin.data());
    hash.Update(bin.data(), bin.size());
  }
  for (auto const& i : response.ud) {
    auto bin = G1ToBin(i);
    hash.Update(bin.data(), bin.size());
  }
  {
    std::array<uint8_t, 64> bin;
    G2ToBin(response.g2d, bin.data());
    hash.Update(bin.data(), bin.size());
  }

  // result
  h256_t digest;
  hash.Final(digest.data());
  return digest;
}

void ComputeChallenge(h256_t const& seed, RomChallenge& challenge) {
  static const std::string suffix_c = "challenge_c";
  static const std::string suffix_e1 = "challenge_e1";
  static const std::string suffix_e2 = "challenge_e2";

  h256_t digest;
  CryptoPP::Keccak_256 hash;

  hash.Update(seed.data(), seed.size());
  hash.Update((uint8_t*)suffix_c.data(), suffix_c.size());
  hash.Final(digest.data());
  challenge.c.setArrayMaskMod(digest.data(), digest.size());
  // std::cout << "session c_: " << c_ << "\n";

  hash.Update(seed.data(), seed.size());
  hash.Update((uint8_t*)suffix_e1.data(), suffix_e1.size());
  hash.Final(digest.data());
  challenge.e1.setArrayMaskMod(digest.data(), digest.size());
  // std::cout << "session e1_: " << e1_ << "\n";

  hash.Update(seed.data(), seed.size());
  hash.Update((uint8_t*)suffix_e2.data(), suffix_e2.size());
  hash.Final(digest.data());
  challenge.e2.setArrayMaskMod(digest.data(), digest.size());
  // std::cout << "session e2_: " << e2_ << "\n";

  challenge.e1_square = challenge.e1 * challenge.e1;
  challenge.e2_square = challenge.e2 * challenge.e2;
  challenge.e1_e2_inverse = FrInv(challenge.e1 * challenge.e2_square -
                                  challenge.e2 * challenge.e1_square);
}
}  // namespace scheme::table::batch3
