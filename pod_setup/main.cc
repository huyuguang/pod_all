#include "ecc.h"
#include "public.h"

bool GenerateAtomicSwapKeyPair(std::string const& output_path, uint64_t count);

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");
  std::string output_path;
  uint64_t count;

  try {
    po::options_description options("command line options");
    options.add_options()("help,h", "Use -h or --help to list all arguments")(
        "output_path,o",
        po::value<std::string>(&output_path)->default_value("zksnark_key"),
        "Provide the output path")(
        "count,c", po::value<uint64_t>(&count)->default_value(1024),
        "Provide the count");

    boost::program_options::variables_map vmap;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, options), vmap);
    boost::program_options::notify(vmap);

    if (vmap.count("help")) {
      std::cout << options << std::endl;
      return -1;
    }

    if (output_path.empty()) {
      std::cout << "Want output_path(-o)\n";
      std::cout << options << std::endl;
      return -1;
    }

    fs::create_directories(output_path);
    if (!fs::is_directory(output_path)) {
      std::cerr << "create directory failed"
                << "\n";
      return -1;
    }
  } catch (std::exception& e) {
    std::cout << "Unknown parameters.\n"
              << e.what() << "\n"
              << "-h or --help to list all arguments.\n";
    return -1;
  }

  InitEcc();

  GenerateAtomicSwapKeyPair(output_path, count);

  return 0;
}
