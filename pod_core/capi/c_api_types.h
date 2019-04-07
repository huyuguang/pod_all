#include <stdint.h>

extern "C" {
typedef void* handle_t;

struct range_t {
  uint64_t start;
  uint64_t count;
};

struct buffer_t {
  uint8_t* p;
  uint64_t len;
};

struct plain_bulletin_t {
  uint64_t size;
  uint64_t s;
  uint64_t n;
  uint8_t sigma_mkl_root[32];
};
}