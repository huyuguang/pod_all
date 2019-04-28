#include <assert.h>
#include <cryptopp/osrng.h>
#include <cryptopp/randpool.h>
#include <iostream>
#include "tick.h"
//
//constexpr size_t kBlockSize = 1024 * 1024;
//std::vector<uint8_t> data(32 * kBlockSize);
//
//void test1() {
//  Tick tick(__FUNCTION__);
////#pragma omp parallel for
//  for (int i = 0; i < 32; ++i) {
//    CryptoPP::AutoSeededRandomPool rng;
//    rng.GenerateBlock(data.data() + i * kBlockSize, kBlockSize);
//  }
//}
//
//void test2() {
//  Tick tick(__FUNCTION__);
////#pragma omp parallel for
//  for (int i = 0; i < 32; ++i) {
//    CryptoPP::AutoSeededX917RNG<CryptoPP::AES> rng;
//    rng.GenerateBlock(data.data() + i * kBlockSize, kBlockSize);
//  }
//}
//
//void test3() {
//  Tick tick(__FUNCTION__);
////#pragma omp parallel for
//  for (int i = 0; i < 32; ++i) {
//    CryptoPP::NonblockingRng rng;
//    rng.GenerateBlock(data.data() + i * kBlockSize, kBlockSize);
//  }
//}
//
//namespace A {
//void f(int) { std::cout << "A\n"; }
//}
//
//namespace B {
//void f(char) { std::cout << "A\n"; }
//}

int main(int /*argc*/, char** /*argv*/) {
  //test1();
  //test2();
  //test3();
  return 0;
}
