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
/*!****************************************************************************
 * @file    filterbank_feature_extract.h
 * @brief   Filterbank Feature Extraction — public API and types.
 *
 * This module implements filterbank-based feature extraction for keyword
 * detection and similar machine-learning inference workloads. It provides
 * an efficient pipeline that:
 *  - accepts a continuous stream of signed 16-bit PCM audio samples,
 *  - packs and overlaps frames into the format expected by the NPU,
 *  - runs MSB and LSB weight passes on the NPU hardware accelerator,
 *  - applies maxpool, scale, offset, and quantization post-processing, and
 *  - maintains a sliding feature window for the downstream classifier.
 *
 * Typical usage sequence:
 * @code
 *     FBFE_Config cfg = { ... };
 *     FBFE_Ctx    ctx;
 *     uint8_t     features[FEATURES_BYTES];
 *
 *     FBFE_Init(&ctx, &cfg, features, sizeof(features));
 *
 *     while (audio_available())
 *     {
 *         FBFE_PushSample(&ctx, get_next_sample());
 *     }
 *
 *     if (frame_complete())
 *     {
 *         FBFE_RunOneFrame(&ctx);
 *         // features[] now contains the updated sliding window
 *     }
 * @endcode
 *
 ******************************************************************************/
#ifndef FILTERBANK_FEATURE_EXTRACT_H
#define FILTERBANK_FEATURE_EXTRACT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "fe_model/model_autogen.h"

/* ============================================================
 * User configuration
 * ============================================================ */

/**
 * @brief Configuration parameters for the filterbank feature extraction.
 *
 * Populate this structure before calling FBFE_Init(). The entire structure
 * is copied into the context during initialization, so the caller does not
 * need to keep it alive afterwards.
 *
 * The bit-manipulation fields (ls_mask, ms_mask, ms_shift, branch_bits)
 * must be consistent with the model's expected input encoding. Incorrect
 * values will produce silently wrong feature vectors.
 */
typedef struct
{
    /**
     * @brief Input sampling rate in Hz.
     *
     * Defines the sampling frequency of the incoming PCM audio stream
     * (e.g., 16000 for 16 kHz). Used to derive frame and context lengths
     * in samples from the millisecond-based window parameters.
     */
    uint32_t sampling_rate_hz;

    /**
     * @brief Processing window size in milliseconds.
     *
     * Duration of each analysis frame. Together with sampling_rate_hz
     * this determines NUM_SAMPLES (the number of PCM samples consumed per
     * call to FBFE_RunOneFrame()).
     */
    uint16_t window_size_ms;

    /**
     * @brief Temporal context size in milliseconds.
     *
     * Amount of past audio retained between consecutive frames to provide
     * temporal context for the neural network. Controls the overlap region
     * copied at each frame boundary inside the NPU input buffer.
     */
    uint16_t context_ms;

    /**
     * @brief Bitmask for extracting the least-significant byte of a sample.
     *
     * Applied to the uint16_t view of the scaled sample to isolate the
     * low-order bits before packing into @c ls_bytes[].
     */
    uint16_t ls_mask;

    /**
     * @brief Bitmask for extracting the most-significant byte of a sample.
     *
     * Applied to the uint16_t view of the scaled sample to isolate the
     * high-order bits before they are right-shifted by ms_shift and
     * packed into @c ms_bytes[].
     */
    uint16_t ms_mask;

    /**
     * @brief Right-shift amount applied after masking the MSB portion.
     *
     * After ms_mask isolates the high-order bits, this shift aligns
     * them to the least-significant position of the staging byte stored
     * in @c ms_bytes[].
     */
    uint8_t ms_shift;

    /**
     * @brief Effective branch bit-width for input sample scaling.
     *
     * Controls how much the raw 16-bit PCM sample is scaled down before
     * it is split into MSB/LSB bytes for NPU packing. The scaling is:
     * @code
     *     scale_shift = 16 - branch_bits
     *     scaled      = round(sample / 2^scale_shift)
     * @endcode
     * Valid range is 1–16. A value of 16 disables scaling entirely.
     */
    uint8_t branch_bits;
} FBFE_Config;

/* ============================================================
 * Filterbank context
 * ============================================================ */

/**
 * @brief Runtime state for a single filterbank feature extraction instance.
 *
 * Holds all mutable state required to operate the filterbank pipeline:
 * the active configuration, pointers to library-owned internal buffers,
 * sample-packing staging arrays, processing index counters, and the
 * caller-supplied output feature window.
 *
 * Callers must allocate an instance of this structure (stack or static)
 * and pass it to FBFE_Init() before using any other API function. The
 * structure should be treated as opaque after initialization; direct
 * field access is not part of the public API contract.
 */
typedef struct
{
    /**
     * @brief Copy of the configuration supplied to FBFE_Init().
     *
     * Stored by value so the caller is free to discard or reuse the
     * original FBFE_Config after initialization returns.
     */
    FBFE_Config cfg;

    /**
     * @brief Pointer to the caller-supplied sliding feature window buffer.
     *
     * Updated by FBFE_RunOneFrame() after every frame. The buffer must
     * remain valid for the lifetime of the context. Its required size is
     * at least FEATURES_BYTES bytes.
     */
    uint8_t *final_features;

    /**
     * @brief Size in bytes of the final_features buffer.
     *
     * Captured at initialization and used to guard against buffer overruns
     * when clearing or updating the feature window.
     */
    size_t final_features_bytes;

    /**
     * @brief Index of the next sample slot within the current frame.
     *
     * Counts from 0 to NUM_SAMPLES - 1, wrapping back to 0 once a full
     * frame has been accumulated. Updated by FBFE_PushSample().
     */
    uint16_t sample_index;

    /**
     * @brief Index into the four-sample packing group (0–3).
     *
     * Tracks which position within the current group of four samples is
     * being filled. When it reaches 3, the four staged bytes are flushed
     * to frame_buffer by FBFE_PackFourSamplesIntoFrameBuffer().
     */
    uint8_t pack_index;

    /**
     * @brief Staging array for the least-significant bytes of up to four samples.
     *
     * Holds the LS byte extracted from each of the four samples in the
     * current packing group, indexed by pack_index.
     */
    uint8_t ls_bytes[4];

    /**
     * @brief Staging array for the most-significant bytes of up to four samples.
     *
     * Holds the MS byte extracted (after masking and shifting) from each
     * of the four samples in the current packing group, indexed by
     * pack_index.
     */
    uint8_t ms_bytes[4];

    /**
     * @brief Pointer to the raw packed-word frame buffer (library-owned).
     *
     * Bound to the internal static array @c g_frame_buffer during
     * FBFE_Init(). Stores one complete frame of packed int16_t words
     * ready to be copied into fb_input at frame boundaries.
     * Capacity: NUM_SAMPLES elements.
     */
    int16_t *frame_buffer;

    /**
     * @brief Pointer to the overlapped NPU input buffer (library-owned).
     *
     * Bound to the internal static array @c g_fb_input during
     * FBFE_Init(). Holds the packed byte stream for the NPU, including
     * the overlap region from the previous frame.
     * Capacity: FB_INPUT_BYTES bytes.
     */
    int8_t *fb_input;

    /**
     * @brief Pointer to the MSB-pass output scratch buffer (library-owned).
     *
     * Bound to @c g_msbTemp during FBFE_Init(). Receives the
     * sign-extended DREG19 results of the MSB pass for one stride before
     * they are combined with the LSB results.
     * Capacity: MODEL_MAX_OUT_CH_PER_ITER elements.
     */
    int32_t *msb_temp;

    /**
     * @brief Pointer to the LSB-pass output scratch buffer (library-owned).
     *
     * Bound to @c g_lsbTemp during FBFE_Init(). Receives the
     * sign-extended DREG19 results of the LSB pass for one stride before
     * combination and maxpool update.
     * Capacity: MODEL_MAX_OUT_CH_PER_ITER elements.
     */
    int32_t *lsb_temp;

    /**
     * @brief Pointer to the per-channel maximum accumulator (library-owned).
     *
     * Bound to @c g_maxValues during FBFE_Init(). Tracks the highest
     * combined MSB+LSB activation seen across all stride positions for
     * every output channel within a single frame.
     * Capacity: MODEL_CONV_OUT_CHANNELS elements.
     */
    int32_t *max_values;

    /**
     * @brief Pointer to the single-pass NPU output buffer (library-owned).
     *
     * Bound to @c g_singleOut during FBFE_Init(). Configured as the
     * OFMAP destination in the NPU's CTL1 register for each pass.
     * Capacity: MODEL_CONV_OUT_CHANNELS bytes.
     */
    uint8_t *single_out;

    /**
     * @brief Pointer to the quantized maxpool output buffer (library-owned).
     *
     * Bound to @c g_maxpool during FBFE_Init(). Receives the final
     * uint8_t feature values produced by FBFE_PostprocessBlock() and
     * is passed to FBFE_ShiftNNInputWindow() to update the feature window.
     * Capacity: MODEL_CONV_OUT_CHANNELS bytes.
     */
    uint8_t *maxpool_outputs;

    /**
     * @brief Initialization guard flag.
     *
     * Set to @c true by FBFE_Init() upon successful completion.
     * All API functions check this flag via FBFE_IsReady() and return
     * @c false immediately if it is not set.
     */
    bool inited;
} FBFE_Ctx;

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
void TINIE_start(void);

/**
 * @brief Initialize a filterbank feature extraction context.
 *
 * Validates all arguments, zeros the context structure, copies the
 * configuration, binds the library-owned internal buffers, clears every
 * buffer to a deterministic all-zero state, and marks the context as ready.
 *
 * This function must be called exactly once before any other API function
 * operates on @p ctx. Calling it a second time on an already-initialized
 * context is safe and resets the pipeline to its initial state.
 *
 * @param[in,out] ctx                  Pointer to the caller-allocated context
 *                                     structure to initialize. Must not be NULL.
 * @param[in]     cfg                  Pointer to the configuration to apply.
 *                                     Copied by value; the caller may free or
 *                                     reuse it after this call returns.
 *                                     Must not be NULL.
 * @param[in]     final_features       Pointer to the caller-supplied buffer that
 *                                     will hold the sliding feature window.
 *                                     Must remain valid for the lifetime of
 *                                     @p ctx. Must not be NULL.
 * @param[in]     final_features_bytes Size in bytes of @p final_features.
 *                                     Must be at least FEATURES_BYTES.
 *
 * @return @c true  if the context was successfully initialized.
 * @return @c false if any pointer argument is NULL or if
 *                  @p final_features_bytes < FEATURES_BYTES.
 */
bool FBFE_Init(FBFE_Ctx       *ctx,
               const FBFE_Config *cfg,
               uint8_t           *final_features,
               size_t             final_features_bytes);

/**
 * @brief Push one signed 16-bit PCM sample into the processing pipeline.
 *
 * Scales the sample according to the configured FBFE_Config::branch_bits,
 * splits it into MSB and LSB byte components, and stages them in the internal
 * packing arrays. Every fourth call flushes the staged bytes into the frame
 * buffer in the interleaved layout required by the NPU. When the last sample
 * of a complete frame is received (sample_index == NUM_SAMPLES - 1), the NPU
 * input buffer is updated by sliding the overlap region forward and appending
 * the new frame.
 *
 * Callers must push exactly NUM_SAMPLES samples between consecutive calls to
 * FBFE_RunOneFrame() to keep the pipeline in sync.
 *
 * @param[in,out] ctx    Pointer to an initialized context. Must not be NULL.
 * @param[in]     sample Signed 16-bit PCM audio sample to push.
 *
 * @return @c true  if the sample was accepted and processed.
 * @return @c false if @p ctx is NULL or has not been initialized.
 */
bool FBFE_PushSample(FBFE_Ctx *ctx,
                     int16_t   sample);

/**
 * @brief Run the filterbank NPU pipeline on the current input frame.
 *
 * Executes the complete feature extraction sequence for one frame:
 * -# Computes NPU addressing registers from the current buffer pointers.
 * -# Iterates over MODEL_NUM_OUTPUT_BLOCKS output channel blocks.
 * -# For each block: loads MMR coefficients, programs common NPU state,
 *    loads the instruction stream (first block only), and loads the
 *    block-specific weight parameters.
 * -# For each of MAX_FB_STRIDE_ITER stride positions: runs an MSB pass
 *    followed by an LSB pass, combines their outputs, and updates the
 *    per-channel maximum accumulator.
 * -# Applies scale, offset, right-shift, and uint8_t clamping to each
 *    channel's accumulated maximum value.
 * -# Appends the resulting quantized feature vector to the sliding NN
 *    input window in FBFE_Ctx::final_features.
 *
 * This function should be called once per frame, after NUM_SAMPLES samples
 * have been pushed via FBFE_PushSample().
 *
 * @param[in,out] ctx Pointer to an initialized context. Must not be NULL.
 *
 * @return @c true  if the frame was processed successfully.
 * @return @c false if @p ctx is NULL, has not been initialized, or if a
 *                  weight-parameter block exceeds MODEL_PARAMS_LOAD_CAPACITY.
 */
bool FBFE_RunOneFrame(FBFE_Ctx *ctx);

#endif /* FILTERBANK_FEATURE_EXTRACT_H */