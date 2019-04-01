#pragma once

#include <string>
#include "basic_types.h"

namespace scheme::table::otbatch {
bool Test(std::string const& publish_path, std::string const& output_file,
          std::vector<Range> const& demands,
          std::vector<Range> const& phantoms);
}  // namespace scheme::table::otbatch