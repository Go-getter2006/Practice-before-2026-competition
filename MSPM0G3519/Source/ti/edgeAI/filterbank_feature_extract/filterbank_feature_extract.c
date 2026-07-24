/*
 * Copyright (c) 2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file filterbank_feature_extract.c
 * @brief Implementation of filterbank-based feature extraction for  processing.
 *
 * This file contains the implementation of  feature extraction using
 * filterbank techniques with NPU hardware acceleration. It processes 
 * samples through a series of filterbank operations to extract meaningful
 * features for machine learning applications.
 * The implementation follows a multi-stage processing pipeline:
* 1. Sample packing into frames with configurable overlap
* 2. NPU-accelerated filterbank processing
* 3. Two-pass processing (MSB and LSB) for extended numerical precision
* 4. Max-pooling for feature dimensionality reduction
* 5. Feature normalization, scaling, and formatting
* 6. Window management for temporal context preservation
*
* This implementation is optimized for low-power operation on TI MSP devices
* with integrated Neural Processing Unit (NPU) hardware acceleration.
*/

#include "filterbank_feature_extract.h"
#include "constants.h"

#include <string.h>
#include <limits.h>

#include "fe_model/fe_model.h"
/* HW includes  */

#include <ti/devices/msp/msp.h>
#include "ti_msp_dl_config.h"
#include <ti/devices/msp/peripherals/hw_npu.h>


/* ============================================================
 * Private macros
 * ============================================================ */

/**
 * @brief Shift bits for NPU CTL1 address field.
 */
#define FBFE_CTL1_ADDR_SHIFT_BITS        (18u)

/**
 * @brief Mask for NPU CTL1 address field.
 */
#define FBFE_CTL1_ADDR_MASK              (0xcfffu)

/**
 * @brief Shift bits for NPU CTL1 OFMAP field.
 */
#define FBFE_CTL1_OFMAP_SHIFT_BITS       (16u)

/**
 * @brief Step size for MMR stride operations.
 */
#define FBFE_MMR_STRIDE_STEP             ((MODEL_CONV_STRIDE_T/4u)*2u)

/**
 * @brief Shift bits for MSB combination in the 2-bit weights flow.
 */
#define FBFE_MSB_COMBINE_SHIFT_BITS      (8u)

/**
 * @brief Shift bits for word addressing.
 */
#define FBFE_WORD_ADDR_SHIFT_BITS        (2u)

/**
 * @brief Mask for 16-bit word addressing.
 */
#define FBFE_WORD_ADDR_16BIT_MASK        (0xffffu)

/**
 * @brief Mask for sign check in ACCREG sign-extension behavior.
 */
#define FE_SIGNCHK_MASK                  (0xFFFDFFFFu)


/**
 * @brief Value for sign check in ACCREG sign-extension behavior.
 */
#define FE_SIGNCHK_VAL                   (0xFFFFFFFFu)

/**
 * @brief OR mask for sign extension in ACCREG sign-extension behavior.
 */
#define FE_SIGNEXT_OR                    (0xFFFC0000u)

/**
 * @brief CTL0 value for MSB pass in 2-bit weights flow.
 */
#ifndef FE_CTL0_MSB
#define FE_CTL0_MSB                      (0x12019u)
#endif

/**
 * @brief CTL0 value for LSB pass in 2-bit weights flow.
 */
#ifndef FE_CTL0_LSB
#define FE_CTL0_LSB                      (0x12011u)
#endif

/**
 * @brief Model-specific alias for CTL0 MSB pass value.
 *
 * Defaults to FE_CTL0_MSB if not overridden by the model header.
 */
#ifndef MODEL_FE_CTL0_MSB
#define MODEL_FE_CTL0_MSB                FE_CTL0_MSB
#endif

/**
 * @brief Model-specific alias for CTL0 LSB pass value.
 *
 * Defaults to FE_CTL0_LSB if not overridden by the model header.
 */
#ifndef MODEL_FE_CTL0_LSB
#define MODEL_FE_CTL0_LSB                FE_CTL0_LSB
#endif

/* ============================================================
 * Private library-owned buffers
 * ============================================================ */

/**
 * @brief Global buffer for storing frame samples.
 *
 * This buffer holds the raw samples for a complete processing frame.
 */
static int16_t g_frame_buffer[NUM_SAMPLES];

/**
 * @brief Global buffer for NPU input data.
 *
 * This buffer holds the packed byte representation of the data,
 * with proper overlapping between frames for continuous processing.
 */
static int8_t  g_fb_input[FB_INPUT_BYTES];

/**
 * @brief Temporary buffer for storing MSB pass results.
 *
 * Used during NPU processing to store most significant byte results
 * before combining with LSB results.
 */
static int32_t g_msbTemp[MODEL_MAX_OUT_CH_PER_ITER];

/**
 * @brief Temporary buffer for storing LSB pass results.
 *
 * Used during NPU processing to store least significant byte results
 * before combining with MSB results.
 */
static int32_t g_lsbTemp[MODEL_MAX_OUT_CH_PER_ITER];

/**
 * @brief Buffer for storing maximum values across all strides.
 *
 * This buffer holds the maximum activation values for each output channel
 * across all stride iterations.
 */
static int32_t g_maxValues[MODEL_CONV_OUT_CHANNELS];

/**
 * @brief Buffer for storing single iteration outputs.
 *
 * This buffer holds the output from a single iteration of the NPU processing.
 */
static uint8_t g_singleOut[MODEL_CONV_OUT_CHANNELS];

/**
 * @brief Buffer for storing maxpool operation results.
 *
 * This buffer holds the final maxpooled feature values after processing
 * and scaling.
 */
static uint8_t g_maxpool[MODEL_CONV_OUT_CHANNELS];

/* ============================================================
 * Private helper forward declarations
 * ============================================================ */

/**
 * @brief Read a sign-extended value from a DREG19 register.
 *
 * @param[in] npu     Pointer to the NPU register block.
 * @param[in] reg_idx Index of the DREG19 register to read.
 *
 * @return Sign-extended 32-bit integer value.
 */
static int32_t FBFE_ReadSignedDREG19(volatile NPU_Regs *npu,
                                     uint16_t reg_idx);

/**
 * @brief Read the output of one NPU pass into a buffer.
 *
 * Reads MODEL_MAX_OUT_CH_PER_ITER lanes from DREG19, reconstructing
 * the full-precision value according to MODEL_WEIGHT_BITS.
 *
 * @param[in]  npu      Pointer to the NPU register block.
 * @param[out] pass_out Pointer to the output array of at least
 *                      MODEL_MAX_OUT_CH_PER_ITER elements.
 */
static void FBFE_ReadPassOutput(volatile NPU_Regs *npu,
                                int32_t *pass_out);

/* ============================================================
 * Private helpers: context and buffers
 * ============================================================ */

/**
 * @brief Check whether a filterbank context is ready for use.
 *
 * @param[in] ctx Pointer to the context to check.
 *
 * @return true if @p ctx is non-NULL and has been successfully initialized,
 *         false otherwise.
 */
static bool FBFE_IsReady(const FBFE_Ctx *ctx)
{
    return ((ctx != NULL) && (ctx->inited));
}

/**
 * @brief Validate the arguments passed to FBFE_Init().
 *
 * Verifies that all pointers are non-NULL and that the supplied
 * features buffer is large enough to hold FEATURES_BYTES bytes.
 *
 * @param[in] ctx                  Pointer to the context to initialize.
 * @param[in] cfg                  Pointer to the configuration structure.
 * @param[in] final_features       Pointer to the output features buffer.
 * @param[in] final_features_bytes Size in bytes of @p final_features.
 *
 * @return true if all arguments are valid, false otherwise.
 */
static bool FBFE_InitArgsAreValid(const FBFE_Ctx *ctx,
                                  const FBFE_Config *cfg,
                                  const uint8_t *final_features,
                                  size_t final_features_bytes)
{
    if ((ctx == NULL) || (cfg == NULL) || (final_features == NULL))
    {
        return false;
    }
    if (final_features_bytes < (size_t)FEATURES_BYTES)
    {
        return false;
    }
    return true;
}

/**
 * @brief Bind the library-owned static buffers to a context.
 *
 * Associates each internal global buffer with the corresponding
 * pointer field in @p ctx so that the rest of the library accesses
 * memory through the context rather than global symbols directly.
 *
 * @param[in,out] ctx Pointer to the context whose buffer pointers
 *                    will be populated.
 */
static void FBFE_BindInternalBuffers(FBFE_Ctx *ctx)
{
    ctx->frame_buffer    = g_frame_buffer;
    ctx->fb_input        = g_fb_input;
    ctx->msb_temp        = g_msbTemp;
    ctx->lsb_temp        = g_lsbTemp;
    ctx->max_values      = g_maxValues;
    ctx->single_out      = g_singleOut;
    ctx->maxpool_outputs = g_maxpool;
}

/**
 * @brief Zero-initialize all internal processing buffers.
 *
 * Clears every library-owned buffer bound to @p ctx so that the
 * pipeline starts from a deterministic, all-zero state.
 *
 * @param[in,out] ctx                  Pointer to the initialized context
 *                                     whose buffers will be cleared.
 * @param[in]     final_features_bytes Number of bytes to clear in
 *                                     ctx->final_features.
 */
static void FBFE_ClearInternalBuffers(FBFE_Ctx *ctx,
                                      size_t final_features_bytes)
{
    memset(ctx->final_features,    0, final_features_bytes);
    memset(ctx->fb_input,          0, sizeof(g_fb_input));
    memset(ctx->frame_buffer,      0, sizeof(g_frame_buffer));
    memset(ctx->max_values,        0, sizeof(g_maxValues));
    memset(ctx->maxpool_outputs,   0, sizeof(g_maxpool));
    memset(ctx->single_out,        0, sizeof(g_singleOut));
}

/* ============================================================
 * Private helpers: input preprocessing and packing
 * ============================================================ */

/**
 * @brief Scale an input audio sample based on the configured branch bit-width.
 *
 * Converts the incoming signed 16-bit sample to the effective input precision
 * expected by the filterbank model. The branch bit-width controls how much the
 * original 16-bit sample is scaled down before it is split into MSB/LSB bytes
 * and packed for TINIE processing.
 *
 * The scaling is derived as:
 * @code
 *     scale_shift = 16 - branch_bits
 *     divisor     = 2 ^ scale_shift
 * @endcode
 *
 * Rounded integer division is used instead of plain truncation, and the
 * result is saturated to the int16_t range before returning.
 *
 * @param[in] sample      The signed 16-bit input audio sample.
 * @param[in] branch_bits Effective branch bit-width (clamped to 16 internally).
 *
 * @return The scaled and saturated signed 16-bit sample.
 */
static int16_t FBFE_ScaleSampleByBranchBits(int16_t sample,
                                            uint8_t branch_bits)
{
    int32_t temp = (int32_t)sample;
    if (branch_bits > 16u)
    {
        branch_bits = 16u;
    }
    const uint8_t scale_shift = (uint8_t)(16u - branch_bits);
    if (scale_shift == 0u)
    {
        return (int16_t)temp;
    }
    const int32_t divisor = (int32_t)(1u << scale_shift);
    const int32_t half    = divisor / 2;
    if (temp >= 0)
    {
        temp = (temp + half) / divisor;
    }
    else
    {
        temp = (temp - half) / divisor;
    }
    if (temp > INT16_MAX)
    {
        temp = INT16_MAX;
    }
    else if (temp < INT16_MIN)
    {
        temp = INT16_MIN;
    }
    return (int16_t)temp;
}

/**
 * @brief Split a scaled sample into its LSB and MSB byte components.
 *
 * Applies the configured ls_mask / ms_mask and ms_shift from @p ctx to
 * decompose @p scaled_sample and stores the results into the ls_bytes and
 * ms_bytes staging arrays at the current pack_index position.
 *
 * @param[in,out] ctx           Pointer to the initialized context.
 * @param[in]     scaled_sample The pre-scaled 16-bit sample to split.
 */
static void FBFE_StoreSampleBytes(FBFE_Ctx *ctx,
                                  int16_t scaled_sample)
{
    uint16_t temp_u16 = (uint16_t)scaled_sample;
    ctx->ls_bytes[ctx->pack_index] =
        (uint8_t)(temp_u16 & ctx->cfg.ls_mask);
    ctx->ms_bytes[ctx->pack_index] =
        (int8_t)((temp_u16 & ctx->cfg.ms_mask) >> ctx->cfg.ms_shift);
}

/**
 * @brief Pack four staged sample bytes into the frame buffer.
 *
 * Reads the four entries currently held in ctx->ms_bytes[] and
 * ctx->ls_bytes[] and writes them into ctx->frame_buffer[] at positions
 * relative to the current sample_index, following the interleaved layout
 * expected by the NPU.
 *
 * This function must only be called when pack_index equals pack_size - 1
 * (i.e., after the fourth sample of a group has been stored).
 *
 * @param[in,out] ctx Pointer to the initialized context.
 */
static void FBFE_PackFourSamplesIntoFrameBuffer(FBFE_Ctx *ctx)
{
    uint16_t i = ctx->sample_index;
    ctx->frame_buffer[i - 3u] =
        (uint16_t)(((ctx->ms_bytes[2] & 0xFFu) << 8) |
                   (ctx->ms_bytes[3] & 0xFFu));
    ctx->frame_buffer[i - 2u] =
        (uint16_t)(((ctx->ms_bytes[0] & 0xFFu) << 8) |
                   (ctx->ms_bytes[1] & 0xFFu));
    ctx->frame_buffer[i - 1u] =
        (uint16_t)(((ctx->ls_bytes[2] & 0xFFu) << 8) |
                   (ctx->ls_bytes[3] & 0xFFu));
    ctx->frame_buffer[i] =
        (uint16_t)(((ctx->ls_bytes[0] & 0xFFu) << 8) |
                   (ctx->ls_bytes[1] & 0xFFu));
}

/**
 * @brief Slide the packed input buffer forward when a full frame is complete.
 *
 * When the last sample of a frame has been received (sample_index == N-1),
 * the overlap region is shifted toward the beginning of fb_input and the
 * newly packed frame is appended, maintaining the continuous overlap required
 * by the NPU input format.
 *
 * Has no effect for any sample index other than N-1.
 *
 * @param[in,out] ctx Pointer to the initialized context.
 */
static void FBFE_UpdateInputBufferAtFrameEnd(FBFE_Ctx *ctx)
{
    const uint16_t N         = NUM_SAMPLES;
    const uint16_t E         = EXTRA_SAMPLES;
    const uint16_t num_bytes = (uint16_t)(N * 2u);
    if (ctx->sample_index == (uint16_t)(N - 1u))
    {
        memcpy((uint8_t *)&ctx->fb_input[0],
               &ctx->fb_input[num_bytes],
               (size_t)(E * 2u));
        memcpy((uint8_t *)&ctx->fb_input[E * 2u],
               ctx->frame_buffer,
               (size_t)num_bytes);
    }
}

/**
 * @brief Advance the pack and sample indices after processing one sample.
 *
 * Increments pack_index modulo pack_size (4) and sample_index modulo
 * NUM_SAMPLES, wrapping both counters back to zero at their respective
 * boundaries.
 *
 * @param[in,out] ctx Pointer to the initialized context.
 */
static void FBFE_AdvanceSampleState(FBFE_Ctx *ctx)
{
    const uint16_t N         = NUM_SAMPLES;
    const uint16_t pack_size = 4u;
    ctx->pack_index   = (uint8_t)((ctx->pack_index   + 1u) % pack_size);
    ctx->sample_index = (uint16_t)((ctx->sample_index + 1u) % N);
}

/* ============================================================
 * Private helpers: final feature window
 * ============================================================ */

/**
 * @brief Shift the neural network input window to incorporate a new frame.
 *
 * Updates the feature window by evicting the oldest frame and appending
 * @p new_frame at the end. Maintains a sliding window of features providing
 * the temporal context required by the neural network input.
 *
 * @param[in,out] ctx       Pointer to the filterbank context.
 * @param[in]     new_frame Pointer to the new frame data
 *                          (MODEL_CONV_OUT_CHANNELS bytes) to append.
 */
static void FBFE_ShiftNNInputWindow(FBFE_Ctx *ctx,
                                    const uint8_t *new_frame)
{
    const uint32_t F = MODEL_CONV_OUT_CHANNELS;
    const uint32_t W = FEATURE_WINDOW;
    memcpy((uint8_t *)&ctx->final_features[F],
           (uint8_t *)&ctx->final_features[2u * F],
           (size_t)(W - 3u) * F);
    memcpy((uint8_t *)&ctx->final_features[(W - 2u) * F],
           (uint8_t *)new_frame,
           F);
}

/* ============================================================
 * Private helpers: NPU address calculations
 * ============================================================ */

/**
 * @brief Compute the MMR0 base address word for the given input buffer.
 *
 * Converts the byte address of @p fb_input to the word-addressed format
 * expected by the NPU MMR0 register by shifting and masking.
 *
 * @param[in] fb_input Pointer to the packed NPU input buffer.
 *
 * @return 32-bit word address value for MMR0.
 */
static uint32_t FBFE_GetMMR0Value(const int8_t *fb_input)
{
    return ((((uint32_t)(uintptr_t)fb_input) >> FBFE_WORD_ADDR_SHIFT_BITS) &
            FBFE_WORD_ADDR_16BIT_MASK);
}

/**
 * @brief Compute the CTL1 register value encoding both IFMAP and OFMAP addresses.
 *
 * Extracts the high-order address bits from @p fb_input (IFMAP) and
 * @p single_out (OFMAP), then packs them into the CTL1 layout by placing
 * the OFMAP field at the FBFE_CTL1_OFMAP_SHIFT_BITS offset.
 *
 * @param[in] fb_input   Pointer to the packed NPU input buffer.
 * @param[in] single_out Pointer to the single-iteration output buffer.
 *
 * @return 32-bit value to be written to the NPU CTL1 register.
 */
static uint32_t FBFE_GetCTL1Value(const int8_t *fb_input,
                                  const uint8_t *single_out)
{
    uint32_t ifmap_addr =
        (((uint32_t)(uintptr_t)fb_input)   >> FBFE_CTL1_ADDR_SHIFT_BITS) &
        FBFE_CTL1_ADDR_MASK;
    uint32_t ofmap_addr =
        (((uint32_t)(uintptr_t)single_out) >> FBFE_CTL1_ADDR_SHIFT_BITS) &
        FBFE_CTL1_ADDR_MASK;
    return ifmap_addr | (ofmap_addr << FBFE_CTL1_OFMAP_SHIFT_BITS);
}

/**
 * @brief Compute the AROUT0 base address word for the given output buffer.
 *
 * Converts the byte address of @p single_out to the word-addressed format
 * expected by the NPU AROUT0 register by shifting and masking.
 *
 * @param[in] single_out Pointer to the single-iteration output buffer.
 *
 * @return 32-bit word address value for AROUT0.
 */
static uint32_t FBFE_GetAROUT0Base(const uint8_t *single_out)
{
    return ((((uint32_t)(uintptr_t)single_out) >> FBFE_WORD_ADDR_SHIFT_BITS) &
            FBFE_WORD_ADDR_16BIT_MASK);
}

/* ============================================================
 * Private helpers: NPU loading and execution
 * ============================================================ */

/**
 * @brief Load the filterbank MMR coefficient table into NPU DREG0.
 *
 * Writes all FBANK_MMR_LEN entries of the FBANK_MMR array into
 * consecutive DREG0 registers starting at index 0.
 *
 * @param[in,out] npu Pointer to the NPU register block.
 */
static void FBFE_LoadMMR(volatile NPU_Regs *npu)
{
    volatile uint32_t *dreg0 = &npu->DREG0;
    for (size_t i = 0; i < FBANK_MMR_LEN; i++)
    {
        dreg0[i] = FBANK_MMR[i];
    }
}

/**
 * @brief Program the NPU registers that are common across all stride passes.
 *
 * Writes the CTL1 address register, the AROUT0 base register, and the
 * MMR0 base address register. These values remain constant for every
 * stride and pass within a single output block.
 *
 * @param[in,out] npu        Pointer to the NPU register block.
 * @param[in]     ctl_val    Value to write to the CTL1 (DREG4) register.
 * @param[in]     arout0Base Value to write to the AROUT0 (DREG8[4]) register.
 * @param[in]     mmr0Val    Value to write to the MMR0 (DREG8[16]) register.
 */
static void FBFE_ProgramCommonNpuState(volatile NPU_Regs *npu,
                                       uint32_t ctl_val,
                                       uint32_t arout0Base,
                                       uint32_t mmr0Val)
{
    npu->DREG4     = ctl_val;
    npu->DREG8[4]  = arout0Base;
    npu->DREG8[16] = mmr0Val;
}

/**
 * @brief Load the filterbank instruction stream into the NPU.
 *
 * Writes all FB_INS_LEN entries of the FB_INS array into the NPU
 * DREG20 instruction registers. Only required once per FBFE_RunOneFrame()
 * call (for block 0); subsequent blocks reuse the same instruction set.
 *
 * @param[in,out] npu Pointer to the NPU register block.
 */
static void FBFE_LoadInstructions(volatile NPU_Regs *npu)
{
    for (size_t i = 0; i < FB_INS_LEN; i++)
    {
        npu->DREG20[i] = FB_INS[i];
    }
}

/**
 * @brief Load the weight parameters for a given output block into the NPU.
 *
 * Selects the parameter slice for @p block from g_fb_param_slices, validates
 * that it fits within MODEL_PARAMS_LOAD_CAPACITY, writes the valid words into
 * DREG21, and zero-pads the remainder of the capacity.
 *
 * Also resets ARWT1 (DREG8[9]) to 0 and programs ARBIAS0 row word count
 * (DREG8[12]) before loading parameters.
 *
 * @param[in,out] npu   Pointer to the NPU register block.
 * @param[in]     block Output block index to load parameters for.
 *
 * @return true if parameters were loaded successfully,
 *         false if the parameter count exceeds MODEL_PARAMS_LOAD_CAPACITY.
 */
static bool FBFE_LoadParamsForBlock(volatile NPU_Regs *npu,
                                    uint16_t block)
{
    const FB_ParamSlice *block_params = &g_fb_param_slices[block][0];
    if (block_params->words > MODEL_PARAMS_LOAD_CAPACITY)
    {
        return false;
    }
    npu->DREG8[9]  = 0u;
    npu->DREG8[12] =
        MODEL_ARBIAS0_ROW_WORD64 & FBFE_WORD_ADDR_16BIT_MASK;
    for (uint32_t i = 0; i < block_params->words; i++)
    {
        npu->DREG21[i] = block_params->params[i];
    }
    for (uint32_t i = block_params->words; i < MODEL_PARAMS_LOAD_CAPACITY; i++)
    {
        npu->DREG21[i] = 0u;
    }
    return true;
}

/**
 * @brief Execute one MSB or LSB TINIE pass and capture the output.
 *
 * Resets the program counter, programs the stride-adjusted MMR0 address,
 * writes the CTL0 control word, triggers NPU execution via TINIE_start(),
 * waits for completion with __WFI(), then reads the pass output into
 * @p pass_out via FBFE_ReadPassOutput().
 *
 * @param[in,out] npu           Pointer to the NPU register block.
 * @param[in]     mmr0Val       Base MMR0 word address for the current frame.
 * @param[in]     stride        Current stride index (0-based).
 * @param[in]     ctl0          CTL0 value selecting MSB or LSB mode
 *                              (MODEL_FE_CTL0_MSB or MODEL_FE_CTL0_LSB).
 * @param[in]     stride_offset Additional offset added to the stride step
 *                              (0 for MSB pass, 1 for LSB pass).
 * @param[out]    pass_out      Pointer to an array of at least
 *                              MODEL_MAX_OUT_CH_PER_ITER elements that will
 *                              receive the pass output values.
 */
static void FBFE_RunTiniePass(volatile NPU_Regs *npu,
                              uint32_t mmr0Val,
                              uint16_t stride,
                              uint32_t ctl0,
                              uint32_t stride_offset,
                              int32_t *pass_out)
{
    npu->DREG7     = 0u;
    npu->DREG8[16] =
        mmr0Val + (uint32_t)((FBFE_MMR_STRIDE_STEP * stride) + stride_offset);
    npu->DREG5 = ctl0;
    TINIE_start();
    __WFI();
    FBFE_ReadPassOutput(npu, pass_out);
}

/* ============================================================
 * Private helpers: DREG19 read and precision reconstruction
 * ============================================================ */

/**
 * @brief Read a sign-extended value from a DREG19 register.
 *
 * Reads the raw 32-bit value at DREG19[@p reg_idx] and applies the
 * ACCREG sign-extension heuristic: if the value, when ORed with
 * FE_SIGNCHK_MASK, equals FE_SIGNCHK_VAL, the upper bits are filled
 * using FE_SIGNEXT_OR to produce the correct signed result.
 *
 * @param[in] npu     Pointer to the NPU register block.
 * @param[in] reg_idx Index into the DREG19 register array.
 *
 * @return Sign-extended 32-bit signed integer value.
 */
static int32_t FBFE_ReadSignedDREG19(volatile NPU_Regs *npu,
                                     uint16_t reg_idx)
{
    uint32_t raw  = npu->DREG19[reg_idx];
    int32_t  temp = (int32_t)raw;
    if ((raw | FE_SIGNCHK_MASK) == FE_SIGNCHK_VAL)
    {
        temp = (int32_t)(raw | FE_SIGNEXT_OR);
    }
    return temp;
}

/**
 * @brief Read the output of one NPU pass into a caller-supplied buffer.
 *
 * Reads MODEL_MAX_OUT_CH_PER_ITER output lanes from the DREG19 register
 * array and reconstructs the full-precision integer value for each lane
 * according to MODEL_WEIGHT_BITS:
 *
 * - 2-bit: one DREG19 word per lane (direct read).
 * - 4-bit: two DREG19 words per lane; result = high_part * 4 + low_part.
 * - 8-bit: four DREG19 words per lane;
 *          result = hh*64 + hl*16 + lh*4 + ll.
 *
 * @param[in]  npu      Pointer to the NPU register block.
 * @param[out] pass_out Pointer to an output array of at least
 *                      MODEL_MAX_OUT_CH_PER_ITER elements.
 */
static void FBFE_ReadPassOutput(volatile NPU_Regs *npu,
                                int32_t *pass_out)
{
#if (MODEL_WEIGHT_BITS == 2u)
    for (uint16_t lane = 0; lane < MODEL_MAX_OUT_CH_PER_ITER; lane++)
    {
        pass_out[lane] = FBFE_ReadSignedDREG19(npu, lane);
    }
#elif (MODEL_WEIGHT_BITS == 4u)
    for (uint16_t lane = 0; lane < MODEL_MAX_OUT_CH_PER_ITER; lane++)
    {
        uint16_t base      = (uint16_t)(2u * lane);
        int32_t  low_part  = FBFE_ReadSignedDREG19(npu, base);
        int32_t  high_part = FBFE_ReadSignedDREG19(npu, (uint16_t)(base + 1u));
        pass_out[lane]     = (high_part * 4) + low_part;
    }
#elif (MODEL_WEIGHT_BITS == 8u)
    for (uint16_t lane = 0; lane < MODEL_MAX_OUT_CH_PER_ITER; lane++)
    {
        uint16_t base    = (uint16_t)(4u * lane);
        int32_t  part_ll = FBFE_ReadSignedDREG19(npu, base);
        int32_t  part_lh = FBFE_ReadSignedDREG19(npu, (uint16_t)(base + 1u));
        int32_t  part_hl = FBFE_ReadSignedDREG19(npu, (uint16_t)(base + 2u));
        int32_t  part_hh = FBFE_ReadSignedDREG19(npu, (uint16_t)(base + 3u));
        pass_out[lane]   =
            (part_hh * 64) +
            (part_hl * 16) +
            (part_lh *  4) +
             part_ll;
    }
#else
#error "Unsupported MODEL_WEIGHT_BITS"
#endif
}

/* ============================================================
 * Private helpers: maxpool and postprocess
 * ============================================================ */

/**
 * @brief Initialize the max-value accumulators for one output block.
 *
 * Sets the max_values entries for all MODEL_MAX_OUT_CH_PER_ITER lanes
 * starting at @p base_ch to INT_MIN, ensuring that the first real
 * activation value written during stride processing is always accepted.
 *
 * @param[in,out] ctx     Pointer to the initialized context.
 * @param[in]     base_ch First channel index of the current output block.
 */
static void FBFE_InitBlockMaxValues(FBFE_Ctx *ctx,
                                    uint16_t base_ch)
{
    for (uint16_t lane = 0; lane < MODEL_MAX_OUT_CH_PER_ITER; lane++)
    {
        ctx->max_values[base_ch + lane] = INT_MIN;
    }
}

/**
 * @brief Update the per-channel maximum values after one stride pass pair.
 *
 * Combines the MSB and LSB pass outputs for each lane using the formula:
 * @code
 *     combined = lsb_temp[lane] + msb_temp[lane] * (1 << FBFE_MSB_COMBINE_SHIFT_BITS)
 * @endcode
 * Then updates max_values[base_ch + lane] if @p combined exceeds the
 * current stored maximum for that channel.
 *
 * @param[in,out] ctx     Pointer to the initialized context.
 * @param[in]     base_ch First channel index of the current output block.
 */
static void FBFE_UpdateMaxValuesForStride(FBFE_Ctx *ctx,
                                          uint16_t base_ch)
{
    for (uint16_t lane = 0; lane < MODEL_MAX_OUT_CH_PER_ITER; lane++)
    {
        const int32_t combined =
            ctx->lsb_temp[lane] +
            (ctx->msb_temp[lane] *
             (int32_t)(1u << FBFE_MSB_COMBINE_SHIFT_BITS));
        const uint16_t idx = (uint16_t)(base_ch + lane);
        if (combined > ctx->max_values[idx])
        {
            ctx->max_values[idx] = combined;
        }
    }
}

/**
 * @brief Apply scale, offset, and quantization to one output channel.
 *
 * Computes:
 * @code
 *     result = ((max_value + FB_OFFSET[idx]) * FB_SCALE[idx]) >> FB_SHIFT[idx]
 * @endcode
 * using 64-bit arithmetic to avoid overflow, then clamps the result to
 * the [0, 255] range before returning it as a uint8_t.
 *
 * @param[in] idx       Global output channel index used to index
 *                      FB_OFFSET, FB_SCALE, and FB_SHIFT tables.
 * @param[in] max_value Maximum activation value accumulated across all
 *                      stride iterations for this channel.
 *
 * @return Quantized uint8_t feature value in [0, 255].
 */
static uint8_t FBFE_PostprocessOneChannel(uint16_t idx,
                                          int32_t max_value)
{
    int64_t result = (int64_t)max_value;
    result = result + (int64_t)FB_OFFSET[idx];
    result = result * (int64_t)FB_SCALE[idx];
    result = result >> FB_SHIFT[idx];
    if (result < 0)
    {
        result = 0;
    }
    else if (result > 255)
    {
        result = 255;
    }
    return (uint8_t)result;
}

/**
 * @brief Apply post-processing to all channels of one output block.
 *
 * Iterates over the MODEL_MAX_OUT_CH_PER_ITER lanes starting at @p base_ch
 * and calls FBFE_PostprocessOneChannel() for each, storing the resulting
 * quantized values into ctx->maxpool_outputs.
 *
 * @param[in,out] ctx     Pointer to the initialized context.
 * @param[in]     base_ch First channel index of the current output block.
 */
static void FBFE_PostprocessBlock(FBFE_Ctx *ctx,
                                  uint16_t base_ch)
{
    for (uint16_t lane = 0; lane < MODEL_MAX_OUT_CH_PER_ITER; lane++)
    {
        const uint16_t idx = (uint16_t)(base_ch + lane);
        ctx->maxpool_outputs[idx] =
            FBFE_PostprocessOneChannel(idx, ctx->max_values[idx]);
    }
}

/* ============================================================
 * Public API
 * ============================================================ */

/**
 * @brief Start the NPU layer execution.
 *
 * This function triggers the execution of a layer in the NPU by setting
 * the START bit in the CTL5 register.
 *
 * @return None
 */
void TINIE_start(void)
{
    // start layer execution, set START bit 1 in CTL5
    volatile uint32_t* mmrs = (uint32_t*) NPU_getCtlReg();
    mmrs[0] = 0x1;
}

/**
 * @brief Initialize the filterbank feature extraction context.
 *
 * Validates all arguments, clears the context structure, stores the
 * configuration and output buffer, binds the library-owned internal
 * buffers, zeros every buffer for a deterministic startup state, and
 * marks the context as ready.
 *
 * @param[in,out] ctx                  Pointer to the context structure
 *                                     to initialize.
 * @param[in]     cfg                  Pointer to the configuration structure.
 * @param[in]     final_features       Pointer to the caller-supplied buffer
 *                                     where the sliding feature window will
 *                                     be maintained.
 * @param[in]     final_features_bytes Size in bytes of @p final_features;
 *                                     must be at least FEATURES_BYTES.
 *
 * @return true if initialization succeeded, false if any argument is
 *         invalid or the buffer is too small.
 */
bool FBFE_Init(FBFE_Ctx *ctx,
               const FBFE_Config *cfg,
               uint8_t *final_features,
               size_t final_features_bytes)
{
    if (!FBFE_InitArgsAreValid(ctx, cfg, final_features, final_features_bytes))
    {
        return false;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->cfg                   = *cfg;
    ctx->final_features        = final_features;
    ctx->final_features_bytes  = final_features_bytes;
    FBFE_BindInternalBuffers(ctx);
    FBFE_ClearInternalBuffers(ctx, final_features_bytes);
    ctx->sample_index = 0u;
    ctx->pack_index   = 0u;
    ctx->inited       = true;
    return true;
}

/**
 * @brief Push a new audio sample into the filterbank processing pipeline.
 *
 * Scales the incoming sample to the configured branch bit-width, splits
 * it into MSB/LSB byte components, and stages them in the internal packing
 * arrays. When every fourth sample has been staged, the four samples are
 * packed into the frame buffer. When the final sample of a complete frame
 * has been received, the NPU input buffer is updated with the new frame
 * and the appropriate overlap region.
 *
 * @param[in,out] ctx    Pointer to the initialized context.
 * @param[in]     sample The signed 16-bit audio sample to push.
 *
 * @return true if the sample was accepted, false if @p ctx is NULL or
 *         has not been initialized.
 */
bool FBFE_PushSample(FBFE_Ctx *ctx,
                     int16_t sample)
{
    const uint16_t pack_size = 4u;
    if (!FBFE_IsReady(ctx))
    {
        return false;
    }
    int16_t scaled_sample =
        FBFE_ScaleSampleByBranchBits(sample, ctx->cfg.branch_bits);
    FBFE_StoreSampleBytes(ctx, scaled_sample);
    if (ctx->pack_index == (uint8_t)(pack_size - 1u))
    {
        FBFE_PackFourSamplesIntoFrameBuffer(ctx);
    }
    FBFE_UpdateInputBufferAtFrameEnd(ctx);
    FBFE_AdvanceSampleState(ctx);
    return true;
}

/**
 * @brief Process one complete frame through the filterbank NPU pipeline.
 *
 * Executes the full filterbank feature extraction sequence:
 * -# Computes NPU addressing parameters from the current input/output buffers.
 * -# Iterates over MODEL_NUM_OUTPUT_BLOCKS output blocks.
 * -# For each block: initializes max accumulators, loads MMR coefficients,
 *    programs common NPU state, loads instructions (block 0 only), and
 *    loads the block weight parameters.
 * -# For each stride within the block: runs an MSB pass and an LSB pass
 *    via FBFE_RunTiniePass(), then updates the per-channel maximum values.
 * -# Applies scale, offset, shift, and uint8_t quantization to each channel.
 * -# Appends the resulting feature vector to the sliding NN input window.
 *
 * @param[in,out] ctx Pointer to the initialized context.
 *
 * @return true if the frame was processed successfully, false if @p ctx
 *         is NULL or uninitialized, or if a parameter block exceeds
 *         MODEL_PARAMS_LOAD_CAPACITY.
 */
bool FBFE_RunOneFrame(FBFE_Ctx *ctx)
{
    if (!FBFE_IsReady(ctx))
    {
        return false;
    }
    int8_t             *fb_input    = ctx->fb_input;
    volatile NPU_Regs  *npu         = (volatile NPU_Regs *)NPU_BASE;
    const uint32_t      mmr0Val     = FBFE_GetMMR0Value(fb_input);
    const uint32_t      ctl_val     = FBFE_GetCTL1Value(fb_input, ctx->single_out);
    const uint32_t      arout0Base  = FBFE_GetAROUT0Base(ctx->single_out);
    for (uint16_t block = 0; block < MODEL_NUM_OUTPUT_BLOCKS; block++)
    {
        const uint16_t base_ch =
            (uint16_t)(block * MODEL_MAX_OUT_CH_PER_ITER);
        FBFE_InitBlockMaxValues(ctx, base_ch);
        FBFE_LoadMMR(npu);
        FBFE_ProgramCommonNpuState(npu, ctl_val, arout0Base, mmr0Val);
        if (block == 0u)
        {
            FBFE_LoadInstructions(npu);
        }
        if (!FBFE_LoadParamsForBlock(npu, block))
        {
            return false;
        }
        for (uint16_t stride = 0; stride < MAX_FB_STRIDE_ITER; stride++)
        {
            FBFE_RunTiniePass(npu,
                              mmr0Val,
                              stride,
                              MODEL_FE_CTL0_MSB,
                              0u,
                              ctx->msb_temp);
            FBFE_RunTiniePass(npu,
                              mmr0Val,
                              stride,
                              MODEL_FE_CTL0_LSB,
                              1u,
                              ctx->lsb_temp);
            FBFE_UpdateMaxValuesForStride(ctx, base_ch);
        }
        FBFE_PostprocessBlock(ctx, base_ch);
    }
    FBFE_ShiftNNInputWindow(ctx, ctx->maxpool_outputs);
    return true;
}