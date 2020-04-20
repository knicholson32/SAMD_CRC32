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

#include <SAMD_CRC32.h>

#if defined(_SAMD_INCLUDED_) && defined(REV_DSU)
#define HARDWARE_CRC32
#endif

#if !defined(HARDWARE_CRC32) && defined(SAMD_CRC32_NO_SOFTWARE_CRC)
#error "Software CRC disabled by precompiler directive, but no hardware CRC support exists for this device. Remove 'SAMD_CRC32_NO_SOFTWARE_CRC' directive."
#endif

bool SAMD_CRC32::_dsu_in_use = false;

SAMD_CRC32::SAMD_CRC32()
{
#if defined(_SAMD_INCLUDED_) && defined(REV_DSU)
    uint32_t rev = DSU->DID.bit.REVISION;  // Get DSU Device Information to assess whether or not this chip
                                           // has the die issue that requires correcting when using hardware
                                           // crc32 on some SAMD21 devices.
    _hardware_crc_errata = (rev == 0x00 || rev == 0x01 || rev == 0x02 || rev == 0x03) && DSU->DID.bit.PROCESSOR == 0x1 && DSU->DID.bit.FAMILY == 0x0;
    _nvm_cache_errata = false;  // TODO: Assess how to detect NVM Cache Errata when using the SAMX5X Series Chips
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
    _using_hardware_crc = true;
#endif
#else
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
    _using_hardware_crc = false;
#endif
#endif
#ifndef SAMD_CRC32_NO_STATUS
    _hardware_status_code = OK;
#endif
}

bool SAMD_CRC32::can_use_hardware_crc32()
{
#if defined(_SAMD_INCLUDED_) && defined(REV_DSU)
    return true;
#else
    return false;
#endif
}

#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
bool SAMD_CRC32::check_used_hardware_crc32()
{
    return _using_hardware_crc;
}

void SAMD_CRC32::force_use_software_crc32(bool val)
{
    SAMD_CRC32::_force_software_crc = val;
};
#endif

#ifndef SAMD_CRC32_NO_STATUS
const char *SAMD_CRC32::get_hardware_status_msg()
{
    return hardware_crc_status_str[_hardware_status_code];
}

uint8_t SAMD_CRC32::get_hardware_status_code()
{
    return _hardware_status_code;
}

const char *SAMD_CRC32::decode_hardware_status_code(uint8_t code)
{
    return hardware_crc_status_str[code];
}
#endif

#if defined(_SAMD_INCLUDED_) && defined(REV_DSU)
    /*************** SAMD Hardware ***************/
    volatile uint8_t SAMD_CRC32::crc32(const void *data, size_t n_bytes, uint32_t *crc)
    {
        if(_dsu_in_use){
        }
        // If the size if the data is not word-aligned (multiple of 32 bits, 4 bytes), the hardware CRC can't process it.
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
        if (_force_software_crc || _dsu_in_use || n_bytes < 4 || (n_bytes & 3) != 0)
#else
        if (_dsu_in_use || n_bytes < 4 || (n_bytes & 3) != 0)
#endif
        {
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
            _using_hardware_crc = false;
            software_crc32(data, n_bytes, crc);
#endif
#ifndef SAMD_CRC32_NO_STATUS
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
            if (_force_software_crc){
                _hardware_status_code = USER_FORCED_SOFTWARE;
                return _hardware_status_code;
            }else if(_dsu_in_use){
                _hardware_status_code = HARDWARE_CRC32_IN_USE;
                return HARDWARE_CRC32_IN_USE;
            }else{
                _hardware_status_code = NOT_WORD_ALIGNED;
                return _hardware_status_code;
            }
#else
            if(_dsu_in_use){
                _hardware_status_code = HARDWARE_CRC32_IN_USE;
                return HARDWARE_CRC32_IN_USE;
            }else{
                _hardware_status_code = NOT_WORD_ALIGNED;
                return _hardware_status_code;
            }
#endif
#else
            return 1; // With no status codes, 1 means that the CRC was not calculated in hardware.
#endif
        }
        else
        {
            _dsu_in_use = true;
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
            _using_hardware_crc = true;
#endif
            uint32_t address = (uint32_t)data;
            bool errata = _hardware_crc_errata && (address >= 0x20000000); // Start address of SRAM in SAMD chips
            if (errata)
            {
                volatile unsigned int *addr = (volatile unsigned int *)0x41007058;
                *addr &= ~0x30000UL;
                // TODO: Enable NVM Caching on certain errata SAMX5X Chips (see _nvm_cache_errata)
            }
            bool AHB_DSU;
            bool APB_DSU;
#ifdef __SAMD51__   /* Using SAMD51 */
            AHB_DSU = MCLK->AHBMASK.reg & MCLK_AHBMASK_DSU;  // Check if the DSU clock is active in the AHBMASK
            APB_DSU = MCLK->APBBMASK.reg & MCLK_APBBMASK_DSU;  // Check if the DSU clock is active in the APBBMASK
            MCLK->AHBMASK.reg |= MCLK_AHBMASK_DSU;    // Enable AHB DSU Clock Domain
            MCLK->APBBMASK.reg |= MCLK_APBBMASK_DSU;  // Enable APB DSU Clock Domain
            if (PAC->STATUSB.reg & 0x02)              // Check if DSU write protection is enabled
                PAC->WRCTRL.reg = PAC_WRCTRL_KEY_CLR_Val | PAC_WRCTRL_PERID(33);  // Removes DSU write protection (allowing access to internal reg.s)
                                                                                  // TODO: Check PERID for the DSU (Bridge B, second peripheral; 32+1)
#else  /* Using SAMD21 */
            AHB_DSU = PM->AHBMASK.reg & PM_AHBMASK_DSU;
            APB_DSU = PM->APBBMASK.reg & PM_APBBMASK_DSU;
            PM->AHBMASK.reg |= PM_AHBMASK_DSU;   // Enable APB DSU Clock Domain
            PM->APBBMASK.reg |= PM_APBBMASK_DSU; // Enable AHB DSU Clock Domain
            if (PAC1->WPCLR.reg & 0x02) // Check if DSU write protection is enabled
                PAC1->WPCLR.reg = 0x02; // b00000000000000000000000000000010 -> Removes DSU write protection (allowing access to internal reg.s)
#endif
            DSU->DATA.reg = 0xFFFFFFFFUL;        // Sets starting CRC data
            DSU->ADDR.reg = DSU_ADDR_ADDR(address >> 2);
            DSU->LENGTH.bit.LENGTH = n_bytes >> 2; // Divide length by four for word alignment
            DSU->STATUSA.bit.BERR;                 // Clear the error flag
            DSU->STATUSA.bit.DONE = 1;             // Clear the done flag
            DSU->CTRL.bit.CRC = 1;                 // Start the CRC operation

            while (!DSU->STATUSA.bit.DONE);


            // Restore the DSU clock status to what it was before the CRC
#ifdef __SAMD51__   /* Using SAMD51 */
            if(!AHB_DSU)
                MCLK->AHBMASK.reg &= ~MCLK_AHBMASK_DSU;
            if(!APB_DSU)
                MCLK->APBBMASK.reg &= ~MCLK_APBBMASK_DSU;
#else  /* Using SAMD21 */
            if(!AHB_DSU)
                PM->AHBMASK.reg &= ~PM_AHBMASK_DSU;
            if(!APB_DSU)
                PM->APBBMASK.reg &= ~PM_APBBMASK_DSU;
#endif


            _dsu_in_use = false;

            if (DSU->STATUSA.bit.BERR)
            {
#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
                _using_hardware_crc = false;
                software_crc32(data, n_bytes, crc);
#endif
#ifndef SAMD_CRC32_NO_STATUS
                _hardware_status_code = BUS_ERROR;
                return _hardware_status_code;
#else
                return 1;  // With no status codes, 1 means that the CRC was not calculated in hardware.
#endif
            }

            *crc = (DSU->DATA.reg) ^ 0xFFFFFFFF;

            if (errata)
            {
                volatile unsigned int *addr = (volatile unsigned int *)0x41007058;
                *addr |= 0x20000UL;
            }

#ifndef SAMD_CRC32_NO_STATUS
            _hardware_status_code = OK;
            return _hardware_status_code;
#else
            return 0;  // With no status codes, 0 means that the CRC was calculated in hardware.
#endif
        }
    }
#else
    /************* Non SAMD Hardware *************/
    volatile uint8_t SAMD_CRC32::crc32(const void *data, size_t n_bytes, uint32_t *crc)
    {
        software_crc32(data, n_bytes, crc);
#ifndef SAMD_CRC32_NO_STATUS
        _hardware_status_code = HARDWARE_NOT_SUPPORTED;
        return _hardware_status_code;
#else
        return 1;  // With no status codes, 1 means that the CRC was not calculated in hardware.
#endif
    }
#endif

#ifndef SAMD_CRC32_NO_SOFTWARE_CRC
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
#endif