/******************************************************************************
*
* Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
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
 ******************************************************************************/

#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/dl_common.h>

/*! 2 flash wait states */
#define    SYSCTL_FLASH_WAIT_STATE_2    0x00000200U

/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
    /* Low Power Mode is configured to be SLEEP0 */
    SYSCTL->SOCLOCK.BORTHRESHOLD = (uint32_t) SYSCTL_BORTHRESHOLD_LEVEL_BORMIN;

    /* Configuring SYSOSC to operate at 32 MHz */
    DL_Common_updateReg(&SYSCTL->SOCLOCK.SYSOSCCFG, (uint32_t)SYSCTL_SYSOSCCFG_FREQ_SYSOSCBASE, SYSCTL_SYSOSCCFG_FREQ_MASK);

    /* Configuring USBFLL to be sourced from SOF */
    SYSCTL->SOCLOCK.USBFLLCTL &= ~(SYSCTL_USBFLLCTL_REFSEL_LFCLK);

    /* Enabling USB oscillator clock */
    SYSCTL->SOCLOCK.USBFLLCTL |= (SYSCTL_USBFLLCTL_CLKEN_TRUE);

    /* Polling for completion of USB oscillator initializiation */
    while((SYSCTL->SOCLOCK.USBFLLSTAT & SYSCTL_USBFLLSTAT_CLKRDY_MASK) !=
            SYSCTL_USBFLLSTAT_CLKRDY_TRUE)
                ;

    /* Enabling USBFLL */
    SYSCTL->SOCLOCK.USBFLLCTL |= SYSCTL_USBFLLCTL_FLLEN_TRUE;

    /* Configuring USBFLL4HSCLK to use USBFLLCLK */
    SYSCTL->SOCLOCK.HSCLKCFG = SYSCTL_HSCLKCFG_USBFLL4HSCLK_USBFLL;

    /* 
     * Setting ULPCLK divider to 2 so ULPCLK will be at 24 MHz
     * 
     * TODO : SYSCTL_MCLKCFG_UDIV_DIVIDE2 and SYSCTL_MCLKCFG_UDIV_MASK
     * defines are currently not available. Follow-up with Michael
     * on missing defiens
     */
    
    DL_Common_updateReg(&SYSCTL->SOCLOCK.MCLKCFG, (uint32_t) 0x00000010U,
                        (uint32_t)0x00000030U);

    /* Setting MCLKCFG to use HSCLK as source */
    SYSCTL->SOCLOCK.MCLKCFG |= SYSCTL_MCLKCFG_USEHSCLK_ENABLE;


}