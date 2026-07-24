/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
 * This file is part of the TinyUSB stack.
 */

/******************************************************************************
*
* Copyright (C) 2025 Texas Instruments Incorporated - http://www.ti.com/
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*  Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*
*  Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the
*  distribution.
*
*  Neither the name of Texas Instruments Incorporated nor the names of
*  its contributors may be used to endorse or promote products derived
*  from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************/

#ifndef TUSB_MUSB_TYPE_H_
#define TUSB_MUSB_TYPE_H_

#include "stdint.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __IO
  #define __IO volatile
#endif

#ifndef __I
  #define __I  volatile const
#endif

#ifndef __O
  #define __O  volatile
#endif

#ifndef __R
  #define __R  volatile const
#endif

/* Control/Status registers for endpoint selected by index register */
typedef struct TU_ATTR_PACKED {
  /* Maximum packet size for IN endpoint (index register to select EP 1-15) */
  __IO  uint8_t tx_max_ep;                            // 0x10: InMaxP
      
  /* 
   * Address space 0x11 is shared between CSR0 and
   * InCSR1, depending the selected endpoint configured
   * through the INDEX register, endpoint 0 will be seen
   * or endpoint 1-15.
   */
  union {
    /* Control/status register for endpoint 0 */
    __IO  uint8_t csr0;                               // 0x11: CSR0
    struct {
      __I   uint8_t rx_packet_ready : 1;              // [0]: OutPktRdy
      __IO  uint8_t tx_packet_ready : 1;              // [1]: InPktRdy
      __IO  uint8_t sent_stall      : 1;              // [2]: SentStall
      __O   uint8_t data_end        : 1;              // [3]: DataEnd
      __I   uint8_t setup_end       : 1;              // [4]: SetupEnd
      __O   uint8_t send_stall      : 1;              // [5]: SendStall
      __O   uint8_t service_rx_pack : 1;              // [6]: ServicedOutPktRdy
      __O   uint8_t service_tx_pack : 1;              // [7]: ServicedSetupEnd
    } csr0_bits;

    /* Control/status register for IN endpoint (index register to select EP 1-15) */
    __IO uint8_t tx_csr1;                             // 0x11: InCSR1
    struct {
      __IO  uint8_t tx_packet_ready : 1;              // [0]: InPktRdy
      __IO  uint8_t fifo_not_empty  : 1;              // [1]: FIFONotEmpty
      __IO  uint8_t under_run       : 1;              // [2]: UnderRun
      __O   uint8_t flush_fifo      : 1;              // [3]: FlushFIFO
      __IO  uint8_t send_stall      : 1;              // [4]: SendStall
      __IO  uint8_t sent_stall      : 1;              // [5]: SentStall
      __O   uint8_t clear_data_tog  : 1;              // [6]: ClrDataTog
      __R   uint8_t csr1_resv       : 1;              // [7]: Reserved
      } tx_csr1_bits;
    };

    /* Control/Status register 2 for IN endpoint (index register to select EP 1-15)*/
    union {
      __IO  uint8_t tx_csr2;                          // 0x12: InCSR2
      struct {
        __R   uint8_t tx_csr2_resv    : 3;            // [0,2]: Reserved
        __IO  uint8_t force_data_tog  : 1;            // [3]: FrcDataTog
        __IO  uint8_t dma_enable      : 1;            // [4]: DMAEnab
        __IO  uint8_t mode            : 1;            // [5]: Mode
        __IO  uint8_t iso             : 1;            // [6]: ISO
        __IO  uint8_t auto_set        : 1;            // [7]: AutoSet
      } tx_csr2_bits;
    };

    /* Maximum packet size for OUT endpoint (index register to select EP 1-15) */
    __IO  uint8_t rx_max_ep;                          // 0x13: OutMaxP

    /* Control/status register 1 for OUT endpoint (index register to select EP 1-15) */
    union {
      __IO  uint8_t rx_csr1;                          // 0x14: OutCSR1
      struct {
        __IO  uint8_t rx_packet_ready : 1;            // [0]: OutPktRdy
        __I   uint8_t fifo_full       : 1;            // [1]: FIFOFull
        __IO  uint8_t over_run        : 1;            // [2]: OverRun
        __I   uint8_t data_error      : 1;            // [3]: DataError
        __O   uint8_t flush_fifo      : 1;            // [4]: FlushFIFO
        __IO  uint8_t send_stall      : 1;            // [5]: SendStall
        __IO  uint8_t sent_stall      : 1;            // [6]: SentStall
        __O   uint8_t clear_data_tog  : 1;            // [7]: ClrDataTog
        } rx_csr1_bits;
      };

    /* Control/status register 2 for OUT endpoint (index register to select EP 1-15) */
    union {
      __IO  uint8_t rx_csr2;                          // 0x15: OutCSR2
      struct {
        __R   uint8_t rx_csr2_resv    : 4;            // [0,3]: Reserved
        __IO  uint8_t dma_mode        : 1;            // [3]: DMAMode
        __IO  uint8_t dma_enable      : 1;            // [4]: DMAEnab
        __IO  uint8_t iso             : 1;            // [6]: ISO
        __IO  uint8_t auto_clear      : 1;            // [7]: AutoClear
      } rx_csr2_bits;
    };

    /* 
     * EP 0: Number of received bytes in EP 0 FIFO
     * EP 1-15: Number of bytes in OUT EP FIFO (lower byte)
     */
    union {
      __I   uint8_t count0;                           // 0x16: Count0
      __I   uint8_t rx_count_lower;                   // 0x16: OutCount1
    };

    /* Number of bytes in OUT EP FIFO (upper byte) */
    __I   uint8_t rx_count_upper:3;                     // 0x17: OutCount2
} musb_ep_csr_t;

/* Struct starts from address 0x10 and ends at 0x17, check that it is size of 0x08 */
TU_VERIFY_STATIC(sizeof(musb_ep_csr_t) == 0x08, "size is not correct");

typedef struct TU_ATTR_PACKED {
  //------------- Common -------------//
  /* Function address register */
  __IO  uint8_t  faddr;                                // 0x00: FADDR

  /* Power management register */
  union {
    __IO  uint8_t  power;                              // 0x01: POWER
    struct {
      __IO  uint8_t suspend_mode_en : 1;               // [0] SUSPEND Mode Enable
      __IO  uint8_t suspend_mode    : 1;               // [1] SUSPEND Mode
      __IO  uint8_t resume_mode     : 1;               // [2] RESUME
      __IO  uint8_t reset           : 1;               // [3] RESET
      __R   uint8_t pwr_reserved0   : 1;               // [4] RESERVED
      __R   uint8_t pwr_reserved1   : 1;               // [5] RESERVED
      __IO  uint8_t soft_conn       : 1;               // [6] Soft Connect/Disconnect <- TI specific bit
      __IO  uint8_t iso_update      : 1;               // [7] Isochronous Update
    } power_bit;
  };

  /* 
   * Interrupt register for Endpoint 0 plus IN endpoints 1 to 7 and
   * endpoints 8 to 15. (Only present if more than 7 IN endpoints
   * are included)
   */
  union {
    struct {
      __O   uint8_t intr_tx1;                          // 0x02: IntrIn1
      __O   uint8_t intr_tx2;                          // 0x03: IntrIn2
    };
    __O   uint8_t intr_txep[2];                        // 0x02-0x03: Interrupt IN EP 0-15
  };

  /* 
   * Interrupt register for Endpoint 0 plus OUT endpoints 1 to 7 and
   * endpoints 8 to 15. (Only present if more than 7 OUT endpoints
   * are included)
   */
  union {
    struct {
      __O   uint8_t intr_rx1;                           // 0x04: IntrOut1
      __O   uint8_t intr_rx2;                           // 0x05: IntrOut2
    };
    __O   uint8_t intr_rxep[2];                         // 0x04-0x05: Interrupt OUT EP 0-15
  };

  /* Interrupt register for common USB interrupts */
  __IO  uint8_t intr_usb;                               // 0x06: IntrUSB

  /*
   * Interrupt enable register for IN EP 0-15, IntrIn2E is only present
   * if there are more than 7 IN endpoints on the device. Otherwise
   * it is reserved.
   */
  union {
    struct {
      __IO  uint8_t intr_txen1;                         // 0x07: IntrIn1E
      __IO  uint8_t intr_txen2;                         // 0x08: IntrIn2E
    };
    __IO  uint8_t intr_txen[2];                         // 0x07-0x08: Interrupt IN enable EP 0-15
  };

  /*
   * Interrupt enable register for OUT EP 0-15, IntrOut2E is only present
   * if there are more than 7 IN endpoints on the device. Otherwise
   * it is reserved.
   */
  union {
    struct {
      __IO  uint8_t intr_rxen1;                         // 0x09: IntrOut1E
      __IO  uint8_t intr_rxen2;                         // 0x0A: IntrOut2E
    };
    __IO  uint8_t intr_rxen[2];                         // 0x09-0x0A: Interrupt IN enable EP 0-15
  };

  /* Interrupt enable register for IntrUSB */
  __IO  uint8_t intr_usben;                             // 0x0B: IntrUSBE

  /*
   * Frame 1 contains bits 0 to 7 and frame 2
   * contains bits 8 to 10 of the frame.
   */
  struct {
    __IO  uint8_t lower;                                // 0x0C: Frame1
    __IO  uint8_t upper;                                // 0x0D: Frame2
  }frame;

  /* 
   * Index registers to select which endpoint status/control 
   * registers to use.
   */
  __IO  uint8_t index;                                  // 0x0E: Index
  __R   uint8_t rsv_0x0f;                               // 0x0F: Unused, always returns 0

  //---------- Endpoint CSR (indexed) ----------//
  musb_ep_csr_t indexed_csr;                            // 0x10-0x17: Indexed CSR 0-15
  __R   uint32_t rsv_0x18_0x1f[2];                      // 0x18-0x1F: Reserved

  //------------- FIFOs -------------//
  TU_ATTR_ALIGNED(1) __IO uint8_t fifo[16];             // 0x20-0x2F: FIFO 0-15
} musb_regs_t;

/* Struct starts from address 0x00 and ends at 0x2F, check that it is size of 0x30 */
TU_VERIFY_STATIC(sizeof(musb_regs_t) == 0x30, "size is not correct");

//--------------------------------------------------------------------+
// Helper
//--------------------------------------------------------------------+

/* Set index register based off endpoint number and return control/status register */
TU_ATTR_ALWAYS_INLINE static inline musb_ep_csr_t* get_ep_csr(musb_regs_t* musb_regs, unsigned epnum) {
  musb_regs->index = epnum;
  return &musb_regs->indexed_csr;
}

/* Grab and return frame number */
TU_ATTR_ALWAYS_INLINE static inline uint16_t get_frame(musb_regs_t* musb_regs) {
  return ((musb_regs->frame.upper << 8) | musb_regs->frame.lower);
}

/* Set index register based off endpoint number and return number of bytes in receive packet */
TU_ATTR_ALWAYS_INLINE static inline uint16_t get_rx_count(musb_regs_t* musb_regs, unsigned epnum) {
  /* Set index to endpoint we wish to receive RX count */
  musb_regs->index = epnum;
  /* If packet is not ready then return nothing, if ready return count */
  if (!musb_regs->indexed_csr.rx_csr1_bits.rx_packet_ready)
    return 0;
  else
    return ((musb_regs->indexed_csr.rx_count_upper << 8) | musb_regs->indexed_csr.rx_count_lower);
}

/* 
 * Return current interrupt status for TX/RX/Common USB interrupts done this
 * way to put less burden on CPU.
 */
#if (TUP_DCD_ENDPOINT_MAX <= 8)
TU_ATTR_ALWAYS_INLINE static inline uint_fast8_t get_intr_tx_status(musb_regs_t* musb_regs)
{
  /* Read interrupt for IN endpoints, upon read interrupt status will be cleared */
  return (musb_regs->intr_tx1);
}

TU_ATTR_ALWAYS_INLINE static inline uint_fast8_t get_intr_rx_status(musb_regs_t* musb_regs)
{
  /* Read interrupt for OUT endpoints, upon read interrupt status will be cleared */
  return (musb_regs->intr_rx1);
}
#else
TU_ATTR_ALWAYS_INLINE static inline uint_fast16_t get_intr_tx_status(musb_regs_t* musb_regs)
{
  /* Read interrupt for IN endpoints, upon read interrupt status will be cleared */
  return ((musb_regs->intr_tx2 << 8) | musb_regs->intr_tx1);
}

TU_ATTR_ALWAYS_INLINE static inline uint_fast16_t get_intr_rx_status(musb_regs_t* musb_regs)
{
  /* Read interrupt for OUT endpoints, upon read interrupt status will be cleared */
  return ((musb_regs->intr_rx2 << 8) | musb_regs->intr_rx1);
}
#endif

//--------------------------------------------------------------------+
// Register Bit Field
//--------------------------------------------------------------------+

// 0x01: Power
#define MUSB_POWER_ISOUP         0x0080  // Isochronous Update
#define MUSB_POWER_SOFTCONN      0x0040  // Soft Connect/Disconnect
#define MUSB_POWER_HSENAB        0x0020  // High Speed Enable
#define MUSB_POWER_HSMODE        0x0010  // High Speed Enable
#define MUSB_POWER_RESET         0x0008  // RESET Signaling
#define MUSB_POWER_RESUME        0x0004  // RESUME Signaling
#define MUSB_POWER_SUSPEND       0x0002  // SUSPEND Mode
#define MUSB_POWER_PWRDNPHY      0x0001  // Power Down PHY

// Interrupt TX/RX Status and Enable: each bit is for an endpoint

// 0x6c: HWVERS
#define MUSB_HWVERS_RC_SHIFT    15
#define MUSB_HWVERS_RC_MASK     0x8000
#define MUSB_HWVERS_MAJOR_SHIFT 10
#define MUSB_HWVERS_MAJOR_MASK  0x7C00
#define MUSB_HWVERS_MINOR_SHIFT 0
#define MUSB_HWVERS_MINOR_MASK  0x03FF

// TX/RXCSR1
#define MUSB_CSR1_PACKET_READY(_rx)      (1u << 0)
#define MUSB_CSR1_FLUSH_FIFO(_rx)        (1u << ((_rx) ? 4 : 3))
#define MUSB_CSR1_SEND_STALL(_rx)        (1u << ((_rx) ? 5 : 4))
#define MUSB_CSR1_STALLED(_rx)           (1u << ((_rx) ? 6 : 5))
#define MUSB_CSR1_CLEAR_DATA_TOGGLE(_rx) (1u << ((_rx) ? 7 : 6))

// 0x13, 0x17: TX/RX CSRH
#define MUSB_CSRH_DISABLE_DOUBLE_PACKET(_rx) (1u << 1)
#define MUSB_CSRH_TX_MODE                    (1u << 5) // 1 = TX, 0 = RX. only relevant for SHARED FIFO
#define MUSB_CSRH_ISO                        (1u << 6)

// 0x62, 0x63: TXFIFO_SZ, RXFIFO_SZ
#define MUSB_FIFOSZ_DOUBLE_PACKET            (1u << 4)


//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_IS register.
//
//*****************************************************************************
#define MUSB_IS_VBUSERR          0x0080  // VBUS Error (OTG only)
#define MUSB_IS_SESREQ           0x0040  // SESSION REQUEST (OTG only)
#define MUSB_IS_DISCON           0x0020  // Session Disconnect (OTG only)
#define MUSB_IS_CONN             0x0010  // Session Connect
#define MUSB_IS_SOF              0x0008  // Start of Frame
#define MUSB_IS_BABBLE           0x0004  // Babble Detected
#define MUSB_IS_RESET            0x0004  // RESET Signaling Detected
#define MUSB_IS_RESUME           0x0002  // RESUME Signaling Detected
#define MUSB_IS_SUSPEND          0x0001  // SUSPEND Signaling Detected

//*****************************************************************************
//
// The following are defines for the bit fields in the INTRUSB register.
//
//*****************************************************************************
#define MUSB_IE_SOF              0x0008  // Enable Start-of-Frame Interrupt
#define MUSB_IE_RESET            0x0004  // Enable RESET Interrupt
#define MUSB_IE_RESUME           0x0002  // Enable RESUME Interrupt
#define MUSB_IE_SUSPND           0x0001  // Enable SUSPEND Interrupt

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_FRAME register.
//
//*****************************************************************************
#define MUSB_FRAME_M             0x07FF  // Frame Number
#define MUSB_FRAME_S             0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_TEST register.
//
//*****************************************************************************
#define MUSB_TEST_FORCEH         0x0080  // Force Host Mode
#define MUSB_TEST_FIFOACC        0x0040  // FIFO Access
#define MUSB_TEST_FORCEFS        0x0020  // Force Full-Speed Mode
#define MUSB_TEST_FORCEHS        0x0010  // Force High-Speed Mode
#define MUSB_TEST_TESTPKT        0x0008  // Test Packet Mode Enable
#define MUSB_TEST_TESTK          0x0004  // Test_K Mode Enable
#define MUSB_TEST_TESTJ          0x0002  // Test_J Mode Enable
#define MUSB_TEST_TESTSE0NAK     0x0001  // Test_SE0_NAK Test Mode Enable

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_DEVCTL register.
//
//*****************************************************************************
#define MUSB_DEVCTL_DEV          0x0080  // Device Mode (OTG only)
#define MUSB_DEVCTL_FSDEV        0x0040  // Full-Speed Device Detected
#define MUSB_DEVCTL_LSDEV        0x0020  // Low-Speed Device Detected
#define MUSB_DEVCTL_VBUS_M       0x0018  // VBUS Level (OTG only)
#define MUSB_DEVCTL_VBUS_NONE    0x0000  // Below SessionEnd
#define MUSB_DEVCTL_VBUS_SEND    0x0008  // Above SessionEnd, below AValid
#define MUSB_DEVCTL_VBUS_AVALID  0x0010  // Above AValid, below VBUSValid
#define MUSB_DEVCTL_VBUS_VALID   0x0018  // Above VBUSValid
#define MUSB_DEVCTL_HOST         0x0004  // Host Mode
#define MUSB_DEVCTL_HOSTREQ      0x0002  // Host Request (OTG only)
#define MUSB_DEVCTL_SESSION      0x0001  // Session Start/End (OTG only)

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_CCONF register.
//
//*****************************************************************************
#define MUSB_CCONF_TXEDMA        0x0002  // TX Early DMA Enable
#define MUSB_CCONF_RXEDMA        0x0001  // TX Early DMA Enable

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_ULPIVBUSCTL
// register.
//
//*****************************************************************************
#define MUSB_ULPIVBUSCTL_USEEXTVBUSIND  0x0002  // Use External VBUS Indicator
#define MUSB_ULPIVBUSCTL_USEEXTVBUS     0x0001  // Use External VBUS

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_ULPIREGDATA
// register.
//
//*****************************************************************************
#define MUSB_ULPIREGDATA_REGDATA_M      0x00FF  // Register Data
#define MUSB_ULPIREGDATA_REGDATA_S      0
//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_ULPIREGADDR
// register.
//
//*****************************************************************************
#define MUSB_ULPIREGADDR_ADDR_M  0x00FF  // Register Address
#define MUSB_ULPIREGADDR_ADDR_S  0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_ULPIREGCTL
// register.
//
//*****************************************************************************
#define MUSB_ULPIREGCTL_RDWR     0x0004  // Read/Write Control
#define MUSB_ULPIREGCTL_REGCMPLT 0x0002  // Register Access Complete
#define MUSB_ULPIREGCTL_REGACC   0x0001  // Initiate Register Access

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_EPINFO register.
//
//*****************************************************************************
#define MUSB_EPINFO_RXEP_M       0x00F0  // RX Endpoints
#define MUSB_EPINFO_TXEP_M       0x000F  // TX Endpoints
#define MUSB_EPINFO_RXEP_S       4
#define MUSB_EPINFO_TXEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_RAMINFO register.
//
//*****************************************************************************
#define MUSB_RAMINFO_DMACHAN_M   0x00F0  // DMA Channels
#define MUSB_RAMINFO_RAMBITS_M   0x000F  // RAM Address Bus Width
#define MUSB_RAMINFO_DMACHAN_S   4
#define MUSB_RAMINFO_RAMBITS_S   0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_CONTIM register.
//
//*****************************************************************************
#define MUSB_CONTIM_WTCON_M      0x00F0  // Connect Wait
#define MUSB_CONTIM_WTID_M       0x000F  // Wait ID
#define MUSB_CONTIM_WTCON_S      4
#define MUSB_CONTIM_WTID_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_VPLEN register.
//
//*****************************************************************************
#define MUSB_VPLEN_VPLEN_M       0x00FF  // VBUS Pulse Length
#define MUSB_VPLEN_VPLEN_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_HSEOF register.
//
//*****************************************************************************
#define MUSB_HSEOF_HSEOFG_M      0x00FF  // HIgh-Speed End-of-Frame Gap
#define MUSB_HSEOF_HSEOFG_S      0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_FSEOF register.
//
//*****************************************************************************
#define MUSB_FSEOF_FSEOFG_M      0x00FF  // Full-Speed End-of-Frame Gap
#define MUSB_FSEOF_FSEOFG_S      0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_LSEOF register.
//
//*****************************************************************************
#define MUSB_LSEOF_LSEOFG_M      0x00FF  // Low-Speed End-of-Frame Gap
#define MUSB_LSEOF_LSEOFG_S      0

//*****************************************************************************
//
// The following are defines for the bit fields in the CSR0 register.
//
//*****************************************************************************
#define MUSB_CSR0_SETENDC       0x0080  // Setup End Clear / ServicedSetupEnd
#define MUSB_CSR0_RXPKTRDYC     0x0040  // RXRDY Clear / ServicedOutPktRdy
#define MUSB_CSR0_SEND_STALL    0x0020  // Send Stall
#define MUSB_CSR0_SETEND        0x0010  // Setup End
#define MUSB_CSR0_DATAEND       0x0008  // Data End
#define MUSB_CSR0_SENT_STALLED  0x0004  // Endpoint Stalled / SentStall
#define MUSB_CSR0_TXRDY         0x0002  // Transmit Packet Ready / InPktRdy
#define MUSB_CSR0_RXRDY         0x0001  // Receive Packet Ready / OutPktRdy

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_CSRH0 register.
//
//*****************************************************************************
#define MUSB_CSRH0_DISPING       0x0008  // PING Disable
#define MUSB_CSRH0_DTWE          0x0004  // Data Toggle Write Enable
#define MUSB_CSRH0_DT            0x0002  // Data Toggle
#define MUSB_CSRH0_FLUSH         0x0001  // Flush FIFO

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_TYPE0 register.
//
//*****************************************************************************
#define MUSB_TYPE0_SPEED_M       0x00C0  // Operating Speed
#define MUSB_TYPE0_SPEED_HIGH    0x0040  // High
#define MUSB_TYPE0_SPEED_FULL    0x0080  // Full
#define MUSB_TYPE0_SPEED_LOW     0x00C0  // Low

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_NAKLMT register.
//
//*****************************************************************************
#define MUSB_NAKLMT_NAKLMT_M     0x001F  // EP0 NAK Limit
#define MUSB_NAKLMT_NAKLMT_S     0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_TXCSRL1 register.
//
//*****************************************************************************
#define MUSB_TXCSRL1_NAKTO       0x0080  // NAK Timeout
#define MUSB_TXCSRL1_CLRDT       0x0040  // Clear Data Toggle
#define MUSB_TXCSRL1_STALLED     0x0020  // Endpoint Stalled
#define MUSB_TXCSRL1_STALL       0x0010  // Send STALL
#define MUSB_TXCSRL1_SETUP       0x0010  // Setup Packet
#define MUSB_TXCSRL1_FLUSH       0x0008  // Flush FIFO
#define MUSB_TXCSRL1_ERROR       0x0004  // Error
#define MUSB_TXCSRL1_UNDRN       0x0004  // Underrun
#define MUSB_TXCSRL1_FIFONE      0x0002  // FIFO Not Empty
#define MUSB_TXCSRL1_TXRDY       0x0001  // Transmit Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_TXCSRH1 register.
//
//*****************************************************************************
#define MUSB_TXCSRH1_AUTOSET     0x0080  // Auto Set
#define MUSB_TXCSRH1_ISO         0x0040  // Isochronous Transfers
#define MUSB_TXCSRH1_MODE        0x0020  // Mode
#define MUSB_TXCSRH1_DMAEN       0x0010  // DMA Request Enable
#define MUSB_TXCSRH1_FDT         0x0008  // Force Data Toggle
#define MUSB_TXCSRH1_DMAMOD      0x0004  // DMA Request Mode
#define MUSB_TXCSRH1_DTWE        0x0002  // Data Toggle Write Enable
#define MUSB_TXCSRH1_DT          0x0001  // Data Toggle

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_RXCSRL1 register.
//
//*****************************************************************************
#define MUSB_RXCSRL1_CLRDT       0x0080  // Clear Data Toggle
#define MUSB_RXCSRL1_STALLED     0x0040  // Endpoint Stalled
#define MUSB_RXCSRL1_STALL       0x0020  // Send STALL
#define MUSB_RXCSRL1_REQPKT      0x0020  // Request Packet
#define MUSB_RXCSRL1_FLUSH       0x0010  // Flush FIFO
#define MUSB_RXCSRL1_DATAERR     0x0008  // Data Error
#define MUSB_RXCSRL1_NAKTO       0x0008  // NAK Timeout
#define MUSB_RXCSRL1_OVER        0x0004  // Overrun
#define MUSB_RXCSRL1_ERROR       0x0004  // Error
#define MUSB_RXCSRL1_FULL        0x0002  // FIFO Full
#define MUSB_RXCSRL1_RXRDY       0x0001  // Receive Packet Ready

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_RXCSRH1 register.
//
//*****************************************************************************
#define MUSB_RXCSRH1_AUTOCL      0x0080  // Auto Clear
#define MUSB_RXCSRH1_AUTORQ      0x0040  // Auto Request
#define MUSB_RXCSRH1_ISO         0x0040  // Isochronous Transfers
#define MUSB_RXCSRH1_DMAEN       0x0020  // DMA Request Enable
#define MUSB_RXCSRH1_DISNYET     0x0010  // Disable NYET
#define MUSB_RXCSRH1_PIDERR      0x0010  // PID Error
#define MUSB_RXCSRH1_DMAMOD      0x0008  // DMA Request Mode
#define MUSB_RXCSRH1_DTWE        0x0004  // Data Toggle Write Enable
#define MUSB_RXCSRH1_DT          0x0002  // Data Toggle
#define MUSB_RXCSRH1_INCOMPRX    0x0001  // Incomplete RX Transmission Status

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_TXTYPE1 register.
//
//*****************************************************************************
#define MUSB_TXTYPE1_SPEED_M     0x00C0  // Operating Speed
#define MUSB_TXTYPE1_SPEED_DFLT  0x0000  // Default
#define MUSB_TXTYPE1_SPEED_HIGH  0x0040  // High
#define MUSB_TXTYPE1_SPEED_FULL  0x0080  // Full
#define MUSB_TXTYPE1_SPEED_LOW   0x00C0  // Low
#define MUSB_TXTYPE1_PROTO_M     0x0030  // Protocol
#define MUSB_TXTYPE1_PROTO_CTRL  0x0000  // Control
#define MUSB_TXTYPE1_PROTO_ISOC  0x0010  // Isochronous
#define MUSB_TXTYPE1_PROTO_BULK  0x0020  // Bulk
#define MUSB_TXTYPE1_PROTO_INT   0x0030  // Interrupt
#define MUSB_TXTYPE1_TEP_M       0x000F  // Target Endpoint Number
#define MUSB_TXTYPE1_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_TXINTERVAL1
// register.
//
//*****************************************************************************
#define MUSB_TXINTERVAL1_NAKLMT_M 0x00FF  // NAK Limit
#define MUSB_TXINTERVAL1_TXPOLL_M 0x00FF  // TX Polling
#define MUSB_TXINTERVAL1_TXPOLL_S 0
#define MUSB_TXINTERVAL1_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_RXTYPE1 register.
//
//*****************************************************************************
#define MUSB_RXTYPE1_SPEED_M     0x00C0  // Operating Speed
#define MUSB_RXTYPE1_SPEED_DFLT  0x0000  // Default
#define MUSB_RXTYPE1_SPEED_HIGH  0x0040  // High
#define MUSB_RXTYPE1_SPEED_FULL  0x0080  // Full
#define MUSB_RXTYPE1_SPEED_LOW   0x00C0  // Low
#define MUSB_RXTYPE1_PROTO_M     0x0030  // Protocol
#define MUSB_RXTYPE1_PROTO_CTRL  0x0000  // Control
#define MUSB_RXTYPE1_PROTO_ISOC  0x0010  // Isochronous
#define MUSB_RXTYPE1_PROTO_BULK  0x0020  // Bulk
#define MUSB_RXTYPE1_PROTO_INT   0x0030  // Interrupt
#define MUSB_RXTYPE1_TEP_M       0x000F  // Target Endpoint Number
#define MUSB_RXTYPE1_TEP_S       0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_RXINTERVAL1
// register.
//
//*****************************************************************************
#define MUSB_RXINTERVAL1_TXPOLL_M 0x00FF  // RX Polling
#define MUSB_RXINTERVAL1_NAKLMT_M 0x00FF  // NAK Limit
#define MUSB_RXINTERVAL1_TXPOLL_S 0
#define MUSB_RXINTERVAL1_NAKLMT_S 0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_DMACTL0 register.
//
//*****************************************************************************
#define MUSB_DMACTL0_BRSTM_M     0x0600  // Burst Mode
#define MUSB_DMACTL0_BRSTM_ANY   0x0000  // Bursts of unspecified length
#define MUSB_DMACTL0_BRSTM_INC4  0x0200  // INCR4 or unspecified length
#define MUSB_DMACTL0_BRSTM_INC8  0x0400  // INCR8, INCR4 or unspecified
                                            // length
#define MUSB_DMACTL0_BRSTM_INC16 0x0600  // INCR16, INCR8, INCR4 or
                                            // unspecified length
#define MUSB_DMACTL0_ERR         0x0100  // Bus Error Bit
#define MUSB_DMACTL0_EP_M        0x00F0  // Endpoint number
#define MUSB_DMACTL0_IE          0x0008  // DMA Interrupt Enable
#define MUSB_DMACTL0_MODE        0x0004  // DMA Transfer Mode
#define MUSB_DMACTL0_DIR         0x0002  // DMA Direction
#define MUSB_DMACTL0_ENABLE      0x0001  // DMA Transfer Enable
#define MUSB_DMACTL0_EP_S        4

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_DMAADDR0 register.
//
//*****************************************************************************
#define MUSB_DMAADDR0_ADDR_M     0xFFFFFFFC  // DMA Address
#define MUSB_DMAADDR0_ADDR_S     2

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_DMACOUNT0
// register.
//
//*****************************************************************************
#define MUSB_DMACOUNT0_COUNT_M   0xFFFFFFFC  // DMA Count
#define MUSB_DMACOUNT0_COUNT_S   2

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_CTO register.
//
//*****************************************************************************
#define MUSB_CTO_CCTV_M          0xFFFF  // Configurable Chirp Timeout Value
#define MUSB_CTO_CCTV_S          0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_HHSRTN register.
//
//*****************************************************************************
#define MUSB_HHSRTN_HHSRTN_M     0xFFFF  // HIgh Speed to UTM Operating
                                            // Delay
#define MUSB_HHSRTN_HHSRTN_S     0

//*****************************************************************************
//
// The following are defines for the bit fields in the MUSB_O_HSBT register.
//
//*****************************************************************************
#define MUSB_HSBT_HSBT_M         0x000F  // High Speed Timeout Adder
#define MUSB_HSBT_HSBT_S         0

#ifdef __cplusplus
 }
#endif

#endif
