#include <assert.h>
#include <cryptopp/osrng.h>
#include <cryptopp/randpool.h>
#include <iostream>
#include "ecc.h"
#include "misc.h"
#include "tick.h"

Fr FrPower2(Fr const& base, mpz_class const& exp) {
  // Fr result(1);
  // bool begin = false;
  // auto e = exp.get_mpz_t();
  // ssize_t n = mpz_sizeinbase(e, 2);
  // for (ssize_t i = n - 1; i >= 0; --i) {
  //  if (begin) {
  //    result = result * result;
  //  }

  //  if (mpz_tstbit(e, i)) {
  //    begin = true;
  //    result *= base;
  //  }
  //}
  // return result;

  auto e = exp.get_mpz_t();
  size_t n = mpz_sizeinbase(e, 2);
  std::vector<Fr> exp_bits(n);
  for (size_t i = 0; i < n; ++i) {
    exp_bits[i] = mpz_tstbit(e, i) ? Fr(1) : Fr(0);
  }

  std::vector<Fr> exp_bit_base(n);
  for (size_t i = 0; i < n; ++i) {
    exp_bit_base[i] = Fr(1) - exp_bits[i] + exp_bits[i] * base;
  }

  std::vector<Fr> result2(n - 1);
  std::vector<Fr> result(n - 2);
  Fr out;

  for (size_t i = 0; i < n - 1; ++i) {
    auto& r_1 = i == 0 ? exp_bit_base[n - 1] : result[i - 1];
    result2[i] = r_1 * r_1;
    auto& r = i == (n - 2) ? out : result[i];
    r = result2[i] * exp_bit_base[n - i - 2];
  }
  return out;
}

Fr FrPower3(Fr const& base, mpz_class const& exp) {
  auto e = exp.get_mpz_t();
  size_t n = mpz_sizeinbase(e, 2);
  std::vector<Fr> exp_bits(n);
  for (size_t i = 0; i < n; ++i) {
    exp_bits[i] = mpz_tstbit(e, i) ? Fr(1) : Fr(0);
  }

  std::vector<Fr> base_pow2(n); // base^(2^i)
  base_pow2[0] = base;
  for (size_t i = 1; i < n; ++i) {
    base_pow2[i] = base_pow2[i - 1] * base_pow2[i - 1];
  }

  std::vector<Fr> result2(n);
  for (size_t i = 0; i < n; ++i) {
    result2[i] = Fr(1) - exp_bits[i] + exp_bits[i] * base_pow2[i];
  }

  std::vector<Fr> result(n);
  result[0] = result2[0];
  for (size_t i = 1; i < n; ++i) {
    result[i] = result[i - 1] * result2[i];
  }
  return result[n-1];
}

void test() {
  for (int i = 0; i < 100; ++i) {
    Fr base = FrRand();
    mpz_class exp = misc::RandMpz32();
    auto a1 = FrPower(base, exp);
    auto a2 = FrPower2(base, exp);
    auto a3 = FrPower3(base, exp);
    assert(a1 == a2);
    assert(a3 == a2);
  }
}

int main(int /*argc*/, char** /*argv*/) {
  InitEcc();
  test();
  return 0;
}
