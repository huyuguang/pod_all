#pragma once

#include "basic_types_serialize.h"
#include "matrix_fr.h"

namespace Eigen {
// save
template<typename Ar, int _Rows, int _Cols>
void serialize(Ar &ar, Matrix<Fr, _Rows, _Cols> const& t) {
  ar &YAS_OBJECT_NVP("matrix", ("r", t.rows()), ("r", t.cols()));
  for (int i = 0; i < t.size(); ++i) {
    ar &(*(t.data() + i));
  }
}

// load
template<typename Ar, int _Rows, int _Cols>
void serialize(Ar &ar, Matrix<Fr, _Rows, _Cols> &t) {
  int rows, cols;
  ar &YAS_OBJECT_NVP("matrix", ("r", rows), ("r", cols));
  t.resize(rows, cols);
  for (int i = 0; i < t.size(); ++i) {
    ar &(*(t.data() + i));
  }  
}
}