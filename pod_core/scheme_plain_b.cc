#include "scheme_plain_b.h"
#include "misc.h"

namespace scheme::plain {
B::B(Bulletin const& bulletin, std::string const& public_path)
    : bulletin_(bulletin), public_path_(public_path) {
  LoadData();
}

B::B(std::string const& bulletin_file, std::string const& public_path)
    : public_path_(public_path) {
  if (!LoadBulletin(bulletin_file, bulletin_))
    throw std::runtime_error("invalid bulletin file");
  LoadData();
}

// throw
bool B::NeedVerify() {
  bool verify = false;
  std::string verify_file = public_path_ + "/.verify";
  if (fs::is_regular_file(verify_file)) {
    time_t verify_time = fs::last_write_time(verify_file);
    auto range =
        boost::make_iterator_range(fs::directory_iterator(public_path_), {});
    for (auto& entry : range) {
      time_t file_time = fs::last_write_time(entry);
      if (file_time > verify_time) {
        verify = true;
        break;
      }
    }
  } else {
    verify = true;
  }
  return verify;
}

// throw
void B::LoadData() {
  if (!bulletin_.n || !bulletin_.s)
    throw std::runtime_error("invalid bulletin");

  std::string verify_file = public_path_ + "/.verify";
  std::string sigma_file = public_path_ + "/sigma";

  h256_t const* check_h;
  bool verify = NeedVerify();

  // sigma
  check_h = verify ? &bulletin_.sigma_mkl_root : nullptr;
  if (!LoadSigma(sigma_file, bulletin_.n, check_h, sigmas_)) {
    assert(false);
    throw std::runtime_error("invalid sigma file");
  }

  if (verify) {
    fs::remove(verify_file);
    fs::ofstream dummy(verify_file);
  }
}

}  // namespace scheme::plain