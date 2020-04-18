/***************************************************
  This is a library for SAMD hardware CRC32 support

  https://github.com/knicholson32/SAMD-CRC32

  This library uses the DSU CRC32 hardware contained in the
  ATSAMD series microcontrollers. If used on a device
  that is not SAMD (such as AVR), a software CRC32 algorithm
  will be used. 

  The software CRC32 algorithm was adapted from this source:
  http://home.thep.lu.se/~bjorn/crc/
  Björn Samuelsson

  CRC32 uses the standard 32-bit CRC parameters:
    Poly:    0x04C11DB7
    Init:    0xFFFFFFFF
    XOR:     0xFFFFFFFF
    Reflect: Yes

  Written by Keenan Nicholson
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#ifndef _SAMD_CRC32_H
#define _SAMD_CRC32_H

#include <Arduino.h>

class SAMD_CRC32 {
 public:

  SAMD_CRC32();

  bool can_use_hardware_crc32();
  void force_use_software_crc32(bool val);
  bool check_using_hardware_crc32();
  volatile bool crc32(const void *data, size_t n_bytes, uint32_t *crc);

private:
  volatile void software_crc32(const void * data, size_t n_bytes, uint32_t *crc);
  uint32_t crc32_for_byte(uint32_t r);
  typedef unsigned long accum_t;
  bool _force_software_crc;
  bool _hardware_crc_errata;
  bool _nvm_cache_errata;
  bool _using_hardware_crc;
};

#endif