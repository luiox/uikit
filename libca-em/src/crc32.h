#ifndef CRC32_H
#define CRC32_H

#include <stdint.h> 


// CRC-32-IEEE 802.3
// x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
uint32_t crc32_ieee(const void* data, size_t size);


#endif // CRC32_H
