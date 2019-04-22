#pragma once

#include <stdint.h>
#include "c_api_types.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
EXPORT handle_t E_PlainANew(char const* publish_path);
EXPORT handle_t E_PlainBNew(char const* bulletin_file, char const* public_path);

EXPORT bool E_PlainAFree(handle_t h);
EXPORT bool E_PlainBFree(handle_t h);

EXPORT bool E_PlainABulletin(handle_t h, plain_bulletin_t* bulletin);
EXPORT bool E_PlainBBulletin(handle_t h, plain_bulletin_t* bulletin);
}

// range
extern "C" {
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
                                       char const* secret_file);

EXPORT bool E_PlainRangeClientGenerateClaim(handle_t c_client,
                                            char const* claim_file);

EXPORT bool E_PlainRangeClientSaveDecrypted(handle_t c_client,
                                            char const* file);

EXPORT bool E_PlainRangeClientFree(handle_t h);
}  // extern "C" range

// otrange
extern "C" {
EXPORT handle_t E_PlainOtRangeSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id);

EXPORT bool E_PlainOtRangeSessionGetNegoRequest(handle_t c_session,
                                                char const* request_file);

EXPORT bool E_PlainOtRangeSessionOnNegoRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file);

EXPORT bool E_PlainOtRangeSessionOnNegoResponse(handle_t c_session,
                                                char const* response_file);

EXPORT bool E_PlainOtRangeSessionOnRequest(handle_t c_session,
                                           char const* request_file,
                                           char const* response_file);

EXPORT bool E_PlainOtRangeSessionOnReceipt(handle_t c_session,
                                           char const* receipt_file,
                                           char const* secret_file);

EXPORT bool E_PlainOtRangeSessionSetEvil(handle_t c_session);

EXPORT bool E_PlainOtRangeSessionFree(handle_t h);

EXPORT handle_t E_PlainOtRangeClientNew(handle_t c_b, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id,
                                        range_t c_demand, range_t c_phantom);

EXPORT bool E_PlainOtRangeClientGetNegoRequest(handle_t c_client,
                                               char const* request_file);

EXPORT bool E_PlainOtRangeClientOnNegoRequest(handle_t c_client,
                                              char const* request_file,
                                              char const* response_file);

EXPORT bool E_PlainOtRangeClientOnNegoResponse(handle_t c_client,
                                               char const* response_file);

EXPORT bool E_PlainOtRangeClientGetRequest(handle_t c_client,
                                           char const* request_file);

EXPORT bool E_PlainOtRangeClientOnResponse(handle_t c_client,
                                           char const* response_file,
                                           char const* receipt_file);

EXPORT bool E_PlainOtRangeClientOnSecret(handle_t c_client,
                                         char const* secret_file);

EXPORT bool E_PlainOtRangeClientGenerateClaim(handle_t c_client,
                                              char const* claim_file);

EXPORT bool E_PlainOtRangeClientSaveDecrypted(handle_t c_client,
                                              char const* file);

EXPORT bool E_PlainOtRangeClientFree(handle_t h);
}  // extern "C" otrange

// batch3
extern "C" {
EXPORT handle_t E_PlainBatch3SessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id);

EXPORT bool E_PlainBatch3SessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file);

EXPORT bool E_PlainBatch3SessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file);

EXPORT bool E_PlainBatch3SessionFree(handle_t h);

EXPORT handle_t E_PlainBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count);

EXPORT bool E_PlainBatch3ClientGetRequest(handle_t c_client,
                                          char const* request_file);

EXPORT bool E_PlainBatch3ClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file);

EXPORT bool E_PlainBatch3ClientOnSecret(handle_t c_client,
                                        char const* secret_file);

EXPORT bool E_PlainBatch3ClientSaveDecrypted(handle_t c_client,
                                             char const* file);

EXPORT bool E_PlainBatch3ClientFree(handle_t h);
}  // extern "C" batch3