#include "table_task.h"

#include "bp.h"
#include "bulletin.h"
#include "chain.h"
#include "csv_parser.h"
#include "csv_row.h"
#include "ecc.h"
#include "ecc_pub.h"
#include "key_meta.h"
#include "misc.h"
#include "mkl_tree.h"
#include "multiexp.h"
#include "public.h"
#include "task_misc.h"

namespace {
typedef std::vector<std::string> Record;
typedef std::vector<Record> Table;

uint64_t GetRecordSize(Record const& record) {
  uint64_t len = record.size() * sizeof(uint32_t);
  for (auto const& i : record) len += i.size();
  return len;
}

uint64_t GetMaxRecordSize(Table const& table) {
  uint64_t max_record_len = 0;
  for (auto const& i : table) {
    auto len = GetRecordSize(i);
    max_record_len = std::max(max_record_len, len);
  }
  return max_record_len;
}

h256_t HashVrfKey(std::string const& k, vrf::Sk<> const& vrf_sk) {
  CryptoPP::SHA256 hash;
  h256_t h_key;
  hash.Update((uint8_t*)k.data(), k.size());
  hash.Final(h_key.data());

  vrf::Fsk fsk = vrf::Vrf(vrf_sk, h_key.data());
  uint8_t fsk_bin[12 * 32];
  fsk.serialize(fsk_bin, sizeof(fsk_bin), mcl::IoMode::IoSerialize);

  h256_t h_fsk;
  hash.Update(fsk_bin, sizeof(fsk_bin));
  hash.Final(h_fsk.data());

  return h_fsk;
}

Fr GetPadFr(uint32_t len) {
  uint8_t bin[31];
  misc::RandomBytes(bin, sizeof(bin));
  len = boost::endian::native_to_big(len);
  memcpy(bin, &len, sizeof(len));
  return BinToFr31(bin, bin + sizeof(bin));
}

void RecordToBin(Record const& record, std::vector<uint8_t>& bin) {
  assert(bin.size() >= GetRecordSize(record));

  uint8_t* p = bin.data();
  for (auto& i : record) {
    uint32_t len = (uint32_t)i.size();
    len = boost::endian::native_to_big(len);
    memcpy(p, &len, sizeof(len));
    p += sizeof(len);
    memcpy(p, i.data(), i.size());
    p += i.size();
  }
  for (auto i = p; i < bin.data() + bin.size(); ++i) {
    *i = 0;
  }
}

// h(k1) h(k2) pad record
void DataToM(Table const& table, std::vector<uint64_t> columens_index,
             uint64_t s, vrf::Sk<> const& vrf_sk, std::vector<Fr>& m) {
  auto record_fr_num = s - 1 - columens_index.size();
  auto n = table.size();

  std::vector<uint8_t> bin(31 * record_fr_num);
  for (uint64_t i = 0; i < n; ++i) {
    auto const& record = table[i];
    auto record_size = GetRecordSize(record);
    auto offset = i * s;
    for (auto j : columens_index) {
      auto h = HashVrfKey(record[j], vrf_sk);
      m[offset++] = BinToFr31(h.data(), h.data() + 31);  // drop the last byte
    }

    m[offset++] = GetPadFr((uint32_t)record_size);

    RecordToBin(record, bin);
    for (uint64_t j = 0; j < record_fr_num; ++j) {
      uint8_t const* p = bin.data() + j * 31;
      m[offset++] = BinToFr31(p, p + 31);
    }
  }
}

void UniqueRecords(Table& table, std::vector<uint64_t> const& vrf_key_colnums) {
  auto unique = [&table](uint64_t pos) {
    std::map<std::string, size_t> count;
    for (auto& record : table) {
      auto& key = record[pos];
      auto& c = count[key];
      key += "_" + std::to_string(c++);
    }
  };

  for (auto i : vrf_key_colnums) {
    unique(i);
  }
}

bool LoadCSV(std::string const& file, std::vector<std::string>& col_names,
             Table& table) {
  using namespace csv;
  try {
    CSVReader reader(file);
    col_names = reader.get_col_names();

    for (CSVRow& row : reader) {  // Input iterator
      Record record;
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

std::vector<h256_t> BuildSigmaMklTree(std::vector<G1> const& sigmas) {
  auto get_sigma = [&sigmas](uint64_t i) -> h256_t {
    return G1ToBin(sigmas[i]);
  };
  return mkl::BuildTree(sigmas.size(), get_sigma);
}

std::vector<h256_t> BuildKeyMklTree(std::vector<Fr> const& m, uint64_t n,
                                    uint64_t s, uint64_t pos) {
  auto get_m = [&m, s, pos](uint64_t i) -> h256_t {
    h256_t h;
    FrToBin(m[i * s + pos], h.data());
    return h;
  };
  return mkl::BuildTree(n, get_m);
}

bool SaveVrfPk(std::string const& output, vrf::Pk<> const& pk) {
  const uint64_t kG2BufSize = 64;
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = pk.size() * kG2BufSize;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < pk.size(); ++i) {
      G2ToBin(pk[i], start + i * kG2BufSize);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveVrfSk(std::string const& output, vrf::Sk<> const& sk) {
  const uint64_t kFrBufSize = 32;
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = sk.size() * kFrBufSize;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < sk.size(); ++i) {
      FrToBin(sk[i], start + i * kFrBufSize);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveBpP1Proof(std::string const& output, bp::P1Proof const& proof) {
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = proof.GetBufSize();
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    if (!proof.serialize(start, params.new_file_size)) {
      assert(false);
      return false;
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

void BuildKeyBp(uint64_t n, uint64_t s, std::vector<Fr> const& m,
                std::vector<G1> const& sigmas, h256_t const& sigma_mkl_root,
                uint64_t key_pos, h256_t keycol_mkl_root,
                bp::P1Proof& p1_proof) {
  assert(sigmas.size() == n);
  assert(m.size() == n * s);

  auto& ecc_pub = GetEccPub();
  auto bp_count = s - 1;

  uint8_t seed[64];
  memcpy(seed, sigma_mkl_root.data(), sigma_mkl_root.size());
  memcpy(seed + 32, keycol_mkl_root.data(), keycol_mkl_root.size());
  std::vector<Fr> v(n);
  for (uint64_t i = 0; i < n; ++i) {
    v[i] = Chain(seed, sizeof(seed), i);
  }

  std::vector<Fr> mv(bp_count);
  for (uint64_t j = 0; j < s; ++j) {
    if (j == key_pos) continue;
    auto jj = j < key_pos ? j : j - 1;
    mv[jj] = FrZero();
    for (uint64_t i = 0; i < n; ++i) {
      mv[jj] += v[i] * m[i * s + j];
    }
  }

  Fr f0 = FrZero();
  auto get_f = [&mv, &f0, bp_count](uint64_t j) -> Fr const& {
    if (j < bp_count)
      return mv[j];
    else
      return f0;
  };

  G1 g0 = G1Zero();
  auto get_g = [&ecc_pub, &g0, key_pos, bp_count](uint64_t j) -> G1 const& {
    if (j < bp_count) {
      auto const& u = ecc_pub.u1();
      if (j < key_pos) {
        return u[j];
      } else {
        return u[j + 1];
      }
    } else {
      return g0;
    }
  };

  p1_proof = bp::P1Prove(get_g, get_f, bp_count);

  // NOTE: just check (do not need in release)
  std::vector<G1> sigmas2(n);
  for (uint64_t i = 0; i < n; ++i) {
    Fr const& mi_key = m[i * s + key_pos];
    G1 u_exp_mi_key = ecc_pub.PowerU1(key_pos, mi_key);
    sigmas2[i] = sigmas[i] - u_exp_mi_key;
  }
  if (MultiExpBdlo12(sigmas2, v) != p1_proof.committment.p) {
    assert(false);
    throw std::runtime_error("Oops! P1Commit failed!");
  }
  if (!P1Verify(p1_proof, get_g, bp_count)) {
    throw std::runtime_error("Oops! P1Verify failed!");
  }
}
}  // namespace

TableTask::TableTask(std::string publish_file, std::string output_path,
                     TableType table_type,
                     std::vector<uint64_t> vrf_colnums_index)
    : publish_file_(std::move(publish_file)),
      output_path_(std::move(output_path)),
      table_type_(std::move(table_type)),
      vrf_colnums_index_(std::move(vrf_colnums_index)) {
  vrf::Generate<>(vrf_pk_, vrf_sk_);
}

bool TableTask::Execute() {
  auto& ecc_pub = GetEccPub();
  bulletin::Table bulletin;
  key_meta::Vrf vrf_meta;
  std::string data_file = output_path_ + "/data";
  std::string matrix_file = output_path_ + "/matrix";
  std::string bulletin_file = output_path_ + "/bulletin";
  std::string sigma_file = output_path_ + "/sigma";
  std::string sigma_mkl_tree_file = output_path_ + "/sigma_mkl_tree";
  std::string vrf_pk_file = output_path_ + "/vrf_pk";
  std::string vrf_sk_file = output_path_ + "/vrf_sk";
  std::string key_meta_file = output_path_ + "/key_meta";
  std::vector<std::string> key_mkl_tree_files(vrf_colnums_index_.size());
  for (size_t i = 0; i < key_mkl_tree_files.size(); ++i) {
    key_mkl_tree_files[i] = output_path_ + "/key_mkl_tree_" + std::to_string(i);
  }
  std::vector<std::string> key_bp_files(vrf_colnums_index_.size());
  for (size_t i = 0; i < key_bp_files.size(); ++i) {
    key_bp_files[i] = output_path_ + "/key_bp_" + std::to_string(i);
  }

  if (!CopyData(publish_file_, data_file)) {
    assert(false);
    return false;
  }

  Table table;
  if (!LoadCSV(data_file, vrf_meta.column_names, table)) {
    assert(false);
    return false;
  }

  vrf_meta.keys.resize(vrf_colnums_index_.size());
  for (uint64_t i = 0; i < vrf_colnums_index_.size(); ++i) {
    vrf_meta.keys[i].column_index = vrf_colnums_index_[i];
    vrf_meta.keys[i].j = i;
  }

  UniqueRecords(table, vrf_colnums_index_);

  n_ = table.size();
  auto max_record_size = GetMaxRecordSize(table);
  auto record_fr_num = (max_record_size + 30) / 31;
  s_ = vrf_colnums_index_.size() + 1 + record_fr_num;
  auto max_s = ecc_pub.u1().size();
  if (s_ > max_s) {
    assert(false);
    return false;
  }

  bulletin.n = n_;
  bulletin.s = s_;

  std::vector<Fr> m(n_ * s_);
  DataToM(table, vrf_colnums_index_, s_, vrf_sk_, m);

  if (!SaveMatrix(matrix_file, m)) {
    assert(false);
    return false;
  }

  // sigma
  std::vector<G1> sigmas = CalcSigma(m, n_, s_);
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
  if (!SaveVrfPk(vrf_pk_file, vrf_pk_)) {
    assert(false);
    return false;
  }

  if (!GetFileSha256(vrf_pk_file, vrf_meta.pk_hash)) {
    assert(false);
    return false;
  }

  // vrf sk
  if (!SaveVrfSk(vrf_sk_file, vrf_sk_)) {
    assert(false);
    return false;
  }

  // key mkl
  for (size_t i = 0; i < vrf_colnums_index_.size(); ++i) {
    auto keycol_mkl_tree = BuildKeyMklTree(m, n_, s_, i);
    vrf_meta.keys[i].mj_mkl_root = keycol_mkl_tree.back();
    if (!SaveMkl(key_mkl_tree_files[i], keycol_mkl_tree)) {
      assert(false);
      return false;
    }
  }

  // bp about relation about mi_key with sigma_i
  std::vector<h256_t> key_bp_hashs(vrf_colnums_index_.size());
  for (size_t i = 0; i < vrf_colnums_index_.size(); ++i) {
    auto& key = vrf_meta.keys[i];
    bp::P1Proof bp_p1_proof;
    BuildKeyBp(n_, s_, m, sigmas, bulletin.sigma_mkl_root, key.column_index,
               key.mj_mkl_root, bp_p1_proof);
    if (!SaveBpP1Proof(key_bp_files[i], bp_p1_proof)) {
      assert(false);
      return false;
    }
    if (!GetFileSha256(key_bp_files[i], key.bp_hash)) {
      assert(false);
      return false;
    }
  }

  if (!SaveVrf(key_meta_file, vrf_meta)) {
    assert(false);
    return false;
  }

  if (!GetFileSha256(key_meta_file, bulletin.key_meta_hash)) {
    assert(false);
    return false;
  }

  if (!bulletin::Save(bulletin_file, bulletin)) {
    assert(false);
    return false;
  }

  return true;
}
