/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024, Brent Kowal (Analog Devices, Inc)
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

#ifndef TUSB_MUSB_TI_H_
#define TUSB_MUSB_TI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#if CFG_TUSB_MCU == OPT_MCU_TM4C123
  #include "TM4C123.h"
  #define FIFO0_WORD FIFO0
  #define FIFO1_WORD FIFO1
//#elif CFG_TUSB_MCU == OPT_MCU_TM4C129
#elif CFG_TUSB_MCU == OPT_MCU_MSP432E4
  #include "msp.h"
#elif (CFG_TUSB_MCU == OPT_MCU_MSPM0G511X || CFG_TUSB_MCU == OPT_MCU_MSPM0G518X)
  #include "msp.h"
  #define USB0_BASE (USB_REGISTERS_OFS+USBFS0_BASE)
  #define USB0_IRQn USBFS0_INT_IRQn
  #define MSPM0USB ((USB_Regs*)USBFS0_BASE)
#elif (CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
  #include "msp.h"
  #define USB0_BASE (USBLC_REGISTERS_OFS+USBLC0_BASE)
  #define USB0_IRQn USBLC0_INT_IRQn
  #define MSPM0USB ((USBLC_Regs*)USBLC0_BASE)
  #define USB_IMASK_INTRTX_SET  USBLC_IMASK_INTRTX_SET
  #define USB_IMASK_INTRRX_SET  USBLC_IMASK_INTRRX_SET
  #define USB_IMASK_INTRUSB_SET USBLC_IMASK_INTRUSB_SET
#else
  #error "Unsupported MCUs"
#endif

#define MUSB_CFG_SHARED_FIFO       0
#if (CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
#define MUSB_CFG_DYNAMIC_FIFO      0
#define MUSB_CFG_FIFO_SIZE 2048
#else
#define MUSB_CFG_DYNAMIC_FIFO      1
#define MUSB_CFG_DYNAMIC_FIFO_SIZE 4096
#endif

const uintptr_t MUSB_BASES[] = { USB0_BASE };

// Header supports both device and host modes. Only include what's necessary
#if CFG_TUD_ENABLED

// Mapping of IRQ numbers to port. Currently just 1.
static const IRQn_Type  musb_irqs[] = {
    USB0_IRQn
};

#if (CFG_TUSB_MCU == OPT_MCU_MSPM0G511X || CFG_TUSB_MCU == OPT_MCU_MSPM0G518X ||\
    CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
static inline void MSPM0_EnableIRQ(uint32_t USB_IRQ);
static inline void MSPM0_DisableIRQ(uint32_t USB_IRQ);
static inline uint32_t MSPM0_GetEnableIRQ(uint32_t USB_IRQ);
static inline void MSPM0_ClearPendingIRQ(uint32_t USB_IRQ);
#endif 

static inline void musb_dcd_phy_init(uint8_t rhport){
  (void)rhport;
  //Nothing to do for this part
}

TU_ATTR_ALWAYS_INLINE static inline void musb_dcd_int_enable(uint8_t rhport) {
  
  #if (CFG_TUSB_MCU == OPT_MCU_MSPM0G511X || CFG_TUSB_MCU == OPT_MCU_MSPM0G518X ||\
      CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
  (void) rhport;
  MSPM0_EnableIRQ(USB0_IRQn);
  #else
  NVIC_EnableIRQ(musb_irqs[rhport]);
  #endif
}

TU_ATTR_ALWAYS_INLINE static inline void musb_dcd_int_disable(uint8_t rhport) {
  #if (CFG_TUSB_MCU == OPT_MCU_MSPM0G511X || CFG_TUSB_MCU == OPT_MCU_MSPM0G518X ||\
      CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
  (void) rhport;
  MSPM0_DisableIRQ(USB0_IRQn);
  #else
  NVIC_DisableIRQ(musb_irqs[rhport]);
  #endif

}

TU_ATTR_ALWAYS_INLINE static inline unsigned musb_dcd_get_int_enable(uint8_t rhport) {

  #if (CFG_TUSB_MCU == OPT_MCU_MSPM0G511X || CFG_TUSB_MCU == OPT_MCU_MSPM0G518X ||\
      CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
  (void) rhport;
  return MSPM0_GetEnableIRQ(USB0_IRQn);
  #else
  return NVIC_GetEnableIRQ(musb_irqs[rhport]);
  #endif
}

TU_ATTR_ALWAYS_INLINE static inline void musb_dcd_int_clear(uint8_t rhport) {
  
  #if (CFG_TUSB_MCU == OPT_MCU_MSPM0G511X || CFG_TUSB_MCU == OPT_MCU_MSPM0G518X ||\
      CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
  (void) rhport;
  MSPM0_ClearPendingIRQ(USB0_IRQn);
  #else
  NVIC_ClearPendingIRQ(musb_irqs[rhport]);
  #endif
}

static inline void musb_dcd_int_handler_enter(uint8_t rhport) {
  (void)rhport;
  //Nothing to do for this part
}

static inline void MSPM0_EnableIRQ(uint32_t USB_IRQ)
{
  (void)USB_IRQ;
  MSPM0USB->CPU_INT.IMASK |= (USB_IMASK_INTRTX_SET |  USB_IMASK_INTRRX_SET | USB_IMASK_INTRUSB_SET);
}

static inline void MSPM0_DisableIRQ(uint32_t USB_IRQ)
{
  (void)USB_IRQ;
  MSPM0USB->CPU_INT.IMASK = 0;
}

static inline void MSPM0_ClearPendingIRQ(uint32_t USB_IRQ)
{
  (void)USB_IRQ;
  MSPM0USB->CPU_INT.ICLR = MSPM0USB->CPU_INT.MIS;
}

static inline uint32_t MSPM0_GetEnableIRQ(uint32_t USB_IRQ)
{
  (void)USB_IRQ;
  return (uint32_t)(MSPM0USB->CPU_INT.IMASK);
  
}

#endif // CFG_TUD_ENABLED

#ifdef __cplusplus
 }
#endif

#endif // TUSB_MUSB_TI_H_
