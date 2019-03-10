#include "key_meta.h"

#include "ecc.h"
#include "misc.h"
#include "public.h"

namespace {
void HexStrToH256(std::string const& str, h256_t& h) {
  if (str.size() != 32 * 2) throw std::invalid_argument("");
  misc::StrToHex(str.c_str(), str.size(), h.data());
}
}  // namespace

namespace key_meta {
bool SaveVrf(std::string const& output, Vrf const& data) {
  if (!data.valid()) {
    assert(false);
    return false;
  }

  try {
    pt::ptree tree;
    tree.put("pk_hash", misc::HexToStr(data.pk_hash));

    pt::ptree names_node;
    for (auto const& i : data.column_names) {
      pt::ptree name_node;
      name_node.put("", i);
      names_node.push_back(std::make_pair("", name_node));
    }
    tree.add_child("column_names", names_node);

    pt::ptree vrf_keys_node;
    for (auto const& i : data.keys) {
      pt::ptree vrf_key_node;
      vrf_key_node.put("j", i.j);
      vrf_key_node.put("column_index", i.column_index);
      vrf_key_node.put("mj_mkl_root", misc::HexToStr(i.mj_mkl_root));
      vrf_key_node.put("bp_hash", misc::HexToStr(i.bp_hash));
      vrf_keys_node.push_back(std::make_pair("", vrf_key_node));
    }
    tree.add_child("keys", vrf_keys_node);
    pt::write_json(output, tree);

    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadVrf(std::string const& input, Vrf& data) {
  try {
    pt::ptree tree;
    std::string str;
    pt::read_json(input, tree);

    HexStrToH256(tree.get<std::string>("pk_hash"), data.pk_hash);

    for (pt::ptree::value_type& name_key : tree.get_child("column_names")) {
      data.column_names.push_back(name_key.second.data());
    }

    for (pt::ptree::value_type& pt_key : tree.get_child("keys")) {
      VrfKey vrf_key;
      vrf_key.j = pt_key.second.get<uint32_t>("j");
      vrf_key.column_index = pt_key.second.get<uint32_t>("column_index");
      str = pt_key.second.get<std::string>("mj_mkl_root");
      HexStrToH256(str, vrf_key.mj_mkl_root);
      str = pt_key.second.get<std::string>("bp_hash");
      HexStrToH256(str, vrf_key.bp_hash);
      data.keys.push_back(vrf_key);
    }
    return data.valid();
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

}  // namespace key_meta