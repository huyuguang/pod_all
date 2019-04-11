#include "ecc_pub.h"
#include <Eigen/Dense>

namespace {

 void TestEigen() {
  using Eigen::MatrixXd;
  MatrixXd m(2, 2);
  m(0, 0) = 3;
  m(1, 0) = 2.5;
  m(0, 1) = -1;
  m(1, 1) = m(1, 0) + m(0, 1);
  std::cout << m << std::endl;

  auto m2 = m.inverse();
  std::cout << m2 << std::endl;

  auto m3 = m * m2;
  std::cout << m3 << std::endl;

  auto n = MatrixXd::Random(2, 2);
  //auto a = n / 3;
  auto b = n * m2;
  //std::cout << a << std::endl;
  std::cout << b << std::endl;
 }
}  // namespace

int main(int /*argc*/, char** /*argv*/) {
  InitEcc();
  TestEigen();

  return 0;
}
