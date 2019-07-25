#include <assert.h>
#include <cryptopp/osrng.h>
#include <cryptopp/randpool.h>
#include <iostream>
#include "../pod_core/zkp_key.h"
#include "ecc.h"
#include "ecc_pub.h"
#include "misc.h"
#include "tick.h"
#include "zkp.h"
#include "public.h"

#include "../pod_core/scheme_atomic_swap_serialize.h"
#include "../pod_core/scheme_atomic_swap_vc_misc.h"

bool InitAll(std::string const& data_dir) {
  InitEcc();

#ifdef _DEBUG
  InitZkp(false);
#else
  InitZkp(true);
#endif

  auto ecc_pub_file = data_dir + "/" + "ecc_pub.bin";

  if (!OpenOrCreateEccPub(ecc_pub_file)) {
    std::cerr << "Open or create ecc pub file " << ecc_pub_file << " failed\n";
    return false;
  }

  std::string zkp_key_dir = data_dir + "/" + "zksnark_key";
  if (zkp_key_dir.empty() || !fs::is_directory(zkp_key_dir)) {
    std::cerr << "Open zkp_key_dir " << zkp_key_dir << " failed\n";
    return false;
  }

  ZkpKey::instance(zkp_key_dir);

  if (ZkpKey::instance().IsEmpty()) {
    std::cerr << "Warning: zk key file not exist.\n";
  }

  return true;
}

bool VerifyZkProof(ZkProof const& proof, ZkVk const& vk,
                   std::vector<uint64_t> const& public_o,
                   std::vector<Fr> const& public_w, Fr seed_mimc3_digest,
                   Fr ip_vw) {
  using namespace libsnark;
  auto const kCount = ZkpMimcCount();
  assert(public_o.size() == kCount);
  assert(public_w.size() == kCount);

  // Create protoboard
  protoboard<ZkFr> pb;

  // Define variables
  pb_variable<ZkFr> digest;
  pb_variable<ZkFr> result;
  pb_variable_array<ZkFr> o;
  pb_variable_array<ZkFr> w;

  // Allocate variables to protoboard
  o.allocate(pb, kCount, "o");    // public
  w.allocate(pb, kCount, "w");    // public
  digest.allocate(pb, "digest");  // public
  result.allocate(pb, "result");  // public

  // This sets up the protoboard variables
  // so that the first one (out) represents the public
  // input and the rest is private input
  pb.set_input_sizes(kCount * 2 + 2);

  //// Add R1CS constraints to protoboard
  // auto mimc3_const = ConvertToZkFr(Mimc3Const());
  // auto mimcinv_const = ConvertToZkFr(MimcInvConst());
  // AtomicSwapVcGadget<ZkFr> g(pb, mimc3_const, mimcinv_const, seed,
  // seed_rand,
  //                           digest, result, o, w);
  // g.generate_r1cs_constraints();

  // public statement
  for (size_t i = 0; i < kCount; ++i) {
    pb.val(o[i]) = public_o[i];
    pb.val(w[i]) = ConvertToZkFr(public_w[i]);
  }
  pb.val(digest) = ConvertToZkFr(seed_mimc3_digest);
  pb.val(result) = ConvertToZkFr(ip_vw);

  // Verify proof
  bool verified =
      r1cs_gg_ppzksnark_verifier_strong_IC<default_r1cs_gg_ppzksnark_pp>(
          vk, pb.primary_input(), proof);
  assert(verified);
  return verified;
}

void CheckProof(std::string const& file) {
  ZkProof proof;
  std::vector<uint64_t> public_offset;
  std::vector<Fr> public_w;
  Fr seed_mimc3_digest;
  Fr ip_vw;

  yas::file_istream is(file.c_str());
  yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
  ia.serialize(proof);
  ia.serialize(public_offset);
  ia.serialize(public_w);
  ia.serialize(seed_mimc3_digest);
  ia.serialize(ip_vw);

  auto const& vk = *ZkpKey::instance().GetZkVk("atomic_swap_vc");

  bool ret = VerifyZkProof(proof, vk, public_offset, public_w,
                           seed_mimc3_digest, ip_vw);
  assert(ret);
  std::cout << "VerifyZkProof: " << file << " " << (ret ? "true" : "false")
            << "\n";
}

int main(int /*argc*/, char** /*argv*/) {
  InitAll(".");

  auto range = boost::make_iterator_range(fs::directory_iterator("./temp"), {});
  for (auto& entry : range) {
    auto fullpath = entry.path().string();
    CheckProof(fullpath);
  }

  return 0;
}
