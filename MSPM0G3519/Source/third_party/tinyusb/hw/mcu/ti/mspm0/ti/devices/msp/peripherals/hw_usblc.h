/*****************************************************************************

  Copyright (C) 2025 Texas Instruments Incorporated - http://www.ti.com/ 

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions 
  are met:

   Redistributions of source code must retain the above copyright 
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the 
   documentation and/or other materials provided with the   
   distribution.

   Neither the name of Texas Instruments Incorporated nor the names of
   its contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/

#ifndef ti_devices_msp_peripherals_hw_usblc__include
#define ti_devices_msp_peripherals_hw_usblc__include

/* Filename: hw_usblc.h */
/* Revised: 2025-05-20 01:38:34 */
/* Revision: 024fb13f0146b0eb8b43cb6270a7e6e1e9694d68 */

#ifndef __CORTEX_M
  #ifdef __cplusplus
    #define __I  volatile        /*!< Defines 'read only' permissions */
  #else
    #define __I  volatile const  /*!< Defines 'read only' permissions */
  #endif
  #define __O  volatile          /*!< Defines 'write only' permissions */
  #define __IO  volatile         /*!< Defines 'read / write' permissions */

  /* following defines should be used for structure members */
  #define __IM  volatile const   /*! Defines 'read only' structure member permissions */
  #define __OM  volatile         /*! Defines 'write only' structure member permissions */
  #define __IOM  volatile        /*! Defines 'read / write' structure member permissions */
#endif

/* Use standard integer types with explicit width */
#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

#if defined ( __CC_ARM )
#pragma anon_unions
#endif

/******************************************************************************
* USBLC Registers
******************************************************************************/
#define USBLC_REGISTERS_OFS                      ((uint32_t)0x00002000U)
#define USBLC_CPU_INT_OFS                        ((uint32_t)0x00001020U)
#define USBLC_GPRCM_OFS                          ((uint32_t)0x00000800U)


/** @addtogroup USBLC_REGISTERS
  @{
*/

typedef struct {
  __IO uint8_t FADDR;                             /* !< (@ 0x00002000) USB Device Functional Address */
  __IO uint8_t POWER;                             /* !< (@ 0x00002001) USB Power Control */
  __I  uint8_t TXIS;                              /* !< (@ 0x00002002) USB TX Endpoint Interrupt Status */
       uint8_t  RESERVED0;
  __I  uint8_t RXIS;                              /* !< (@ 0x00002004) USB RX Endpoint Interrupt Status */
       uint8_t  RESERVED1;
  __IO uint8_t USBIS;                             /* !< (@ 0x00002006) USB General Interrupt Status */
  __IO uint8_t TXIE;                              /* !< (@ 0x00002007) USB IN Endpoint Interrupt Enable */
       uint8_t  RESERVED2;
  __IO uint8_t RXIE;                              /* !< (@ 0x00002009) USB Receive Interrupt Enable */
       uint8_t  RESERVED3;
  __IO uint8_t USBIE;                             /* !< (@ 0x0000200B) USB General Interrupt Enable */
  __IO uint8_t FRAMEL;                            /* !< (@ 0x0000200C) USB Last Received Frame Number (lower byte) */
  __IO uint8_t FRAMEH;                            /* !< (@ 0x0000200D) USB Last Received Frame Number (upper byte) */
  __IO uint8_t EPINDEX;                           /* !< (@ 0x0000200E) USB Endpoint Index */
       uint8_t  RESERVED4;
  __IO uint8_t IDXTXMAXP;                         /* !< (@ 0x00002010) USB Maximum Transmit Data Endpoint [n] */
  __IO uint8_t IDXTXCSRL;                         /* !< (@ 0x00002011) USB Transmit Control and Status Endpoint [n] Low */
  __IO uint8_t IDXTXCSRH;                         /* !< (@ 0x00002012) USB Transmit Control and Status Endpoint [n] High */
  __IO uint8_t IDXRXMAXP;                         /* !< (@ 0x00002013) USB Maximum Receive Data Endpoint [n] */
  __IO uint8_t IDXRXCSRL;                         /* !< (@ 0x00002014) USB Receive Control and Status Endpoint [n] Low */
  __IO uint8_t IDXRXCSRH;                         /* !< (@ 0x00002015) Indexed Receive Control and Status High */
  __IO uint8_t IDXRXCOUNTL;                       /* !< (@ 0x00002016) USB Receive Byte Count Endpoint [n] */
  __IO uint8_t IDXRXCOUNTH;                       /* !< (@ 0x00002017) USB Receive Byte Count Endpoint [n] */
       uint32_t RESERVED5[2];
  __IO uint8_t FIFO[16];                          /* !< (@ 0x00002020) USB FIFO Endpoint [n] */
} USBLC_REGISTERS_Regs;

/*@}*/ /* end of group USBLC_REGISTERS */

/** @addtogroup USBLC_CPU_INT
  @{
*/

typedef struct {
  __I  uint32_t IIDX;                              /* !< (@ 0x00001020) Interrupt index */
       uint32_t RESERVED0;
  __IO uint32_t IMASK;                             /* !< (@ 0x00001028) Interrupt mask */
       uint32_t RESERVED1;
  __I  uint32_t RIS;                               /* !< (@ 0x00001030) Raw interrupt status */
       uint32_t RESERVED2;
  __I  uint32_t MIS;                               /* !< (@ 0x00001038) Masked interrupt status */
       uint32_t RESERVED3;
  __O  uint32_t ISET;                              /* !< (@ 0x00001040) Interrupt set */
       uint32_t RESERVED4;
  __O  uint32_t ICLR;                              /* !< (@ 0x00001048) Interrupt clear */
} USBLC_CPU_INT_Regs;

/*@}*/ /* end of group USBLC_CPU_INT */

/** @addtogroup USBLC_GPRCM
  @{
*/

typedef struct {
  __IO uint32_t PWREN;                             /* !< (@ 0x00000800) Power enable */
  __O  uint32_t RSTCTL;                            /* !< (@ 0x00000804) Reset Control */
  __IO uint32_t CLKCTL;                            /* !< (@ 0x00000808) Clock control */
       uint32_t RESERVED0[2];
  __I  uint32_t STAT;                              /* !< (@ 0x00000814) Status Register */
} USBLC_GPRCM_Regs;

/*@}*/ /* end of group USBLC_GPRCM */

/** @addtogroup USBLC
  @{
*/

typedef struct {
       uint32_t RESERVED0[512];
  USBLC_GPRCM_Regs  GPRCM;                             /* !< (@ 0x00000800) */
       uint32_t RESERVED1[514];
  USBLC_CPU_INT_Regs  CPU_INT;                           /* !< (@ 0x00001020) */
       uint32_t RESERVED2[37];
  __I  uint32_t EVT_MODE;                          /* !< (@ 0x000010E0) Event Mode */
       uint32_t RESERVED3[6];
  __I  uint32_t DESC;                              /* !< (@ 0x000010FC) Module Description */
  __IO uint32_t USBMODE;                           /* !< (@ 0x00001100) USB mode control */
  __IO uint32_t USBMONITOR;                        /* !< (@ 0x00001104) USB Clock Control Register */
  __IO uint32_t USBDMASEL;                         /* !< (@ 0x00001108) USB DMA trigger select */
       uint32_t RESERVED4[957];
  USBLC_REGISTERS_Regs  REGISTERS;                         /* !< (@ 0x00002000) */
} USBLC_Regs;

/*@}*/ /* end of group USBLC */



#if defined ( __CC_ARM )
#pragma no_anon_unions
#endif

/******************************************************************************
* USBLC Register Control Bits
******************************************************************************/

/* USBLC_FADDR Bits */
/* USBLC_FADDR[FUNCADDR] Bits */
#define USBLC_FADDR_FUNCADDR_OFS                 (0)                             /* !< FUNCADDR Offset */
#define USBLC_FADDR_FUNCADDR_MASK                ((uint8_t)0x0000003FU)          /* !< FAddr is an 7-bit register that
                                                                                    should be written with the 7-bit
                                                                                    address of the peripheral part of the
                                                                                    transaction. This register should be
                                                                                    written with the address received
                                                                                    through a SET_ADDRESS command, which
                                                                                    will then be used for decoding the
                                                                                    function address in subsequent token
                                                                                    packets. */
/* USBLC_FADDR[UPDATE] Bits */
#define USBLC_FADDR_UPDATE_OFS                   (7)                             /* !< UPDATE Offset */
#define USBLC_FADDR_UPDATE_MASK                  ((uint8_t)0x00000080U)          /* !< Set when FAddr is written. Cleared
                                                                                    when the new address takes effect (at
                                                                                    the end of the current transfer) */

/* USBLC_POWER Bits */
/* USBLC_POWER[ENSUSPENDM] Bits */
#define USBLC_POWER_ENSUSPENDM_OFS               (0)                             /* !< ENSUSPENDM Offset */
#define USBLC_POWER_ENSUSPENDM_MASK              ((uint8_t)0x00000001U)          /* !< Set by the CPU to enable the
                                                                                    Suspend mode for the USB PHY. */
#define USBLC_POWER_ENSUSPENDM_DISABLE           ((uint8_t)0x00000000U)          /* !< USB PHY is powered */
#define USBLC_POWER_ENSUSPENDM_ENABLE            ((uint8_t)0x00000001U)          /* !< Powers down the internal USB PHY. */
/* USBLC_POWER[SUSPEND] Bits */
#define USBLC_POWER_SUSPEND_OFS                  (1)                             /* !< SUSPEND Offset */
#define USBLC_POWER_SUSPEND_MASK                 ((uint8_t)0x00000002U)          /* !< Set by the USB when Suspend mode is
                                                                                    entered. Cleared when the CPU reads
                                                                                    the interrupt register, or sets the
                                                                                    Resume bit of this register. */
#define USBLC_POWER_SUSPEND_DISABLE              ((uint8_t)0x00000000U)          /* !< Ends SUSPEND signaling */
#define USBLC_POWER_SUSPEND_ENABLE               ((uint8_t)0x00000002U)          /* !< Enables SUSPEND signaling */
/* USBLC_POWER[RESUME] Bits */
#define USBLC_POWER_RESUME_OFS                   (2)                             /* !< RESUME Offset */
#define USBLC_POWER_RESUME_MASK                  ((uint8_t)0x00000004U)          /* !< Set by the CPU to generate Resume
                                                                                    signaling when the device is in
                                                                                    Suspend mode.   The CPU should clear
                                                                                    this bit after 10 ms (a maximum of 15
                                                                                    ms), to end Resume signaling. */
#define USBLC_POWER_RESUME_DISABLE               ((uint8_t)0x00000000U)          /* !< Ends RESUME signaling on the bus. */
#define USBLC_POWER_RESUME_ENABLE                ((uint8_t)0x00000004U)          /* !< Enables RESUME signaling when the
                                                                                    Device is in SUSPEND mode. */
/* USBLC_POWER[RESET] Bits */
#define USBLC_POWER_RESET_OFS                    (3)                             /* !< RESET Offset */
#define USBLC_POWER_RESET_MASK                   ((uint8_t)0x00000008U)          /* !< This bit is set when Reset
                                                                                    signaling is present on the bus. */
#define USBLC_POWER_RESET_DISABLE                ((uint8_t)0x00000000U)          /* !< Ends RESET signaling on the bus. */
#define USBLC_POWER_RESET_ENABLE                 ((uint8_t)0x00000008U)          /* !< Enables RESET signaling on the bus. */
/* USBLC_POWER[ISOUP] Bits */
#define USBLC_POWER_ISOUP_OFS                    (7)                             /* !< ISOUP Offset */
#define USBLC_POWER_ISOUP_MASK                   ((uint8_t)0x00000080U)          /* !< Isochronous Update. When set by the
                                                                                    CPU, the USB controller will wait for
                                                                                    an SOF token from the time TXRDY bit
                                                                                    is set in the USBTXCSRLn before
                                                                                    sending the packet. If an IN token is
                                                                                    received before an SOF token, then a
                                                                                    zero length data packet will be sent. */
#define USBLC_POWER_ISOUP_NO_EFFECT              ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_POWER_ISOUP_WAIT_SOF               ((uint8_t)0x00000080U)          /* !< The USB controller waits for an SOF
                                                                                    token. */

/* USBLC_TXIS Bits */
/* USBLC_TXIS[EP0] Bits */
#define USBLC_TXIS_EP0_OFS                       (0)                             /* !< EP0 Offset */
#define USBLC_TXIS_EP0_MASK                      ((uint8_t)0x00000001U)          /* !< Endpoint 0 Interrupt */
#define USBLC_TXIS_EP0_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_TXIS_EP0_INT                       ((uint8_t)0x00000001U)          /* !< The Endpoint 0 transmit and receive
                                                                                    interrupt is asserted. */
/* USBLC_TXIS[EP1] Bits */
#define USBLC_TXIS_EP1_OFS                       (1)                             /* !< EP1 Offset */
#define USBLC_TXIS_EP1_MASK                      ((uint8_t)0x00000002U)          /* !< TX Endpoint 1 Interrupt */
#define USBLC_TXIS_EP1_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_TXIS_EP1_INT                       ((uint8_t)0x00000002U)          /* !< The Endpoint 1 transmit interrupt
                                                                                    is asserted. */
/* USBLC_TXIS[EP2] Bits */
#define USBLC_TXIS_EP2_OFS                       (2)                             /* !< EP2 Offset */
#define USBLC_TXIS_EP2_MASK                      ((uint8_t)0x00000004U)          /* !< TX Endpoint 2 Interrupt */
#define USBLC_TXIS_EP2_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_TXIS_EP2_INT                       ((uint8_t)0x00000004U)          /* !< The Endpoint 2 transmit interrupt
                                                                                    is asserted. */
/* USBLC_TXIS[EP3] Bits */
#define USBLC_TXIS_EP3_OFS                       (3)                             /* !< EP3 Offset */
#define USBLC_TXIS_EP3_MASK                      ((uint8_t)0x00000008U)          /* !< TX Endpoint 3 Interrupt */
#define USBLC_TXIS_EP3_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_TXIS_EP3_INT                       ((uint8_t)0x00000008U)          /* !< The Endpoint 3 transmit interrupt
                                                                                    is asserted. */

/* USBLC_RXIS Bits */
/* USBLC_RXIS[EP1] Bits */
#define USBLC_RXIS_EP1_OFS                       (1)                             /* !< EP1 Offset */
#define USBLC_RXIS_EP1_MASK                      ((uint8_t)0x00000002U)          /* !< RX Endpoint 1 Interrupt */
#define USBLC_RXIS_EP1_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_RXIS_EP1_INT                       ((uint8_t)0x00000002U)          /* !< The Endpoint 1 transmit interrupt
                                                                                    is asserted. */
/* USBLC_RXIS[EP2] Bits */
#define USBLC_RXIS_EP2_OFS                       (2)                             /* !< EP2 Offset */
#define USBLC_RXIS_EP2_MASK                      ((uint8_t)0x00000004U)          /* !< RX Endpoint 2 Interrupt */
#define USBLC_RXIS_EP2_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_RXIS_EP2_INT                       ((uint8_t)0x00000004U)          /* !< The Endpoint 2 transmit interrupt
                                                                                    is asserted. */
/* USBLC_RXIS[EP3] Bits */
#define USBLC_RXIS_EP3_OFS                       (3)                             /* !< EP3 Offset */
#define USBLC_RXIS_EP3_MASK                      ((uint8_t)0x00000008U)          /* !< RX Endpoint 3 Interrupt */
#define USBLC_RXIS_EP3_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_RXIS_EP3_INT                       ((uint8_t)0x00000008U)          /* !< The Endpoint 3 transmit interrupt
                                                                                    is asserted. */

/* USBLC_USBIS Bits */
/* USBLC_USBIS[SUSPEND] Bits */
#define USBLC_USBIS_SUSPEND_OFS                  (0)                             /* !< SUSPEND Offset */
#define USBLC_USBIS_SUSPEND_MASK                 ((uint8_t)0x00000001U)          /* !< Set when Suspend signaling is
                                                                                    detected on the bus. */
#define USBLC_USBIS_SUSPEND_NO_INT               ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_USBIS_SUSPEND_INT                  ((uint8_t)0x00000001U)          /* !< SUSPEND signaling has been detected
                                                                                    on the bus. */
/* USBLC_USBIS[RESUME] Bits */
#define USBLC_USBIS_RESUME_OFS                   (1)                             /* !< RESUME Offset */
#define USBLC_USBIS_RESUME_MASK                  ((uint8_t)0x00000002U)          /* !< Set when Resume signaling is
                                                                                    detected on the bus while the USB
                                                                                    controller is in Suspend mode. */
#define USBLC_USBIS_RESUME_NO_INT                ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_USBIS_RESUME_INT                   ((uint8_t)0x00000002U)          /* !< RESUME signaling has been detected
                                                                                    on the bus. */
/* USBLC_USBIS[RESET] Bits */
#define USBLC_USBIS_RESET_OFS                    (2)                             /* !< RESET Offset */
#define USBLC_USBIS_RESET_MASK                   ((uint8_t)0x00000004U)          /* !< Set when Reset signaling is
                                                                                    detected on the bus. */
#define USBLC_USBIS_RESET_NO_INT                 ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_USBIS_RESET_INT                    ((uint8_t)0x00000004U)          /* !< RESET / BABBLE signaling has been
                                                                                    detected on the bus. */
/* USBLC_USBIS[SOF] Bits */
#define USBLC_USBIS_SOF_OFS                      (3)                             /* !< SOF Offset */
#define USBLC_USBIS_SOF_MASK                     ((uint8_t)0x00000008U)          /* !< Set at start of each frame. */
#define USBLC_USBIS_SOF_NO_INT                   ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_USBIS_SOF_INT                      ((uint8_t)0x00000008U)          /* !< A new frame has started. */

/* USBLC_TXIE Bits */
/* USBLC_TXIE[EP0] Bits */
#define USBLC_TXIE_EP0_OFS                       (0)                             /* !< EP0 Offset */
#define USBLC_TXIE_EP0_MASK                      ((uint8_t)0x00000001U)          /* !< Endpoint 0 Interrupt Enable */
#define USBLC_TXIE_EP0_DISABLE                   ((uint8_t)0x00000000U)          /* !< The EP1 transmit interrupt is
                                                                                    suppressed and not sent to the
                                                                                    interrupt controller. */
#define USBLC_TXIE_EP0_ENABLE                    ((uint8_t)0x00000001U)          /* !< An interrupt is sent to the
                                                                                    interrupt controller when the EP1 bit
                                                                                    in the USBTXIS register is set. */
/* USBLC_TXIE[EP1] Bits */
#define USBLC_TXIE_EP1_OFS                       (1)                             /* !< EP1 Offset */
#define USBLC_TXIE_EP1_MASK                      ((uint8_t)0x00000002U)          /* !< Transmit Endpoint 1 Interrupt
                                                                                    Enable */
#define USBLC_TXIE_EP1_DISABLE                   ((uint8_t)0x00000000U)          /* !< The EP1 transmit interrupt is
                                                                                    suppressed and not sent to the
                                                                                    interrupt controller. */
#define USBLC_TXIE_EP1_ENABLE                    ((uint8_t)0x00000002U)          /* !< An interrupt is sent to the
                                                                                    interrupt controller when the EP1 bit
                                                                                    in the USBTXIS register is set. */
/* USBLC_TXIE[EP2] Bits */
#define USBLC_TXIE_EP2_OFS                       (2)                             /* !< EP2 Offset */
#define USBLC_TXIE_EP2_MASK                      ((uint8_t)0x00000004U)          /* !< Transmit Endpoint 2 Interrupt
                                                                                    Enable */
#define USBLC_TXIE_EP2_DISABLE                   ((uint8_t)0x00000000U)          /* !< The EP2 transmit interrupt is
                                                                                    suppressed and not sent to the
                                                                                    interrupt controller. */
#define USBLC_TXIE_EP2_ENABLE                    ((uint8_t)0x00000004U)          /* !< An interrupt is sent to the
                                                                                    interrupt controller when the EP2 bit
                                                                                    in the USBTXIS register is set. */
/* USBLC_TXIE[EP3] Bits */
#define USBLC_TXIE_EP3_OFS                       (3)                             /* !< EP3 Offset */
#define USBLC_TXIE_EP3_MASK                      ((uint8_t)0x00000008U)          /* !< Transmit Endpoint 3 Interrupt
                                                                                    Enable */
#define USBLC_TXIE_EP3_DISABLE                   ((uint8_t)0x00000000U)          /* !< The EP3 transmit interrupt is
                                                                                    suppressed and not sent to the
                                                                                    interrupt controller. */
#define USBLC_TXIE_EP3_ENABLE                    ((uint8_t)0x00000008U)          /* !< An interrupt is sent to the
                                                                                    interrupt controller when the EP3 bit
                                                                                    in the USBTXIS register is set. */

/* USBLC_RXIE Bits */
/* USBLC_RXIE[EP1] Bits */
#define USBLC_RXIE_EP1_OFS                       (1)                             /* !< EP1 Offset */
#define USBLC_RXIE_EP1_MASK                      ((uint8_t)0x00000002U)          /* !< Recieve Endpoint 1 Interrupt Enable */
#define USBLC_RXIE_EP1_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_RXIE_EP1_INT                       ((uint8_t)0x00000002U)          /* !< The Endpoint 1 transmit interrupt
                                                                                    is asserted. */
/* USBLC_RXIE[EP2] Bits */
#define USBLC_RXIE_EP2_OFS                       (2)                             /* !< EP2 Offset */
#define USBLC_RXIE_EP2_MASK                      ((uint8_t)0x00000004U)          /* !< Recieve Endpoint 2 Interrupt Enable */
#define USBLC_RXIE_EP2_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_RXIE_EP2_INT                       ((uint8_t)0x00000004U)          /* !< The Endpoint 2 transmit interrupt
                                                                                    is asserted. */
/* USBLC_RXIE[EP3] Bits */
#define USBLC_RXIE_EP3_OFS                       (3)                             /* !< EP3 Offset */
#define USBLC_RXIE_EP3_MASK                      ((uint8_t)0x00000008U)          /* !< Recieve Endpoint 3 Interrupt Enable */
#define USBLC_RXIE_EP3_NO_INT                    ((uint8_t)0x00000000U)          /* !< No interrupt */
#define USBLC_RXIE_EP3_INT                       ((uint8_t)0x00000008U)          /* !< The Endpoint 3 transmit interrupt
                                                                                    is asserted. */

/* USBLC_USBIE Bits */
/* USBLC_USBIE[SUSPEND] Bits */
#define USBLC_USBIE_SUSPEND_OFS                  (0)                             /* !< SUSPEND Offset */
#define USBLC_USBIE_SUSPEND_MASK                 ((uint8_t)0x00000001U)          /* !< Interrupt enable for SUSPEND
                                                                                    signaling detected. */
#define USBLC_USBIE_SUSPEND_DISABLE              ((uint8_t)0x00000000U)          /* !< The SUSPEND interrupt is suppressed
                                                                                    and not sent to the interrupt
                                                                                    controller. */
#define USBLC_USBIE_SUSPEND_ENABLE               ((uint8_t)0x00000001U)          /* !< An interrupt is sent to the
                                                                                    interrupt controller when the DISCON
                                                                                    bit in the USBIS register is set. */
/* USBLC_USBIE[RESUME] Bits */
#define USBLC_USBIE_RESUME_OFS                   (1)                             /* !< RESUME Offset */
#define USBLC_USBIE_RESUME_MASK                  ((uint8_t)0x00000002U)          /* !< Interrupt enable for RESUME
                                                                                    signaling detected. */
#define USBLC_USBIE_RESUME_DISABLE               ((uint8_t)0x00000000U)          /* !< The RESUME interrupt is suppressed
                                                                                    and not sent to the interrupt
                                                                                    controller. */
#define USBLC_USBIE_RESUME_ENABLE                ((uint8_t)0x00000002U)          /* !< An interrupt is sent to the
                                                                                    interrupt controller when the RESUME
                                                                                    bit in the USBIS register is set. */
/* USBLC_USBIE[RESET] Bits */
#define USBLC_USBIE_RESET_OFS                    (2)                             /* !< RESET Offset */
#define USBLC_USBIE_RESET_MASK                   ((uint8_t)0x00000004U)          /* !< Interrupt enable for RESET
                                                                                    signaling detected. */
#define USBLC_USBIE_RESET_DSIABLE                ((uint8_t)0x00000000U)          /* !< The RESET interrupt is suppressed
                                                                                    and not sent to the interrupt
                                                                                    controller. */
#define USBLC_USBIE_RESET_ENABLE                 ((uint8_t)0x00000004U)          /* !< An interrupt is sent to the
                                                                                    interrupt controller when the RESET
                                                                                    bit in the USBIS register is set. */
/* USBLC_USBIE[SOF] Bits */
#define USBLC_USBIE_SOF_OFS                      (3)                             /* !< SOF Offset */
#define USBLC_USBIE_SOF_MASK                     ((uint8_t)0x00000008U)          /* !< Interrupt enable for Start of
                                                                                    frame. */
#define USBLC_USBIE_SOF_DISABLE                  ((uint8_t)0x00000000U)          /* !< The SOF interrupt is suppressed and
                                                                                    not sent to the interrupt controller. */
#define USBLC_USBIE_SOF_ENABLE                   ((uint8_t)0x00000008U)          /* !< An interrupt is sent to the
                                                                                    interrupt controller when the SOF bit
                                                                                    in the USBIS register is set. */

/* USBLC_FRAMEL Bits */
/* USBLC_FRAMEL[FRAME] Bits */
#define USBLC_FRAMEL_FRAME_OFS                   (0)                             /* !< FRAME Offset */
#define USBLC_FRAMEL_FRAME_MASK                  ((uint8_t)0x000000FFU)          /* !< Frame Number (lower 8-bit) */

/* USBLC_FRAMEH Bits */
/* USBLC_FRAMEH[FRAME] Bits */
#define USBLC_FRAMEH_FRAME_OFS                   (0)                             /* !< FRAME Offset */
#define USBLC_FRAMEH_FRAME_MASK                  ((uint8_t)0x00000007U)          /* !< Frame Number (upper 3-bit) */

/* USBLC_EPINDEX Bits */
/* USBLC_EPINDEX[EPIDX] Bits */
#define USBLC_EPINDEX_EPIDX_OFS                  (0)                             /* !< EPIDX Offset */
#define USBLC_EPINDEX_EPIDX_MASK                 ((uint8_t)0x0000000FU)          /* !< Endpoint Index.  #br#This bit field
                                                                                    configures which endpoint is accessed
                                                                                    when reading or writing to one of the
                                                                                    USB controller's indexed registers. A
                                                                                    value of 0x0 corresponds to Endpoint
                                                                                    0 and a value of 0xF corresponds to
                                                                                    Endpoint 15. */
#define USBLC_EPINDEX_EPIDX_EP0                  ((uint8_t)0x00000000U)          /* !< Select EP0 as indexed register */
#define USBLC_EPINDEX_EPIDX_EP1                  ((uint8_t)0x00000001U)          /* !< Select EP1 as indexed register */
#define USBLC_EPINDEX_EPIDX_EP2                  ((uint8_t)0x00000002U)          /* !< Select EP2 as indexed register */
#define USBLC_EPINDEX_EPIDX_EP3                  ((uint8_t)0x00000003U)          /* !< Select EP3 as indexed register */
#define USBLC_EPINDEX_EPIDX_EP4                  ((uint8_t)0x00000004U)          /* !< Select EP4 as indexed register */
#define USBLC_EPINDEX_EPIDX_EP5                  ((uint8_t)0x00000005U)          /* !< Select EP5 as indexed register */
#define USBLC_EPINDEX_EPIDX_EP6                  ((uint8_t)0x00000006U)          /* !< Select EP6 as indexed register */
#define USBLC_EPINDEX_EPIDX_EP7                  ((uint8_t)0x00000007U)          /* !< Select EP7 as indexed register */

/* USBLC_IDXTXMAXP Bits */
/* USBLC_IDXTXMAXP[MAXLOAD] Bits */
#define USBLC_IDXTXMAXP_MAXLOAD_OFS              (0)                             /* !< MAXLOAD Offset */
#define USBLC_IDXTXMAXP_MAXLOAD_MASK             ((uint8_t)0x000000FFU)          /* !< Maximum Payload specifies the
                                                                                    maximum payload in bytes per
                                                                                    transaction. */

/* USBLC_IDXTXCSRL Bits */
/* USBLC_IDXTXCSRL[TXRDY] Bits */
#define USBLC_IDXTXCSRL_TXRDY_OFS                (0)                             /* !< TXRDY Offset */
#define USBLC_IDXTXCSRL_TXRDY_MASK               ((uint8_t)0x00000001U)          /* !< Transmit Packet Ready. The CPU sets
                                                                                    this bit after loading a data packet
                                                                                    into the FIFO.  This bit is cleared
                                                                                    automatically when a data packet has
                                                                                    been transmitted. The EPn bit in the
                                                                                    USBTXIS register is also set at this
                                                                                    point. TXRDY is also automatically
                                                                                    cleared prior to loading a second
                                                                                    packet into a double-buffered FIFO.
                                                                                    Note: Host and device mode */
#define USBLC_IDXTXCSRL_TXRDY_WAIT               ((uint8_t)0x00000000U)          /* !< No transmit packet is ready. */
#define USBLC_IDXTXCSRL_TXRDY_READY              ((uint8_t)0x00000001U)          /* !< Software sets this bit after
                                                                                    loading a data packet into the TX
                                                                                    FIFO. */
/* USBLC_IDXTXCSRL[FIFONE] Bits */
#define USBLC_IDXTXCSRL_FIFONE_OFS               (1)                             /* !< FIFONE Offset */
#define USBLC_IDXTXCSRL_FIFONE_MASK              ((uint8_t)0x00000002U)          /* !< FIFO Not Empty. The USB controller
                                                                                    sets this bit when there is at least
                                                                                    1 packet in the TX FIFO. Note: Host
                                                                                    and device mode. */
#define USBLC_IDXTXCSRL_FIFONE_EMPTY             ((uint8_t)0x00000000U)          /* !< The FIFO is empty */
#define USBLC_IDXTXCSRL_FIFONE_NOT_EMPTY         ((uint8_t)0x00000002U)          /* !< At least one packet is in the
                                                                                    transmit FIFO. */
/* USBLC_IDXTXCSRL[UNDERRUN] Bits */
#define USBLC_IDXTXCSRL_UNDERRUN_OFS             (2)                             /* !< UNDERRUN Offset */
#define USBLC_IDXTXCSRL_UNDERRUN_MASK            ((uint8_t)0x00000004U)          /* !< Underrun In ISO mode, this bit is
                                                                                    set when a zero length data packet is
                                                                                    sent after receiving an IN token with
                                                                                    the TXRDY bit not set. In
                                                                                    Bulk/Interrupt mode, this bit is set
                                                                                    when a NAK is returned in response to
                                                                                    an IN token. The CPU should clear
                                                                                    this bit. */
#define USBLC_IDXTXCSRL_UNDERRUN_CLEAR           ((uint8_t)0x00000000U)          /* !< No underrun or error condition */
#define USBLC_IDXTXCSRL_UNDERRUN_SET             ((uint8_t)0x00000004U)          /* !< Underrun or error condition exist. */
/* USBLC_IDXTXCSRL[FLUSH] Bits */
#define USBLC_IDXTXCSRL_FLUSH_OFS                (3)                             /* !< FLUSH Offset */
#define USBLC_IDXTXCSRL_FLUSH_MASK               ((uint8_t)0x00000008U)          /* !< Flush FIFO. The CPU writes a 1 to
                                                                                    this bit to flush the latest packet
                                                                                    from the endpoint TX FIFO. The FIFO
                                                                                    pointer is reset, the TXRDY bit is
                                                                                    cleared and an interrupt is
                                                                                    generated. May be set simultaneously
                                                                                    with TXRDY to abort the packet that
                                                                                    is currently being loaded into the
                                                                                    FIFO.  Note: FLUSH should only be
                                                                                    used when TXRDY is set. At other
                                                                                    times, it may cause data to be
                                                                                    corrupted. Also note that, if the
                                                                                    FIFO is double-buffered, FLUSH may
                                                                                    need to be set twice to completely
                                                                                    clear the FIFO. */
#define USBLC_IDXTXCSRL_FLUSH_NO_EFFECT          ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXTXCSRL_FLUSH_EFFECT             ((uint8_t)0x00000008U)          /* !< Flushes the latest packet from the
                                                                                    endpoint transmit FIFO. */
/* USBLC_IDXTXCSRL[STALL] Bits */
#define USBLC_IDXTXCSRL_STALL_OFS                (4)                             /* !< STALL Offset */
#define USBLC_IDXTXCSRL_STALL_MASK               ((uint8_t)0x00000010U)          /* !< Send stall The CPU writes a 1 to
                                                                                    this bit to issue a STALL handshake
                                                                                    to an IN token. The CPU clears this
                                                                                    bit to terminate the stall condition.
                                                                                    Note: This bit has no effect where
                                                                                    the TX endpoint is being used for
                                                                                    Isochronous transfers. */
#define USBLC_IDXTXCSRL_STALL_CLEAR              ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXTXCSRL_STALL_SET                ((uint8_t)0x00000010U)          /* !< Send stall or setup packet request. */
/* USBLC_IDXTXCSRL[STALLED] Bits */
#define USBLC_IDXTXCSRL_STALLED_OFS              (5)                             /* !< STALLED Offset */
#define USBLC_IDXTXCSRL_STALLED_MASK             ((uint8_t)0x00000020U)          /* !< Sent stall This bit is set when a
                                                                                    STALL handshake is transmitted. The
                                                                                    FIFO is flushed and the TXRDY bit is
                                                                                    cleared. The CPU should clear this
                                                                                    bit. */
#define USBLC_IDXTXCSRL_STALLED_CLEAR            ((uint8_t)0x00000000U)          /* !< Clear stall indication. */
#define USBLC_IDXTXCSRL_STALLED_SET              ((uint8_t)0x00000020U)          /* !< Indicates that a STALL handshake
                                                                                    has been sent or received. */
/* USBLC_IDXTXCSRL[CLRDT] Bits */
#define USBLC_IDXTXCSRL_CLRDT_OFS                (6)                             /* !< CLRDT Offset */
#define USBLC_IDXTXCSRL_CLRDT_MASK               ((uint8_t)0x00000040U)          /* !< Clear DataToggle. The CPU writes a
                                                                                    1 to this bit to reset the endpoint
                                                                                    data toggle to 0. */
#define USBLC_IDXTXCSRL_CLRDT_NO_EFFECT          ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXTXCSRL_CLRDT_EFFECT             ((uint8_t)0x00000040U)          /* !< Writing a 1 to this bit clears the
                                                                                    DT bit in the USBTXCSRH[n] register. */

/* USBLC_IDXTXCSRH Bits */
/* USBLC_IDXTXCSRH[FDT] Bits */
#define USBLC_IDXTXCSRH_FDT_OFS                  (3)                             /* !< FDT Offset */
#define USBLC_IDXTXCSRH_FDT_MASK                 ((uint8_t)0x00000008U)          /* !< Force Data Toggle. The CPU sets
                                                                                    this bit to force the endpoints TX
                                                                                    data toggle to switch after each data
                                                                                    packet is sent regardless of whether
                                                                                    an ACK was received. This can be used
                                                                                    by interrupt TX endpoints which are
                                                                                    used to communicate rate feedback for
                                                                                    Isochronous endpoints. */
#define USBLC_IDXTXCSRH_FDT_NO_EFFECT            ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXTXCSRH_FDT_EFFECT               ((uint8_t)0x00000008U)          /* !< Forces data toggle */
/* USBLC_IDXTXCSRH[DMAEN] Bits */
#define USBLC_IDXTXCSRH_DMAEN_OFS                (4)                             /* !< DMAEN Offset */
#define USBLC_IDXTXCSRH_DMAEN_MASK               ((uint8_t)0x00000010U)          /* !< DMA Request Enable The CPU sets
                                                                                    this bit to enable the DMA request
                                                                                    for the TX endpoint. */
#define USBLC_IDXTXCSRH_DMAEN_DISABLE            ((uint8_t)0x00000000U)          /* !< Disables the DMA request for the
                                                                                    transmit endpoint. */
#define USBLC_IDXTXCSRH_DMAEN_ENABLE             ((uint8_t)0x00000010U)          /* !< Enables the DMA request for the
                                                                                    transmit endpoint. */
/* USBLC_IDXTXCSRH[MODE] Bits */
#define USBLC_IDXTXCSRH_MODE_OFS                 (5)                             /* !< MODE Offset */
#define USBLC_IDXTXCSRH_MODE_MASK                ((uint8_t)0x00000020U)          /* !< Mode The CPU sets this bit to
                                                                                    enable the endpoint direction as TX,
                                                                                    and clears the bit to enable it as
                                                                                    RX.  Note: This bit only has an
                                                                                    effect when the same endpoint FIFO is
                                                                                    used for both transmit and receive
                                                                                    transactions. */
#define USBLC_IDXTXCSRH_MODE_RX                  ((uint8_t)0x00000000U)          /* !< Enables the endpoint direction as
                                                                                    RX. */
#define USBLC_IDXTXCSRH_MODE_TX                  ((uint8_t)0x00000020U)          /* !< Enables the endpoint direction as
                                                                                    TX. */
/* USBLC_IDXTXCSRH[ISO] Bits */
#define USBLC_IDXTXCSRH_ISO_OFS                  (6)                             /* !< ISO Offset */
#define USBLC_IDXTXCSRH_ISO_MASK                 ((uint8_t)0x00000040U)          /* !< Isochronous Transfers. The CPU sets
                                                                                    this bit to enable the TX endpoint
                                                                                    for Isochronous transfers, and clears
                                                                                    it to enable the TX endpoint for Bulk
                                                                                    or Interrupt transfers. */
#define USBLC_IDXTXCSRH_ISO_BULK_INT_TRANSFER    ((uint8_t)0x00000000U)          /* !< Enables the transmit endpoint for
                                                                                    bulk or interrupt transfers. */
#define USBLC_IDXTXCSRH_ISO_ISO_TRANSFER         ((uint8_t)0x00000040U)          /* !< Enables the transmit endpoint for
                                                                                    isochronous transfers. */
/* USBLC_IDXTXCSRH[AUTOSET] Bits */
#define USBLC_IDXTXCSRH_AUTOSET_OFS              (7)                             /* !< AUTOSET Offset */
#define USBLC_IDXTXCSRH_AUTOSET_MASK             ((uint8_t)0x00000080U)          /* !< Auto Set. If the CPU sets this bit,
                                                                                    the TXRDY bit will be be
                                                                                    automatically set when data of the
                                                                                    maximum packet size (value in
                                                                                    IDXTXMAXP) is loaded into the
                                                                                    transmit FIFO. If a packet of less
                                                                                    than the maximum packet size is
                                                                                    loaded, then the TXRDY bit must be
                                                                                    set manually. Note: Should not be set
                                                                                    for either high-bandwidth Isochronous
                                                                                    endpoints or high-bandwidth Interrupt
                                                                                    endpoints. */
#define USBLC_IDXTXCSRH_AUTOSET_MANUAL           ((uint8_t)0x00000000U)          /* !< The TXRDY bit must be set manually. */
#define USBLC_IDXTXCSRH_AUTOSET_AUTO             ((uint8_t)0x00000080U)          /* !< The TXRDY bit is automatically set
                                                                                    when maximum packet size is loaded
                                                                                    into the TX FIFO. */

/* USBLC_IDXRXMAXP Bits */
/* USBLC_IDXRXMAXP[MAXLOAD] Bits */
#define USBLC_IDXRXMAXP_MAXLOAD_OFS              (0)                             /* !< MAXLOAD Offset */
#define USBLC_IDXRXMAXP_MAXLOAD_MASK             ((uint8_t)0x000000FFU)          /* !< Maximum Payload specifies the
                                                                                    maximum payload in bytes per
                                                                                    transaction. */

/* USBLC_IDXRXCSRL Bits */
/* USBLC_IDXRXCSRL[RXRDY] Bits */
#define USBLC_IDXRXCSRL_RXRDY_OFS                (0)                             /* !< RXRDY Offset */
#define USBLC_IDXRXCSRL_RXRDY_MASK               ((uint8_t)0x00000001U)          /* !< Receive Packet Ready. This bit is
                                                                                    set when a data packet has been
                                                                                    received. The CPU should clear this
                                                                                    bit when the packet has been unloaded
                                                                                    from the Rx FIFO. An interrupt is
                                                                                    generated when the bit is set. If the
                                                                                    AUTOCLR bit in the IDXRXCSRH register
                                                                                    is set, then the this bit is
                                                                                    automatically cleared when a packet
                                                                                    of IDXRXMAXP bytes has been unloaded
                                                                                    from the receive FIFO. If the AUTOCLR
                                                                                    bit is clear, or if packets of less
                                                                                    than the maximum packet size are
                                                                                    unloaded, then software must clear
                                                                                    this bit manually when the packet has
                                                                                    been unloaded from the receive FIFO. */
#define USBLC_IDXRXCSRL_RXRDY_WAIT               ((uint8_t)0x00000000U)          /* !< No data packet has been received. */
#define USBLC_IDXRXCSRL_RXRDY_READY              ((uint8_t)0x00000001U)          /* !< Indicates that a data packet has
                                                                                    been received. The EP[j] bit in the
                                                                                    USBTXIS register is also set in this
                                                                                    situation */
/* USBLC_IDXRXCSRL[FULL] Bits */
#define USBLC_IDXRXCSRL_FULL_OFS                 (1)                             /* !< FULL Offset */
#define USBLC_IDXRXCSRL_FULL_MASK                ((uint8_t)0x00000002U)          /* !< FIFO Full. This bit is set when no
                                                                                    more packets can be loaded into the
                                                                                    RX FIFO. */
#define USBLC_IDXRXCSRL_FULL_NO                  ((uint8_t)0x00000000U)          /* !< The receive FIFO is not full. */
#define USBLC_IDXRXCSRL_FULL_YES                 ((uint8_t)0x00000002U)          /* !< The receive FIFO is full. No more
                                                                                    packets can be loaded into the
                                                                                    receive FIFO. */
/* USBLC_IDXRXCSRL[OVERRUN] Bits */
#define USBLC_IDXRXCSRL_OVERRUN_OFS              (2)                             /* !< OVERRUN Offset */
#define USBLC_IDXRXCSRL_OVERRUN_MASK             ((uint8_t)0x00000004U)          /* !< Over run Over run - This bit is set
                                                                                    if an RX packet cannot be loaded into
                                                                                    the receive FIFO. The CPU should
                                                                                    clear this bit. - Note: This bit is
                                                                                    only valid when the endpoint is
                                                                                    operating in ISO mode. In Bulk mode,
                                                                                    it always returns zero. */
#define USBLC_IDXRXCSRL_OVERRUN_CLEAR            ((uint8_t)0x00000000U)          /* !< No over run or error condition. */
#define USBLC_IDXRXCSRL_OVERRUN_SET              ((uint8_t)0x00000004U)          /* !< Under run or error condition exist. */
/* USBLC_IDXRXCSRL[DATAERR] Bits */
#define USBLC_IDXRXCSRL_DATAERR_OFS              (3)                             /* !< DATAERR Offset */
#define USBLC_IDXRXCSRL_DATAERR_MASK             ((uint8_t)0x00000008U)          /* !< Data Error This bit is set when
                                                                                    RXRDY is set, if the data packet has
                                                                                    a CRC or bit-stuff error. It is
                                                                                    cleared when RXRDY is cleared. -
                                                                                    Note: This bit is only valid when the
                                                                                    endpoint is operating in ISO mode. In
                                                                                    Bulk mode, it always returns zero. */
#define USBLC_IDXRXCSRL_DATAERR_CLEAR            ((uint8_t)0x00000000U)          /* !< Normal operation */
#define USBLC_IDXRXCSRL_DATAERR_SET              ((uint8_t)0x00000008U)          /* !< Data error or NAK Timeout condition
                                                                                    exist. */
/* USBLC_IDXRXCSRL[FLUSH] Bits */
#define USBLC_IDXRXCSRL_FLUSH_OFS                (4)                             /* !< FLUSH Offset */
#define USBLC_IDXRXCSRL_FLUSH_MASK               ((uint8_t)0x00000010U)          /* !< Flush FIFO. The CPU writes a 1 to
                                                                                    this bit to flush the next packet to
                                                                                    be read from the endpoint RX FIFO.
                                                                                    The FIFO pointer is reset and the
                                                                                    RXRDY bit is cleared.  Note: Thsi bit
                                                                                    should only be set when RXRDY is set.
                                                                                    At other times, it may cause data to
                                                                                    be corrupted.  Note: If the FIFO is
                                                                                    double-buffered, FLUSH may need to be
                                                                                    set twice to completely clear the
                                                                                    FIFO. Note: Device and host mode. */
#define USBLC_IDXRXCSRL_FLUSH_NO_EFFECT          ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXRXCSRL_FLUSH_EFFECT             ((uint8_t)0x00000010U)          /* !< Flushes the next packet to be read
                                                                                    from the endpoint receive FIFO. The
                                                                                    FIFO pointer is reset and the RXRDY
                                                                                    bit is cleared */
/* USBLC_IDXRXCSRL[STALL] Bits */
#define USBLC_IDXRXCSRL_STALL_OFS                (5)                             /* !< STALL Offset */
#define USBLC_IDXRXCSRL_STALL_MASK               ((uint8_t)0x00000020U)          /* !< Send Stall The CPU writes a 1 to
                                                                                    this bit to issue a STALL handshake.
                                                                                    The CPU clears this bit to terminate
                                                                                    the stall condition. - Note: This bit
                                                                                    has no effect where the endpoint is
                                                                                    being used for Isochronous transfers. */
#define USBLC_IDXRXCSRL_STALL_CLEAR              ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXRXCSRL_STALL_SET                ((uint8_t)0x00000020U)          /* !< Send stall or packet request. */
/* USBLC_IDXRXCSRL[STALLED] Bits */
#define USBLC_IDXRXCSRL_STALLED_OFS              (6)                             /* !< STALLED Offset */
#define USBLC_IDXRXCSRL_STALLED_MASK             ((uint8_t)0x00000040U)          /* !< Stall Sent  This bit is set when a
                                                                                    STALL handshake is transmitted. The
                                                                                    CPU should clear this bit. */
#define USBLC_IDXRXCSRL_STALLED_STALLED_WAIT     ((uint8_t)0x00000000U)          /* !< No handshake has been received. */
#define USBLC_IDXRXCSRL_STALLED_STALLED_RECEIVE  ((uint8_t)0x00000040U)          /* !< A STALL handshake has been
                                                                                    received. The EPn bit in the USBRXIS
                                                                                    register is also set. */
/* USBLC_IDXRXCSRL[CLRDT] Bits */
#define USBLC_IDXRXCSRL_CLRDT_OFS                (7)                             /* !< CLRDT Offset */
#define USBLC_IDXRXCSRL_CLRDT_MASK               ((uint8_t)0x00000080U)          /* !< Clear Data Toggle. The CPU writes a
                                                                                    1 to this bit to reset the endpoint
                                                                                    data toggle to 0. */
#define USBLC_IDXRXCSRL_CLRDT_NO_EFFECT          ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXRXCSRL_CLRDT_EFFECT             ((uint8_t)0x00000080U)          /* !< Writing a 1 to this bit clears the
                                                                                    DT bit in the USBRXCSRH[n] register. */

/* USBLC_IDXRXCSRH Bits */
/* USBLC_IDXRXCSRH[DMAMOD] Bits */
#define USBLC_IDXRXCSRH_DMAMOD_OFS               (4)                             /* !< DMAMOD Offset */
#define USBLC_IDXRXCSRH_DMAMOD_MASK              ((uint8_t)0x00000010U)          /* !< DMA request mode Two modes of DMA
                                                                                    operation are supported: DMA Mode 0
                                                                                    in which a DMA request is generated
                                                                                    for all received packets, together
                                                                                    with an interrupt (if enabled); and
                                                                                    DMA Mode 1 in which a DMA request
                                                                                    (but no interrupt) is generated for
                                                                                    RX packets of size RXMAXP bytes and
                                                                                    an interrupt (but no DMA request) is
                                                                                    generated for RX packets of any other
                                                                                    size. The CPU sets this bit to select
                                                                                    DMA Mode 1 and clears this bit to
                                                                                    select DMA Mode 0. */
#define USBLC_IDXRXCSRH_DMAMOD_MODE0             ((uint8_t)0x00000000U)          /* !< An interrupt is generated after
                                                                                    every DMA packet transfer. */
#define USBLC_IDXRXCSRH_DMAMOD_MODE1             ((uint8_t)0x00000010U)          /* !< An interrupt is generated only
                                                                                    after the entire DMA transfer is
                                                                                    complete. */
/* USBLC_IDXRXCSRH[DMAEN] Bits */
#define USBLC_IDXRXCSRH_DMAEN_OFS                (5)                             /* !< DMAEN Offset */
#define USBLC_IDXRXCSRH_DMAEN_MASK               ((uint8_t)0x00000020U)          /* !< DMA Request Enable The CPU sets
                                                                                    this bit to enable the DMA request
                                                                                    for the RX endpoint. */
#define USBLC_IDXRXCSRH_DMAEN_DISABLE            ((uint8_t)0x00000000U)          /* !< Disables the DMA request for the
                                                                                    receive endpoint. */
#define USBLC_IDXRXCSRH_DMAEN_ENABLE             ((uint8_t)0x00000020U)          /* !< Enables the DMA request for the
                                                                                    receive endpoint. */
/* USBLC_IDXRXCSRH[ISO] Bits */
#define USBLC_IDXRXCSRH_ISO_OFS                  (6)                             /* !< ISO Offset */
#define USBLC_IDXRXCSRH_ISO_MASK                 ((uint8_t)0x00000040U)          /* !< Isochronous transfer mode The CPU
                                                                                    sets this bit to enable the RX
                                                                                    endpoint for Isochronous transfers,
                                                                                    and clears it to enable the RX
                                                                                    endpoint for Bulk/Interrupt
                                                                                    transfers. */
#define USBLC_IDXRXCSRH_ISO_DISABLE              ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXRXCSRH_ISO_ENABLE               ((uint8_t)0x00000040U)          /* !< Sets ISO transfer mode or enables
                                                                                    the REQPKT bit to be automatically
                                                                                    set when the RXRDY bit is cleared. */
/* USBLC_IDXRXCSRH[AUTOCLR] Bits */
#define USBLC_IDXRXCSRH_AUTOCLR_OFS              (7)                             /* !< AUTOCLR Offset */
#define USBLC_IDXRXCSRH_AUTOCLR_MASK             ((uint8_t)0x00000080U)          /* !< Auto Clear. If the CPU sets this
                                                                                    bit then the RXRDY bit will be
                                                                                    automatically cleared when a packet
                                                                                    of RXMAXP bytes has been unloaded
                                                                                    from the RX FIFO. When packets of
                                                                                    less than the maximum packet size are
                                                                                    unloaded, RXRDY will have to be
                                                                                    cleared manually. When using a DMA to
                                                                                    unload the RX-FIFO, data is read from
                                                                                    the RX-FIFO in 4 byte chunks
                                                                                    regardless of the RXMAXP setting. */
#define USBLC_IDXRXCSRH_AUTOCLR_DSIABLE          ((uint8_t)0x00000000U)          /* !< No effect */
#define USBLC_IDXRXCSRH_AUTOCLR_ENABLE           ((uint8_t)0x00000080U)          /* !< Enables the RXRDY bit to be
                                                                                    automatically cleared when a packet
                                                                                    of USBRXMAXP[n] bytes has been
                                                                                    unloaded from the receive FIFO. */

/* USBLC_IDXRXCOUNTL Bits */
/* USBLC_IDXRXCOUNTL[COUNT] Bits */
#define USBLC_IDXRXCOUNTL_COUNT_OFS              (0)                             /* !< COUNT Offset */
#define USBLC_IDXRXCOUNTL_COUNT_MASK             ((uint8_t)0x000000FFU)          /* !< Receive Packet Count indicates the
                                                                                    number of bytes in the receive
                                                                                    packet. */

/* USBLC_IDXRXCOUNTH Bits */
/* USBLC_IDXRXCOUNTH[COUNT] Bits */
#define USBLC_IDXRXCOUNTH_COUNT_OFS              (0)                             /* !< COUNT Offset */
#define USBLC_IDXRXCOUNTH_COUNT_MASK             ((uint8_t)0x00000007U)          /* !< Receive Packet Count indicates the
                                                                                    number of bytes in the receive
                                                                                    packet. */

/* USBLC_FIFO Bits */
/* USBLC_FIFO[EPDATA] Bits */
#define USBLC_FIFO_EPDATA_OFS                    (0)                             /* !< EPDATA Offset */
#define USBLC_FIFO_EPDATA_MASK                   ((uint8_t)0x000000FFU)          /* !< Endpoint Data. Writing to this
                                                                                    register loads the data into the
                                                                                    Transmit FIFO and reading unloads
                                                                                    data from the Receive FIFO. */

/* USBLC_IIDX Bits */
/* USBLC_IIDX[STAT] Bits */
#define USBLC_IIDX_STAT_OFS                      (0)                             /* !< STAT Offset */
#define USBLC_IIDX_STAT_MASK                     ((uint32_t)0x000000FFU)         /* !< Interrupt index status */
#define USBLC_IIDX_STAT_NO_INTR                  ((uint32_t)0x00000000U)         /* !< No bit is set means there is no
                                                                                    pending interrupt request */
#define USBLC_IIDX_STAT_INTRTX                   ((uint32_t)0x00000001U)         /* !< Endpoint 0 and the TX Endpoints
                                                                                    interrupt */
#define USBLC_IIDX_STAT_INTRRX                   ((uint32_t)0x00000002U)         /* !< RX Endpoints interrupt */
#define USBLC_IIDX_STAT_INTRUSB                  ((uint32_t)0x00000003U)         /* !< USB Interrupts */
#define USBLC_IIDX_STAT_VUSBPWRDN                ((uint32_t)0x00000004U)         /* !< VUSB power down */
#define USBLC_IIDX_STAT_DMADONEARX               ((uint32_t)0x00000005U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_IIDX_STAT_DMADONEATX               ((uint32_t)0x00000006U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_IIDX_STAT_DMADONEBRX               ((uint32_t)0x00000007U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_IIDX_STAT_DMADONEBTX               ((uint32_t)0x00000008U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_IIDX_STAT_DMADONECRX               ((uint32_t)0x00000009U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_IIDX_STAT_DMADONECTX               ((uint32_t)0x0000000AU)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_IIDX_STAT_DMADONEDRX               ((uint32_t)0x0000000BU)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_IIDX_STAT_DMADONEDTX               ((uint32_t)0x0000000CU)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_IIDX_STAT_DMAPREARX                ((uint32_t)0x0000000DU)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_IIDX_STAT_DMAPREATX                ((uint32_t)0x0000000EU)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_IIDX_STAT_DMAPREBRX                ((uint32_t)0x0000000FU)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_IIDX_STAT_DMAPREBTX                ((uint32_t)0x00000010U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_IIDX_STAT_DMAPRECRX                ((uint32_t)0x00000011U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_IIDX_STAT_DMAPRECTX                ((uint32_t)0x00000012U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_IIDX_STAT_DMAPREDRX                ((uint32_t)0x00000013U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_IIDX_STAT_DMAPREDTX                ((uint32_t)0x00000014U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-TX */

/* USBLC_IMASK Bits */
/* USBLC_IMASK[INTRTX] Bits */
#define USBLC_IMASK_INTRTX_OFS                   (0)                             /* !< INTRTX Offset */
#define USBLC_IMASK_INTRTX_MASK                  ((uint32_t)0x00000001U)         /* !< Endpoint 0 and the TX Endpoints
                                                                                    interrupt */
#define USBLC_IMASK_INTRTX_CLR                   ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_INTRTX_SET                   ((uint32_t)0x00000001U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[INTRRX] Bits */
#define USBLC_IMASK_INTRRX_OFS                   (1)                             /* !< INTRRX Offset */
#define USBLC_IMASK_INTRRX_MASK                  ((uint32_t)0x00000002U)         /* !< RX Endpoints interrupt */
#define USBLC_IMASK_INTRRX_CLR                   ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_INTRRX_SET                   ((uint32_t)0x00000002U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[INTRUSB] Bits */
#define USBLC_IMASK_INTRUSB_OFS                  (2)                             /* !< INTRUSB Offset */
#define USBLC_IMASK_INTRUSB_MASK                 ((uint32_t)0x00000004U)         /* !< USB Interrupts */
#define USBLC_IMASK_INTRUSB_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_INTRUSB_SET                  ((uint32_t)0x00000004U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[VUSBPWRDN] Bits */
#define USBLC_IMASK_VUSBPWRDN_OFS                (3)                             /* !< VUSBPWRDN Offset */
#define USBLC_IMASK_VUSBPWRDN_MASK               ((uint32_t)0x00000008U)         /* !< VUSB Power Down */
#define USBLC_IMASK_VUSBPWRDN_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_VUSBPWRDN_SET                ((uint32_t)0x00000008U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMADONEARX] Bits */
#define USBLC_IMASK_DMADONEARX_OFS               (4)                             /* !< DMADONEARX Offset */
#define USBLC_IMASK_DMADONEARX_MASK              ((uint32_t)0x00000010U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_IMASK_DMADONEARX_CLR               ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMADONEARX_SET               ((uint32_t)0x00000010U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMADONEATX] Bits */
#define USBLC_IMASK_DMADONEATX_OFS               (5)                             /* !< DMADONEATX Offset */
#define USBLC_IMASK_DMADONEATX_MASK              ((uint32_t)0x00000020U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_IMASK_DMADONEATX_CLR               ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMADONEATX_SET               ((uint32_t)0x00000020U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMADONEBRX] Bits */
#define USBLC_IMASK_DMADONEBRX_OFS               (6)                             /* !< DMADONEBRX Offset */
#define USBLC_IMASK_DMADONEBRX_MASK              ((uint32_t)0x00000040U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_IMASK_DMADONEBRX_CLR               ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMADONEBRX_SET               ((uint32_t)0x00000040U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMADONEBTX] Bits */
#define USBLC_IMASK_DMADONEBTX_OFS               (7)                             /* !< DMADONEBTX Offset */
#define USBLC_IMASK_DMADONEBTX_MASK              ((uint32_t)0x00000080U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_IMASK_DMADONEBTX_CLR               ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMADONEBTX_SET               ((uint32_t)0x00000080U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMADONECRX] Bits */
#define USBLC_IMASK_DMADONECRX_OFS               (8)                             /* !< DMADONECRX Offset */
#define USBLC_IMASK_DMADONECRX_MASK              ((uint32_t)0x00000100U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_IMASK_DMADONECRX_CLR               ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMADONECRX_SET               ((uint32_t)0x00000100U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMADONECTX] Bits */
#define USBLC_IMASK_DMADONECTX_OFS               (9)                             /* !< DMADONECTX Offset */
#define USBLC_IMASK_DMADONECTX_MASK              ((uint32_t)0x00000200U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_IMASK_DMADONECTX_CLR               ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMADONECTX_SET               ((uint32_t)0x00000200U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMADONEDRX] Bits */
#define USBLC_IMASK_DMADONEDRX_OFS               (10)                            /* !< DMADONEDRX Offset */
#define USBLC_IMASK_DMADONEDRX_MASK              ((uint32_t)0x00000400U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_IMASK_DMADONEDRX_CLR               ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMADONEDRX_SET               ((uint32_t)0x00000400U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMADONEDTX] Bits */
#define USBLC_IMASK_DMADONEDTX_OFS               (11)                            /* !< DMADONEDTX Offset */
#define USBLC_IMASK_DMADONEDTX_MASK              ((uint32_t)0x00000800U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_IMASK_DMADONEDTX_CLR               ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMADONEDTX_SET               ((uint32_t)0x00000800U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMAPREARX] Bits */
#define USBLC_IMASK_DMAPREARX_OFS                (12)                            /* !< DMAPREARX Offset */
#define USBLC_IMASK_DMAPREARX_MASK               ((uint32_t)0x00001000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_IMASK_DMAPREARX_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMAPREARX_SET                ((uint32_t)0x00001000U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMAPREATX] Bits */
#define USBLC_IMASK_DMAPREATX_OFS                (13)                            /* !< DMAPREATX Offset */
#define USBLC_IMASK_DMAPREATX_MASK               ((uint32_t)0x00002000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_IMASK_DMAPREATX_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMAPREATX_SET                ((uint32_t)0x00002000U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMAPREBRX] Bits */
#define USBLC_IMASK_DMAPREBRX_OFS                (14)                            /* !< DMAPREBRX Offset */
#define USBLC_IMASK_DMAPREBRX_MASK               ((uint32_t)0x00004000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_IMASK_DMAPREBRX_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMAPREBRX_SET                ((uint32_t)0x00004000U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMAPREBTX] Bits */
#define USBLC_IMASK_DMAPREBTX_OFS                (15)                            /* !< DMAPREBTX Offset */
#define USBLC_IMASK_DMAPREBTX_MASK               ((uint32_t)0x00008000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_IMASK_DMAPREBTX_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMAPREBTX_SET                ((uint32_t)0x00008000U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMAPRECRX] Bits */
#define USBLC_IMASK_DMAPRECRX_OFS                (16)                            /* !< DMAPRECRX Offset */
#define USBLC_IMASK_DMAPRECRX_MASK               ((uint32_t)0x00010000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_IMASK_DMAPRECRX_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMAPRECRX_SET                ((uint32_t)0x00010000U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMAPRECTX] Bits */
#define USBLC_IMASK_DMAPRECTX_OFS                (17)                            /* !< DMAPRECTX Offset */
#define USBLC_IMASK_DMAPRECTX_MASK               ((uint32_t)0x00020000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_IMASK_DMAPRECTX_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMAPRECTX_SET                ((uint32_t)0x00020000U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMAPREDRX] Bits */
#define USBLC_IMASK_DMAPREDRX_OFS                (18)                            /* !< DMAPREDRX Offset */
#define USBLC_IMASK_DMAPREDRX_MASK               ((uint32_t)0x00040000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_IMASK_DMAPREDRX_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMAPREDRX_SET                ((uint32_t)0x00040000U)         /* !< Interrupt Enabled. */
/* USBLC_IMASK[DMAPREDTX] Bits */
#define USBLC_IMASK_DMAPREDTX_OFS                (19)                            /* !< DMAPREDTX Offset */
#define USBLC_IMASK_DMAPREDTX_MASK               ((uint32_t)0x00080000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_IMASK_DMAPREDTX_CLR                ((uint32_t)0x00000000U)         /* !< Interrupt Disabled. */
#define USBLC_IMASK_DMAPREDTX_SET                ((uint32_t)0x00080000U)         /* !< Interrupt Enabled. */

/* USBLC_RIS Bits */
/* USBLC_RIS[INTRTX] Bits */
#define USBLC_RIS_INTRTX_OFS                     (0)                             /* !< INTRTX Offset */
#define USBLC_RIS_INTRTX_MASK                    ((uint32_t)0x00000001U)         /* !< Endpoint 0 and the TX Endpoints
                                                                                    interrupt */
#define USBLC_RIS_INTRTX_CLR                     ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_INTRTX_SET                     ((uint32_t)0x00000001U)         /* !< Interrupt occured */
/* USBLC_RIS[INTRRX] Bits */
#define USBLC_RIS_INTRRX_OFS                     (1)                             /* !< INTRRX Offset */
#define USBLC_RIS_INTRRX_MASK                    ((uint32_t)0x00000002U)         /* !< RX Endpoints interrupt */
#define USBLC_RIS_INTRRX_CLR                     ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_INTRRX_SET                     ((uint32_t)0x00000002U)         /* !< Interrupt occured */
/* USBLC_RIS[INTRUSB] Bits */
#define USBLC_RIS_INTRUSB_OFS                    (2)                             /* !< INTRUSB Offset */
#define USBLC_RIS_INTRUSB_MASK                   ((uint32_t)0x00000004U)         /* !< USB Interrupts */
#define USBLC_RIS_INTRUSB_CLR                    ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_INTRUSB_SET                    ((uint32_t)0x00000004U)         /* !< Interrupt occured */
/* USBLC_RIS[VUSBPWRDN] Bits */
#define USBLC_RIS_VUSBPWRDN_OFS                  (3)                             /* !< VUSBPWRDN Offset */
#define USBLC_RIS_VUSBPWRDN_MASK                 ((uint32_t)0x00000008U)         /* !< VUSB Power Down */
#define USBLC_RIS_VUSBPWRDN_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_VUSBPWRDN_SET                  ((uint32_t)0x00000008U)         /* !< Interrupt occured */
/* USBLC_RIS[DMADONEARX] Bits */
#define USBLC_RIS_DMADONEARX_OFS                 (4)                             /* !< DMADONEARX Offset */
#define USBLC_RIS_DMADONEARX_MASK                ((uint32_t)0x00000010U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_RIS_DMADONEARX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMADONEARX_SET                 ((uint32_t)0x00000010U)         /* !< Interrupt occured */
/* USBLC_RIS[DMADONEATX] Bits */
#define USBLC_RIS_DMADONEATX_OFS                 (5)                             /* !< DMADONEATX Offset */
#define USBLC_RIS_DMADONEATX_MASK                ((uint32_t)0x00000020U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_RIS_DMADONEATX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMADONEATX_SET                 ((uint32_t)0x00000020U)         /* !< Interrupt occured */
/* USBLC_RIS[DMADONEBRX] Bits */
#define USBLC_RIS_DMADONEBRX_OFS                 (6)                             /* !< DMADONEBRX Offset */
#define USBLC_RIS_DMADONEBRX_MASK                ((uint32_t)0x00000040U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_RIS_DMADONEBRX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMADONEBRX_SET                 ((uint32_t)0x00000040U)         /* !< Interrupt occured */
/* USBLC_RIS[DMADONEBTX] Bits */
#define USBLC_RIS_DMADONEBTX_OFS                 (7)                             /* !< DMADONEBTX Offset */
#define USBLC_RIS_DMADONEBTX_MASK                ((uint32_t)0x00000080U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_RIS_DMADONEBTX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMADONEBTX_SET                 ((uint32_t)0x00000080U)         /* !< Interrupt occured */
/* USBLC_RIS[DMADONECRX] Bits */
#define USBLC_RIS_DMADONECRX_OFS                 (8)                             /* !< DMADONECRX Offset */
#define USBLC_RIS_DMADONECRX_MASK                ((uint32_t)0x00000100U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_RIS_DMADONECRX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMADONECRX_SET                 ((uint32_t)0x00000100U)         /* !< Interrupt occured */
/* USBLC_RIS[DMADONECTX] Bits */
#define USBLC_RIS_DMADONECTX_OFS                 (9)                             /* !< DMADONECTX Offset */
#define USBLC_RIS_DMADONECTX_MASK                ((uint32_t)0x00000200U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_RIS_DMADONECTX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMADONECTX_SET                 ((uint32_t)0x00000200U)         /* !< Interrupt occured */
/* USBLC_RIS[DMADONEDRX] Bits */
#define USBLC_RIS_DMADONEDRX_OFS                 (10)                            /* !< DMADONEDRX Offset */
#define USBLC_RIS_DMADONEDRX_MASK                ((uint32_t)0x00000400U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_RIS_DMADONEDRX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMADONEDRX_SET                 ((uint32_t)0x00000400U)         /* !< Interrupt occured */
/* USBLC_RIS[DMADONEDTX] Bits */
#define USBLC_RIS_DMADONEDTX_OFS                 (11)                            /* !< DMADONEDTX Offset */
#define USBLC_RIS_DMADONEDTX_MASK                ((uint32_t)0x00000800U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_RIS_DMADONEDTX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMADONEDTX_SET                 ((uint32_t)0x00000800U)         /* !< Interrupt occured */
/* USBLC_RIS[DMAPREARX] Bits */
#define USBLC_RIS_DMAPREARX_OFS                  (12)                            /* !< DMAPREARX Offset */
#define USBLC_RIS_DMAPREARX_MASK                 ((uint32_t)0x00001000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_RIS_DMAPREARX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMAPREARX_SET                  ((uint32_t)0x00001000U)         /* !< Interrupt occured */
/* USBLC_RIS[DMAPREATX] Bits */
#define USBLC_RIS_DMAPREATX_OFS                  (13)                            /* !< DMAPREATX Offset */
#define USBLC_RIS_DMAPREATX_MASK                 ((uint32_t)0x00002000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_RIS_DMAPREATX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMAPREATX_SET                  ((uint32_t)0x00002000U)         /* !< Interrupt occured */
/* USBLC_RIS[DMAPREBRX] Bits */
#define USBLC_RIS_DMAPREBRX_OFS                  (14)                            /* !< DMAPREBRX Offset */
#define USBLC_RIS_DMAPREBRX_MASK                 ((uint32_t)0x00004000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_RIS_DMAPREBRX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMAPREBRX_SET                  ((uint32_t)0x00004000U)         /* !< Interrupt occured */
/* USBLC_RIS[DMAPREBTX] Bits */
#define USBLC_RIS_DMAPREBTX_OFS                  (15)                            /* !< DMAPREBTX Offset */
#define USBLC_RIS_DMAPREBTX_MASK                 ((uint32_t)0x00008000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_RIS_DMAPREBTX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMAPREBTX_SET                  ((uint32_t)0x00008000U)         /* !< Interrupt occured */
/* USBLC_RIS[DMAPRECRX] Bits */
#define USBLC_RIS_DMAPRECRX_OFS                  (16)                            /* !< DMAPRECRX Offset */
#define USBLC_RIS_DMAPRECRX_MASK                 ((uint32_t)0x00010000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_RIS_DMAPRECRX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMAPRECRX_SET                  ((uint32_t)0x00010000U)         /* !< Interrupt occured */
/* USBLC_RIS[DMAPRECTX] Bits */
#define USBLC_RIS_DMAPRECTX_OFS                  (17)                            /* !< DMAPRECTX Offset */
#define USBLC_RIS_DMAPRECTX_MASK                 ((uint32_t)0x00020000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_RIS_DMAPRECTX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMAPRECTX_SET                  ((uint32_t)0x00020000U)         /* !< Interrupt occured */
/* USBLC_RIS[DMAPREDRX] Bits */
#define USBLC_RIS_DMAPREDRX_OFS                  (18)                            /* !< DMAPREDRX Offset */
#define USBLC_RIS_DMAPREDRX_MASK                 ((uint32_t)0x00040000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_RIS_DMAPREDRX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMAPREDRX_SET                  ((uint32_t)0x00040000U)         /* !< Interrupt occured */
/* USBLC_RIS[DMAPREDTX] Bits */
#define USBLC_RIS_DMAPREDTX_OFS                  (19)                            /* !< DMAPREDTX Offset */
#define USBLC_RIS_DMAPREDTX_MASK                 ((uint32_t)0x00080000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_RIS_DMAPREDTX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_RIS_DMAPREDTX_SET                  ((uint32_t)0x00080000U)         /* !< Interrupt occured */

/* USBLC_MIS Bits */
/* USBLC_MIS[INTRTX] Bits */
#define USBLC_MIS_INTRTX_OFS                     (0)                             /* !< INTRTX Offset */
#define USBLC_MIS_INTRTX_MASK                    ((uint32_t)0x00000001U)         /* !< Endpoint 0 and the TX Endpoints
                                                                                    interrupt */
#define USBLC_MIS_INTRTX_CLR                     ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_INTRTX_SET                     ((uint32_t)0x00000001U)         /* !< Interrupt occured */
/* USBLC_MIS[INTRRX] Bits */
#define USBLC_MIS_INTRRX_OFS                     (1)                             /* !< INTRRX Offset */
#define USBLC_MIS_INTRRX_MASK                    ((uint32_t)0x00000002U)         /* !< RX Endpoints interrupt */
#define USBLC_MIS_INTRRX_CLR                     ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_INTRRX_SET                     ((uint32_t)0x00000002U)         /* !< Interrupt occured */
/* USBLC_MIS[INTRUSB] Bits */
#define USBLC_MIS_INTRUSB_OFS                    (2)                             /* !< INTRUSB Offset */
#define USBLC_MIS_INTRUSB_MASK                   ((uint32_t)0x00000004U)         /* !< USB Interrupts */
#define USBLC_MIS_INTRUSB_CLR                    ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_INTRUSB_SET                    ((uint32_t)0x00000004U)         /* !< Interrupt occured */
/* USBLC_MIS[VUSBPWRDN] Bits */
#define USBLC_MIS_VUSBPWRDN_OFS                  (3)                             /* !< VUSBPWRDN Offset */
#define USBLC_MIS_VUSBPWRDN_MASK                 ((uint32_t)0x00000008U)         /* !< VUSB Power Down */
#define USBLC_MIS_VUSBPWRDN_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_VUSBPWRDN_SET                  ((uint32_t)0x00000008U)         /* !< Interrupt occured */
/* USBLC_MIS[DMADONEARX] Bits */
#define USBLC_MIS_DMADONEARX_OFS                 (4)                             /* !< DMADONEARX Offset */
#define USBLC_MIS_DMADONEARX_MASK                ((uint32_t)0x00000010U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_MIS_DMADONEARX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMADONEARX_SET                 ((uint32_t)0x00000010U)         /* !< Interrupt occured */
/* USBLC_MIS[DMADONEATX] Bits */
#define USBLC_MIS_DMADONEATX_OFS                 (5)                             /* !< DMADONEATX Offset */
#define USBLC_MIS_DMADONEATX_MASK                ((uint32_t)0x00000020U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_MIS_DMADONEATX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMADONEATX_SET                 ((uint32_t)0x00000020U)         /* !< Interrupt occured */
/* USBLC_MIS[DMADONEBRX] Bits */
#define USBLC_MIS_DMADONEBRX_OFS                 (6)                             /* !< DMADONEBRX Offset */
#define USBLC_MIS_DMADONEBRX_MASK                ((uint32_t)0x00000040U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_MIS_DMADONEBRX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMADONEBRX_SET                 ((uint32_t)0x00000040U)         /* !< Interrupt occured */
/* USBLC_MIS[DMADONEBTX] Bits */
#define USBLC_MIS_DMADONEBTX_OFS                 (7)                             /* !< DMADONEBTX Offset */
#define USBLC_MIS_DMADONEBTX_MASK                ((uint32_t)0x00000080U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_MIS_DMADONEBTX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMADONEBTX_SET                 ((uint32_t)0x00000080U)         /* !< Interrupt occured */
/* USBLC_MIS[DMADONECRX] Bits */
#define USBLC_MIS_DMADONECRX_OFS                 (8)                             /* !< DMADONECRX Offset */
#define USBLC_MIS_DMADONECRX_MASK                ((uint32_t)0x00000100U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_MIS_DMADONECRX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMADONECRX_SET                 ((uint32_t)0x00000100U)         /* !< Interrupt occured */
/* USBLC_MIS[DMADONECTX] Bits */
#define USBLC_MIS_DMADONECTX_OFS                 (9)                             /* !< DMADONECTX Offset */
#define USBLC_MIS_DMADONECTX_MASK                ((uint32_t)0x00000200U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_MIS_DMADONECTX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMADONECTX_SET                 ((uint32_t)0x00000200U)         /* !< Interrupt occured */
/* USBLC_MIS[DMADONEDRX] Bits */
#define USBLC_MIS_DMADONEDRX_OFS                 (10)                            /* !< DMADONEDRX Offset */
#define USBLC_MIS_DMADONEDRX_MASK                ((uint32_t)0x00000400U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_MIS_DMADONEDRX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMADONEDRX_SET                 ((uint32_t)0x00000400U)         /* !< Interrupt occured */
/* USBLC_MIS[DMADONEDTX] Bits */
#define USBLC_MIS_DMADONEDTX_OFS                 (11)                            /* !< DMADONEDTX Offset */
#define USBLC_MIS_DMADONEDTX_MASK                ((uint32_t)0x00000800U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_MIS_DMADONEDTX_CLR                 ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMADONEDTX_SET                 ((uint32_t)0x00000800U)         /* !< Interrupt occured */
/* USBLC_MIS[DMAPREARX] Bits */
#define USBLC_MIS_DMAPREARX_OFS                  (12)                            /* !< DMAPREARX Offset */
#define USBLC_MIS_DMAPREARX_MASK                 ((uint32_t)0x00001000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_MIS_DMAPREARX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMAPREARX_SET                  ((uint32_t)0x00001000U)         /* !< Interrupt occured */
/* USBLC_MIS[DMAPREATX] Bits */
#define USBLC_MIS_DMAPREATX_OFS                  (13)                            /* !< DMAPREATX Offset */
#define USBLC_MIS_DMAPREATX_MASK                 ((uint32_t)0x00002000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_MIS_DMAPREATX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMAPREATX_SET                  ((uint32_t)0x00002000U)         /* !< Interrupt occured */
/* USBLC_MIS[DMAPREBRX] Bits */
#define USBLC_MIS_DMAPREBRX_OFS                  (14)                            /* !< DMAPREBRX Offset */
#define USBLC_MIS_DMAPREBRX_MASK                 ((uint32_t)0x00004000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_MIS_DMAPREBRX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMAPREBRX_SET                  ((uint32_t)0x00004000U)         /* !< Interrupt occured */
/* USBLC_MIS[DMAPREBTX] Bits */
#define USBLC_MIS_DMAPREBTX_OFS                  (15)                            /* !< DMAPREBTX Offset */
#define USBLC_MIS_DMAPREBTX_MASK                 ((uint32_t)0x00008000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_MIS_DMAPREBTX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMAPREBTX_SET                  ((uint32_t)0x00008000U)         /* !< Interrupt occured */
/* USBLC_MIS[DMAPRECRX] Bits */
#define USBLC_MIS_DMAPRECRX_OFS                  (16)                            /* !< DMAPRECRX Offset */
#define USBLC_MIS_DMAPRECRX_MASK                 ((uint32_t)0x00010000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_MIS_DMAPRECRX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMAPRECRX_SET                  ((uint32_t)0x00010000U)         /* !< Interrupt occured */
/* USBLC_MIS[DMAPRECTX] Bits */
#define USBLC_MIS_DMAPRECTX_OFS                  (17)                            /* !< DMAPRECTX Offset */
#define USBLC_MIS_DMAPRECTX_MASK                 ((uint32_t)0x00020000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_MIS_DMAPRECTX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMAPRECTX_SET                  ((uint32_t)0x00020000U)         /* !< Interrupt occured */
/* USBLC_MIS[DMAPREDRX] Bits */
#define USBLC_MIS_DMAPREDRX_OFS                  (18)                            /* !< DMAPREDRX Offset */
#define USBLC_MIS_DMAPREDRX_MASK                 ((uint32_t)0x00040000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_MIS_DMAPREDRX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMAPREDRX_SET                  ((uint32_t)0x00040000U)         /* !< Interrupt occured */
/* USBLC_MIS[DMAPREDTX] Bits */
#define USBLC_MIS_DMAPREDTX_OFS                  (19)                            /* !< DMAPREDTX Offset */
#define USBLC_MIS_DMAPREDTX_MASK                 ((uint32_t)0x00080000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_MIS_DMAPREDTX_CLR                  ((uint32_t)0x00000000U)         /* !< Interrupt did not occur */
#define USBLC_MIS_DMAPREDTX_SET                  ((uint32_t)0x00080000U)         /* !< Interrupt occured */

/* USBLC_ISET Bits */
/* USBLC_ISET[INTRTX] Bits */
#define USBLC_ISET_INTRTX_OFS                    (0)                             /* !< INTRTX Offset */
#define USBLC_ISET_INTRTX_MASK                   ((uint32_t)0x00000001U)         /* !< Endpoint 0 and the TX Endpoints
                                                                                    interrupt */
#define USBLC_ISET_INTRTX_NO_EFFECT              ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_INTRTX_SET                    ((uint32_t)0x00000001U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[INTRRX] Bits */
#define USBLC_ISET_INTRRX_OFS                    (1)                             /* !< INTRRX Offset */
#define USBLC_ISET_INTRRX_MASK                   ((uint32_t)0x00000002U)         /* !< RX Endpoints interrupt */
#define USBLC_ISET_INTRRX_NO_EFFECT              ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_INTRRX_SET                    ((uint32_t)0x00000002U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[INTRUSB] Bits */
#define USBLC_ISET_INTRUSB_OFS                   (2)                             /* !< INTRUSB Offset */
#define USBLC_ISET_INTRUSB_MASK                  ((uint32_t)0x00000004U)         /* !< USB Interrupts */
#define USBLC_ISET_INTRUSB_NO_EFFECT             ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_INTRUSB_SET                   ((uint32_t)0x00000004U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[VUSBPWRDN] Bits */
#define USBLC_ISET_VUSBPWRDN_OFS                 (3)                             /* !< VUSBPWRDN Offset */
#define USBLC_ISET_VUSBPWRDN_MASK                ((uint32_t)0x00000008U)         /* !< VUSB Power Down */
#define USBLC_ISET_VUSBPWRDN_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_VUSBPWRDN_SET                 ((uint32_t)0x00000008U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMADONEARX] Bits */
#define USBLC_ISET_DMADONEARX_OFS                (4)                             /* !< DMADONEARX Offset */
#define USBLC_ISET_DMADONEARX_MASK               ((uint32_t)0x00000010U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_ISET_DMADONEARX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMADONEARX_SET                ((uint32_t)0x00000010U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMADONEATX] Bits */
#define USBLC_ISET_DMADONEATX_OFS                (5)                             /* !< DMADONEATX Offset */
#define USBLC_ISET_DMADONEATX_MASK               ((uint32_t)0x00000020U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_ISET_DMADONEATX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMADONEATX_SET                ((uint32_t)0x00000020U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMADONEBRX] Bits */
#define USBLC_ISET_DMADONEBRX_OFS                (6)                             /* !< DMADONEBRX Offset */
#define USBLC_ISET_DMADONEBRX_MASK               ((uint32_t)0x00000040U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_ISET_DMADONEBRX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMADONEBRX_SET                ((uint32_t)0x00000040U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMADONEBTX] Bits */
#define USBLC_ISET_DMADONEBTX_OFS                (7)                             /* !< DMADONEBTX Offset */
#define USBLC_ISET_DMADONEBTX_MASK               ((uint32_t)0x00000080U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_ISET_DMADONEBTX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMADONEBTX_SET                ((uint32_t)0x00000080U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMADONECRX] Bits */
#define USBLC_ISET_DMADONECRX_OFS                (8)                             /* !< DMADONECRX Offset */
#define USBLC_ISET_DMADONECRX_MASK               ((uint32_t)0x00000100U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_ISET_DMADONECRX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMADONECRX_SET                ((uint32_t)0x00000100U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMADONECTX] Bits */
#define USBLC_ISET_DMADONECTX_OFS                (9)                             /* !< DMADONECTX Offset */
#define USBLC_ISET_DMADONECTX_MASK               ((uint32_t)0x00000200U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_ISET_DMADONECTX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMADONECTX_SET                ((uint32_t)0x00000200U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMADONEDRX] Bits */
#define USBLC_ISET_DMADONEDRX_OFS                (10)                            /* !< DMADONEDRX Offset */
#define USBLC_ISET_DMADONEDRX_MASK               ((uint32_t)0x00000400U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_ISET_DMADONEDRX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMADONEDRX_SET                ((uint32_t)0x00000400U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMADONEDTX] Bits */
#define USBLC_ISET_DMADONEDTX_OFS                (11)                            /* !< DMADONEDTX Offset */
#define USBLC_ISET_DMADONEDTX_MASK               ((uint32_t)0x00000800U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_ISET_DMADONEDTX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMADONEDTX_SET                ((uint32_t)0x00000800U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMAPREARX] Bits */
#define USBLC_ISET_DMAPREARX_OFS                 (12)                            /* !< DMAPREARX Offset */
#define USBLC_ISET_DMAPREARX_MASK                ((uint32_t)0x00001000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_ISET_DMAPREARX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMAPREARX_SET                 ((uint32_t)0x00001000U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMAPREATX] Bits */
#define USBLC_ISET_DMAPREATX_OFS                 (13)                            /* !< DMAPREATX Offset */
#define USBLC_ISET_DMAPREATX_MASK                ((uint32_t)0x00002000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_ISET_DMAPREATX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMAPREATX_SET                 ((uint32_t)0x00002000U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMAPREBRX] Bits */
#define USBLC_ISET_DMAPREBRX_OFS                 (14)                            /* !< DMAPREBRX Offset */
#define USBLC_ISET_DMAPREBRX_MASK                ((uint32_t)0x00004000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_ISET_DMAPREBRX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMAPREBRX_SET                 ((uint32_t)0x00004000U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMAPREBTX] Bits */
#define USBLC_ISET_DMAPREBTX_OFS                 (15)                            /* !< DMAPREBTX Offset */
#define USBLC_ISET_DMAPREBTX_MASK                ((uint32_t)0x00008000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_ISET_DMAPREBTX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMAPREBTX_SET                 ((uint32_t)0x00008000U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMAPRECRX] Bits */
#define USBLC_ISET_DMAPRECRX_OFS                 (16)                            /* !< DMAPRECRX Offset */
#define USBLC_ISET_DMAPRECRX_MASK                ((uint32_t)0x00010000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_ISET_DMAPRECRX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMAPRECRX_SET                 ((uint32_t)0x00010000U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMAPRECTX] Bits */
#define USBLC_ISET_DMAPRECTX_OFS                 (17)                            /* !< DMAPRECTX Offset */
#define USBLC_ISET_DMAPRECTX_MASK                ((uint32_t)0x00020000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_ISET_DMAPRECTX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMAPRECTX_SET                 ((uint32_t)0x00020000U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMAPREDRX] Bits */
#define USBLC_ISET_DMAPREDRX_OFS                 (18)                            /* !< DMAPREDRX Offset */
#define USBLC_ISET_DMAPREDRX_MASK                ((uint32_t)0x00040000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_ISET_DMAPREDRX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMAPREDRX_SET                 ((uint32_t)0x00040000U)         /* !< Write 1 to set the interrupt */
/* USBLC_ISET[DMAPREDTX] Bits */
#define USBLC_ISET_DMAPREDTX_OFS                 (19)                            /* !< DMAPREDTX Offset */
#define USBLC_ISET_DMAPREDTX_MASK                ((uint32_t)0x00080000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_ISET_DMAPREDTX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ISET_DMAPREDTX_SET                 ((uint32_t)0x00080000U)         /* !< Write 1 to set the interrupt */

/* USBLC_ICLR Bits */
/* USBLC_ICLR[INTRTX] Bits */
#define USBLC_ICLR_INTRTX_OFS                    (0)                             /* !< INTRTX Offset */
#define USBLC_ICLR_INTRTX_MASK                   ((uint32_t)0x00000001U)         /* !< Endpoint 0 and the TX Endpoints
                                                                                    interrupt */
#define USBLC_ICLR_INTRTX_NO_EFFECT              ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_INTRTX_CLR                    ((uint32_t)0x00000001U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[INTRRX] Bits */
#define USBLC_ICLR_INTRRX_OFS                    (1)                             /* !< INTRRX Offset */
#define USBLC_ICLR_INTRRX_MASK                   ((uint32_t)0x00000002U)         /* !< RX Endpoints interrupt */
#define USBLC_ICLR_INTRRX_NO_EFFECT              ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_INTRRX_CLR                    ((uint32_t)0x00000002U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[INTRUSB] Bits */
#define USBLC_ICLR_INTRUSB_OFS                   (2)                             /* !< INTRUSB Offset */
#define USBLC_ICLR_INTRUSB_MASK                  ((uint32_t)0x00000004U)         /* !< USB Interrupts */
#define USBLC_ICLR_INTRUSB_NO_EFFECT             ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_INTRUSB_CLR                   ((uint32_t)0x00000004U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[VUSBPWRDN] Bits */
#define USBLC_ICLR_VUSBPWRDN_OFS                 (3)                             /* !< VUSBPWRDN Offset */
#define USBLC_ICLR_VUSBPWRDN_MASK                ((uint32_t)0x00000008U)         /* !< VUSB Power Down */
#define USBLC_ICLR_VUSBPWRDN_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_VUSBPWRDN_CLR                 ((uint32_t)0x00000008U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMADONEARX] Bits */
#define USBLC_ICLR_DMADONEARX_OFS                (4)                             /* !< DMADONEARX Offset */
#define USBLC_ICLR_DMADONEARX_MASK               ((uint32_t)0x00000010U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_ICLR_DMADONEARX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMADONEARX_CLR                ((uint32_t)0x00000010U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMADONEATX] Bits */
#define USBLC_ICLR_DMADONEATX_OFS                (5)                             /* !< DMADONEATX Offset */
#define USBLC_ICLR_DMADONEATX_MASK               ((uint32_t)0x00000020U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_ICLR_DMADONEATX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMADONEATX_CLR                ((uint32_t)0x00000020U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMADONEBRX] Bits */
#define USBLC_ICLR_DMADONEBRX_OFS                (6)                             /* !< DMADONEBRX Offset */
#define USBLC_ICLR_DMADONEBRX_MASK               ((uint32_t)0x00000040U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_ICLR_DMADONEBRX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMADONEBRX_CLR                ((uint32_t)0x00000040U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMADONEBTX] Bits */
#define USBLC_ICLR_DMADONEBTX_OFS                (7)                             /* !< DMADONEBTX Offset */
#define USBLC_ICLR_DMADONEBTX_MASK               ((uint32_t)0x00000080U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_ICLR_DMADONEBTX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMADONEBTX_CLR                ((uint32_t)0x00000080U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMADONECRX] Bits */
#define USBLC_ICLR_DMADONECRX_OFS                (8)                             /* !< DMADONECRX Offset */
#define USBLC_ICLR_DMADONECRX_MASK               ((uint32_t)0x00000100U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_ICLR_DMADONECRX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMADONECRX_CLR                ((uint32_t)0x00000100U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMADONECTX] Bits */
#define USBLC_ICLR_DMADONECTX_OFS                (9)                             /* !< DMADONECTX Offset */
#define USBLC_ICLR_DMADONECTX_MASK               ((uint32_t)0x00000200U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_ICLR_DMADONECTX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMADONECTX_CLR                ((uint32_t)0x00000200U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMADONEDRX] Bits */
#define USBLC_ICLR_DMADONEDRX_OFS                (10)                            /* !< DMADONEDRX Offset */
#define USBLC_ICLR_DMADONEDRX_MASK               ((uint32_t)0x00000400U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_ICLR_DMADONEDRX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMADONEDRX_CLR                ((uint32_t)0x00000400U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMADONEDTX] Bits */
#define USBLC_ICLR_DMADONEDTX_OFS                (11)                            /* !< DMADONEDTX Offset */
#define USBLC_ICLR_DMADONEDTX_MASK               ((uint32_t)0x00000800U)         /* !< DMA-Done interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_ICLR_DMADONEDTX_NO_EFFECT          ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMADONEDTX_CLR                ((uint32_t)0x00000800U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMAPREARX] Bits */
#define USBLC_ICLR_DMAPREARX_OFS                 (12)                            /* !< DMAPREARX Offset */
#define USBLC_ICLR_DMAPREARX_MASK                ((uint32_t)0x00001000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-RX */
#define USBLC_ICLR_DMAPREARX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMAPREARX_CLR                 ((uint32_t)0x00001000U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMAPREATX] Bits */
#define USBLC_ICLR_DMAPREATX_OFS                 (13)                            /* !< DMAPREATX Offset */
#define USBLC_ICLR_DMAPREATX_MASK                ((uint32_t)0x00002000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-A-TX */
#define USBLC_ICLR_DMAPREATX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMAPREATX_CLR                 ((uint32_t)0x00002000U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMAPREBRX] Bits */
#define USBLC_ICLR_DMAPREBRX_OFS                 (14)                            /* !< DMAPREBRX Offset */
#define USBLC_ICLR_DMAPREBRX_MASK                ((uint32_t)0x00004000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-RX */
#define USBLC_ICLR_DMAPREBRX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMAPREBRX_CLR                 ((uint32_t)0x00004000U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMAPREBTX] Bits */
#define USBLC_ICLR_DMAPREBTX_OFS                 (15)                            /* !< DMAPREBTX Offset */
#define USBLC_ICLR_DMAPREBTX_MASK                ((uint32_t)0x00008000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-B-TX */
#define USBLC_ICLR_DMAPREBTX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMAPREBTX_CLR                 ((uint32_t)0x00008000U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMAPRECRX] Bits */
#define USBLC_ICLR_DMAPRECRX_OFS                 (16)                            /* !< DMAPRECRX Offset */
#define USBLC_ICLR_DMAPRECRX_MASK                ((uint32_t)0x00010000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-RX */
#define USBLC_ICLR_DMAPRECRX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMAPRECRX_CLR                 ((uint32_t)0x00010000U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMAPRECTX] Bits */
#define USBLC_ICLR_DMAPRECTX_OFS                 (17)                            /* !< DMAPRECTX Offset */
#define USBLC_ICLR_DMAPRECTX_MASK                ((uint32_t)0x00020000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-C-TX */
#define USBLC_ICLR_DMAPRECTX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMAPRECTX_CLR                 ((uint32_t)0x00020000U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMAPREDRX] Bits */
#define USBLC_ICLR_DMAPREDRX_OFS                 (18)                            /* !< DMAPREDRX Offset */
#define USBLC_ICLR_DMAPREDRX_MASK                ((uint32_t)0x00040000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-RX */
#define USBLC_ICLR_DMAPREDRX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMAPREDRX_CLR                 ((uint32_t)0x00040000U)         /* !< Write 1 to clear the interrupt */
/* USBLC_ICLR[DMAPREDTX] Bits */
#define USBLC_ICLR_DMAPREDTX_OFS                 (19)                            /* !< DMAPREDTX Offset */
#define USBLC_ICLR_DMAPREDTX_MASK                ((uint32_t)0x00080000U)         /* !< DMA-Pre interrupt for Trigger
                                                                                    USB-D-TX */
#define USBLC_ICLR_DMAPREDTX_NO_EFFECT           ((uint32_t)0x00000000U)         /* !< Writing 0 has No Effect */
#define USBLC_ICLR_DMAPREDTX_CLR                 ((uint32_t)0x00080000U)         /* !< Write 1 to clear the interrupt */

/* USBLC_PWREN Bits */
/* USBLC_PWREN[ENABLE] Bits */
#define USBLC_PWREN_ENABLE_OFS                   (0)                             /* !< ENABLE Offset */
#define USBLC_PWREN_ENABLE_MASK                  ((uint32_t)0x00000001U)         /* !< Enable the power */
#define USBLC_PWREN_ENABLE_DISABLE               ((uint32_t)0x00000000U)         /* !< Disable Power */
#define USBLC_PWREN_ENABLE_ENABLE                ((uint32_t)0x00000001U)         /* !< Enable Power */
/* USBLC_PWREN[KEY] Bits */
#define USBLC_PWREN_KEY_OFS                      (24)                            /* !< KEY Offset */
#define USBLC_PWREN_KEY_MASK                     ((uint32_t)0xFF000000U)         /* !< KEY to allow Power State Change */
#define USBLC_PWREN_KEY_UNLOCK_W                 ((uint32_t)0x26000000U)         /* !< KEY to allow write access to this
                                                                                    register */

/* USBLC_RSTCTL Bits */
/* USBLC_RSTCTL[RESETSTKYCLR] Bits */
#define USBLC_RSTCTL_RESETSTKYCLR_OFS            (1)                             /* !< RESETSTKYCLR Offset */
#define USBLC_RSTCTL_RESETSTKYCLR_MASK           ((uint32_t)0x00000002U)         /* !< Clear the RESETSTKY bit in the STAT
                                                                                    register */
#define USBLC_RSTCTL_RESETSTKYCLR_NOP            ((uint32_t)0x00000000U)         /* !< Writing 0 has no effect */
#define USBLC_RSTCTL_RESETSTKYCLR_CLR            ((uint32_t)0x00000002U)         /* !< Clear reset sticky bit */
/* USBLC_RSTCTL[RESETASSERT] Bits */
#define USBLC_RSTCTL_RESETASSERT_OFS             (0)                             /* !< RESETASSERT Offset */
#define USBLC_RSTCTL_RESETASSERT_MASK            ((uint32_t)0x00000001U)         /* !< Assert reset to the peripheral */
#define USBLC_RSTCTL_RESETASSERT_NOP             ((uint32_t)0x00000000U)         /* !< Writing 0 has no effect */
#define USBLC_RSTCTL_RESETASSERT_ASSERT          ((uint32_t)0x00000001U)         /* !< Assert reset */
/* USBLC_RSTCTL[KEY] Bits */
#define USBLC_RSTCTL_KEY_OFS                     (24)                            /* !< KEY Offset */
#define USBLC_RSTCTL_KEY_MASK                    ((uint32_t)0xFF000000U)         /* !< Unlock key */
#define USBLC_RSTCTL_KEY_UNLOCK_W                ((uint32_t)0xB1000000U)         /* !< KEY to allow write access to this
                                                                                    register */

/* USBLC_CLKCTL Bits */
/* USBLC_CLKCTL[CLKDIV] Bits */
#define USBLC_CLKCTL_CLKDIV_OFS                  (0)                             /* !< CLKDIV Offset */
#define USBLC_CLKCTL_CLKDIV_MASK                 ((uint32_t)0x0000000FU)         /* !< Divider settings for the functional
                                                                                    clock */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_1             ((uint32_t)0x00000000U)         /* !< Divide by 1 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_2             ((uint32_t)0x00000001U)         /* !< Divide by 2 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_3             ((uint32_t)0x00000002U)         /* !< Divide by 3 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_4             ((uint32_t)0x00000003U)         /* !< Divide by 4 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_5             ((uint32_t)0x00000004U)         /* !< Divide by 5 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_6             ((uint32_t)0x00000005U)         /* !< Divide by 6 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_10            ((uint32_t)0x00000006U)         /* !< Divide by 10 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_12            ((uint32_t)0x00000007U)         /* !< Divide by 12 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_15            ((uint32_t)0x00000008U)         /* !< Divide by 15 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_20            ((uint32_t)0x00000009U)         /* !< Divide by 20 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_30            ((uint32_t)0x0000000AU)         /* !< Divide by 30 */
#define USBLC_CLKCTL_CLKDIV_DIV_BY_60            ((uint32_t)0x0000000BU)         /* !< Divide by 60 */

/* USBLC_STAT Bits */
/* USBLC_STAT[ENABLE] Bits */
#define USBLC_STAT_ENABLE_OFS                    (8)                             /* !< ENABLE Offset */
#define USBLC_STAT_ENABLE_MASK                   ((uint32_t)0x00000100U)         /* !< Peripheral enable status */
#define USBLC_STAT_ENABLE_OFF                    ((uint32_t)0x00000000U)         /* !< Peripheral is disabled */
#define USBLC_STAT_ENABLE_ON                     ((uint32_t)0x00000100U)         /* !< Peripheral is enabled */
/* USBLC_STAT[RESETSTKY] Bits */
#define USBLC_STAT_RESETSTKY_OFS                 (16)                            /* !< RESETSTKY Offset */
#define USBLC_STAT_RESETSTKY_MASK                ((uint32_t)0x00010000U)         /* !< This bit indicates, if the
                                                                                    peripheral was reset, since this bit
                                                                                    was cleared by RESETSTKYCLR in the
                                                                                    RSTCTL register */
#define USBLC_STAT_RESETSTKY_NORES               ((uint32_t)0x00000000U)         /* !< The peripheral has not been reset
                                                                                    since this bit was last cleared by
                                                                                    RESETSTKYCLR in the RSTCTL register */
#define USBLC_STAT_RESETSTKY_RESET               ((uint32_t)0x00010000U)         /* !< The peripheral was reset since the
                                                                                    last bit clear */
/* USBLC_STAT[SWENABLE] Bits */
#define USBLC_STAT_SWENABLE_OFS                  (0)                             /* !< SWENABLE Offset */
#define USBLC_STAT_SWENABLE_MASK                 ((uint32_t)0x00000001U)         /* !< Software enable status. Shows if
                                                                                    enabled via PWREN.ENABLE bit */
#define USBLC_STAT_SWENABLE_DISABLED             ((uint32_t)0x00000000U)         /* !< Not enabled via PWREN.ENABLE bit */
#define USBLC_STAT_SWENABLE_ENABLED              ((uint32_t)0x00000001U)         /* !< Enabled via PWREN.ENABLE bit */

/* USBLC_EVT_MODE Bits */
/* USBLC_EVT_MODE[EVT0_CFG] Bits */
#define USBLC_EVT_MODE_EVT0_CFG_OFS              (0)                             /* !< EVT0_CFG Offset */
#define USBLC_EVT_MODE_EVT0_CFG_MASK             ((uint32_t)0x00000003U)         /* !< Event line mode select for event
                                                                                    corresponding to
                                                                                    (IPSTANDARD.INT_EVENT)(0) */
#define USBLC_EVT_MODE_EVT0_CFG_DISABLE          ((uint32_t)0x00000000U)         /* !< The interrupt or event line is
                                                                                    disabled. */
#define USBLC_EVT_MODE_EVT0_CFG_SOFTWARE         ((uint32_t)0x00000001U)         /* !< The interrupt or event line is in
                                                                                    software mode. Software must clear
                                                                                    the RIS. */
#define USBLC_EVT_MODE_EVT0_CFG_HARDWARE         ((uint32_t)0x00000002U)         /* !< The interrupt or event line is in
                                                                                    hardware mode. The hardware (another
                                                                                    module) clears automatically the
                                                                                    associated RIS flag. */

/* USBLC_DESC Bits */
/* USBLC_DESC[MINREV] Bits */
#define USBLC_DESC_MINREV_OFS                    (0)                             /* !< MINREV Offset */
#define USBLC_DESC_MINREV_MASK                   ((uint32_t)0x0000000FU)         /* !< Minor rev of the IP */
#define USBLC_DESC_MINREV_MINIMUM                ((uint32_t)0x00000000U)         /* !< Smallest value */
#define USBLC_DESC_MINREV_MAXIMUM                ((uint32_t)0x0000000FU)         /* !< Highest possible value */
/* USBLC_DESC[MAJREV] Bits */
#define USBLC_DESC_MAJREV_OFS                    (4)                             /* !< MAJREV Offset */
#define USBLC_DESC_MAJREV_MASK                   ((uint32_t)0x000000F0U)         /* !< Major rev of the IP */
#define USBLC_DESC_MAJREV_MINIMUM                ((uint32_t)0x00000000U)         /* !< Smallest value */
#define USBLC_DESC_MAJREV_MAXIMUM                ((uint32_t)0x000000F0U)         /* !< Highest possible value */
/* USBLC_DESC[FEATUREVER] Bits */
#define USBLC_DESC_FEATUREVER_OFS                (12)                            /* !< FEATUREVER Offset */
#define USBLC_DESC_FEATUREVER_MASK               ((uint32_t)0x0000F000U)         /* !< Feature Set for the module
                                                                                    *instance* */
#define USBLC_DESC_FEATUREVER_MINIMUM            ((uint32_t)0x00000000U)         /* !< Smallest value */
#define USBLC_DESC_FEATUREVER_MAXIMUM            ((uint32_t)0x0000F000U)         /* !< Highest possible value */
/* USBLC_DESC[MODULEID] Bits */
#define USBLC_DESC_MODULEID_OFS                  (16)                            /* !< MODULEID Offset */
#define USBLC_DESC_MODULEID_MASK                 ((uint32_t)0xFFFF0000U)         /* !< Module identification contains a
                                                                                    unique peripheral identification
                                                                                    number. The assignments are
                                                                                    maintained in a central database for
                                                                                    all of the platform modules to ensure
                                                                                    uniqueness. */
#define USBLC_DESC_MODULEID_MINIMUM              ((uint32_t)0x00000000U)         /* !< Smallest value */
#define USBLC_DESC_MODULEID_MAXIMUM              ((uint32_t)0xFFFF0000U)         /* !< Highest possible value */

/* USBLC_USBMODE Bits */
/* USBLC_USBMODE[HOST] Bits */
#define USBLC_USBMODE_HOST_OFS                   (0)                             /* !< HOST Offset */
#define USBLC_USBMODE_HOST_MASK                  ((uint32_t)0x00000001U)         /* !< USB is configured as HOST mode */
#define USBLC_USBMODE_HOST_DISABLE               ((uint32_t)0x00000000U)         /* !< USB is configured as DEVICE or
                                                                                    HOST. */
#define USBLC_USBMODE_HOST_ENABLE                ((uint32_t)0x00000001U)         /* !< USB is configured as HOST */
/* USBLC_USBMODE[DEVICEONLY] Bits */
#define USBLC_USBMODE_DEVICEONLY_OFS             (1)                             /* !< DEVICEONLY Offset */
#define USBLC_USBMODE_DEVICEONLY_MASK            ((uint32_t)0x00000002U)         /* !< USB is configured as DEVICE_ONLY
                                                                                    mode */
#define USBLC_USBMODE_DEVICEONLY_DISABLE         ((uint32_t)0x00000000U)         /* !< USB is configured as DEVICE or
                                                                                    HOST. */
#define USBLC_USBMODE_DEVICEONLY_ENABLE          ((uint32_t)0x00000002U)         /* !< USB is configured as DEVICE only
                                                                                    mode. */
/* USBLC_USBMODE[PHYMODE] Bits */
#define USBLC_USBMODE_PHYMODE_OFS                (4)                             /* !< PHYMODE Offset */
#define USBLC_USBMODE_PHYMODE_MASK               ((uint32_t)0x00000010U)         /* !< USB PHY mode.  Defines whether
                                                                                    DP/DM pins are controled by USB
                                                                                    module or IOMUX/GPIO module. */
#define USBLC_USBMODE_PHYMODE_GPIO               ((uint32_t)0x00000000U)         /* !< The DP/DM pins are assighed to
                                                                                    IOMUX/GPIO module, */
#define USBLC_USBMODE_PHYMODE_USB                ((uint32_t)0x00000010U)         /* !< The DP/DM pins are assighed to USB
                                                                                    module. */

/* USBLC_USBMONITOR Bits */
/* USBLC_USBMONITOR[VUSBEN] Bits */
#define USBLC_USBMONITOR_VUSBEN_OFS              (0)                             /* !< VUSBEN Offset */
#define USBLC_USBMONITOR_VUSBEN_MASK             ((uint32_t)0x00000001U)         /* !< Enable the 1/3 resistor divider for
                                                                                    VUSB supply. Set this bit to monitor
                                                                                    the VUSB supply with the ADC. */
#define USBLC_USBMONITOR_VUSBEN_DISABLE          ((uint32_t)0x00000000U)         /* !< VUSB monitor disabled */
#define USBLC_USBMONITOR_VUSBEN_ENABLE           ((uint32_t)0x00000001U)         /* !< VUSB monitor enabled */

/* USBLC_USBDMASEL Bits */
/* USBLC_USBDMASEL[TRIGARX] Bits */
#define USBLC_USBDMASEL_TRIGARX_OFS              (0)                             /* !< TRIGARX Offset */
#define USBLC_USBDMASEL_TRIGARX_MASK             ((uint32_t)0x00000007U)         /* !< TRIG A RX select specifies the RX
                                                                                    mapping of the USB endpoint on DMA
                                                                                    trigger USB-A-RX */
#define USBLC_USBDMASEL_TRIGARX_DIS              ((uint32_t)0x00000000U)         /* !< Disabled */
#define USBLC_USBDMASEL_TRIGARX_EP1              ((uint32_t)0x00000001U)         /* !< Endpoint 1 */
#define USBLC_USBDMASEL_TRIGARX_EP2              ((uint32_t)0x00000002U)         /* !< Endpoint 2 */
#define USBLC_USBDMASEL_TRIGARX_EP3              ((uint32_t)0x00000003U)         /* !< Endpoint 3 */
#define USBLC_USBDMASEL_TRIGARX_EP4              ((uint32_t)0x00000004U)         /* !< Endpoint 4 */
#define USBLC_USBDMASEL_TRIGARX_EP5              ((uint32_t)0x00000005U)         /* !< Endpoint 5 */
#define USBLC_USBDMASEL_TRIGARX_EP6              ((uint32_t)0x00000006U)         /* !< Endpoint 6 */
#define USBLC_USBDMASEL_TRIGARX_EP7              ((uint32_t)0x00000007U)         /* !< Endpoint 7 */
/* USBLC_USBDMASEL[TRIGATX] Bits */
#define USBLC_USBDMASEL_TRIGATX_OFS              (4)                             /* !< TRIGATX Offset */
#define USBLC_USBDMASEL_TRIGATX_MASK             ((uint32_t)0x00000070U)         /* !< TRIG A TX select specifies the TX
                                                                                    mapping of the USB endpoint on DMA
                                                                                    trigger USB-A-TX */
#define USBLC_USBDMASEL_TRIGATX_DIS              ((uint32_t)0x00000000U)         /* !< Disabled */
#define USBLC_USBDMASEL_TRIGATX_EP1              ((uint32_t)0x00000010U)         /* !< Endpoint 1 */
#define USBLC_USBDMASEL_TRIGATX_EP2              ((uint32_t)0x00000020U)         /* !< Endpoint 2 */
#define USBLC_USBDMASEL_TRIGATX_EP3              ((uint32_t)0x00000030U)         /* !< Endpoint 3 */
#define USBLC_USBDMASEL_TRIGATX_EP4              ((uint32_t)0x00000040U)         /* !< Endpoint 4 */
#define USBLC_USBDMASEL_TRIGATX_EP5              ((uint32_t)0x00000050U)         /* !< Endpoint 5 */
#define USBLC_USBDMASEL_TRIGATX_EP6              ((uint32_t)0x00000060U)         /* !< Endpoint 6 */
#define USBLC_USBDMASEL_TRIGATX_EP7              ((uint32_t)0x00000070U)         /* !< Endpoint 7 */
/* USBLC_USBDMASEL[TRIGBRX] Bits */
#define USBLC_USBDMASEL_TRIGBRX_OFS              (8)                             /* !< TRIGBRX Offset */
#define USBLC_USBDMASEL_TRIGBRX_MASK             ((uint32_t)0x00000700U)         /* !< TRIG B RX select specifies the RX
                                                                                    mapping of the USB endpoint on DMA
                                                                                    trigger USB-B-RX */
#define USBLC_USBDMASEL_TRIGBRX_DIS              ((uint32_t)0x00000000U)         /* !< Disabled */
#define USBLC_USBDMASEL_TRIGBRX_EP1              ((uint32_t)0x00000100U)         /* !< Endpoint 1 */
#define USBLC_USBDMASEL_TRIGBRX_EP2              ((uint32_t)0x00000200U)         /* !< Endpoint 2 */
#define USBLC_USBDMASEL_TRIGBRX_EP3              ((uint32_t)0x00000300U)         /* !< Endpoint 3 */
#define USBLC_USBDMASEL_TRIGBRX_EP4              ((uint32_t)0x00000400U)         /* !< Endpoint 4 */
#define USBLC_USBDMASEL_TRIGBRX_EP5              ((uint32_t)0x00000500U)         /* !< Endpoint 5 */
#define USBLC_USBDMASEL_TRIGBRX_EP6              ((uint32_t)0x00000600U)         /* !< Endpoint 6 */
#define USBLC_USBDMASEL_TRIGBRX_EP7              ((uint32_t)0x00000700U)         /* !< Endpoint 7 */
/* USBLC_USBDMASEL[TRIGBTX] Bits */
#define USBLC_USBDMASEL_TRIGBTX_OFS              (12)                            /* !< TRIGBTX Offset */
#define USBLC_USBDMASEL_TRIGBTX_MASK             ((uint32_t)0x00007000U)         /* !< TRIG B TX select specifies the TX
                                                                                    mapping of the USB endpoint on DMA
                                                                                    trigger USB-B-TX */
#define USBLC_USBDMASEL_TRIGBTX_DIS              ((uint32_t)0x00000000U)         /* !< Disabled */
#define USBLC_USBDMASEL_TRIGBTX_EP1              ((uint32_t)0x00001000U)         /* !< Endpoint 1 */
#define USBLC_USBDMASEL_TRIGBTX_EP2              ((uint32_t)0x00002000U)         /* !< Endpoint 2 */
#define USBLC_USBDMASEL_TRIGBTX_EP3              ((uint32_t)0x00003000U)         /* !< Endpoint 3 */
#define USBLC_USBDMASEL_TRIGBTX_EP4              ((uint32_t)0x00004000U)         /* !< Endpoint 4 */
#define USBLC_USBDMASEL_TRIGBTX_EP5              ((uint32_t)0x00005000U)         /* !< Endpoint 5 */
#define USBLC_USBDMASEL_TRIGBTX_EP6              ((uint32_t)0x00006000U)         /* !< Endpoint 6 */
#define USBLC_USBDMASEL_TRIGBTX_EP7              ((uint32_t)0x00007000U)         /* !< Endpoint 7 */
/* USBLC_USBDMASEL[TRIGCRX] Bits */
#define USBLC_USBDMASEL_TRIGCRX_OFS              (16)                            /* !< TRIGCRX Offset */
#define USBLC_USBDMASEL_TRIGCRX_MASK             ((uint32_t)0x00070000U)         /* !< TRIG C RX select specifies the RX
                                                                                    mapping of the USB endpoint on DMA
                                                                                    trigger USB-C-RX */
#define USBLC_USBDMASEL_TRIGCRX_DIS              ((uint32_t)0x00000000U)         /* !< Disabled */
#define USBLC_USBDMASEL_TRIGCRX_EP1              ((uint32_t)0x00010000U)         /* !< Endpoint 1 */
#define USBLC_USBDMASEL_TRIGCRX_EP2              ((uint32_t)0x00020000U)         /* !< Endpoint 2 */
#define USBLC_USBDMASEL_TRIGCRX_EP3              ((uint32_t)0x00030000U)         /* !< Endpoint 3 */
#define USBLC_USBDMASEL_TRIGCRX_EP4              ((uint32_t)0x00040000U)         /* !< Endpoint 4 */
#define USBLC_USBDMASEL_TRIGCRX_EP5              ((uint32_t)0x00050000U)         /* !< Endpoint 5 */
#define USBLC_USBDMASEL_TRIGCRX_EP6              ((uint32_t)0x00060000U)         /* !< Endpoint 6 */
#define USBLC_USBDMASEL_TRIGCRX_EP7              ((uint32_t)0x00070000U)         /* !< Endpoint 7 */
/* USBLC_USBDMASEL[TRIGCTX] Bits */
#define USBLC_USBDMASEL_TRIGCTX_OFS              (20)                            /* !< TRIGCTX Offset */
#define USBLC_USBDMASEL_TRIGCTX_MASK             ((uint32_t)0x00700000U)         /* !< TRIG C TX select specifies the TX
                                                                                    mapping of the USB endpoint on DMA
                                                                                    trigger USB-C-TX */
#define USBLC_USBDMASEL_TRIGCTX_DIS              ((uint32_t)0x00000000U)         /* !< Disabled */
#define USBLC_USBDMASEL_TRIGCTX_EP1              ((uint32_t)0x00100000U)         /* !< Endpoint 1 */
#define USBLC_USBDMASEL_TRIGCTX_EP2              ((uint32_t)0x00200000U)         /* !< Endpoint 2 */
#define USBLC_USBDMASEL_TRIGCTX_EP3              ((uint32_t)0x00300000U)         /* !< Endpoint 3 */
#define USBLC_USBDMASEL_TRIGCTX_EP4              ((uint32_t)0x00400000U)         /* !< Endpoint 4 */
#define USBLC_USBDMASEL_TRIGCTX_EP5              ((uint32_t)0x00500000U)         /* !< Endpoint 5 */
#define USBLC_USBDMASEL_TRIGCTX_EP6              ((uint32_t)0x00600000U)         /* !< Endpoint 6 */
#define USBLC_USBDMASEL_TRIGCTX_EP7              ((uint32_t)0x00700000U)         /* !< Endpoint 7 */
/* USBLC_USBDMASEL[TRIGDRX] Bits */
#define USBLC_USBDMASEL_TRIGDRX_OFS              (24)                            /* !< TRIGDRX Offset */
#define USBLC_USBDMASEL_TRIGDRX_MASK             ((uint32_t)0x07000000U)         /* !< TRIG D RX select specifies the RX
                                                                                    mapping of the USB endpoint on DMA
                                                                                    trigger USB-D-RX */
#define USBLC_USBDMASEL_TRIGDRX_DIS              ((uint32_t)0x00000000U)         /* !< Disabled */
#define USBLC_USBDMASEL_TRIGDRX_EP1              ((uint32_t)0x01000000U)         /* !< Endpoint 1 */
#define USBLC_USBDMASEL_TRIGDRX_EP2              ((uint32_t)0x02000000U)         /* !< Endpoint 2 */
#define USBLC_USBDMASEL_TRIGDRX_EP3              ((uint32_t)0x03000000U)         /* !< Endpoint 3 */
#define USBLC_USBDMASEL_TRIGDRX_EP4              ((uint32_t)0x04000000U)         /* !< Endpoint 4 */
#define USBLC_USBDMASEL_TRIGDRX_EP5              ((uint32_t)0x05000000U)         /* !< Endpoint 5 */
#define USBLC_USBDMASEL_TRIGDRX_EP6              ((uint32_t)0x06000000U)         /* !< Endpoint 6 */
#define USBLC_USBDMASEL_TRIGDRX_EP7              ((uint32_t)0x07000000U)         /* !< Endpoint 7 */
/* USBLC_USBDMASEL[TRIGDTX] Bits */
#define USBLC_USBDMASEL_TRIGDTX_OFS              (28)                            /* !< TRIGDTX Offset */
#define USBLC_USBDMASEL_TRIGDTX_MASK             ((uint32_t)0x70000000U)         /* !< TRIG D TX select specifies the TX
                                                                                    mapping of the USB endpoint on DMA
                                                                                    trigger USB-D-TX */
#define USBLC_USBDMASEL_TRIGDTX_DIS              ((uint32_t)0x00000000U)         /* !< Disabled */
#define USBLC_USBDMASEL_TRIGDTX_EP1              ((uint32_t)0x10000000U)         /* !< Endpoint 1 */
#define USBLC_USBDMASEL_TRIGDTX_EP2              ((uint32_t)0x20000000U)         /* !< Endpoint 2 */
#define USBLC_USBDMASEL_TRIGDTX_EP3              ((uint32_t)0x30000000U)         /* !< Endpoint 3 */
#define USBLC_USBDMASEL_TRIGDTX_EP4              ((uint32_t)0x40000000U)         /* !< Endpoint 4 */
#define USBLC_USBDMASEL_TRIGDTX_EP5              ((uint32_t)0x50000000U)         /* !< Endpoint 5 */
#define USBLC_USBDMASEL_TRIGDTX_EP6              ((uint32_t)0x60000000U)         /* !< Endpoint 6 */
#define USBLC_USBDMASEL_TRIGDTX_EP7              ((uint32_t)0x70000000U)         /* !< Endpoint 7 */


#ifdef __cplusplus
}
#endif

#endif /* ti_devices_msp_peripherals_hw_usblc__include */
