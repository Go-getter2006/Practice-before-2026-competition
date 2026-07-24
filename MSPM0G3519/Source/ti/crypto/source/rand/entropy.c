/*
 * Copyright (C) 2025 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 * Limited License.  
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free,
 * non-exclusive license under copyrights and patents it now or hereafter
 * owns or controls to make, have made, use, import, offer to sell and sell
 * ("Utilize") this software subject to the terms herein.  With respect to the
 * foregoing patent license, such license is granted  solely to the extent that
 * any such patent is necessary to Utilize the software alone.  The patent
 * license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI
 * (TI Devices).  No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this
 * license (including the above copyright notice and the disclaimer and (if
 * applicable) source code license limitations below) in the documentation
 * and/or other materials provided with the distribution.
 *
 * Redistribution and use in binary form, without modification, are permitted
 * provided that the following conditions are met:
 *
 *	* No reverse engineering, decompilation, or disassembly of this software is
 *     permitted with respect to any software provided in binary form.
 *	* Any redistribution and use are licensed by TI for use only with
 *     TI Devices.
 *	* Nothing shall obligate TI to provide you with source code for the software
 *     licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution
 * of the source code are permitted provided that the following conditions are
 * met:
 *
 *   * any redistribution and use of the source code, including any resulting
 *     derivative works, are licensed by TI for use only with TI Devices.
 *   * any redistribution and use of any object code compiled from the source
 *     code and any resulting derivative works, are licensed by TI for use only
 *     with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its
 * suppliers may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*!****************************************************************************
 *  @file       entropy.c
 *
 ******************************************************************************
 */

#include <stdint.h>
#ifdef CRYPTO_USE_SDK_HSF
#include "ti/devices/msp/msp.h"
#else
#include "ti/devices/msp/hwcrypto.h"
#endif
#include "ti/crypto/include/rand/entropy.h"

#if CRYPTO_HW_USE_MSPM0_TRNG == 1 || CRYPTO_HW_USE_MSPM33_TRNG == 1

/* Internal definitions */

/** @brief The number of leading samples to trim from the TRNG after init
 *         The purpose of this trim is to remove any remaining deterministic
 *         information left over from the self-tests.
 */
#define CRYPTO_ENTROPY_TRNG_LEADING_SAMPLE_TRIM 8U

/**
 * @brief The default decimation rate for the TRNG module.
 */
#define CRYPTO_ENTROPY_TRNG_DEFAULT_DECIMATION 4U

/**
 * @brief The CPU counter timeout after which a TRNG operation which has not
 *        completed will be aborted and CRYPTO_ENTROPY_TRNG_TIMEOUT_ERR will
 *        be returned to the application.
 */
#define CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT 80000000U

/* Internal type declarations */

/**
 *  @brief      TRNG command values.
 *
 *              These commands are used to control the operation mode of
 *              the TRNG. Commands are passed to Crypto_runTRNGCmd to 
 *              change the TRNG state.
 */
typedef enum {
    /** Turn off the TRNG */
    CRYPTO_ENTROPY_TRNG_CMD_OFF = 0U,
    /** Run power on self test of the digital functions */
    CRYPTO_ENTROPY_TRNG_CMD_POST_DIG = 1U,
    /** Run power on self test of the analog functions */
    CRYPTO_ENTROPY_TRNG_CMD_POST_ANA = 2U,
    /* Put the TRNG into normal function mode */
    CRYPTO_ENTROPY_TRNG_CMD_NORM = 3U,
} crypto_entropy_trng_cmd_t;

typedef enum {
    /** State is OFF */
    CRYPTO_ENTROPY_FSM_OFF = 0U,
    /** State is PWRUP */
    CRYPTO_ENTROPY_FSM_PWRUP = 1U,
    /** State is PWRDOWN */
    CRYPTO_ENTROPY_FSM_PWRDOWN = 2U,
    /** State is NORM_FUNC */
    CRYPTO_ENTROPY_FSM_NORM_FUNC = 3U,
    /** State is TEST_DIG */
    CRYPTO_ENTROPY_FSM_TEST_DIG = 7U,
    /** State is ERROR */
    CRYPTO_ENTROPY_FSM_ERROR = 10U,
    /** State is TEST_ANA */
    CRYPTO_ENTROPY_FSM_TEST_ANA = 11U,
    /** Read error (status read failed) */
    CRYPTO_ENTROPY_FSM_STATE_READ_ERR = 0xFF,
} crypto_entropy_fsm_state_t;

/* Internal function declarations */

/**
 *  @brief      Initialize the TRNG for use.
 *
 *  @param[in]  inClkFreq      The frequency of the bus clock used by the TRNG
 *
 *  @return     The status of the operation, one of crypto_entropy_stat_t.
 *  @retval     CRYPTO_ENTROPY_OK if len words of random data were
 *              successfully collected and stored in the output buffer.
 *  @retval     CRYPTO_ENTROPY_TRNG_CLOCK_SEL_ERR if the TRNG input clock
 *              frequency did not support the correct TRNG clock range.
 */
static crypto_entropy_stat_t Crypto_initTRNG(uint32_t inClkFreq);

/**
 *  @brief      Set TRNG clock based on bus clock frequency
 *
 *  @param[in]  inClkFreq      The frequency of the bus clock used by the TRNG
 *
 *  @return     The status of the operation, one of crypto_entropy_stat_t.
 *  @retval     CRYPTO_ENTROPY_OK if len words of random data were
 *              successfully collected and stored in the output buffer.
 *  @retval     CRYPTO_ENTROPY_TRNG_CLOCK_SEL_ERR if the TRNG input clock
 *              frequency did not support the correct TRNG clock range.
 */
static crypto_entropy_stat_t Crypto_setTRNGClock(uint32_t inClkFreq);

/**
 *  @brief      Set the TRNG decimation rate.
 *
 *              This function sets the TRNG decimation rate.
 *
 *  @return     None
 */
static void Crypto_setTRNGDecimation(uint32_t decimation);

/**
 *  @brief      Get TRNG FSM state
 *
 *  @return     The FSM state, one of crypto_entropy_fsm_state_t.
 */
static crypto_entropy_fsm_state_t Crypto_getTRNGFSMState(void);

/**
 *  @brief      Execute a TRNG command
 *
 *  @param[in]  cmd            The command to execute
 *  @param[in]  timeout        The maximum number of completion check iterations
 *                             to wait for the command to complete.
 *
 *  @return     The status of the operation, one of crypto_entropy_stat_t.
 *  @retval     CRYPTO_ENTROPY_OK if len words of random data were
 *              successfully collected and stored in the output buffer.
 *  @retval     CRYPTO_ENTROPY_TRNG_TIMEOUT_ERR if the command failed to
 *              complete within the timeout period.
 */
static crypto_entropy_stat_t Crypto_runTRNGCmd(crypto_entropy_trng_cmd_t cmd, \
                                               uint32_t timeout);

/**
 *  @brief      Run the power-on self-tests.
 *
 *              This function executes the power-on self-tests.
 *
 *  @param[in]  timeout        The maximum number of completion check iterations
 *                             to wait for the command to complete.
 *
 *  @return     The status of the operation, one of crypto_entropy_stat_t.
 *  @retval     CRYPTO_ENTROPY_OK if the power-on self-tests completed
 *              successfully.
 *  @retval     CRYPTO_ENTROPY_TRNG_POST_DIG_FAIL if the power-on self-test
 *              of the TRNG entropy source digital logic failed.
 *  @retval     CRYPTO_ENTROPY_TRNG_POST_ANA_FAIL if the power-on self-test
 *              of the TRNG entropy source analog logic failed.
 */
static crypto_entropy_stat_t Crypto_runPowerOnSelfTests(uint32_t timeout);

/* Internal function implementations */

crypto_entropy_stat_t Crypto_initTRNG(uint32_t inClkFreq)
{
    crypto_entropy_stat_t stat;

    TRNG->GPRCM.RSTCTL = TRNG_RSTCTL_RESETASSERT_ASSERT |\
                         TRNG_RSTCTL_KEY_UNLOCK_W;
    TRNG->GPRCM.PWREN = TRNG_PWREN_KEY_UNLOCK_W |\
                        TRNG_PWREN_ENABLE_ENABLE;
    stat = Crypto_setTRNGClock(inClkFreq);
    TRNG->CPU_INT.ICLR = TRNG_ICLR_IRQ_HEALTH_FAIL_CLR;

    return stat;
}

crypto_entropy_stat_t Crypto_setTRNGClock(uint32_t inClkFreq)
{
    crypto_entropy_stat_t stat;
    uint32_t trngClkDiv;

    stat = CRYPTO_ENTROPY_OK;

    if (inClkFreq <= 10000000U) {
        stat = CRYPTO_ENTROPY_TRNG_CLOCK_SEL_ERR;
    } else if (inClkFreq <= 20000000U) {
        trngClkDiv = TRNG_CLKDIVIDE_RATIO_DIV_BY_1;
    } else if (inClkFreq <= 40000000U) {
        trngClkDiv = TRNG_CLKDIVIDE_RATIO_DIV_BY_2;
    } else if (inClkFreq <= 80000000U) {
        trngClkDiv = TRNG_CLKDIVIDE_RATIO_DIV_BY_4;
    } else {
        stat = CRYPTO_ENTROPY_TRNG_CLOCK_SEL_ERR;
    }

    if (stat == CRYPTO_ENTROPY_OK) {
        TRNG->CLKDIVIDE = trngClkDiv;
    }

    return stat;
}

crypto_entropy_fsm_state_t Crypto_getTRNGFSMState(void)
{
    uint32_t fsmState1;
    uint32_t fsmState2;
    crypto_entropy_fsm_state_t state;
    
    /* Read twice due to metastability requirement (from TRM) */
    fsmState1 = (TRNG->STAT & TRNG_STAT_FSM_STATE_MASK);
    fsmState2 = (TRNG->STAT & TRNG_STAT_FSM_STATE_MASK);

    if (fsmState1 == fsmState2) {
        state = (crypto_entropy_fsm_state_t)(fsmState1 >> TRNG_STAT_FSM_STATE_OFS);
    } else {
        state = CRYPTO_ENTROPY_FSM_STATE_READ_ERR;
    }

    return state;
}

crypto_entropy_stat_t Crypto_runTRNGCmd(crypto_entropy_trng_cmd_t cmd, 
                                        uint32_t timeout)
{
    uint32_t reg;
    crypto_entropy_stat_t stat;

    stat = CRYPTO_ENTROPY_OK;

    TRNG->CPU_INT.ICLR = TRNG_ICLR_IRQ_CMD_DONE_CLR;

    reg = TRNG->CTL;
    reg &= ~TRNG_CTL_CMD_MASK;
    TRNG->CTL = reg | (cmd & TRNG_CTL_CMD_MASK);

    while ((TRNG->CPU_INT.RIS & TRNG_RIS_IRQ_CMD_DONE_SET) == 0U) {
        if (timeout-- > 0U) {
            continue;
        } else {
            stat = CRYPTO_ENTROPY_TRNG_TIMEOUT_ERR;
            break;
        }
    }

    return stat;
}

void Crypto_setTRNGDecimation(uint32_t decimation)
{
    uint32_t reg;

    decimation &= 0x07;

    reg = TRNG->CTL;
    reg &= ~TRNG_CTL_DECIM_RATE_MASK;
    TRNG->CTL = reg | (decimation << TRNG_CTL_DECIM_RATE_OFS);
}

crypto_entropy_stat_t Crypto_runPowerOnSelfTests(uint32_t timeout)
{
    crypto_entropy_stat_t stat;

    stat = CRYPTO_ENTROPY_OK;

    /* Run the power-up digital self-test. */
    stat = Crypto_runTRNGCmd(CRYPTO_ENTROPY_TRNG_CMD_POST_DIG, \
                             CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
    if (stat != CRYPTO_ENTROPY_OK) {
        goto exit;
    } else {
        if ((TRNG->TEST_RESULTS & TRNG_TEST_RESULTS_DIG_TEST_MASK) != 0xFFU) {
            stat = CRYPTO_ENTROPY_TRNG_POST_DIG_FAIL;
            goto exit;
        } else {
            /* Continue */
        }
    }

    /* Run the power-up analog self-test. */
    stat = Crypto_runTRNGCmd(CRYPTO_ENTROPY_TRNG_CMD_POST_ANA, \
                             CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
    if (stat != CRYPTO_ENTROPY_OK) {
        goto exit;
    } else {
        if ((TRNG->TEST_RESULTS & TRNG_TEST_RESULTS_ANA_TEST_MASK) != 0x100U) {
            (void)Crypto_runTRNGCmd(CRYPTO_ENTROPY_TRNG_CMD_OFF, \
                CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
            stat = CRYPTO_ENTROPY_TRNG_POST_ANA_FAIL;
            goto exit;
        } else {
            /* Continue */
        }
    }

exit:
    return stat;
}

/* External function implementations */

crypto_entropy_stat_t Crypto_powerOnTRNG(uint32_t inClkFreq, uint32_t decimation)
{
    crypto_entropy_stat_t stat;
    uint32_t preTrimLen;
    uint32_t trimmedData;

    /* Start the TRNG module by enabling it and bringing it to NORMAL mode. */
    stat = Crypto_initTRNG(inClkFreq);
    if (stat != CRYPTO_ENTROPY_OK) {
        goto exit;
    } else {
        /* Continue */
    }

    stat = Crypto_runTRNGCmd(CRYPTO_ENTROPY_TRNG_CMD_NORM, \
                             CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
    if (stat != CRYPTO_ENTROPY_OK) {
        goto exit;
    } else {
        /* Continue */
    }

    /* Run the power-on self tests*/
    stat = Crypto_runPowerOnSelfTests(CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
    if (stat != CRYPTO_ENTROPY_OK) {
        goto exit;
    } else {
        /* Continue */
    }

    /* Clear the ready interrupt, set decimiation, 
     * and reload the normal state 
     */
    TRNG->CPU_INT.ICLR = TRNG_ICLR_IRQ_CAPTURED_RDY_CLR;
    Crypto_setTRNGDecimation(decimation);
    stat = Crypto_runTRNGCmd(CRYPTO_ENTROPY_TRNG_CMD_NORM, \
                             CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
    if (stat != CRYPTO_ENTROPY_OK) {
        goto exit;
    } else {
        /* Continue */
    }

    /* Throw away samples at beginning of capture that are impacted by POST */
    preTrimLen = CRYPTO_ENTROPY_TRNG_LEADING_SAMPLE_TRIM;
    while((preTrimLen-- > 0U) && (stat == CRYPTO_ENTROPY_OK))
    {
        stat = Crypto_getTRNGData(&trimmedData, 
                                  CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
    }

exit:
    return stat;
}

crypto_entropy_stat_t Crypto_getTRNGData(uint32_t *data, uint32_t timeout)
{
    crypto_entropy_stat_t stat;

    stat = CRYPTO_ENTROPY_OK;

    while ((TRNG->CPU_INT.RIS & TRNG_RIS_IRQ_CAPTURED_RDY_SET) == 0U) {
        if (timeout-- > 0U) {
            continue;
        } else {
            stat = CRYPTO_ENTROPY_TRNG_TIMEOUT_ERR;
            break;
        }
    }
    if (stat == CRYPTO_ENTROPY_OK) {
        TRNG->CPU_INT.ICLR = TRNG_ICLR_IRQ_CAPTURED_RDY_CLR;
        *data = TRNG->DATA_CAPTURE;
    } else {
        *data = 0x00000000U;
    }

    if ((TRNG->CPU_INT.RIS & TRNG_RIS_IRQ_HEALTH_FAIL_SET) != 0U) {
        stat = CRYPTO_ENTROPY_TRNG_HEATH_FAIL;
    } else {
        /* Continue */
    }

    return stat;
}

crypto_entropy_stat_t Crypto_powerOffTRNG(void)
{
    crypto_entropy_fsm_state_t fsmState;
    crypto_entropy_stat_t stat;
    
    /* If TRNG is not powered on, return error status. */
    if ((TRNG->GPRCM.PWREN & TRNG_PWREN_ENABLE_MASK) != TRNG_PWREN_ENABLE_ENABLE) 
    {
        stat = CRYPTO_ENTROPY_TRNG_PWROFF_ERR;
        goto exit;
    } else {
        /* Continue */
    }

    /* Read the state of the TRNG module */
    fsmState = Crypto_getTRNGFSMState();
    if (fsmState == CRYPTO_ENTROPY_FSM_STATE_READ_ERR)
    {
        stat = CRYPTO_ENTROPY_TRNG_PWROFF_ERR;
        goto exit;
    } else {
        /* Continue */
    }

    /* If state is NORM_FUNC or ERROR, run command to move to 
     * OFF state.  If state is OFF, proceed to power down
     * the peripheral.  If state is none of these, return analog
     * error condition as the TRNG is not able to be powered down
     * via the correct procedure.
     */
    if ((fsmState == CRYPTO_ENTROPY_FSM_NORM_FUNC) ||\
        (fsmState == CRYPTO_ENTROPY_FSM_ERROR) ) {
        stat = Crypto_runTRNGCmd(CRYPTO_ENTROPY_TRNG_CMD_OFF, \
                                 CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
    } else if (fsmState == CRYPTO_ENTROPY_FSM_OFF) {
        stat = CRYPTO_ENTROPY_OK;
    } else {
        stat = CRYPTO_ENTROPY_TRNG_PWROFF_ERR;
        goto exit;
    }

    /* As the last step, disable power to the module once OFF state
     * was reached successfully. 
     */
    TRNG->GPRCM.PWREN = TRNG_PWREN_KEY_UNLOCK_W |\
                        TRNG_PWREN_ENABLE_DISABLE;

exit:
    return stat;
}

crypto_entropy_stat_t Crypto_getEntropy(uint32_t *output, uint32_t len, 
                                        uint32_t inClkFreq)
{
    crypto_entropy_stat_t stat;
    crypto_entropy_stat_t powerOffStat;

    stat = Crypto_powerOnTRNG(inClkFreq, CRYPTO_ENTROPY_TRNG_DEFAULT_DECIMATION);

    while((len-- > 0U) && (stat == CRYPTO_ENTROPY_OK)) {
        stat = Crypto_getTRNGData(output++, CRYPTO_ENTROPY_TRNG_CMD_TIMEOUT);
    }

    powerOffStat = Crypto_powerOffTRNG();
    if (powerOffStat != CRYPTO_ENTROPY_OK) {
        stat = powerOffStat;
    } else {
        /* No power down error, leave last status. */
    }

    return stat;
}

#endif /* CRYPTO_HW_USE_MSPM0_TRNG == 1 || CRYPTO_HW_USE_MSPM33_TRNG == 1 */
