#pragma once

#include <string>
#include "basic_types.h"

namespace scheme::table::batch2 {
bool Test(std::string const& publish_path, std::string const& output_file,
          std::vector<Range> const& demands);
}  // namespace scheme::table::batch