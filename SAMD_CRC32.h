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

// Define 'SAMD_CRC32_NO_STATUS' to remove status codes and status code helpers (reduces program size by ~ 24 bytes)
// #define SAMD_CRC32_NO_STATUS

// Define 'SAMD_CRC32_NO_SOFTWARE_CRC' to remove software CRC32 support (reduces program size by ~ 184 bytes)
// #define SAMD_CRC32_NO_SOFTWARE_CRC

#ifndef SAMD_CRC32_NO_STATUS
#define FOREACH_STATUS(STATUS)   \
  STATUS(OK)                     \
  STATUS(BUS_ERROR)              \
  STATUS(NOT_WORD_ALIGNED)       \
  STATUS(USER_FORCED_SOFTWARE)   \
  STATUS(HARDWARE_NOT_SUPPORTED) \
  STATUS(HARDWARE_CRC32_IN_USE)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum hardware_crc_status
{
  FOREACH_STATUS(GENERATE_ENUM)
};

static const char *hardware_crc_status_str[] = {
    FOREACH_STATUS(GENERATE_STRING)};

#endif

class SAMD_CRC32 {
 public:

  SAMD_CRC32();

  // ---- Setters ----
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
  void force_use_software_crc32(bool val);  // Forces the use of the software CRC32 implementation
#endif

  // ---- Getters ----
  bool can_use_hardware_crc32();  // Returns whether or not the device has suported CRC32 hardware
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
  bool check_used_hardware_crc32();  // Returns whether or not the last CRC32 was calculated in hardware
#endif
#ifndef SAMD_CRC32_NO_STATUS
  const char *get_hardware_status_msg();  // Returns the last CRC32 hardware status message
  uint8_t get_hardware_status_code();  // Returns the last CRC32 hardware status code

  // ---- Helpers ----
  const char *decode_hardware_status_code(uint8_t code);  // Returns the message version of a CRC32 hardware status code
  static bool _dsu_in_use;
#endif
  // ---- Primary ----
  volatile uint8_t crc32(const void *data, size_t n_bytes, uint32_t *crc);  // Calculates a CRC32

private:
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
  volatile void software_crc32(const void * data, size_t n_bytes, uint32_t *crc);
  uint32_t crc32_for_byte(uint32_t r);
  typedef unsigned long accum_t;
  bool _force_software_crc;
  bool _using_hardware_crc;
#endif
  bool _hardware_crc_errata;
  bool _nvm_cache_errata;
#ifndef SAMD_CRC32_NO_STATUS
  hardware_crc_status _hardware_status_code;
#endif
};

#endif