#ifndef CRC_H
#define CRC_H

#include "datatype.h" 

// CRC-32-IEEE 802.3
// x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
u32 crc32_ieee(const void* data, usize size);

// CRC-16-modbus
// x^16 + x^15 + x^2 + 1
u16 crc16_modbus(const void* data, usize size);

#endif // CRC_H
