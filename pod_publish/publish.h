#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "scheme_misc.h"
#include "scheme_table.h"
#include "scheme_plain.h"

bool PublishTable(std::string publish_file, std::string output_path,
                     scheme_misc::table::Type table_type,
                     std::vector<uint64_t> vrf_colnums_index);

bool PublishPlain(std::string publish_file, std::string output_path,
                  uint64_t column_num);