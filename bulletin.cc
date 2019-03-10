#include "bulletin.h"

#include "ecc.h"
#include "ecc_pub.h"
#include "misc.h"
#include "public.h"

namespace {
void HexStrToH256(std::string const& str, h256_t& h) {
  if (str.size() != 32 * 2) throw std::invalid_argument("");
  misc::StrToHex(str.c_str(), str.size(), h.data());
}
}  // namespace

namespace bulletin {
bool IsValid(Plain const& data) {
  auto const& ecc_pub = GetEccPub();
  return data.size && data.s > 0 && data.s <= ecc_pub.u1().size();
}

bool IsValid(Table const& data) {
  auto const& ecc_pub = GetEccPub();
  return data.n && data.s > 0 && data.s <= ecc_pub.u1().size();
}

bool Save(std::string const& output, Table const& data) {
  if (!IsValid(data)) {
    assert(false);
    return false;
  }

  try {
    pt::ptree tree;
    tree.put("n", data.n);
    tree.put("s", data.s);
    tree.put("sigma_mkl_root", misc::HexToStr(data.sigma_mkl_root));
    tree.put("key_meta_hash", misc::HexToStr(data.key_meta_hash));
    pt::write_json(output, tree);
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool Load(std::string const& input, Table& data) {
  try {
    pt::ptree tree;
    pt::read_json(input, tree);
    data.n = tree.get<uint64_t>("n");
    data.s = tree.get<uint64_t>("s");
    HexStrToH256(tree.get<std::string>("sigma_mkl_root"), data.sigma_mkl_root);
    HexStrToH256(tree.get<std::string>("key_meta_hash"), data.key_meta_hash);
    return IsValid(data);
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool Save(std::string const& output, Plain const& data) {
  try {
    pt::ptree tree;
    tree.put("size", data.size);
    tree.put("s", data.s);
    tree.put("sigma_mkl_root", misc::HexToStr(data.sigma_mkl_root));
    pt::write_json(output, tree);
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool Load(std::string const& input, Plain& data) {
  try {
    pt::ptree tree;
    pt::read_json(input, tree);
    data.size = tree.get<uint64_t>("size");
    data.s = tree.get<uint64_t>("s");
    HexStrToH256(tree.get<std::string>("sigma_mkl_root"), data.sigma_mkl_root);
    return IsValid(data);
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}
}  // namespace bulletin