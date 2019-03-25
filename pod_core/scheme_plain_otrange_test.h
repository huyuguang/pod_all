#pragma once

#include <string>
#include "basic_types.h"

namespace scheme::plain::otrange {
bool Test(std::string const& publish_path, std::string const& output_path,
          Range const& demand, Range const& phantom);
}  // namespace scheme::plain::otrange