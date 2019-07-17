#include <assert.h>
#include <cryptopp/osrng.h>
#include <cryptopp/randpool.h>
#include <iostream>
#include "misc.h"
#include "tick.h"
#include "ecc.h"
#include "ecc_pub.h"

std::vector<Fr> fr;
std::vector<Fr> fr2;
std::vector<Fr> fr3;

void Test1() {    
  Tick _tick_(__FUNCTION__);
  FrInv(fr2);
}

void Test2() {
  Tick _tick_(__FUNCTION__);
  for (auto& i : fr3) {
    i = FrInv(i);
  }
}

int main(int /*argc*/, char** /*argv*/) {
  InitEcc();

  size_t count = 1000000;
  fr.resize(count);
  for (auto& i : fr) i = FrRand();

  fr2 = fr;
  fr3 = fr;
  Test1();
  Test2();
  if (fr2 != fr3) {
    for (auto& i : fr2) {
      std::cout << i << "\n";
    }
    std::cout << "\n";
    for (auto& i : fr3) {
      std::cout << i << "\n";
    }
    std::cout << "oops\n";
  } else {
    std::cout << "ok\n";
  }
  return 0;
}
