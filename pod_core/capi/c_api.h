#pragma once

#include <stdint.h>
#include "c_api_plain.h"
#include "c_api_table.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
EXPORT bool E_InitAll(char const* data_dir);
}