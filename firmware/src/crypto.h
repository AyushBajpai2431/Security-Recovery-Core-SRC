/**
 * Cryptographic Functions
 * Signature verification and hashing
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Calculate SHA-256 hash */
void crypto_sha256(const uint8_t *data, size_t size, uint8_t *hash);

/* Sign data (returns signature) */
bool crypto_sign(const uint8_t *data, size_t size, 
                uint8_t *signature, size_t *sig_size);

/* Verify signature */
bool crypto_verify(const uint8_t *data, size_t size, 
                  const uint8_t *signature, size_t sig_size);

/* Initialize crypto system (load keys, etc.) */
bool crypto_init(void);

#endif /* CRYPTO_H */
