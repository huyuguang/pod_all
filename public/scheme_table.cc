#include "scheme_table.h"
#include "misc.h"
#include "public.h"
#include "scheme_misc.h"

namespace scheme::table {

std::istream& operator>>(std::istream& in, Type& t) {
  std::string token;
  in >> token;
  if (token == "csv") {
    t = Type::kCsv;
  } else {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

std::ostream& operator<<(std::ostream& os, Type const& t) {
  if (t == Type::kCsv) {
    os << "csv";
  } else {
    os.setstate(std::ios_base::failbit);
  }
  return os;
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

// hash(fsk(sk, hash(key)))
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

bool BinToRecord(std::vector<uint8_t> const& bin, Record& record) {
  uint8_t const* p = bin.data();
  size_t left_len = bin.size();
  for (;;) {
    uint32_t item_len;
    if (left_len < sizeof(item_len)) return false;    
    memcpy(&item_len, p, sizeof(item_len));
    item_len = boost::endian::big_to_native(item_len);
    if (item_len > left_len) return false;
    p += sizeof(item_len);
    left_len -= sizeof(item_len);    
    record.resize(record.size() + 1);
    auto& r = record.back();
    r.assign((char*)p, item_len);
    p += item_len;
    left_len -= item_len;
    if (left_len == 0) break;
  }
  return true;
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

VrfKeyMeta const* GetKeyMetaByName(VrfMeta const& vrf_meta,
                                   std::string const& name) {
  auto const& names = vrf_meta.column_names;
  auto it_name = std::find(names.begin(), names.end(), name);
  if (it_name == names.end()) return nullptr;
  auto col_index = std::distance(names.begin(), it_name);

  for (uint64_t i = 0; i < vrf_meta.keys.size(); ++i) {
    auto const& vrf_key = vrf_meta.keys[i];
    if (vrf_meta.keys[i].column_index == (uint64_t)col_index) return &vrf_key;
  }
  return nullptr;
}

}  // namespace scheme::table
