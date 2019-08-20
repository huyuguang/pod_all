#include <assert.h>
#include <cryptopp/osrng.h>
#include <cryptopp/randpool.h>
#include <iostream>
#include "../pod_core/zkp_key.h"
#include "ecc.h"
#include "ecc_pub.h"
#include "misc.h"
#include "public.h"
#include "tick.h"
#include "zkp.h"

#include "../pod_core/scheme_atomic_swap_serialize.h"
#include "../pod_core/scheme_atomic_swap_vc_misc.h"

// return -m0^-1 % 2^64
uint64_t test_inv(uint64_t m0) {
  uint64_t inv = 1;
  for (int i = 0; i < 63; ++i) {
    inv = inv * inv;
    inv = inv * m0;
  }
  assert(1 == (inv * m0));
  inv = (-(int64_t)inv);

  return inv;
}

uint32_t test_inv32(uint32_t m0) {
  uint32_t inv = 1;
  for (int i = 0; i < 63; ++i) {
    inv = inv * inv;
    inv = inv * m0;
  }
  assert(1 == (inv * m0));
  inv = (-(int32_t)inv);
  return inv;
}

mpz_class rand_mpz256() {
  uint8_t s[32];
  for (auto& i : s) {
    i = rand() % 256;
  }
  auto str = misc::HexToStr(s, 32);
  return mpz_class(str.data(), 16);
}

mpz_class rand_mpz512() {
  uint8_t s[64];
  for (auto& i : s) {
    i = rand() % 256;
  }
  auto str = misc::HexToStr(s, 64);
  return mpz_class(str.data(), 16);
}

#ifdef _DEBUG
const int kLoop = 1000;
#else
const int kLoop = 1000000;
#endif

//std::array<mpz_class, kLoop> ga, gb, gc;
//auto gp = mpz_class(
//    "218882428718392752222464057452572750885483644004160343436982041865758084"
//    "95617");

//void init_random() {
//  Tick _tick_(__FUNCTION__);
//  for (auto& i : ga) i = rand_mpz256();
//  for (auto& i : gb) i = rand_mpz256();
//}

//void test_gmp_mul() {
//  Tick _tick_(__FUNCTION__);
//  for (int i = 0; i < kLoop; ++i) {
//    gc[i] = ga[i] * gb[i];    
//  }
//}
//
//void test_gmp_squared() {
//  Tick _tick_(__FUNCTION__);
//  for (int i = 0; i < kLoop; ++i) {
//    gc[i] = ga[i] * ga[i];    
//  }
//}
//
//void test_gmp_div() {
//  Tick _tick_(__FUNCTION__);
//  for (int i = 0; i < kLoop; ++i) {
//    gc[i] = ga[i] / gb[i];
//  }
//}
//
//void test_gmp_mod() {
//  Tick _tick_(__FUNCTION__);
//  for (int i = 0; i < kLoop; ++i) {
//    gc[i] = ga[i] % gp;
//  }
//}
//
//void test_gmp_mod512() {
//  Tick _tick_(__FUNCTION__);
//  auto aa = rand_mpz512();
//  for (int i = 0; i < kLoop; ++i) {
//    gc[i] = aa % gp;
//  }
//}
//
//void test_gmp_mulmod() {
//  Tick _tick_(__FUNCTION__);
//  for (int i = 0; i < kLoop; ++i) {
//    gc[i] = ga[i] * gb[i];
//    gc[i] = gc[i] % gp;
//  }
//}
//
//void test_gmp_add() {
//  Tick _tick_(__FUNCTION__);
//  for (int i = 0; i < kLoop; ++i) {
//    //mpn_add_n(gc[i].get_mpz_t(), ga[i].get_mpz_t(), gb[i].get_mpz_t(), 4);
//    gc[i] = ga[i] + gb[i];
//  }
//}
//
//void test_gmp_del() {
//  Tick _tick_(__FUNCTION__);
//  for (int i = 0; i < kLoop; ++i) {
//    gc[i] = ga[i] - gb[i];
//  }
//}

void test_mcl_fr_mul() {
  std::vector<Fr> a(kLoop);
  std::vector<Fr> b(kLoop);
  std::vector<Fr> c(kLoop);
  for (auto& i : a) i = FrRand();
  for (auto& i : b) i = FrRand();
  Fr d = FrZero();
  Tick _tick_(__FUNCTION__);
  for (int i = 0; i < kLoop; ++i) {
    c[i] = a[i] * b[i];
    d += c[i];
  }
  std::cout << d << "\n";
}

void test_ff_fr_mul_inv_sqrt() {
  typedef libff::Fr<libsnark::default_r1cs_gg_ppzksnark_pp> ZkFr;
  std::vector<ZkFr> a(kLoop);
  std::vector<ZkFr> b(kLoop);
  std::vector<ZkFr> c(kLoop);
  for (auto& i : a) i = ZkFr::random_element();
  for (auto& i : b) i = ZkFr::random_element();

  ZkFr d;
  {
    Tick _tick_("ff add");
    for (int i = 0; i < kLoop; ++i) {
      d += a[i];
    }
  }
  {
    Tick _tick_("ff del");
    for (int i = 0; i < kLoop; ++i) {
      d -= a[i];
    }
  }

  {
    Tick _tick_("ff mul");
    for (int i = 0; i < kLoop; ++i) {
      c[i] = a[i] * b[i];
    }
  }

  {
    Tick _tick_("ff inv");
    for (auto& i : a) i = i.inverse();
  }

  {
    Tick _tick_("ff power");
    for (int i = 0; i < kLoop/1000; ++i) {
      c[i] = a[i] ^ c[i].as_bigint();
    }
  }

  {
    Tick _tick_("ff squared");
    for (int i = 0; i < kLoop; ++i) {
      a[i] = a[i].squared();
    }
  }
  {
    Tick _tick_("ff sqrt");
    for (int i = 0; i < kLoop/1000; ++i) {
      a[i] = a[i].sqrt();
    }
  }

  std::cout << d << "\n";
}


int main(int /*argc*/, char** /*argv*/) {
  InitEcc();
  libsnark::default_r1cs_gg_ppzksnark_pp::init_public_params();

  //init_random();
  //test_gmp_mul();
  //test_gmp_squared();
  //test_gmp_add();
  //test_gmp_del();
  //test_gmp_div();
  //test_gmp_mod();
  //test_gmp_mod512();
  //test_gmp_mulmod();
  test_mcl_fr_mul();
  test_ff_fr_mul_inv_sqrt();
  return 0;
}
