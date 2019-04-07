#pragma once

#include <stdint.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

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

EXPORT void E_InitEcc();
EXPORT bool E_LoadEccPub(char const* ecc_pub_file);

EXPORT handle_t E_PlainANew(char const* publish_path);
EXPORT handle_t E_PlainBNew(char const* bulletin_file, char const* public_path);

EXPORT bool E_PlainAFree(handle_t h);
EXPORT bool E_PlainBFree(handle_t h);

EXPORT bool E_PlainABulletin(handle_t h, plain_bulletin_t* bulletin);
EXPORT bool E_PlainBBulletin(handle_t h, plain_bulletin_t* bulletin);

EXPORT handle_t E_PlainRangeSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id);

EXPORT bool E_PlainRangeSessionOnRequest(handle_t c_session,
                                         char const* request_file,
                                         char const* response_file);

EXPORT bool E_PlainRangeSessionOnReceipt(handle_t c_session,
                                         char const* receipt_file,
                                         char const* secret_file);

EXPORT bool E_PlainRangeSessionSetEvil(handle_t c_session);

EXPORT bool E_PlainRangeSessionFree(handle_t h);

EXPORT handle_t E_PlainRangeClientNew(handle_t c_b, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id,
                                      range_t c_demand);

EXPORT bool E_PlainRangeClientGetRequest(handle_t c_client,
                                         char const* request_file);

EXPORT bool E_PlainRangeClientOnResponse(handle_t c_client,
                                         char const* response_file,
                                         char const* receipt_file);

EXPORT bool E_PlainRangeClientOnSecret(handle_t c_client,
                                       char const* secret_file,
                                       char const* claim_file);

EXPORT bool E_PlainRangeClientSaveDecrypted(handle_t c_client,
                                            char const* file);

EXPORT bool E_PlainRangeClientFree(handle_t h);
}