#pragma once

#include <stdint.h>
#include <string>

#include "basic_types.h"
#include "bulletin_plain.h"
#include "mkl_tree.h"
#include "scheme_misc.h"

namespace scheme_misc::plain {

class B {
 public:
  B(Bulletin const& bulletin, std::string const& public_path);
  B(std::string const& bulletin_file, std::string const& public_path);
  Bulletin const& bulletin() const { return bulletin_; }
  std::vector<G1> sigmas() const { return sigmas_; }

 private:
  void LoadData();
  bool NeedVerify();

 private:
  Bulletin bulletin_;
  std::string public_path_;

 private:
  std::vector<G1> sigmas_;
};

}  // namespace scheme_misc::plain