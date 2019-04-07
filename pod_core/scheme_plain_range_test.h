#pragma once

#include <string>
#include "basic_types.h"

namespace scheme::plain::range {
bool Test(std::string const& publish_path, std::string const& output_path,
          Range const& demand, bool test_evil);
}  // namespace scheme::plain::range