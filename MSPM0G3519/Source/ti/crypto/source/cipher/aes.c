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
 *  @file       aes.c
 *  @brief      AES cipher implementation
 * 
 *  This cipher module implements the advanced encryption standard through a
 *  common top level API (implemented in this file) which can be mapped
 *  to different underlying hardware/software implementations.
 *
 *  Currently the following internal implementations are supported:
 *    - MSP AESADV HW peripheral accelerator
 *
 ******************************************************************************
 */

#include <stdbool.h>
#include "ti/crypto/include/utilities/utilities.h"
#include "ti/crypto/include/cipher/aes_type.h"
#include "ti/crypto/include/cipher/aes_hw_aesadv.h"

void Crypto_AES_init(void)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    Crypto_AES_HW_AESADV_init();
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

void Crypto_AES_deinit(void)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    Crypto_AES_HW_AESADV_deinit();
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_setSessionKey(const uint32_t *key, \
                                           crypto_aes_keytype_t keyType)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_setSessionKey(key, keyType);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_encrypt(const uint32_t *in, uint32_t *out, \
                                     crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_runBlock(
        in, out,
        key, CRYPTO_AES_OP_ENC);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_decrypt(const uint32_t *in, uint32_t *out, \
                                     crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_runBlock(
        in, out,
        key, CRYPTO_AES_OP_DEC);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_CBC_encrypt(const uint32_t *in, uint32_t *out, \
                                         uint32_t len, const uint32_t *iv, \
                                         crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_CBC_runSeq(
        in, out,
        len, iv,
        key, CRYPTO_AES_OP_ENC);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_CBC_decrypt(const uint32_t *in, uint32_t *out, \
    uint32_t len, const uint32_t *iv, \
    crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_CBC_runSeq(
        in, out,
        len, iv,
        key, CRYPTO_AES_OP_DEC);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_CBC_MAC(const uint32_t *in, uint32_t *out, \
                                     uint32_t len, crypto_aes_key_t key)
{
    uint32_t iv[CRYPTO_AES_STATE_WORDS];

    Crypto_set32(&iv[0], 0U, CRYPTO_AES_STATE_WORDS);

#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_CBC_runSeq(
        in, out,
        len, iv,
        key, CRYPTO_AES_OP_TAG);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_CTR_encrypt(const uint32_t *in, uint32_t *out, \
                                         uint32_t len, \
                                         const uint32_t *nonce, \
                                         uint32_t counter, \
                                         crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_CTR_runSeq(
        in, out,
        len, nonce, counter,
        key, CRYPTO_AES_OP_ENC);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_CTR_decrypt(const uint32_t *in, uint32_t *out, \
                                         uint32_t len, \
                                         const uint32_t *nonce, \
                                         uint32_t counter, \
                                         crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_CTR_runSeq(
        in, out,
        len, nonce, counter,
        key, CRYPTO_AES_OP_DEC);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_CCM_encryptAndGen(const uint8_t *nonce, \
                                               uint8_t nonceLen, \
                                               const uint8_t *aad, \
                                               uint32_t aadLen, \
                                               const uint8_t *in, \
                                               uint8_t *out, \
                                               uint32_t len, \
                                               uint8_t *tag, \
                                               uint8_t tagLen, \
                                               crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_CCM_runSeq(
        nonce, nonceLen,
        aad, aadLen,
        in, out, len,
        tag, tagLen,
        key, CRYPTO_AES_OP_ENC);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_CCM_decryptAndVer(const uint8_t *nonce, \
                                               uint8_t nonceLen, \
                                               const uint8_t *aad, \
                                               uint32_t aadLen, \
                                               const uint8_t *in, \
                                               uint8_t *out, \
                                               uint32_t len, \
                                               uint8_t *tag, \
                                               uint8_t tagLen, \
                                               crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_CCM_runSeq(
        nonce, nonceLen,
        aad, aadLen,
        in, out, len,
        tag, tagLen,
        key, CRYPTO_AES_OP_DEC);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif
}

crypto_aes_stat_t Crypto_AES_CMAC_genTag(const uint32_t *in, \
                                         uint8_t *out, \
                                         uint32_t mlen, \
                                         uint8_t tagLen, \
                                         crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    return Crypto_AES_HW_AESADV_CMAC_runSeq(in, out, mlen, tagLen, key);
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif    
}

crypto_aes_stat_t Crypto_AES_CMAC_verTag(const uint32_t *in, \
                                         const uint8_t *tag, \
                                         uint32_t mlen, \
                                         uint8_t tagLen, \
                                         crypto_aes_key_t key)
{
#if CRYPTO_HW_USE_MSPM0_AESADV == 1 || CRYPTO_HW_USE_MSPM33_AESADV == 1
    crypto_block128_t calculatedTag;
    uint8_t result;
    crypto_aes_stat_t stat;

    stat = Crypto_AES_HW_AESADV_CMAC_runSeq(
        in, calculatedTag.b, mlen, tagLen, key);
    if (stat == CRYPTO_AES_OK) {
        if(Crypto_memcmp(tag, calculatedTag.b, tagLen) == 0U) {
            stat = CRYPTO_AES_OK_TAG_VALID;
        } else {
            stat = CRYPTO_AES_TAG_INVALID_ERR;
        }
    } else {
        /* Leave status as is for application to receive error */
    }
    return stat;
#else
    #error CRYPTO_CIPHER_NO_AES_DEFINED
#endif    
}
