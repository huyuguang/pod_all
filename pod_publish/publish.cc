#include "publish.h"

#include "bp.h"
#include "bulletin_plain.h"
#include "bulletin_table.h"
#include "chain.h"
#include "csv_parser.h"
#include "csv_row.h"
#include "ecc.h"
#include "ecc_pub.h"
#include "misc.h"
#include "mkl_tree.h"
#include "multiexp.h"
#include "public.h"
#include "scheme_misc.h"
#include "vrf_meta.h"

namespace {

using namespace scheme_misc;
bool LoadCsvTable(std::string const& file, std::vector<std::string>& col_names,
                  table::Table& table) {
  using namespace csv;
  try {
    CSVReader reader(file);
    col_names = reader.get_col_names();

    for (CSVRow& row : reader) {  // Input iterator
      table::Record record;
      for (CSVField& field : row) {
        record.push_back(std::string(field.get<>()));
      }
      assert(record.size() == col_names.size());
      table.emplace_back(std::move(record));
    }
    reader.close();
    return true;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool LoadTable(std::string const& file, table::Type table_type,
               std::vector<std::string>& col_names, table::Table& table) {
  if (table_type != table::Type::kCsv) return false;  // not support yet
  return LoadCsvTable(file, col_names, table);
}

void AddRubbishTail(table::Table& table,std::vector<uint64_t> key_index) {
  // make sure the rubbish do not has same value
}
}  // namespace

bool PublishTable(std::string publish_file, std::string output_path,
                  scheme_misc::table::Type table_type,
                  std::vector<uint64_t> vrf_colnums_index) {
  using namespace scheme_misc;
  using namespace scheme_misc::table;
  using namespace misc;

  auto& ecc_pub = GetEccPub();
  boost::system::error_code err;
  std::string public_path = output_path + "/public";
  if (!fs::is_directory(public_path, err) &&
      !fs::create_directories(public_path, err)) {
    assert(false);
    return false;
  }
  std::string private_path = output_path + "/private";
  if (!fs::is_directory(private_path, err) &&
      !fs::create_directories(private_path, err)) {
    assert(false);
    return false;
  }

  std::string bulletin_file = output_path + "/bulletin";
  std::string data_file = private_path + "/data";
  std::string matrix_file = private_path + "/matrix";
  std::string sigma_file = public_path + "/sigma";
  std::string sigma_mkl_tree_file = public_path + "/sigma_mkl_tree";
  std::string vrf_pk_file = public_path + "/vrf_pk";
  std::string vrf_sk_file = private_path + "/vrf_sk";
  std::string vrf_meta_file = public_path + "/vrf_meta";
  std::vector<std::string> key_bp_files(vrf_colnums_index.size());
  std::vector<std::string> key_m_files(vrf_colnums_index.size());
  for (size_t i = 0; i < key_bp_files.size(); ++i) {
    std::string str_i = std::to_string(i);
    key_bp_files[i] = public_path + "/key_bp_" + str_i;
    key_m_files[i] = public_path + "/key_m_" + str_i;
  }

  if (!CopyData(publish_file, data_file)) {
    assert(false);
    return false;
  }

  vrf::Pk<> vrf_pk;
  vrf::Sk<> vrf_sk;

  vrf::Generate<>(vrf_pk, vrf_sk);

  Table table;
  VrfMeta vrf_meta;
  if (!LoadTable(data_file, table_type, vrf_meta.column_names, table)) {
    assert(false);
    return false;
  }

  vrf_meta.keys.resize(vrf_colnums_index.size());
  for (uint64_t i = 0; i < vrf_colnums_index.size(); ++i) {
    vrf_meta.keys[i].column_index = vrf_colnums_index[i];
    vrf_meta.keys[i].j = i;
  }

  UniqueRecords(table, vrf_colnums_index);

  Bulletin bulletin;
  bulletin.n = table.size();
  auto max_record_size = GetMaxRecordSize(table);
  auto record_fr_num = (max_record_size + 30) / 31;
  bulletin.s = vrf_colnums_index.size() + 1 + record_fr_num;
  auto max_s = ecc_pub.u1().size();
  if (bulletin.s > max_s) {
    assert(false);
    return false;
  }

  std::vector<Fr> m(bulletin.n * bulletin.s);
  DataToM(table, vrf_colnums_index, bulletin.s, vrf_sk, m);

  if (!SaveMatrix(matrix_file, m)) {
    assert(false);
    return false;
  }

  // sigma
  std::vector<G1> sigmas = CalcSigma(m, bulletin.n, bulletin.s);
  if (!SaveSigma(sigma_file, sigmas)) {
    assert(false);
    return false;
  }

  // build sigma mkl tree
  auto sigma_mkl_tree = BuildSigmaMklTree(sigmas);
  if (!SaveMkl(sigma_mkl_tree_file, sigma_mkl_tree)) {
    assert(false);
    return false;
  }
  bulletin.sigma_mkl_root = sigma_mkl_tree.back();

  // vrf pk
  if (!SaveVrfPk(vrf_pk_file, vrf_pk)) {
    assert(false);
    return false;
  }

  if (!GetFileSha256(vrf_pk_file, vrf_meta.pk_digest)) {
    assert(false);
    return false;
  }

  // vrf sk
  if (!SaveVrfSk(vrf_sk_file, vrf_sk)) {
    assert(false);
    return false;
  }

  // key and mkl_root
  for (size_t j = 0; j < vrf_colnums_index.size(); ++j) {
    std::vector<Fr> km(bulletin.n);
    for (size_t i = 0; i < bulletin.n; ++i) {
      km[i] = m[i * bulletin.s + j];
    }
    // save key_m_files
    if (!SaveMatrix(key_m_files[j], km)) {
      assert(false);
      return false;
    }

    auto get_item = [&km](uint64_t i) -> h256_t {
      if (i < km.size()) {
        h256_t h;
        FrToBin(km[i], h.data());
        return h;
      } else {
        return h256_t();
      }
    };
    vrf_meta.keys[j].mj_mkl_root = mkl::CalcRoot(get_item, bulletin.n);
  }

  // key bp proof: bp about relation about mi_key with sigma_i
  for (size_t i = 0; i < vrf_colnums_index.size(); ++i) {
    auto& key = vrf_meta.keys[i];
    bp::P1Proof bp_p1_proof;
    BuildKeyBp(bulletin.n, bulletin.s, m, sigmas, bulletin.sigma_mkl_root,
               key.column_index, key.mj_mkl_root, bp_p1_proof);
    if (!SaveBpP1Proof(key_bp_files[i], bp_p1_proof)) {
      assert(false);
      return false;
    }
    if (!GetFileSha256(key_bp_files[i], key.bp_digest)) {
      assert(false);
      return false;
    }
  }

  if (!SaveVrfMeta(vrf_meta_file, vrf_meta)) {
    assert(false);
    return false;
  }

  if (!GetFileSha256(vrf_meta_file, bulletin.vrf_meta_digest)) {
    assert(false);
    return false;
  }

  if (!SaveBulletin(bulletin_file, bulletin)) {
    assert(false);
    return false;
  }

  return true;
}

bool PublishPlain(std::string publish_file, std::string output_path,
                  uint64_t column_num) {
  using namespace scheme_misc;
  using namespace scheme_misc::plain;
  using namespace misc;

  auto& ecc_pub = GetEccPub();
  auto max_s = ecc_pub.u1().size();
  if (column_num > max_s) {
    std::cerr << "column_num too large! The upper bound is " << max_s
              << std::endl;
    return false;
  }

  Bulletin bulletin;
  bulletin.size = fs::file_size(publish_file);
  if (!bulletin.size) return false;
  bulletin.s = column_num + 1;
  uint64_t n = GetDataBlockCount(bulletin.size, column_num);

  std::string data_file = output_path + "/data";
  std::string matrix_file = output_path + "/matrix";
  std::string bulletin_file = output_path + "/bulletin";
  std::string sigma_file = output_path + "/sigma";
  std::string sigma_mkl_file = output_path + "/sigma_mkl";

  if (!CopyData(publish_file, data_file)) {
    assert(false);
    return false;
  }

  std::vector<Fr> m;
  if (!DataToM(data_file, bulletin.size, n, column_num, m)) {
    assert(false);
    return false;
  }

  if (!SaveMatrix(matrix_file, m)) {
    assert(false);
    return false;
  }

  std::vector<G1> sigmas = CalcSigma(m, n, bulletin.s);

  if (!SaveSigma(sigma_file, sigmas)) {
    assert(false);
    return false;
  }

  // mkl
  auto sigma_mkl_tree = BuildSigmaMklTree(sigmas);
  if (!SaveMkl(sigma_mkl_file, sigma_mkl_tree)) {
    assert(false);
    return false;
  }
  bulletin.sigma_mkl_root = sigma_mkl_tree.back();

  // meta
  if (!SaveBulletin(bulletin_file, bulletin)) {
    assert(false);
    return false;
  }

#ifdef _DEBUG
  std::string debug_data_file = data_file + ".debug";
  if (!MToFile(debug_data_file, bulletin.size, bulletin.s, 0, n, m)) {
    assert(false);
    return false;
  }

  if (!IsSameFile(debug_data_file, data_file)) {
    assert(false);
    return false;
  }
  fs::remove(debug_data_file);
#endif
  return true;
}