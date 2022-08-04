/*
 * Copyright (c) 2015 James Darnley
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/attributes.h"
#include "libavutil/cpu.h"
#include "libavutil/x86/cpu.h"
#include "libavfilter/removegrain.h"

void ff_rg_fl_mode_1_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_10_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_11_12_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_13_14_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_19_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_20_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_21_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_22_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
#if ARCH_X86_64
void ff_rg_fl_mode_2_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_3_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_4_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_5_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_6_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_7_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_8_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_9_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_15_16_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_17_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_18_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_23_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
void ff_rg_fl_mode_24_sse2(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int pixels);
#endif

av_cold void ff_removegrain_init_x86(RemoveGrainContext *rg)
{
}
