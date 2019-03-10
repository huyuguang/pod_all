#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "vrf.h"

enum TableType {
  kCsv,  // now only support csv format
};

inline std::istream& operator>>(std::istream& in, TableType& t) {
  std::string token;
  in >> token;
  if (token == "csv") {
    t = TableType::kCsv;
  } else {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

inline std::ostream& operator<<(std::ostream& os, TableType const& t) {
  if (t == TableType::kCsv) {
    os << "csv";
  } else {
    os.setstate(std::ios_base::failbit);
  }
  return os;
}

class TableTask {
 public:
  TableTask(std::string publish_file, std::string output_path,
            TableType table_type, std::vector<uint64_t> vrf_colnums_index);
  bool Execute();

 private:
  std::string const publish_file_;
  std::string const output_path_;
  TableType const table_type_;
  std::vector<uint64_t> const vrf_colnums_index_;
  uint64_t n_;
  uint64_t s_ = 0;
  vrf::Pk<> vrf_pk_;
  vrf::Sk<> vrf_sk_;
};
