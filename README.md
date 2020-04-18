# SAMD-CRC32
CRC32 Library for SAMD Microcontrollers

## Usage

Include and create the SAMD_CRC32 object.
```C++
#include <SAMD_CRC32.h>

// Create the CRC object
SAMD_CRC32 crc = SAMD_CRC32();
```

Provide a pointer to the data, the size of the data in bytes (must be in 32bit word form), and a pointer to a variable to save the result to. Any addressable variable is acceptiable (program memory, ram, EEPROM, etc.).
```c++
uint32_t crc_result = 0;
crc.crc32(&data, sizeof(data), &crc_result);
```

## Considerations

### Endianness
The endian type of the device will make a difference for the CRC. In hardware, the CRC is calculated by stepping through memory directly. Note the endianness of the hardware you are using and note that you may have to flip the endianness of your data in order to get it to validate the CRC.

### Non SAMD Devices
When loaded to a device that does not have a supported CRC unit, a software CRC will be calculated instead. This is slower than the hardware CRC, but will work on most any devices. The software algorithm was adapted from (this source)[http://home.thep.lu.se/~bjorn/crc/].
