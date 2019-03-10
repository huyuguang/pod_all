#pragma once

#include <stdint.h>
#include <algorithm>
#include <functional>
#include <memory>

#include "ecc.h"
#include "ecc_pub.h"
#include "tick.h"

// g1 is generator of G1, g2 is generator of G2
// the g in paper is g1, the u in paper is g2
// pk = (g1, g2, u2, t1,...,tn), t1,...tn = g2^si
// sk = (g1, g2, u2, s1,...,sn), s1,...sn are random Fr
// F(sk,x) = e(g1^1/II(x[i]+sk[i]),u2)
// Pie(sk,x) = {g1^1/II(x[j]+sk[j])}
// Verify(pk,x,fsk,Pie):
//    e(pie[i], g2^x[i]*t[i]) == e(pie[i-1], g2)
//    e(pie[n], u2) == fsk


namespace vrf {

namespace detail {
  inline G2 GetU() {
    auto& ecc_pub = GetEccPub();
    return ecc_pub.u2()[1];
  }
}  // namespace detail

template <size_t N = 32>
using Pk = std::array<G2, N>;

template <size_t N = 32>
using Sk = std::array<Fr, N>;

template <size_t N = 32>
using Psk = std::array<G1, N>;

using Fsk = Fp12;

template <size_t N = 32>
void Generate(Pk<N>& pk, Sk<N>& sk) {
  Tick tick(__FUNCTION__);
  auto& ecc_pub = GetEccPub();
  for (size_t i = 0; i < N; ++i) {
    sk[i] = FrRand();
    pk[i] = ecc_pub.PowerG2(sk[i]);
  }
}

template <size_t N = 32>
Fsk Vrf(Sk<N> const& sk, uint8_t const* x) {
  Tick tick(__FUNCTION__);
  auto& ecc_pub = GetEccPub();
  Fr a = FrOne();
  for (size_t i = 0; i < N; ++i) {
    a *= (sk[i] + x[i]);
  }
  a = FrInv(a);
  G1 ga = ecc_pub.PowerG1(a);

  Fp12 e;
  mcl::bn256::pairing(e, ga, detail::GetU());
  return e;
}

template <size_t N = 32>
void Prove(Sk<N> const& sk, uint8_t const* x, Psk<N>& psk) {
  Tick tick(__FUNCTION__);
  auto& ecc_pub = GetEccPub();
  std::array<Fr, N> a;
  a[0] = sk[0] + x[0];
  for (size_t i = 1; i < N; ++i) {
    a[i] = a[i - 1] * (sk[i] + x[i]);
  }
  for (auto& i : a) {
    i = FrInv(i);
  }
  for (size_t i = 0; i < N; ++i) {
    psk[i] = ecc_pub.PowerG1(a[i]);
  }
}

template <size_t N = 32>
bool Verify(Pk<N> const& pk, uint8_t const* x, Fsk const& fsk,
            Psk<N> const& psk) {
  Tick tick(__FUNCTION__);
  auto& ecc_pub = GetEccPub();
  auto g1 = G1One();

  bool ret = PairingMatch(g1, psk[0], ecc_pub.PowerG2(x[0]) + pk[0]);
  if (!ret) {
    assert(false);
    return false;
  }

  for (size_t i = 1; i < N; ++i) {
    ret = PairingMatch(psk[i - 1], psk[i], ecc_pub.PowerG2(x[i]) + pk[i]);
    if (!ret) {
      assert(false);
      return false;
    }
  }

  Fp12 e;
  mcl::bn256::pairing(e, psk.back(), detail::GetU());

  ret = e == fsk;
  assert(ret);

  return ret;
}

inline bool Test() {
  Pk<> pk;
  Sk<> sk;
  Generate<>(pk, sk);
  std::array<uint8_t, 32> x;
  for (auto& i : x) {
    i = (uint8_t)rand();
  }

  Fsk fsk = Vrf<>(sk, x.data());

  Psk<> psk;
  Prove(sk, x.data(), psk);

  bool ret = Verify<>(pk, x.data(), fsk, psk);
  assert(ret);

  return ret;
}
}  // namespace vrf