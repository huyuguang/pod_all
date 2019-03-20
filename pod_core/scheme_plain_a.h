#pragma once

#include <stdint.h>
#include <memory>
#include <string>

#include "basic_types.h"
#include "bp.h"
#include "bulletin_plain.h"
#include "mkl_tree.h"
#include "scheme_misc.h"

namespace scheme_misc::plain {
class A {
 public:
  A(std::string const& publish_path);
  Bulletin const& bulletin() const { return bulletin_; }
  std::vector<G1> const& sigmas() const { return sigmas_; }
  std::vector<Fr> const& m() const { return m_; }

 private:
  std::string const publish_path_;
  scheme_misc::plain::Bulletin bulletin_;
  std::vector<G1> sigmas_;
  mkl::Tree sigma_mkl_tree_;
  std::vector<Fr> m_;  // secret
};

typedef std::shared_ptr<A> APtr;
}  // namespace scheme_misc::plain