#include "ecc_pub.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain_otrange_test.h"
#include "scheme_plain_range_test.h"
#include "scheme_table_otvrfq_test.h"
#include "scheme_table_vrfq_test.h"

namespace boost::program_options {
// Called by program_options to parse a set of Range arguments
void validate(boost::any& v, std::vector<std::string> const& values, Range*,
              int) {
  Range r;
  // Extract tokens from values string vector and populate Model struct.
  if (values.size() != 2) {
    throw po::validation_error(po::validation_error::invalid_option_value,
                               "start count");
  }

  r.start = boost::lexical_cast<uint64_t>(values[0]);
  r.count = boost::lexical_cast<uint64_t>(values[1]);
  v = r;
}
}  // namespace

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");

  using scheme::Mode;
  Mode mode;
  std::string publish_path;
  std::string output_path;
  std::string ecc_pub_file;
  std::pair<uint64_t, uint64_t> aaaa;
  Range demand_range;
  Range phantom_range;
  std::string query_key;
  std::vector<std::string> query_values;
  std::vector<std::string> phantom_values;
  uint32_t omp_thread_num;

  try {
    po::options_description options("command line options");
    options.add_options()("help,h", "Use -h or --help to list all arguments")(
        "ecc_pub_file,e",
        po::value<std::string>(&ecc_pub_file)->default_value(""),
        "Provide the ecc pub file")(
        "mode,m", po::value<Mode>(&mode)->default_value(Mode::kPlain),
        "Provide pod mode (plain, table)")(
        "publish_path,p",
        po::value<std::string>(&publish_path)->default_value(""),
        "Provide the publish path")(
        "output_path,o",
        po::value<std::string>(&output_path)->default_value(""),
        "Provide the output path")("demand_range,d",
                                   po::value<Range>(&demand_range)->multitoken(),
                                   "Provide the demand range(plain mode)")(
        "phantom_range,g", po::value<Range>(&phantom_range)->multitoken(),
        "Provide the phantom range(plain mode)")(
        "query_key,k", po::value<std::string>(&query_key)->default_value(""),
        "Provide the query key name(table mode)")(
        "key_value,ot_v",
        po::value<std::vector<std::string>>(&query_values)->multitoken(),
        "Provide the query key values(table mode, for example "
        "-ot_v value_a "
        "value_b value_c)")(
        "phantom_key,pk",
        po::value<std::vector<std::string>>(&phantom_values)->multitoken(),
        "Provide the query key phantoms(table mode, for example -n "
        "phantoms_a "
        "phantoms_b phantoms_c)")(
        "omp_thread_num,t",
        po::value<uint32_t>(&omp_thread_num)->default_value(0),
        "Provide the number of the openmp thread, 1: disable "
        "openmp, 0: "
        "default.");

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
      if (demand_range.count == 0) {
        std::cout << "demand count can not be 0.\n";
        return -1;
      }

      if (phantom_range.count != 0) {
        if (phantom_range.start < demand_range.start || phantom_range.count < demand_range.count) {
          std::cout << "phantom must overwrite demand.\n";
          return -1;
        }
      }

    } else {
      if (query_key.empty() || query_values.empty()) {
        std::cout << "query_key and query_values can not be empty.\n";
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

  if (omp_thread_num) {
    std::cout << "omp_set_num_threads: " << omp_thread_num << "\n";
    omp_set_num_threads(omp_thread_num);
  }

  std::cout << "omp_get_max_threads: " << omp_get_max_threads() << "\n";

  InitEcc();

  if (!InitEccPub(ecc_pub_file)) {
    std::cerr << "Open ecc pub file " << ecc_pub_file << " failed" << std::endl;
    return -1;
  }

  if (mode == Mode::kPlain) {
    auto output_file = output_path + "/decrypted_data";
    if (!phantom_range.count) {
      return scheme::plain::range::Test(publish_path, output_file, demand_range) ? 0
                                                                           : -1;
    } else {
      return scheme::plain::otrange::Test(publish_path, output_file, demand_range,
                                          phantom_range)
                 ? 0
                 : -1;
    }
  } else {
    if (phantom_values.empty()) {
      return scheme::table::vrfq::Test(publish_path, query_key, query_values)
                 ? 0
                 : -1;
    } else {
      return scheme::table::otvrfq::Test(publish_path, query_key, query_values,
                                         phantom_values)
                 ? 0
                 : -1;
    }
  }
}
