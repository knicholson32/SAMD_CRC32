/***************************************************
  This is a library for SAMD hardware CRC32 support

  https://github.com/knicholson32/SAMD-CRC32

  This library uses the DSU CRC32 hardware contained in the
  ATSAMD series microcontrollers. If used on a device
  that is not SAMD (such as AVR), a software CRC32 algorithm
  will be used. 

  The software CRC32 algorithm was adapted from this source:
  http://home.thep.lu.se/~bjorn/crc/

  Written by Keenan Nicholson
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#if defined(_SAMD_INCLUDED_) && defined(REV_DSU)
#define HARDWARE_CRC32
#endif

#include <SAMD_CRC32.h>


SAMD_CRC32::SAMD_CRC32(){
#if defined(_SAMD_INCLUDED_) && defined(REV_DSU)
    uint32_t rev = DSU->DID.bit.REVISION;  // Get DSU Device Information to assess whether or not this chip
                                           // has the die issue that requires correcting when using hardware
                                           // crc32 on some SAMD21 devices.
    _hardware_crc_errata = (rev == 0x00 || rev == 0x01 || rev == 0x02 || rev == 0x03) && DSU->DID.bit.PROCESSOR == 0x1 && DSU->DID.bit.FAMILY == 0x0;
    _nvm_cache_errata = false;  // TODO: Assess how to detect NVM Cache Errata when using the SAMX5X Series Chips
    _using_hardware_crc = true;
#else
    _using_hardware_crc = false;
#endif
}

bool SAMD_CRC32::can_use_hardware_crc32() {
#if defined(_SAMD_INCLUDED_) && defined(REV_DSU)
    return true;
#else
    return false;
#endif
}

bool SAMD_CRC32::check_using_hardware_crc32(){
    return _using_hardware_crc;
}

void SAMD_CRC32::force_use_software_crc32(bool val){
    SAMD_CRC32::_force_software_crc = val;
};

#if defined(_SAMD_INCLUDED_) && defined(REV_DSU)
    /*************** SAMD Hardware ***************/
    volatile bool SAMD_CRC32::crc32(const void *data, size_t n_bytes, uint32_t *crc)
    {
        if (_force_software_crc)
        {
            _using_hardware_crc = false;
            software_crc32(data, n_bytes, crc);
            return true;
        }
        else
        {
            _using_hardware_crc = true;
            uint32_t address = (uint32_t)data;
            bool errata = _hardware_crc_errata && (address >= 0x20000000); // Start address of SRAM in SAMD chips
            if (errata)
            {
                volatile unsigned int *addr = (volatile unsigned int *)0x41007058;
                *addr &= ~0x30000UL;
                // TODO: Enable NVM Caching on certain errata SAMX5X Chips (see _nvm_cache_errata)
            }
            PM->AHBMASK.reg |= PM_AHBMASK_DSU;   // Enable APB DSU Clock Domain
            PM->APBBMASK.reg |= PM_APBBMASK_DSU; // Enable AHB DSU Clock Domain
            if (PAC1->WPCLR.reg & 0x02)          // Check if DSU write protection is enabled
                PAC1->WPCLR.reg = 0x02;          // b00000000000000000000000000000010 -> Removes DSU write protection (allowing access to internal reg.s)
            DSU->DATA.reg = 0xFFFFFFFFUL;        // Sets starting CRC data
            DSU->ADDR.reg = DSU_ADDR_ADDR(address >> 2);
            DSU->LENGTH.bit.LENGTH = n_bytes >> 2; // Divide length by four for word alignment
            DSU->STATUSA.bit.BERR;                 // Clear the error flag
            DSU->STATUSA.bit.DONE = 1;             // Clear the done flag
            DSU->CTRL.bit.CRC = 1;                 // Start the CRC operation

            while (!DSU->STATUSA.bit.DONE)
            {
            }

            if (DSU->STATUSA.bit.BERR)
            {
                _using_hardware_crc = false;
                software_crc32(data, n_bytes, crc);
                return false;
            }

            *crc = (DSU->DATA.reg) ^ 0xFFFFFFFF;

            if (errata)
            {
                volatile unsigned int *addr = (volatile unsigned int *)0x41007058;
                *addr |= 0x20000UL;
            }

            return true;
        }
    }
#else
    /************* Non SAMD Hardware *************/
    volatile bool SAMD_CRC32::crc32(const void * data, size_t n_bytes, uint32_t * crc)
    {
        software_crc32(data, n_bytes, crc);
        return true;
    }
#endif

/********** Software CRC32 **********/
volatile void SAMD_CRC32::software_crc32(const void *data, size_t n_bytes, uint32_t *crc)
{
    _using_hardware_crc = false;
    static uint32_t table[0x100];
    if (!*table)
        for (size_t i = 0; i < 0x100; ++i)
            table[i] = crc32_for_byte(i);
    for (size_t i = 0; i < n_bytes; ++i)
        *crc = table[(uint8_t)*crc ^ ((uint8_t *)data)[i]] ^ *crc >> 8;
}

uint32_t SAMD_CRC32::crc32_for_byte(uint32_t r)
{
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}