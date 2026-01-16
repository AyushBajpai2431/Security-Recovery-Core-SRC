/**
 * Cryptographic Implementation
 * Uses ECDSA or RSA for signatures, SHA-256 for hashing
 */

#include "crypto.h"
#include "platform.h"
#include <string.h>

static bool crypto_initialized = false;

bool crypto_init(void) {
    if (crypto_initialized) {
        return true;
    }
    
    /* Platform-specific crypto initialization */
    if (!platform_crypto_init()) {
        return false;
    }
    
    crypto_initialized = true;
    return true;
}

void crypto_sha256(const uint8_t *data, size_t size, uint8_t *hash) {
    if (!data || !hash || size == 0) {
        return;
    }
    
    /* Platform-specific SHA-256 implementation */
    platform_sha256(data, size, hash);
}

bool crypto_sign(const uint8_t *data, size_t size, 
                uint8_t *signature, size_t *sig_size) {
    if (!data || !signature || !sig_size || size == 0) {
        return false;
    }
    
    /* Calculate hash first */
    uint8_t hash[32];
    crypto_sha256(data, size, hash);
    
    /* Platform-specific signing */
    return platform_sign(hash, 32, signature, sig_size);
}

bool crypto_verify(const uint8_t *data, size_t size, 
                  const uint8_t *signature, size_t sig_size) {
    if (!data || !signature || size == 0 || sig_size == 0) {
        return false;
    }
    
    /* Calculate hash first */
    uint8_t hash[32];
    crypto_sha256(data, size, hash);
    
    /* Platform-specific verification */
    return platform_verify(hash, 32, signature, sig_size);
}
