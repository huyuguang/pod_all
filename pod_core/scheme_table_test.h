#pragma once

#include <string>
#include <vector>

namespace scheme_misc::table {
bool Test(std::string const& publish_path, std::string const& key_name,
          std::vector<std::string> const& key_values);
}