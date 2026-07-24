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
 *  @file       aes_hw_aesadv.c
 *  @brief      AES cipher implementation - hardware accelerated with AESADV
 * 
 *  This cipher module implements the advanced encryption standard using the
 *  MSP AESADV peripheral hardware.
 *
 ******************************************************************************
 */

#include <stdbool.h>
#ifdef CRYPTO_USE_SDK_HSF
#include "ti/devices/msp/msp.h"
#else
#include "ti/devices/msp/hwcrypto.h"
#endif
#include "ti/crypto/include/utilities/utilities.h"
#include "ti/crypto/include/cipher/aes_type.h"

#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1

void Crypto_AES_HW_AESADV_init(void)
{
    /* Reset the AESADV engine */
    AESADV->GPRCM.RSTCTL = AESADV_RSTCTL_KEY_UNLOCK_W |\
                           AESADV_RSTCTL_RESETSTKYCLR_CLR;
    while (AESADV->GPRCM.STAT != AESADV_STAT_RESETSTKY_NORES);
    AESADV->GPRCM.RSTCTL = AESADV_RSTCTL_KEY_UNLOCK_W |\
                           AESADV_RSTCTL_RESETASSERT_ASSERT;
    while (AESADV->GPRCM.STAT != AESADV_STAT_RESETSTKY_RESET);

    /* Enable the AESADV engine */
    AESADV->GPRCM.PWREN = AESADV_PWREN_KEY_UNLOCK_W |\
                            AESADV_PWREN_ENABLE_ENABLE;
    Crypto_delay(CRYPTO_PMCU_DELAY);

    return;
}

void Crypto_AES_HW_AESADV_deinit(void)
{
    /* Disable the AESADV engine */
    AESADV->GPRCM.PWREN = AESADV_PWREN_KEY_UNLOCK_W |\
                          AESADV_PWREN_ENABLE_DISABLE;
    Crypto_delay(CRYPTO_PMCU_DELAY);

    /* Reset the AESADV engine */
    AESADV->GPRCM.RSTCTL = AESADV_RSTCTL_KEY_UNLOCK_W |\
                           AESADV_RSTCTL_RESETSTKYCLR_CLR;
    while (AESADV->GPRCM.STAT != AESADV_STAT_RESETSTKY_NORES);
    AESADV->GPRCM.RSTCTL = AESADV_RSTCTL_KEY_UNLOCK_W |\
                           AESADV_RSTCTL_RESETASSERT_ASSERT;
    while (AESADV->GPRCM.STAT != AESADV_STAT_RESETSTKY_RESET);

    return;
}

crypto_aes_stat_t Crypto_AES_HW_AESADV_setSessionKey( \
    const uint32_t *key, crypto_aes_keytype_t keyType)
{
    uint32_t hwKeySel;
    uint8_t n;
    crypto_aes_stat_t stat;

    if (key == 0U) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((uint32_t)key % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((AESADV->GPRCM.PWREN & AESADV_PWREN_ENABLE_MASK) != \
                               AESADV_PWREN_ENABLE_ENABLE) {
        stat = CRYPTO_AES_HW_ERR;
    } else if ((AESADV->STATUS & AESADV_STATUS_KEYWR_MASK) != \
                                 AESADV_STATUS_KEYWR_ENABLED) {
        stat = CRYPTO_AES_KEYWRITE_ERR;
    } else {
        stat = CRYPTO_AES_OK;
    }
    
    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    if (keyType == CRYPTO_AES_KEYTYPE_256)
    {
        hwKeySel = AESADV_CTRL_KEYSIZE_K256;
        n = CRYPTO_AES_KEY_256_WORDS;
    }
    else if (keyType == CRYPTO_AES_KEYTYPE_128)
    {
        hwKeySel = AESADV_CTRL_KEYSIZE_K128;
        n = CRYPTO_AES_KEY_128_WORDS;
    } else {
        stat = CRYPTO_AES_PARAM_ERR;
        goto exit;
    }

    AESADV->CTRL &= ~AESADV_CTRL_KEYSIZE_MASK;
    AESADV->CTRL |= hwKeySel;
    Crypto_copy32(key, (uint32_t*)&(AESADV->KEY0), n);

exit:
    return stat;
}

crypto_aes_stat_t Crypto_AES_HW_AESADV_runBlock( \
    const uint32_t *in, uint32_t *out, \
    crypto_aes_key_t key, crypto_aes_op_t op)
{
    uint32_t dir;
    crypto_aes_stat_t stat;

    /* Check preconditions and set status accordingly */
    if ((in == 0U) || (out == 0U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((uint32_t)in % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((uint32_t)out % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((AESADV->GPRCM.PWREN & AESADV_PWREN_ENABLE_MASK) != \
                                      AESADV_PWREN_ENABLE_ENABLE) {
        stat = CRYPTO_AES_HW_ERR;
    } else {
        stat = CRYPTO_AES_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    /* Key selection
     * Currently only session keys are supported, not key store keys.
     */

    if (op == CRYPTO_AES_OP_ENC) {
        dir = AESADV_CTRL_DIR_ENCRYPT;
    } else if (op == CRYPTO_AES_OP_DEC) {
        dir = AESADV_CTRL_DIR_DECRYPT;
    } else {
        stat = CRYPTO_AES_PARAM_ERR;
        goto exit;
    }

    AESADV->CTRL &= ~(AESADV_CTRL_DIR_MASK |\
                      AESADV_CTRL_CBC_MASK | AESADV_CTRL_CTR_MASK |\
                      AESADV_CTRL_ICM_MASK | AESADV_CTRL_CFB_MASK |\
                      AESADV_CTRL_CBCMAC_MASK | AESADV_CTRL_GCM_MASK |\
                      AESADV_CTRL_CCM_MASK |\
                      AESADV_CTRL_OFB_GCM_CCM_CONT_MASK |\
                      AESADV_CTRL_SAVE_CNTXT_MASK); 
    AESADV->CTRL |= (dir |\
                     AESADV_CTRL_CBC_DISABLE | AESADV_CTRL_CTR_DISABLE |\
                     AESADV_CTRL_ICM_DISABLE | AESADV_CTRL_CFB_DISABLE |\
                     AESADV_CTRL_CBCMAC_DISABLE | 0U |\
                     AESADV_CTRL_CCM_DISABLE |\
                     0U |\
                     AESADV_CTRL_SAVE_CNTXT_NO_EFFECT);
    AESADV->C_LENGTH_0 = 0U;
    AESADV->C_LENGTH_1 = 0U;

    while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                          AESADV_CTRL_INPUT_RDY_EMPTY);

    Crypto_copy32(in, (uint32_t*)&(AESADV->DATA0), CRYPTO_AES_STATE_WORDS);

    while((AESADV->CTRL & AESADV_CTRL_OUTPUT_RDY_MASK) != \
                          AESADV_CTRL_OUTPUT_RDY_READY);

    Crypto_copy32((uint32_t*)&(AESADV->DATA0), out, CRYPTO_AES_STATE_WORDS);
    
exit:
    return stat;
}

crypto_aes_stat_t Crypto_AES_HW_AESADV_CBC_runSeq( \
    const uint32_t *in, uint32_t *out, \
    uint32_t len, const uint32_t *iv, \
    crypto_aes_key_t key, crypto_aes_op_t op)
{
    uint32_t dir;
    uint32_t blocks;
    crypto_aes_stat_t stat;

    /* Check preconditions and set status accordingly */
    if ((in == 0U) || (out == 0U) || (iv == 0U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((uint32_t)in % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((uint32_t)out % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((uint32_t)iv % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if (len % CRYPTO_AES_STATE_BYTES) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((AESADV->GPRCM.PWREN & AESADV_PWREN_ENABLE_MASK) != \
                                      AESADV_PWREN_ENABLE_ENABLE) {
        stat = CRYPTO_AES_HW_ERR;
    } else {
        stat = CRYPTO_AES_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    if (op == CRYPTO_AES_OP_ENC) {
        dir = AESADV_CTRL_DIR_ENCRYPT;
    } else if (op == CRYPTO_AES_OP_DEC) {
        dir = AESADV_CTRL_DIR_DECRYPT;
    } else if (op == CRYPTO_AES_OP_TAG) {
        dir = AESADV_CTRL_DIR_ENCRYPT;
    } else {
        stat = CRYPTO_AES_PARAM_ERR;
        goto exit;
    }

    AESADV->CTRL &= ~(AESADV_CTRL_DIR_MASK |\
                      AESADV_CTRL_CBC_MASK | AESADV_CTRL_CTR_MASK |\
                      AESADV_CTRL_ICM_MASK | AESADV_CTRL_CFB_MASK |\
                      AESADV_CTRL_CBCMAC_MASK | AESADV_CTRL_GCM_MASK |\
                      AESADV_CTRL_CCM_MASK |\
                      AESADV_CTRL_OFB_GCM_CCM_CONT_MASK |\
                      AESADV_CTRL_SAVE_CNTXT_MASK); 
    AESADV->CTRL |= (dir |\
                     AESADV_CTRL_CBC_ENABLE | AESADV_CTRL_CTR_DISABLE |\
                     AESADV_CTRL_ICM_DISABLE | AESADV_CTRL_CFB_DISABLE |\
                     AESADV_CTRL_CBCMAC_DISABLE | 0U |\
                     AESADV_CTRL_CCM_DISABLE |\
                     0U |\
                     AESADV_CTRL_SAVE_CNTXT_NO_EFFECT);
    AESADV->C_LENGTH_0 = len;
    AESADV->C_LENGTH_1 = 0U;

    while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                          AESADV_CTRL_INPUT_RDY_EMPTY);

    Crypto_copy32(iv, (uint32_t*)&(AESADV->IV0), CRYPTO_AES_STATE_WORDS);
    
    blocks = len / CRYPTO_AES_STATE_BYTES;
    while (blocks-- > 0U) {
        while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                              AESADV_CTRL_INPUT_RDY_EMPTY);
        Crypto_copy32(in, (uint32_t*)&(AESADV->DATA0), CRYPTO_AES_STATE_WORDS);
        in += CRYPTO_AES_STATE_WORDS;
        while((AESADV->CTRL & AESADV_CTRL_OUTPUT_RDY_MASK) != \
                              AESADV_CTRL_OUTPUT_RDY_READY);
        Crypto_copy32((uint32_t*)&(AESADV->DATA0), out, CRYPTO_AES_STATE_WORDS);                      
        if (op != CRYPTO_AES_OP_TAG) {
            out += CRYPTO_AES_STATE_WORDS;
        } else {
            /* For tag generation we will not increment the block pointer. */
        }
    }
exit:
    return stat;
}

crypto_aes_stat_t Crypto_AES_HW_AESADV_CTR_runSeq( \
    const uint32_t *in, uint32_t *out, \
    uint32_t len, const uint32_t *nonce, uint32_t counter, \
    crypto_aes_key_t key, crypto_aes_op_t op)
{
    uint32_t dir;
    uint32_t blocks;
    crypto_aes_stat_t stat;

    /* Check preconditions and set status accordingly */
    if ((in == 0U) || (out == 0U) || (nonce == 0U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((uint32_t)in % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((uint32_t)out % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((uint32_t)nonce % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if (len % CRYPTO_AES_STATE_BYTES) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((AESADV->GPRCM.PWREN & AESADV_PWREN_ENABLE_MASK) != \
                                      AESADV_PWREN_ENABLE_ENABLE) {
        stat = CRYPTO_AES_HW_ERR;
    } else {
        stat = CRYPTO_AES_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    if (op == CRYPTO_AES_OP_ENC) {
        dir = AESADV_CTRL_DIR_ENCRYPT;
    } else if (op == CRYPTO_AES_OP_DEC) {
        dir = AESADV_CTRL_DIR_DECRYPT;
    } else {
        stat = CRYPTO_AES_PARAM_ERR;
        goto exit;
    }

    AESADV->CTRL &= ~(AESADV_CTRL_DIR_MASK |\
                      AESADV_CTRL_CBC_MASK | AESADV_CTRL_CTR_MASK |\
                      AESADV_CTRL_CTR_WIDTH_MASK |\
                      AESADV_CTRL_ICM_MASK | AESADV_CTRL_CFB_MASK |\
                      AESADV_CTRL_CBCMAC_MASK | AESADV_CTRL_GCM_MASK |\
                      AESADV_CTRL_CCM_MASK |\
                      AESADV_CTRL_OFB_GCM_CCM_CONT_MASK |\
                      AESADV_CTRL_SAVE_CNTXT_MASK); 
    AESADV->CTRL |= (dir |\
                     AESADV_CTRL_CBC_DISABLE | AESADV_CTRL_CTR_ENABLE |\
                     AESADV_CTRL_CTR_WIDTH_CTR32 |\
                     AESADV_CTRL_ICM_DISABLE | AESADV_CTRL_CFB_DISABLE |\
                     AESADV_CTRL_CBCMAC_DISABLE | 0U |\
                     AESADV_CTRL_CCM_DISABLE |\
                     0U |\
                     AESADV_CTRL_SAVE_CNTXT_NO_EFFECT);
    AESADV->C_LENGTH_0 = len;
    AESADV->C_LENGTH_1 = 0U;

    while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                          AESADV_CTRL_INPUT_RDY_EMPTY);

    Crypto_copy32(nonce, (uint32_t*)&(AESADV->IV0), CRYPTO_AES_CTR_NONCE_WORDS);
    AESADV->IV3 = Crypto_reverseBytes32(counter);

    blocks = len / CRYPTO_AES_STATE_BYTES;
    while (blocks-- > 0U) {
        while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                              AESADV_CTRL_INPUT_RDY_EMPTY);
        Crypto_copy32(in, (uint32_t*)&(AESADV->DATA0), CRYPTO_AES_STATE_WORDS);
        in += CRYPTO_AES_STATE_WORDS;
        while((AESADV->CTRL & AESADV_CTRL_OUTPUT_RDY_MASK) != \
                              AESADV_CTRL_OUTPUT_RDY_READY);
        Crypto_copy32((uint32_t*)&(AESADV->DATA0), out, CRYPTO_AES_STATE_WORDS);                      
        out += CRYPTO_AES_STATE_WORDS;
    }
exit:
    return stat;
}

crypto_aes_stat_t Crypto_AES_HW_AESADV_CCM_runSeq( \
    const uint8_t *nonce, uint8_t nonceLen, \
    const uint8_t *aad, uint32_t aadLen, \
    const uint8_t *in,  uint8_t *out, uint32_t len, \
    uint8_t *tag, uint8_t tagLen, \
    crypto_aes_key_t key, crypto_aes_op_t op)
{
    crypto_block128_t tempBlock;
    uint32_t dir;
    uint32_t aadIdx;
    uint32_t inIdx;
    uint32_t outIdx;
    uint8_t qEncoded;
    uint8_t tEncoded;
    uint8_t byteIdx;
    crypto_aes_stat_t stat;

    /* Check preconditions and set status accordingly */
    if (nonce == 0U) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((nonceLen < CRYPTO_AES_CCM_NONCE_BYTES_MIN) ||\
               (nonceLen > CRYPTO_AES_CCM_NONCE_BYTES_MAX)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((aadLen > 0U) && (aad == 0U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((aadLen == 0U) && (aad != 0U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((len > 0U) && ((in == 0U) || (out == 0U))) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((len == 0U) && (aadLen == 0U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    }  else if ((tagLen < CRYPTO_AES_CCM_TAG_BYTES_MIN) ||\
               (tagLen > CRYPTO_AES_CCM_TAG_BYTES_MAX) ||\
               (tagLen % 2U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((AESADV->GPRCM.PWREN & AESADV_PWREN_ENABLE_MASK) != \
                                      AESADV_PWREN_ENABLE_ENABLE) {
        stat = CRYPTO_AES_HW_ERR;
    } else {
        stat = CRYPTO_AES_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    if (op == CRYPTO_AES_OP_ENC) {
        dir = AESADV_CTRL_DIR_ENCRYPT;
    } else if (op == CRYPTO_AES_OP_DEC) {
        dir = AESADV_CTRL_DIR_DECRYPT;
    } else {
        stat = CRYPTO_AES_PARAM_ERR;
        goto exit;
    }

    /* Get encoded values */
    tEncoded = (tagLen - 2U) >> 1U;
    qEncoded = 15U - nonceLen - 1U;

    /* Load IV  with flag byte followed by nonce bytes and then zeros */
    tempBlock.b[0] = qEncoded;
    for (byteIdx=0U; byteIdx<nonceLen; byteIdx++) {
        tempBlock.b[byteIdx+1] = nonce[byteIdx];
    }
    for (byteIdx=nonceLen+1; byteIdx<CRYPTO_BLOCK128_LEN; byteIdx++) {
        tempBlock.b[byteIdx] = 0x00;
    }
    Crypto_copy32(&tempBlock.w[0], (uint32_t*)&(AESADV->IV0), \
                  CRYPTO_BLOCK128_LEN_WORDS);

    /* Configure operation */
    AESADV->CTRL &= ~(AESADV_CTRL_DIR_MASK |\
        AESADV_CTRL_CBC_MASK | AESADV_CTRL_CTR_MASK |\
        AESADV_CTRL_ICM_MASK | AESADV_CTRL_CFB_MASK |\
        AESADV_CTRL_CBCMAC_MASK | AESADV_CTRL_GCM_MASK |\
        AESADV_CTRL_CCM_MASK |\
        AESADV_CTRL_CCML_MASK | AESADV_CTRL_CCMM_MASK |\
        AESADV_CTRL_OFB_GCM_CCM_CONT_MASK |\
        AESADV_CTRL_SAVE_CNTXT_MASK); 
    AESADV->CTRL |= (dir |\
       AESADV_CTRL_CBC_DISABLE | AESADV_CTRL_CTR_ENABLE |\
       AESADV_CTRL_ICM_DISABLE | AESADV_CTRL_CFB_DISABLE |\
       AESADV_CTRL_CBCMAC_DISABLE | 0U |\
       AESADV_CTRL_CCM_ENABLE |\
       tEncoded << AESADV_CTRL_CCMM_OFS |\
       qEncoded << AESADV_CTRL_CCML_OFS |\
       0U |\
       AESADV_CTRL_SAVE_CNTXT_ENABLE);

    /* Set length registers */
    AESADV->C_LENGTH_0 = len;
    AESADV->C_LENGTH_1 = 0;
    AESADV->AAD_LENGTH = aadLen;

    /* Load AAD to hardware */
    aadIdx = 0U;
    while (aadIdx < aadLen) {
        for (byteIdx=0U; byteIdx<CRYPTO_BLOCK128_LEN; byteIdx++) {
            if (aadIdx < aadLen) {
                tempBlock.b[byteIdx] = aad[aadIdx++];
            } else {
                tempBlock.b[byteIdx] = 0x00;
            }
        }
        while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != AESADV_CTRL_INPUT_RDY_EMPTY);
        Crypto_copy32(&tempBlock.w[0], (uint32_t*)&(AESADV->DATA0), \
                  CRYPTO_BLOCK128_LEN_WORDS);
    }
    
    /* Load payload to hardware */
    inIdx = 0U;
    outIdx = 0U;
    while (inIdx < len) {
        for (byteIdx=0U; byteIdx<CRYPTO_BLOCK128_LEN; byteIdx++) {
            if (inIdx < len) {
                tempBlock.b[byteIdx] = in[inIdx++];
            } else {
                tempBlock.b[byteIdx] = 0x00;
            }
        }
        while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != AESADV_CTRL_INPUT_RDY_EMPTY);
        Crypto_copy32(&tempBlock.w[0], (uint32_t*)&(AESADV->DATA0), \
                  CRYPTO_BLOCK128_LEN_WORDS);
        while((AESADV->CTRL & AESADV_CTRL_OUTPUT_RDY_MASK) != AESADV_CTRL_OUTPUT_RDY_READY);
        Crypto_copy32((uint32_t*)&(AESADV->DATA0), &tempBlock.w[0], \
                  CRYPTO_BLOCK128_LEN_WORDS);
        for (byteIdx=0U; byteIdx<CRYPTO_BLOCK128_LEN; byteIdx++) {
            if (outIdx < len) {
                out[outIdx++] = tempBlock.b[byteIdx];
            } else {
                break;
            }
        }
    }

    /* Write out or verify CCM tag */
    while((AESADV->CTRL & AESADV_CTRL_SAVED_CNTXT_RDY_MASK) != AESADV_CTRL_SAVED_CNTXT_RDY_READY);
    Crypto_copy32((uint32_t*)&(AESADV->TAG0), &tempBlock.w[0], CRYPTO_BLOCK128_LEN_WORDS);
    if (op == CRYPTO_AES_OP_ENC) {
        for (byteIdx=0; byteIdx<tagLen; byteIdx++) {
            tag[byteIdx] = tempBlock.b[byteIdx];
        }
    } else {
        stat = CRYPTO_AES_OK_TAG_VALID;
        for (byteIdx=0U; byteIdx<tagLen; byteIdx++) {
            if(tag[byteIdx] != tempBlock.b[byteIdx]) {
                stat = CRYPTO_AES_TAG_INVALID_ERR;
                break;
            } else {
                /* Tag still valid */
            }
        }
    }

exit:
    return stat;
}

crypto_aes_stat_t Crypto_AES_HW_AESADV_CMAC_runSeq( \
    const uint32_t *in, uint8_t *out,
    uint32_t mLen, uint8_t tagLen, crypto_aes_key_t key)
{
    crypto_block128_t l;
    crypto_block128_t k1;
    crypto_block128_t k2;
    crypto_block128_t lastBlock;
    uint32_t blocks;
    uint32_t remainder;
    uint32_t carry;
    uint32_t swapped;
    int32_t i;
    uint8_t *lastBytes;
    crypto_aes_stat_t stat;

    /* Check preconditions and set status accordingly */
    if ((in == 0U) || (out == 0U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((uint32_t)in % sizeof(uint32_t)) {
        stat = CRYPTO_AES_ALIGNMENT_ERR;
    } else if ((tagLen < 4U) || (tagLen > 16U)) {
        stat = CRYPTO_AES_PARAM_ERR;
    } else if ((AESADV->GPRCM.PWREN & AESADV_PWREN_ENABLE_MASK) != \
                                      AESADV_PWREN_ENABLE_ENABLE) {
        stat = CRYPTO_AES_HW_ERR;
    } else {
        stat = CRYPTO_AES_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    /* Calculate k1, k2 */

    AESADV->CTRL &= ~(AESADV_CTRL_DIR_MASK |\
                      AESADV_CTRL_CBC_MASK | AESADV_CTRL_CTR_MASK |\
                      AESADV_CTRL_ICM_MASK | AESADV_CTRL_CFB_MASK |\
                      AESADV_CTRL_CBCMAC_MASK | AESADV_CTRL_GCM_MASK |\
                      AESADV_CTRL_CCM_MASK |\
                      AESADV_CTRL_OFB_GCM_CCM_CONT_MASK |\
                      AESADV_CTRL_SAVE_CNTXT_MASK); 
    AESADV->CTRL |= (AESADV_CTRL_DIR_ENCRYPT |\
                     AESADV_CTRL_CBC_DISABLE | AESADV_CTRL_CTR_DISABLE |\
                     AESADV_CTRL_ICM_DISABLE | AESADV_CTRL_CFB_DISABLE |\
                     AESADV_CTRL_CBCMAC_DISABLE | 0U |\
                     AESADV_CTRL_CCM_DISABLE |\
                     0U |\
                     AESADV_CTRL_SAVE_CNTXT_NO_EFFECT);
    AESADV->C_LENGTH_0 = 0U;
    AESADV->C_LENGTH_1 = 0U;

    while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                          AESADV_CTRL_INPUT_RDY_EMPTY);
    Crypto_set32((uint32_t*)&(AESADV->DATA0), 0U, CRYPTO_AES_STATE_WORDS);
    while((AESADV->CTRL & AESADV_CTRL_OUTPUT_RDY_MASK) != \
                          AESADV_CTRL_OUTPUT_RDY_READY);
    Crypto_copy32((uint32_t*)&(AESADV->DATA0), l.w, CRYPTO_AES_STATE_WORDS);
    
    /* left shift L by 1 */
    carry = 0;
    for (i = 3; i >= 0; i--) {
        swapped = Crypto_reverseBytes32(l.w[i]);
        k1.w[i] = Crypto_reverseBytes32(((swapped << 1) + carry));
        carry = ((0x80000000 & swapped) == 0x80000000) ? 1 : 0;
    }

    /* if MSB(L) is not 0 */
    if (carry != 0) {
        k1.w[3] = (k1.w[3] ^ 0x87000000);
    }

    /* left shift K1 by 1 */
    carry = 0;
    for (i = 3; i >= 0; i--) {
        swapped = Crypto_reverseBytes32(k1.w[i]);
        k2.w[i] = Crypto_reverseBytes32(((swapped << 1) + carry));
        carry = ((0x80000000 & swapped) == 0x80000000) ? 1 : 0;
    }

    /* if MSB(K1) is not 0 */
    if (carry != 0) {
        k2.w[3] = (k2.w[3] ^ 0x87000000);
    }

    /* Load CMAC k1, k2*/
    Crypto_copy32(k1.w, (uint32_t*)&(AESADV->GHASH_H0),
                  CRYPTO_AES_STATE_WORDS);
    Crypto_copy32(k2.w, (uint32_t*)&(AESADV->GCMCCM_TAG0),
                  CRYPTO_AES_STATE_WORDS);

    /* Zero out IV value */
    while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                          AESADV_CTRL_INPUT_RDY_EMPTY);
    Crypto_set32((uint32_t*)&(AESADV->IV0), 0U, CRYPTO_AES_STATE_WORDS);

    /* Re-configure for CMAC */
    AESADV->CTRL &= ~(AESADV_CTRL_DIR_MASK |\
                      AESADV_CTRL_CBC_MASK | AESADV_CTRL_CTR_MASK |\
                      AESADV_CTRL_CTR_WIDTH_MASK |\
                      AESADV_CTRL_ICM_MASK | AESADV_CTRL_CFB_MASK |\
                      AESADV_CTRL_CBCMAC_MASK | AESADV_CTRL_GCM_MASK |\
                      AESADV_CTRL_CCM_MASK |\
                      AESADV_CTRL_OFB_GCM_CCM_CONT_MASK |\
                      AESADV_CTRL_SAVE_CNTXT_MASK); 
    AESADV->CTRL |= (AESADV_CTRL_DIR_ENCRYPT |\
                     AESADV_CTRL_CBC_ENABLE | AESADV_CTRL_CTR_DISABLE |\
                     AESADV_CTRL_ICM_DISABLE | AESADV_CTRL_CFB_DISABLE |\
                     AESADV_CTRL_CBCMAC_DISABLE | 0U |\
                     AESADV_CTRL_CCM_DISABLE |\
                     0x02000000 |\
                     AESADV_CTRL_SAVE_CNTXT_ENABLE);
    if (mLen > 0U) {
        AESADV->C_LENGTH_0 = mLen;
    } else {
        AESADV->C_LENGTH_0 = 1U;
    }
    AESADV->C_LENGTH_1 = 0U;
    AESADV->CPU_INT.ICLR |= AESADV_CPU_INT_ICLR_SAVEDCNTXTRDY_SET;

    /* Pass through the input data (complete blocks) */
    blocks = mLen / CRYPTO_AES_STATE_BYTES;
    while (blocks-- > 0U) {
        while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                              AESADV_CTRL_INPUT_RDY_EMPTY);
        Crypto_copy32(in, (uint32_t*)&(AESADV->DATA0), CRYPTO_AES_STATE_WORDS);
        in += CRYPTO_AES_STATE_WORDS;
    }

    /* Pass through special last block if input is not 16 byte aligned. */
    Crypto_set32(lastBlock.w, 0U, CRYPTO_AES_STATE_WORDS);
    remainder = mLen % CRYPTO_AES_STATE_BYTES;
    lastBytes = (uint8_t*)(in);
    i = 0;
    if ((mLen == 0U) || (remainder > 0U)) {
        for (i=0; i<CRYPTO_AES_STATE_BYTES; i++) {
            if (remainder == 0U) {
                lastBlock.b[i] = 0x80;
                break;
            } else {
                lastBlock.b[i] = *(lastBytes++);
                remainder--;
            }
        }
        while((AESADV->CTRL & AESADV_CTRL_INPUT_RDY_MASK) != \
                              AESADV_CTRL_INPUT_RDY_EMPTY);
        Crypto_copy32(lastBlock.w, (uint32_t*)&(AESADV->DATA0), 
                      CRYPTO_AES_STATE_WORDS);
    } else {
        /* Continue */
    }

    /* Wait for tag generation, then save tag */
    while((AESADV->CPU_INT.RIS & AESADV_CPU_INT_RIS_SAVEDCNTXTRDY_MASK) == 0U);
    Crypto_copy8((uint8_t*)&(AESADV->TAG0), out, tagLen);

    /* Clean up */
    AESADV->CTRL &= ~0x02000000;
    Crypto_set32(l.w, 0U, CRYPTO_AES_STATE_WORDS);
    Crypto_set32(k1.w, 0U, CRYPTO_AES_STATE_WORDS);
    Crypto_set32(k2.w, 0U, CRYPTO_AES_STATE_WORDS);
    carry = 0U;
    swapped = 0U;
    
exit:
    return stat;
}

#endif /* CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1 */
