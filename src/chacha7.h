#pragma once

#include <openssl/chacha.h>

#ifdef __cplusplus
extern "C" {
#endif
void ChaCha7(ChaCha_ctx *ctx, unsigned char *out, const unsigned char *in, size_t len);
#ifdef __cplusplus
}
#endif