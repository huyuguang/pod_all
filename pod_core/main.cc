#include "ecc_pub.h"
#include "public.h"
#include "scheme_table_test.h"
#include "scheme_plain_test.h"
#include "scheme_misc.h"

namespace {

}  // namespace

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");

  using scheme_misc::Mode;
  Mode mode;
  std::string publish_path;
  std::string output_path;
  std::string ecc_pub_file;
  uint64_t start;
  uint64_t count;
  std::string key_name;
  std::string key_value;

  try {
    po::options_description options("command line options");
    options.add_options()("help,h", "Use -h or --help to list all arguments")(
        "ecc_pub_file,e",
        po::value<std::string>(&ecc_pub_file)->default_value(""),
        "Provide the ecc pub file")(
        "mode,m",
        po::value<Mode>(&mode)->default_value(Mode::kPlain),
        "Provide pod mode (plain, table)")(
        "publish_path,p",
        po::value<std::string>(&publish_path)->default_value(""),
        "Provide the publish path")(
        "output_path,o",
        po::value<std::string>(&output_path)->default_value(""),
        "Provide the output path")(
        "start,s",
        po::value<uint64_t>(&start)->default_value(0),
        "Provide the start block position(plain mode)")(
        "count,c",
        po::value<uint64_t>(&count)->default_value(1),
        "Provide the retrieve block count(plain mode)")(
        "key_name,k",
        po::value<std::string>(&key_name)->default_value(""),
        "Provide the query key name(table mode)")(
        "key_value,v",
        po::value<std::string>(&key_value)->default_value(""),
        "Provide the query key value(table mode)");

    boost::program_options::variables_map vmap;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, options), vmap);
    boost::program_options::notify(vmap);

    if (vmap.count("help")) {
      std::cout << options << std::endl;
      return -1;
    }

    if (ecc_pub_file.empty() || !fs::is_regular(ecc_pub_file)) {
      std::cout << "Open ecc_pub_file " << ecc_pub_file << " failed"
                << std::endl;
      return -1;
    }

    if (output_path.empty()) {
      std::cout << "Want output_path(-o)" << std::endl;
      return -1;
    }

    if (publish_path.empty() || !fs::is_directory(publish_path)) {
      std::cout << "Open publish_path " << publish_path << " failed"
                << std::endl;
      return -1;
    }

    if (!fs::is_directory(output_path) &&
        !fs::create_directories(output_path)) {
      std::cout << "Create " << output_path << " failed" << std::endl;
      return -1;
    }

    if (mode == Mode::kPlain) {
      if (count == 0) {
        std::cout << "count can not be 0.\n";
        std::cout << options << std::endl;
        return -1;
      }
    } else {
      if (key_name.empty() || key_value.empty()) {
        std::cout << "key_name and key_value can not be empty.\n";
        std::cout << options << std::endl;
        return -1;
      }
    }
  } catch (std::exception& e) {
    std::cout << "Unknown parameters.\n"
              << e.what() << "\n"
              << "-h or --help to list all arguments.\n";
    return -1;
  }

  InitEcc();

  if (!InitEccPub(ecc_pub_file)) {
    std::cerr << "Open ecc pub file " << ecc_pub_file << " failed" << std::endl;
    return -1;
  }
  
  if (mode == Mode::kPlain) {
    return scheme_misc::plain::Test(publish_path, start, count)? 0: -1;
  } else {
    return scheme_misc::table::Test(publish_path, key_name, key_value)? 0:-1;
  }
}
