#include <algorithm>
#include <array>
#include <bitset>
#include <vector>

#include "ecc_pub.h"
#include "plain_task.h"
#include "public.h"
#include "table_task.h"

namespace {

enum TaskMode { kPlain, kTable };

std::istream& operator>>(std::istream& in, TaskMode& t) {
  std::string token;
  in >> token;
  if (token == "plain") {
    t = TaskMode::kPlain;
  } else if (token == "table") {
    t = TaskMode::kTable;
  } else {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

std::ostream& operator<<(std::ostream& os, TaskMode const& t) {
  if (t == TaskMode::kPlain) {
    os << "plain";
  } else if (t == TaskMode::kTable) {
    os << "table";
  } else {
    os.setstate(std::ios_base::failbit);
  }
  return os;
}
}  // namespace

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");

  TaskMode task_mode;
  std::string publish_file;
  std::string output_path;
  TableType table_type;
  std::vector<uint64_t> vrf_colnum_index;
  uint64_t column_num;
  std::string ecc_pub_file;

  try {
    po::options_description options("command line options");
    options.add_options()("help,h", "Use -h or --help to list all arguments")(
        "-p ecc_pub_file -m table -f file -o output_path -t table_type -v keys",
        "publish table file")(
        "-p ecc_pub_file -m plain -f file -o output_path -c column_num",
        "publish plain file")(
        "ecc_pub_file,p",
        po::value<std::string>(&ecc_pub_file)->default_value(""),
        "Provide the ecc pub file")(
        "mode,m",
        po::value<TaskMode>(&task_mode)->default_value(TaskMode::kPlain),
        "Provide pod mode (plain, table)")(
        "publish_file,f",
        po::value<std::string>(&publish_file)->default_value(""),
        "Provide the file which want to publish")(
        "output_path,o",
        po::value<std::string>(&output_path)->default_value(""),
        "Provide the publish path")(
        "table_type,t",
        po::value<TableType>(&table_type)->default_value(TableType::kCsv),
        "Provide the publish file type in table mode (csv)")(
        "column_num,c", po::value<uint64_t>(&column_num)->default_value(1024),
        "Provide the column number per block(line) in "
        "plain mode (default 1024)")(
        "vrf_colnum_index,v",
        po::value<std::vector<uint64_t>>(&vrf_colnum_index)->multitoken(),
        "Provide the publish file vrf key column index"
        "positions in table mode (for example: -v 0 1 3)");

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

    if (publish_file.empty() || !fs::is_regular(publish_file)) {
      std::cout << "Open publish_file " << publish_file << " failed"
                << std::endl;
      return -1;
    }

    if (fs::file_size(publish_file) == 0) {
      std::cout << "The file size of " << publish_file << " is 0" << std::endl;
      return -1;
    }

    if (!fs::is_directory(output_path) &&
        !fs::create_directories(output_path)) {
      std::cout << "Create " << output_path << " failed" << std::endl;
      return -1;
    }

    if (task_mode == TaskMode::kPlain) {
      if (column_num == 0) {
        std::cout << "column_num can not be 0.\n";
        std::cout << options << std::endl;
        return -1;
      }
    } else {
      std::sort(vrf_colnum_index.begin(), vrf_colnum_index.end());
      vrf_colnum_index.erase(
          std::unique(vrf_colnum_index.begin(), vrf_colnum_index.end()),
          vrf_colnum_index.end());

      if (vrf_colnum_index.empty()) {
        std::cout << "Want vrf_colnum_index in table mode.\n";
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

  auto& ecc_pub = GetEccPub();

  switch (task_mode) {
    case TaskMode::kPlain: {
      auto max_s = ecc_pub.u1().size();
      if (column_num > max_s) {
        std::cerr << "column_num too large! The upper bound is " << max_s
                  << std::endl;
        return -1;
      }
      PlainTask task(std::move(publish_file), std::move(output_path),
                     column_num);
      return task.Execute() ? 0 : -1;
    }
    case TaskMode::kTable: {
      TableTask task(std::move(publish_file), std::move(output_path),
                     std::move(table_type), std::move(vrf_colnum_index));
      return task.Execute() ? 0 : -1;
    }
    default:
      throw std::runtime_error("never reach");
  }
}
