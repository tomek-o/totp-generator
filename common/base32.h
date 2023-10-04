#ifndef BASE32_H
#define BASE32_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Decode a base32 encoding of `len' bytes of `data' into the buffer `dst'.
 *
 * @param dst		destination buffer
 * @param size		length of destination
 * @param data		start of data to decode
 * @param len		amount of encoded data to decode
 *
 * @return the amount of bytes decoded into the destination, -1 on error
 */
int base32_decode(unsigned char *dst, size_t size, const unsigned char *data, size_t len);


/**
 * Encode in base32 `len' bytes of `data' into the buffer `dst'.
 *
 * @param dst		destination buffer
 * @param size		length of destination
 * @param data		start of data to encode
 * @param len		amount of bytes to encode
 *
 * @return the amount of bytes generated into the destination.
 */
int base32_encode(char *dst, size_t size, const unsigned char *data, size_t len);


#ifdef __cplusplus
}
#endif

#endif
