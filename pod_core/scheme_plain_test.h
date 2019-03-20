#pragma once

#include <string>


namespace scheme_misc::plain {

bool Test(std::string const& publish_path, std::string const& output_path,
          uint64_t start, uint64_t count);
}  // namespace scheme_misc::plain