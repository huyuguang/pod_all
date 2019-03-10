#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "ecc.h"

class PlainTask {
 public:
  PlainTask(std::string publish_file, std::string output_path,
            uint64_t column_num);
  bool Execute();

 private:
  std::string const publish_file_;
  std::string const output_path_;
  uint64_t file_size_;
  uint64_t n_;
  uint64_t const s_;
};
