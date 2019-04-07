#pragma once

#include <stdint.h>
#include "c_api_types.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
EXPORT handle_t E_TableANew(char const* publish_path);
EXPORT handle_t E_TableBNew(char const* bulletin_file, char const* public_path);

EXPORT bool E_TableAFree(handle_t h);
EXPORT bool E_TableBFree(handle_t h);

EXPORT bool E_TableABulletin(handle_t h, table_bulletin_t* bulletin);
EXPORT bool E_TableBBulletin(handle_t h, table_bulletin_t * bulletin);
}

// batch
extern "C" {
EXPORT handle_t E_TableBatchSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id);

EXPORT bool E_TableBatchSessionOnRequest(handle_t c_session,
                                         char const* request_file,
                                         char const* response_file);

EXPORT bool E_TableBatchSessionOnReceipt(handle_t c_session,
                                         char const* receipt_file,
                                         char const* secret_file);

EXPORT bool E_TableBatchSessionSetEvil(handle_t c_session);

EXPORT bool E_TableBatchSessionFree(handle_t h);

EXPORT handle_t E_TableBatchClientNew(handle_t c_b, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id,
                                      range_t const* c_demand,
                                      uint64_t c_demand_count);

EXPORT bool E_TableBatchClientGetRequest(handle_t c_client,
                                         char const* request_file);

EXPORT bool E_TableBatchClientOnResponse(handle_t c_client,
                                         char const* response_file,
                                         char const* receipt_file);

EXPORT bool E_TableBatchClientOnSecret(handle_t c_client,
                                       char const* secret_file,
                                       char const* claim_file);

EXPORT bool E_TableBatchClientSaveDecrypted(handle_t c_client,
                                            char const* file);

EXPORT bool E_TableBatchClientFree(handle_t h);
} // extern "C" batch