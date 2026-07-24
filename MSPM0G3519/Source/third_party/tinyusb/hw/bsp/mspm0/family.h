/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Ha Thach (tinyusb.org)
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

#ifndef FAMILY_H_
#define FAMILY_H_

/* Defines utilized for USB to ensure compatiblity between MSPM0 USB IP */
#if (CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
/* Define for USB handler */
#define MSPUSB                          ((USBLC_Regs*)USBLC0_BASE)
/* Define for USB interrupt enum */
#define USB_INT_IRQn                    (USBLC0_INT_IRQn)
#else
/* Define for USB handler */
#define MSPUSB                          ((USB_Regs*)USBFS0_BASE)
/* Define for USB interrupt enum */
#define USB_INT_IRQn                    (USBFS0_INT_IRQn)
#endif

/* USB defines for compatibility between MSPM0 USB IP */
#define USB_RSTCTL_KEY_UNLOCK_W         ((uint32_t)0xB1000000U)
#define USB_RSTCTL_RESETSTKYCLR_CLR     ((uint32_t)0x00000002U)
#define USB_RSTCTL_RESETASSERT_ASSERT   ((uint32_t)0x00000001U)
#define USB_PWREN_ENABLE_ENABLE         ((uint32_t)0x00000001U)
#define USB_PWREN_KEY_UNLOCK_W          ((uint32_t)0x26000000U)
#define USB_USBMODE_DEVICEONLY_ENABLE   ((uint32_t)0x00000002U)
#define USB_USBMODE_PHYMODE_USB         ((uint32_t)0x00000010U)
#define USB_ICLR_INTRUSB_CLR            ((uint32_t)0x00000004U)
#define USB_ICLR_VUSBPWRDN_CLR          ((uint32_t)0x00000008U)

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef __cplusplus
 }
#endif

#endif