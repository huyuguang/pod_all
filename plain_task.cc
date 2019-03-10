#include "plain_task.h"

#include "ecc.h"
#include "ecc_pub.h"
#include "bulletin.h"
#include "misc.h"
#include "mkl_tree.h"
#include "multiexp.h"
#include "public.h"
#include "task_misc.h"

namespace {

uint64_t GetDataBlockCount(uint64_t size, uint64_t column_num) {
  uint64_t slice_count = (size + 31 - 1) / 31;
  return (slice_count + column_num - 1) / column_num;
}

// pad a random fr in the front of every block
bool DataToM(std::string const& pathname, uint64_t size, uint64_t n,
             uint64_t column_num, std::vector<Fr>& m) {
  try {
    io::mapped_file_params params;
    params.path = pathname;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    if (view.size() != size) return false;

    auto start = (uint8_t*)view.data();
    auto end = start + view.size();

    auto s = column_num + 1;
    m.resize(n * s);
    for (uint64_t i = 0; i < n; ++i) {
      m[i * s] = FrRand();  // pad random fr
      for (uint64_t j = 1; j < s; ++j) {
        LoadMij(start, end, i, j, s, m[i * s + j]);
      }
    }
    return true;
  } catch (std::exception&) {
    return false;
  }
}

bool SavePartMToFile(std::string const& file, uint64_t size, uint64_t s,
                     uint64_t start, uint64_t count,
                     std::vector<Fr> const& part_m) {
  if (s < 1 || !count || !size) return false;
  uint64_t column_num = s - 1;
  uint64_t n = GetDataBlockCount(size, column_num);
  if (!n || start >= n || (start + count) > n) return false;
  if (part_m.size() != count * s) return false;

  fs::remove(file);
  io::mapped_file_params file_params;
  file_params.path = file;
  file_params.flags = io::mapped_file_base::readwrite;
  file_params.new_file_size = count * column_num * 31 + 1;
  io::mapped_file file_view = io::mapped_file(file_params);
  if (!file_view.data()) return false;

  uint8_t* p = (uint8_t*)file_view.data();
  for (uint64_t i = 0; i < count; ++i) {
    for (uint64_t j = 1; j < s; ++j) {
      Fr const& fr = part_m[i * s + j];
      FrToBin(fr, p);
      if (p[31]) {
        assert(false);
        return false;
      }
      p += 31;
    }
  }

  file_view.close();

  uint64_t file_size = column_num * count * 31;
  if (start + count == n) {
    uint64_t tail_pad_len = n * column_num * 31 - size;
    assert(tail_pad_len < column_num * 31);
    file_size -= tail_pad_len;
  }

  fs::resize_file(file, file_size);
  return true;
}

bool IsSameFile(std::string const& file1, std::string const& file2) {
  try {
    io::mapped_file_params params1;
    params1.path = file1;
    params1.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view1(params1);

    io::mapped_file_params params2;
    params2.path = file2;
    params2.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view2(params2);

    if (view1.size() != view2.size()) return false;

    return memcmp(view1.data(), view2.data(), view1.size()) == 0;
  } catch (std::exception&) {
    return false;
  }
}
}  // namespace

PlainTask::PlainTask(std::string publish_file, std::string output_path,
                     uint64_t column_num)
    : publish_file_(std::move(publish_file)),
      output_path_(std::move(output_path)),
      s_(column_num + 1) {
  file_size_ = fs::file_size(publish_file_);
  if (!file_size_) throw std::runtime_error("empty file");
  n_ = GetDataBlockCount(file_size_, column_num);
}

bool PlainTask::Execute() {
  std::string data_file = output_path_ + "/data";
  std::string matrix_file = output_path_ + "/matrix";
  std::string bulletin_file = output_path_ + "/bulletin";
  std::string sigma_file = output_path_ + "/sigma";
  std::string sigma_mkl_file = output_path_ + "/sigma_mkl";

  bulletin::Plain bulletin;
  bulletin.size = file_size_;
  bulletin.s = s_;

  if (!CopyData(publish_file_, data_file)) {
    assert(false);
    return false;
  }

  std::vector<Fr> m;
  if (!DataToM(data_file, file_size_, n_, s_ - 1, m)) {
    assert(false);
    return false;
  }

  if (!SaveMatrix(matrix_file, m)) {
    assert(false);
    return false;
  }

  std::vector<G1> sigmas = CalcSigma(m, n_, s_);

  if (!SaveSigma(sigma_file, sigmas)) {
    assert(false);
    return false;
  }

  // mkl
  auto get_sigma = [&sigmas](uint64_t i) -> h256_t {
    return G1ToBin(sigmas[i]);
  };
  auto sigma_mkl_tree = mkl::BuildTree(n_, get_sigma);
  if (!SaveMkl(sigma_mkl_file, sigma_mkl_tree)) {
    assert(false);
    return false;
  }
  bulletin.sigma_mkl_root = sigma_mkl_tree.back();

  // meta
  if (!bulletin::Save(bulletin_file, bulletin)) {
    assert(false);
    return false;
  }

#ifdef _DEBUG
  std::string debug_data_file = data_file + ".debug";
  if (!SavePartMToFile(debug_data_file, file_size_, s_, 0, n_, m)) {
    assert(false);
    return false;
  }

  if (!IsSameFile(debug_data_file, data_file)) {
    assert(false);
    return false;
  }
  fs::remove(debug_data_file);
#endif
  return true;
}
