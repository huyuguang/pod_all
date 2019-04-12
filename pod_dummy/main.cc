#include "matrix_fr.h"

namespace {

void TestEigen() {
  using Eigen::Dynamic;
  using Eigen::Matrix;
  Matrix<Fr, Dynamic, Dynamic> aa(2, 10);

  Matrix<Fr, 2, 2> a;
  a(0, 0) = Fr(1);
  a(1, 0) = Fr(2);
  a(0, 1) = Fr(3);
  a(1, 1) = Fr(4);
  std::cout << "a=\n";
  std::cout << a << "\n\n";

  Matrix<Fr, 2, 2> b;
  b(0, 0) = Fr(11);
  b(1, 0) = Fr(22);
  b(0, 1) = Fr(33);
  b(1, 1) = Fr(44);
  std::cout << "b=\n";
  std::cout << b << "\n\n";

  auto c = a * b;
  std::cout << "c=a*b=\n";
  std::cout << c << "\n\n";
  
  auto d = c.inverse();
  std::cout << "d=~c=\n";
  std::cout << d << "\n\n";

  auto e = c * d;
  std::cout << "e=c*d=\n";
  std::cout << e << "\n\n";

  auto f = a * d;
  std::cout << "f=a*d=\n";
  std::cout << f << "\n\n";

  auto g = f * b;
  std::cout << "g=f*b=\n";
  std::cout << g << "\n\n";

  // left col == right row
  Matrix<Fr, 2, Dynamic> aaa(2,5);
  for (int i = 0; i < aaa.rows(); ++i) {
    for (int j = 0; j < aaa.cols(); ++j) {
      aaa(i, j) = FrRand();
    }
  }
  auto bbb = d * aaa;
  std::cout << bbb << "\r\n";
  // MatrixXd m(2, 2);
  // m(0, 0) = 3;
  // m(1, 0) = 2.5;
  // m(0, 1) = -1;
  // m(1, 1) = m(1, 0) + m(0, 1);
  // std::cout << m << std::endl;

  // auto m2 = m.inverse();
  // std::cout << m2 << std::endl;

  // auto m3 = m * m2;
  // std::cout << m3 << std::endl;

  // auto n = MatrixXd::Random(2, 2);
  ////auto a = n / 3;
  // auto b = n * m2;
  ////std::cout << a << std::endl;
  // std::cout << b << std::endl;
}
}  // namespace

int main(int /*argc*/, char** /*argv*/) {
  InitEcc();
  TestEigen();

  return 0;
}
