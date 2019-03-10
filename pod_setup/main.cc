#include "ecc_pub.h"

namespace {
bool operator==(G1WM const& a, G1WM const& b) {
  if (a.bitSize_ != b.bitSize_) return false;
  if (a.winSize_ != b.winSize_) return false;
  if (a.tbl_.size() != b.tbl_.size()) return false;
  for (size_t i = 0; i < a.tbl_.size(); ++i) {
    auto const& ga = a.tbl_[i];
    auto const& gb = b.tbl_[i];
    if (ga != gb) {
      return false;
    }
  }
  return true;
}

bool operator!=(G1WM const& a, G1WM const& b) { return !(a == b); }

bool operator==(G2WM const& a, G2WM const& b) {
  if (a.bitSize_ != b.bitSize_) return false;
  if (a.winSize_ != b.winSize_) return false;
  if (a.tbl_.size() != b.tbl_.size()) return false;
  for (size_t i = 0; i < a.tbl_.size(); ++i) {
    if (a.tbl_[i] != b.tbl_[i]) return false;
  }
  return true;
}

bool operator!=(G2WM const& a, G2WM const& b) { return !(a == b); }

bool operator==(EccPub const& a, EccPub const& b) {
  if (a.g1_wm() != b.g1_wm()) return false;
  if (a.g2_wm() != b.g2_wm()) return false;
  if (a.u1() != b.u1()) return false;
  auto const& a_u1_wm = a.u1_wm();
  auto const& b_u1_wm = b.u1_wm();
  if (a_u1_wm.size() != b_u1_wm.size()) return false;
  for (size_t i = 0; i < a_u1_wm.size(); ++i) {
    if (a_u1_wm[i] != b_u1_wm[i]) return false;
  }

  if (a.u2() != b.u2()) return false;
  auto const& a_u2_wm = a.u2_wm();
  auto const& b_u2_wm = b.u2_wm();
  if (a_u2_wm.size() != b_u2_wm.size()) return false;
  for (size_t i = 0; i < a_u2_wm.size(); ++i) {
    if (a_u2_wm[i] != b_u2_wm[i]) return false;
  }

  return true;
}

bool operator!=(EccPub const& a, EccPub const& b) { return !(a == b); }

}  // namespace

int main(int argc, char** argv) {
  InitEcc();

  if (argc != 4) {
    std::cerr << "Usage: output_file u1_size u2_size." << std::endl;
    return -1;
  }

  std::string file = argv[1];
  size_t u1_size = std::stoi(argv[2]);
  if (u1_size <= 4) {
    std::cerr << "u1_size must greater than 4." << std::endl;
    return -1;
  }

  size_t u2_size = std::stoi(argv[3]);
  if (u2_size <= 1) {
    std::cerr << "u2_size must greater than 1." << std::endl;
    return -1;
  }

  try {
    EccPub ecc_pub(u1_size, u2_size);
    if (!ecc_pub.Save(file)) {
      std::cerr << "Save data failed." << std::endl;
      return -1;
    }

    EccPub ecc_pub2(file);

    if (ecc_pub != ecc_pub2) {
      std::cerr << "Oops!" << std::endl;
      return -1;
    }
    std::cout << "Success." << std::endl;
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return -1;
  }
}
