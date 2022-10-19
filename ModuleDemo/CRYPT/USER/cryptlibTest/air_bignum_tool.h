#ifndef __AIR_BIGNUM_TOOL_H
#define __AIR_BIGNUM_TOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

    int bn_read_string( uint32_t *result, uint32_t len, const char *s );
    int bn_write_string(char *s, uint32_t *slen, const uint32_t *content, uint32_t len);
    int bn_printf(const uint32_t *result, uint32_t len);
    int bn_read_string_to_bytes( uint8_t *r, uint32_t len, const char *s );
    int bn_read_string_from_head( uint8_t *r, uint32_t len, const char *s );

#ifdef __cplusplus
}
#endif

#endif

