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
 *  @file       drbg.c
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "ti/crypto/include/utilities/utilities.h"
#include "ti/crypto/include/cipher/aes.h"
#include "ti/crypto/include/rand/drbg.h"

/* Internal definitions and type declarations */

/**
 * @brief Init tag set in context struct to indicate when the DRBG
 *        has been initialized.
 */
#define CRYPTO_DRBG_INIT_TAG (0x5ABD7EEC)

/**
 * @brief The CTR DRBG key length in 32-bit words
 */
#define CRYPTO_CTRDRBG_KEY_LEN_WORDS (8U)

/**
 * @brief The CTR DRBG block length in 32-bit words
 */
#define CRYPTO_CTRDRBG_BLOCK_LEN_WORDS (4U)

/**
 * @brief The CTR DRBG entropy input length in 32-bit words
 */
#define CRYPTO_CTRDRBG_ENTROPY_LEN_WORDS (8U)

/**
 * @brief The CTR DRBG nonce input length in 32-bit words
 */
#define CRYPTO_CTRDRBG_NONCE_LEN_WORDS (4U)

/**
 * @brief The CTR DRBG seed length in 32-bit words
 *        This is the block len + key len
 */
#define CRYPTO_CTRDRBG_SEED_LEN_WORDS (CRYPTO_CTRDRBG_KEY_LEN_WORDS + \
                                       CRYPTO_CTRDRBG_BLOCK_LEN_WORDS)

/* Internal function declarations */

/**
 *  @brief      Execute CTR DRBG derivation function (DF).
 *              This function is used by the init function and is not
 *              intended to be called directly by the application.
 *              Unlike the NIST SP 800-90A specification which allows for
 *              a variable length string, this condensed implementation
 *              always expects 384 bits of input (256 bits of entropy and 128
 *              bits of nonce) and provides 384 bits of output for seeding.
 *              Other input/output sizes are not supported.
 *              
 *  @param[in]  entropy        Pointer to buffer containing entropy input.
 *                             Must be 32-bit word aligned and contain
 *                             8 words (256 bits) of entropy.
 *  @param[in]  nonce          Pointer to buffer containing nonce input.
 *                             Must be 32-bit word aligned and contain
 *                             4 words (128 bits) of nonce content.
 *  @param[out] out            Pointer to the 32-bit aligned output buffer.
 *                             This buffer must be 12 words in length.
 *
 *  @return     The status of the operation, one of crypto_drbg_stat_t.
 *  @retval     CRYPTO_DRBG_OK if the DRBG was successfully initialized.
 *  @retval     CRYPTO_DRBG_PARAM_ERR if a pointer is invalid.
 *  @retval     CRYPTO_DRBG_AES_ERR if the AES module encountered an error.
 */
static crypto_drbg_stat_t Crypto_DRBG_dervFxn(const uint32_t *entropy,
                                              const uint32_t *nonce,
                                              uint32_t *out);

/**
 *  @brief      Update a CTR DRBG instance state.
 *              This function is used by the init function and is not
 *              intended to be called directly by the application.         
 *
 *  @param[out] ctx            Pointer to the context structure for the
 *                             instance to update.
 *  @param[in]  providedData   Pointer to array of provided data to use
 *                             in the CTR DRBG update; must always
 *                             be of length CRYPTO_CTRDRBG_SEED_LEN_WORDS
 *                             (12 32-bit words or 384 bits).
 *
 *  @return     The status of the operation, one of crypto_drbg_stat_t.
 *  @retval     CRYPTO_DRBG_OK if the DRBG was successfully initialized.
 *  @retval     CRYPTO_DRBG_PARAM_ERR if a pointer is invalid.
 *  @retval     CRYPTO_DRBG_AES_ERR if the AES module encountered an error.
 */
static crypto_drbg_stat_t Crypto_DRBG_updateFxn(crypto_drbg_ctx_t *ctx,
                                                const uint32_t *providedData);

/* Internal function implementations */

crypto_drbg_stat_t Crypto_DRBG_dervFxn(const uint32_t *entropy,
                                       const uint32_t *nonce,
                                       uint32_t *out)
{
    uint32_t iv_and_s[CRYPTO_CTRDRBG_BLOCK_LEN_WORDS*5];
    uint32_t temp[CRYPTO_CTRDRBG_SEED_LEN_WORDS];
    crypto_block256_t k;
    uint32_t i;
    crypto_aes_stat_t aesStat;
    crypto_drbg_stat_t stat;

    /* Check input pointer parameters for validity */
    if ((entropy == 0U) || (nonce == 0U) || (out == 0U)) {
        stat = CRYPTO_DRBG_PARAM_ERR;
    } else {
        stat = CRYPTO_DRBG_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_DRBG_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    /* Set up 5 128-bit AES blocks of 'iv_and_s' as the following:
     * IV[4] || L[1] || N[1] || ENTROPY+NONCE[12] || 0x00000080[1] || ZPAD[1]
     */
    Crypto_set32(&iv_and_s[0], 0U, CRYPTO_CTRDRBG_BLOCK_LEN_WORDS);
    iv_and_s[4] = Crypto_reverseBytes32(48U); /* 'L' value, 384/8 = 48 bytes */
    iv_and_s[5] = Crypto_reverseBytes32(48U); /* 'N' value, 384/8 = 48 bytes */
    for (i=0U; i<CRYPTO_CTRDRBG_ENTROPY_LEN_WORDS; i++) {
        iv_and_s[i+6] = entropy[i];
    }
    for (i=0U; i<CRYPTO_CTRDRBG_NONCE_LEN_WORDS; i++) {
        iv_and_s[i+14] = nonce[i];
    }
    iv_and_s[18] = 0x00000080; /* Add end indicator and pad zeros */
    iv_and_s[19] = 0x00000000; /* Additonal pad zeros to end of block */

    /* Set up 'K' with values from FIPS */
    for (i=0U; i<CRYPTO_CTRDRBG_KEY_LEN_WORDS*4U; i++) {
        k.b[i] = i;
    }
    aesStat = Crypto_AES_setSessionKey(&(k.w[0]), CRYPTO_AES_KEYTYPE_256);
    if (aesStat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        stat = CRYPTO_DRBG_AES_ERR;
        goto exit;
    }

    /* Run BCC loop */
    i = 0U;
    while (i < 3U) {
        iv_and_s[0] = Crypto_reverseBytes32(i); /* First block is a counter */
        aesStat = Crypto_AES_CBC_MAC(&iv_and_s[0], /* CBC-MAC input */
                                     &temp[i*4], /* CBC-MAC output */
                                     CRYPTO_CTRDRBG_BLOCK_LEN_WORDS*4*5, /* Len */
                                     CRYPTO_AES_KEYSEL_SESSION); /* Key */
        if (aesStat == CRYPTO_AES_OK) {
            /* Continue normally */
        } else {
            stat = CRYPTO_DRBG_AES_ERR;
            goto exit;
        }
        i++;
    }

    /* Reset key to left-most 256 bits of temp */
    aesStat = Crypto_AES_setSessionKey(&temp[0], CRYPTO_AES_KEYTYPE_256);
    if (aesStat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        stat = CRYPTO_DRBG_AES_ERR;
        goto exit;
    }

    /* Generate final output with block encryption */
    aesStat = Crypto_AES_encrypt(&temp[CRYPTO_CTRDRBG_KEY_LEN_WORDS],
                                 &out[0],
                                 CRYPTO_AES_KEYSEL_SESSION);
    if (aesStat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        stat = CRYPTO_DRBG_AES_ERR;
        goto exit;
    }
    aesStat = Crypto_AES_encrypt(&out[0],
                                 &out[4],
                                 CRYPTO_AES_KEYSEL_SESSION);
    if (aesStat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        stat = CRYPTO_DRBG_AES_ERR;
        goto exit;
    }
    aesStat = Crypto_AES_encrypt(&out[4],
                                 &out[8],
                                 CRYPTO_AES_KEYSEL_SESSION);
    if (aesStat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        stat = CRYPTO_DRBG_AES_ERR;
        goto exit;
    }

exit:
    return stat;
}

crypto_drbg_stat_t Crypto_DRBG_updateFxn(crypto_drbg_ctx_t *ctx,
                                         const uint32_t *providedData)
{
    uint32_t temp[CRYPTO_CTRDRBG_SEED_LEN_WORDS];
    uint32_t x;
    uint32_t i;
    crypto_aes_stat_t aesStat;
    crypto_drbg_stat_t stat;

    /* Check preconditions and set status accordingly */
    if ((ctx == 0U) || (providedData == 0U)) {
        stat = CRYPTO_DRBG_PARAM_ERR;
    } else {
        stat = CRYPTO_DRBG_OK;
    }
    
    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_DRBG_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    /* Iterate through 3 128-bit blocks */
    aesStat = Crypto_AES_setSessionKey(&ctx->k.w[0], CRYPTO_AES_KEYTYPE_256);
    if (aesStat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        stat = CRYPTO_DRBG_AES_ERR;
        goto exit;
    }

    i = 0U;
    while (i < 3U) {
        x = Crypto_reverseBytes32(ctx->v.w[3]);
        x++;
        ctx->v.w[3] = Crypto_reverseBytes32(x);
        aesStat = Crypto_AES_encrypt(&ctx->v.w[0],
                                     &temp[CRYPTO_CTRDRBG_BLOCK_LEN_WORDS*i],
                                     CRYPTO_AES_KEYSEL_SESSION);
        if (aesStat == CRYPTO_AES_OK) {
            /* Continue normally */
        } else {
            stat = CRYPTO_DRBG_AES_ERR;
            goto exit;
        }
        i++;
    }

    /* XOR with the context string */
    Crypto_xor32(&temp[0], &providedData[0], CRYPTO_CTRDRBG_SEED_LEN_WORDS);

    /* Save temp back to state */
    Crypto_copy32(&temp[0], &ctx->k.w[0], CRYPTO_CTRDRBG_KEY_LEN_WORDS);
    Crypto_copy32(&temp[CRYPTO_CTRDRBG_KEY_LEN_WORDS], \
                  &ctx->v.w[0], \
                  CRYPTO_CTRDRBG_BLOCK_LEN_WORDS);

exit:
    return stat;
}

/* External function implementations */

crypto_drbg_stat_t Crypto_DRBG_initialize(crypto_drbg_ctx_t *ctx,
                                          const uint32_t *entropy,
                                          const uint32_t *nonce)
{
    uint32_t seed[CRYPTO_CTRDRBG_SEED_LEN_WORDS];
    crypto_drbg_stat_t stat;

    /* Check preconditions and set status accordingly */
    if ((ctx == 0U) || (entropy == 0U) || (nonce == 0U)) {
        stat = CRYPTO_DRBG_PARAM_ERR;
    } else {
        stat = CRYPTO_DRBG_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_DRBG_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    /* Enable AES */
    Crypto_AES_init();

    /* Run the derivation function over the entropy. */
    stat = Crypto_DRBG_dervFxn(entropy, nonce, &seed[0]);
    if (stat == CRYPTO_DRBG_OK) {
        /* Continue normally */
    } else {
        goto exit_disable_aes;
    }

    /* Set up initial state/key with all zeros and run first update */
    Crypto_set32(&(ctx->k.w[0]), 0U, CRYPTO_CTRDRBG_KEY_LEN_WORDS);
    Crypto_set32(&(ctx->v.w[0]), 0U, CRYPTO_CTRDRBG_BLOCK_LEN_WORDS);
    stat = Crypto_DRBG_updateFxn(ctx, &seed[0]);
    if (stat == CRYPTO_DRBG_OK) {
        /* Continue normally */
    } else {
        goto exit_disable_aes;
    }

    /* Init the counter and mark the instance as initialized. */
    ctx->reseedCounter = 1U;
    ctx->initialized = CRYPTO_DRBG_INIT_TAG;

exit_disable_aes:
    Crypto_AES_deinit();

exit:
    return stat;
}

crypto_drbg_stat_t Crypto_DRBG_generate(crypto_drbg_ctx_t *ctx,
                                        uint32_t *output,
                                        uint32_t outputLen)
{
    uint32_t addtlData[CRYPTO_CTRDRBG_SEED_LEN_WORDS];
    uint32_t outputBlock[CRYPTO_CTRDRBG_BLOCK_LEN_WORDS];
    uint32_t x;
    uint32_t i;
    crypto_aes_stat_t aesStat;
    crypto_drbg_stat_t stat;

    /* Check preconditions and set status accordingly */
    if ((ctx == 0U) || (output == 0U) || (outputLen == 0U)) {
        stat = CRYPTO_DRBG_PARAM_ERR;
    } else if (ctx->initialized != CRYPTO_DRBG_INIT_TAG) {
        stat = CRYPTO_DRBG_NOINIT_ERR;
    } else if (ctx->reseedCounter == CRYPTO_DRBG_CTR_RESEED_INTERVAL) {
        stat = CRYPTO_DRBG_RESEED_REQUIRED;
    } else {
        stat = CRYPTO_DRBG_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_DRBG_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    /* Enable AES */
    Crypto_AES_init();

    /* Generate pseudorandom bits for output */
    aesStat = Crypto_AES_setSessionKey(&ctx->k.w[0], CRYPTO_AES_KEYTYPE_256);
    if (aesStat == CRYPTO_AES_OK) {
        /* Continue normally */
    } else {
        stat = CRYPTO_DRBG_AES_ERR;
        goto exit_disable_aes;
    }
    while (outputLen > 0U) {
        x = Crypto_reverseBytes32(ctx->v.w[3]);
        x++;
        ctx->v.w[3] = Crypto_reverseBytes32(x);
        aesStat = Crypto_AES_encrypt(&ctx->v.w[0],
                                     &outputBlock[0],
                                     CRYPTO_AES_KEYSEL_SESSION);
        if (aesStat == CRYPTO_AES_OK) {
            /* Continue normally */
        } else {
            stat = CRYPTO_DRBG_AES_ERR;
            goto exit_disable_aes;
        }

        for (i=0U; i<CRYPTO_CTRDRBG_BLOCK_LEN_WORDS; i++) {
            if (outputLen > 0U) {
                *(output++) = outputBlock[i];
                outputLen--;
            } else {
                break;
            }
        }
    }

    /* Update internal state */
    ctx->reseedCounter++;
    Crypto_set32(&addtlData[0], 0U, CRYPTO_CTRDRBG_SEED_LEN_WORDS);
    stat = Crypto_DRBG_updateFxn(ctx, &addtlData[0]);
    if (stat == CRYPTO_DRBG_OK) {
        /* Continue normally */
    } else {
        goto exit_disable_aes;
    }

exit_disable_aes:
    Crypto_AES_deinit();

exit:
    return stat;
}

crypto_drbg_stat_t Crypto_DRBG_uninit(crypto_drbg_ctx_t *ctx)
{
    crypto_drbg_stat_t stat;

    stat = CRYPTO_DRBG_OK;

    /* Check preconditions and set status accordingly */
    if (ctx == 0U) {
        stat = CRYPTO_DRBG_PARAM_ERR;
    } else if (ctx->initialized != CRYPTO_DRBG_INIT_TAG) {
        stat = CRYPTO_DRBG_NOINIT_ERR;
    } else {
        stat = CRYPTO_DRBG_OK;
    }

    /* Bail to an error exit if the status is not OK. */
    if (stat == CRYPTO_DRBG_OK) {
        /* Continue normally */
    } else {
        goto exit;
    }

    ctx->initialized = 0U;
    Crypto_set32(&(ctx->v.w[0]), 0U, CRYPTO_BLOCK128_LEN_WORDS);
    Crypto_set32(&(ctx->k.w[0]), 0U, CRYPTO_BLOCK256_LEN_WORDS);

exit:
    return stat;
}
