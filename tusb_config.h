/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2024 Matthew Bennett
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <lwip/opt.h>

// Pico USB device roothub port number
#ifndef PICO_TUD_RHPORT
#define PICO_TUD_RHPORT 0
#endif

//--------------------------------------------------------------------
// Common Configuration
//--------------------------------------------------------------------

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG 0
#endif

// Enable Device stack
#define CFG_TUD_ENABLED 1

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED OPT_MODE_DEFAULT_SPEED

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

// set ECM (RNDIS) or NCM
#define USE_ECM 0
// #define INCLUDE_IPERF

//--------------------------------------------------------------------
// NCM CLASS CONFIGURATION, SEE "ncm.h" FOR PERFORMANCE TUNING
//--------------------------------------------------------------------

// Must be >> MTU
// Can be set to 2048 without impact
#define CFG_TUD_NCM_IN_NTB_MAX_SIZE (2 * TCP_MSS + 100)

// Must be >> MTU
// Can be set to smaller values if wNtbOutMaxDatagrams==1
#define CFG_TUD_NCM_OUT_NTB_MAX_SIZE (2 * TCP_MSS + 100)

// Number of NCM transfer blocks for reception side
#ifndef CFG_TUD_NCM_OUT_NTB_N
#define CFG_TUD_NCM_OUT_NTB_N 1
#endif

// Number of NCM transfer blocks for transmission side
#ifndef CFG_TUD_NCM_IN_NTB_N
#define CFG_TUD_NCM_IN_NTB_N 1
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

//------------- CLASS -------------//

// Network class has 2 drivers: ECM/RNDIS and NCM.
// Only one of the drivers can be enabled
#define CFG_TUD_ECM_RNDIS USE_ECM
#define CFG_TUD_NCM (1 - CFG_TUD_ECM_RNDIS)

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
