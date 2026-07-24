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
 *  @file       utilities.c
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "ti/crypto/include/utilities/utilities.h"

void Crypto_copy8(const uint8_t *src, uint8_t *dst, uint32_t byteCnt)
{
    while (byteCnt-- > 0U)
    {
        *(dst++) = *(src++);
    };
    return;
}

void Crypto_copy32(const uint32_t *src, uint32_t *dst, uint32_t wordCnt)
{
    while (wordCnt-- > 0U)
    {
        *(dst++) = *(src++);
    };
    return;
}

void Crypto_set8(uint8_t *dst, uint8_t val, uint32_t byteCnt)
{
    while (byteCnt-- > 0U)
    {
        *(dst++) = val;
    };
    return;
}

void Crypto_set32(uint32_t *dst, uint32_t val, uint32_t wordCnt)
{
    while (wordCnt-- > 0U)
    {
        *(dst++) = val;
    };
    return;
}

void Crypto_xor32(uint32_t *x, const uint32_t *y, uint32_t wordCnt)
{
    while (wordCnt-- > 0U)
    {
        *(x++) ^= *(y++);
    };
    return;
}

uint32_t Crypto_reverseBytes32(uint32_t x)
{
    /* REV is a assembly command that swaps byte order */
    asm volatile("REV %0, %0" : "+r"(x));
    return x;
}

void Crypto_copyReverse8(const uint8_t *src, uint8_t *dest, uint32_t len)
{
    uint32_t i = 0U;

    while(len > 0U) {
        dest[i++] = src[--len];
    }

    return;
}

void Crypto_delay(volatile uint32_t iterations)
{
    while (iterations > 0U) {
        iterations--;
    }

    return;
}

uint8_t Crypto_memcmp(const uint8_t *x, const uint8_t *y, const uint32_t len)
{
    uint8_t status = 0x00;
    for(int i = 0; i < len; i++)
    {
        status |= ((uint8_t)x[i] ^ (uint8_t)y[i]);
    }
    return status;
}

uint32_t Crypto_memcmp32(
    const volatile uint32_t* volatile buffer0,
    const volatile uint32_t* volatile buffer1, uint32_t bufferByteLength)
{
    volatile uint32_t tempResult = 0U;
    uint32_t word0;
    uint32_t word1;
    uint32_t i;

    /* We could skip the branch and just set tempResult equal to the
     * statement below for the same effect but this is more explicit.
     */
    if ((bufferByteLength % sizeof(uint32_t)) != 0U) {
        tempResult = 1U;
    }

    /* XOR each 32-bit word of the buffer together and OR the results.
     * If the OR'd result is non-zero, the buffers do not match.
     * There is no branch based on the content of the buffers here to avoid
     * timing attacks.
     */
    else {
        for (i = (uint32_t) 0U; i < (bufferByteLength / sizeof(uint32_t));
            i++) {
            word0 = buffer0[i];
            word1 = buffer1[i];

            tempResult |= word0 ^ word1;
        }
    }

    return tempResult == 0U;
}

void Crypto_copyReversePad(
    const void* source, uint32_t* destination, uint32_t sourceLength)
{
    uint32_t i;
    uint8_t remainder;
    uint32_t temp;
    uint8_t* tempBytePointer;
    const uint8_t* sourceBytePointer;

    remainder         = (uint8_t) (sourceLength % sizeof(uint32_t));
    temp              = (uint32_t) 0U;
    tempBytePointer   = (uint8_t*) &temp;
    sourceBytePointer = (uint8_t*) source;

    /* Copy source to destination starting at the end of source and the
     * beginning of destination.
     * We assemble each word in byte-reversed order and write one word at a
     * time since the PKA_RAM requires word-aligned reads and writes.
     */

    for (i = (uint32_t) 0U; i < (sourceLength / sizeof(uint32_t)); i++) {
        uint32_t sourceOffset = sourceLength - 1U - (sizeof(uint32_t) * i);

        tempBytePointer[3U] = sourceBytePointer[sourceOffset - 3U];
        tempBytePointer[2U] = sourceBytePointer[sourceOffset - 2U];
        tempBytePointer[1U] = sourceBytePointer[sourceOffset - 1U];
        tempBytePointer[0U] = sourceBytePointer[sourceOffset - 0U];

        *(destination + i) = temp;
    }

    /* Reset to 0 so we do not have to zero-out individual bytes */
    temp = (uint32_t) 0U;

    /* If sourceLength is not a word-multiple, we need to copy over the
     * remaining bytes and zero pad the word we are writing to PKA_RAM.
     */
    if (remainder == 1U) {
        tempBytePointer[0U] = sourceBytePointer[0U];

        /* i is reused from the loop above. This write  zero-pads the
         * destination buffer to word-length.
         */
        *(destination + i) = temp;
    } else if (remainder == 2U) {
        tempBytePointer[0U] = sourceBytePointer[1U];
        tempBytePointer[1U] = sourceBytePointer[0U];

        *(destination + i) = temp;
    } else if (remainder == 3U) {
        tempBytePointer[0U] = sourceBytePointer[2U];
        tempBytePointer[1U] = sourceBytePointer[1U];
        tempBytePointer[2U] = sourceBytePointer[0U];

        *(destination + i) = temp;
    } else {
        /* Continue */
    }
}

void Crypto_reverseInPlaceBytewise(void* buffer, size_t bufferByteLength)
{
    uint8_t* bufferLow  = buffer;
    uint8_t* bufferHigh = &bufferLow[bufferByteLength - 1U];
    uint8_t tmp;

    while (bufferLow < bufferHigh) {
        tmp         = *bufferLow;
        *bufferLow  = *bufferHigh;
        *bufferHigh = tmp;
        bufferLow++;
        bufferHigh--;
    }
}
