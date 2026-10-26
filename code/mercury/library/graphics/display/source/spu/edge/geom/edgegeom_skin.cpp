/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_printf.h>
#include <spu_intrinsics.h>

#include "edge/geom/edgegeom.h"

/* from edgegeom_masks.cpp */
extern const vec_uchar16 s_000A000C000E000G;
extern const vec_uchar16 s_0b0d0f0h0j0l0n0p;
extern const vec_uchar16 s_0Dd0;
extern const vec_uchar16 s_AaBb;
extern const vec_uchar16 s_ABC0;
extern const vec_uchar16 s_BaA0;
extern const vec_uchar16 s_BbAa;
extern const vec_uchar16 s_CcCc;
extern const vec_uchar16 s_Ccb0;
extern const vec_uchar16 s_dBC0;
extern const vec_uchar16 s_BCAD;

extern "C" void edgeGeomSkinUniformTable(EdgeGeomSpuContext *ctx, vec_float4 *uniform, const vec_uchar16 *indexesAndWeights,
	vec_float4 *matrices, int matrixCount)
{
	if(!matrixCount)
		return;
		
	// Various select and shuffle masks used below
	const vec_uint4 mask_XXX0 = (vec_uint4)spu_maskb(0xFFF0);
	const vec_uchar16 s_000I000K000M000O = (vec_uchar16)spu_add((vec_int4)s_000A000C000E000G, 0x08);
	const vec_uchar16 s_AAAA             = (vec_uchar16)spu_splats(0x00010203);
	const vec_uchar16 s_BBBB             = (vec_uchar16)spu_splats(0x04050607);
	const vec_uchar16 s_CCCC             = (vec_uchar16)spu_splats(0x08090A0B);
	const vec_uchar16 s_DDDD             = (vec_uchar16)spu_splats(0x0C0D0E0F);
	const vec_uchar16 s_CABD             = spu_shuffle(s_BCAD, s_BCAD, s_BCAD);
	const vec_float4  k1_0f              = (vec_float4)si_ilhu(0x3f80);
	
	const qword pMatRow0 = si_from_ptr(matrices + 0);
	const qword pMatRow1 = si_from_ptr(matrices + 1);
	const qword pMatRow2 = si_from_ptr(matrices + 2);

	// Variables used by actual skinning (which
	// must persist across loop iterations)
	vec_float4 w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15, w16;
	vec_uint4 mat1, mat2, mat3, mat4, mat5, mat6, mat7, mat8, mat9, mat10, mat11, mat12, mat13, mat14, mat15, mat16;
	vec_uchar16 ind1, ind2;
	vec_float4 M1_1={0}, M1_2={0}, M1_3={0}, M1_4={0};
	vec_float4 M2_1={0}, M2_2={0}, M2_3={0}, M2_4={0};
	vec_float4 M3_1={0}, M3_2={0}, M3_3={0}, M3_4={0};
	vec_float4 M4_1={0}, M4_2={0}, M4_3={0}, M4_4={0};
	
	const vec_float4 weightScale = (vec_float4)spu_splats(1.0f / 255.0f);

	//// Preamble
	qword pSrc = si_from_ptr(uniform);

	ind1 = *indexesAndWeights;
	ind2 = *(indexesAndWeights + 1);
	indexesAndWeights = indexesAndWeights + 2;
	
	// Unpack the weights into four quadwords, and scale them back up to floats
	vec_float4 weights1 = spu_convtf( (vec_uint4)spu_shuffle(ind1, ind1, s_000A000C000E000G), 0);
	vec_float4 weights2 = spu_convtf( (vec_uint4)spu_shuffle(ind1, ind1, s_000I000K000M000O), 0);
	vec_float4 weights3 = spu_convtf( (vec_uint4)spu_shuffle(ind2, ind2, s_000A000C000E000G), 0);
	vec_float4 weights4 = spu_convtf( (vec_uint4)spu_shuffle(ind2, ind2, s_000I000K000M000O), 0);
	weights1 = spu_mul(weights1, weightScale);
	weights2 = spu_mul(weights2, weightScale);
	weights3 = spu_mul(weights3, weightScale);
	weights4 = spu_mul(weights4, weightScale);

	vec_ushort8 indexes1 = (vec_ushort8)spu_shuffle(ind1, ind1, s_0b0d0f0h0j0l0n0p);
	vec_ushort8 indexes2 = (vec_ushort8)spu_shuffle(ind2, ind2, s_0b0d0f0h0j0l0n0p);

	vec_float4 pos1 = (vec_float4)si_lqd(pSrc, 0x00);
	vec_float4 pos2 = (vec_float4)si_lqd(pSrc, 0x10);
	vec_float4 pos3 = (vec_float4)si_lqd(pSrc, 0x20);
	vec_float4 pos4 = (vec_float4)si_lqd(pSrc, 0x30);
	vec_float4 pos1_ = spu_shuffle(pos1, pos1, s_BCAD);
	vec_float4 pos2_ = spu_shuffle(pos2, pos2, s_BCAD);
	vec_float4 pos3_ = spu_shuffle(pos3, pos3, s_BCAD);
	vec_float4 pos4_ = spu_shuffle(pos4, pos4, s_BCAD);
	vec_float4 pos1__ = spu_shuffle(pos1, pos1, s_CABD);
	vec_float4 pos2__ = spu_shuffle(pos2, pos2, s_CABD);
	vec_float4 pos3__ = spu_shuffle(pos3, pos3, s_CABD);
	vec_float4 pos4__ = spu_shuffle(pos4, pos4, s_CABD);

	uint32_t iterCount = ctx->spuConfigInfo.numVertexes / 4;
	//asm volatile ("ESLT:");
	while(true)
	{
		//__asm__ volatile ("EdgeSkin_LoopTop:");
		//
		// Load mat1-mat16, weighting them by w1-w16, and combining
		// each block of four into the final matrices M1-M4.  The
		// final matrices need to have their fourth row transposed
		// into place from their fourth column, with a 1.0 in the
		// upper right corner (to preserve the input vectors' W component).
		//

		// Load the next pair of index/weight quads

		// Broadcast each weight into all four elements of its own quadword
		w1  = spu_shuffle(weights1, weights1, s_AAAA);
		w2  = spu_shuffle(weights1, weights1, s_BBBB);
		w3  = spu_shuffle(weights1, weights1, s_CCCC);
		w4  = spu_shuffle(weights1, weights1, s_DDDD);
		w5  = spu_shuffle(weights2, weights2, s_AAAA);
		w6  = spu_shuffle(weights2, weights2, s_BBBB);
		w7  = spu_shuffle(weights2, weights2, s_CCCC);
		w8  = spu_shuffle(weights2, weights2, s_DDDD);
		w9  = spu_shuffle(weights3, weights3, s_AAAA);
		w10 = spu_shuffle(weights3, weights3, s_BBBB);
		w11 = spu_shuffle(weights3, weights3, s_CCCC);
		w12 = spu_shuffle(weights3, weights3, s_DDDD);
		w13 = spu_shuffle(weights4, weights4, s_AAAA);
		w14 = spu_shuffle(weights4, weights4, s_BBBB);
		w15 = spu_shuffle(weights4, weights4, s_CCCC);
		w16 = spu_shuffle(weights4, weights4, s_DDDD);

		// Unpack the indexes, scaling each index up by 48 (to convert
		// from a matrix index to a byte offset)
		mat2  = (vec_uint4)spu_add( spu_sl(indexes1, 5), spu_sl(indexes1, 4) );	
		mat10 = (vec_uint4)spu_add( spu_sl(indexes2, 5), spu_sl(indexes2, 4) );
		// Move each index into the preferred word of a quadword.  We
		// do not need to clear the high halfwords, since their low 4
		// bits are guarenteed to be zero (and that's enough for an
		// 18-bit address).
		mat4  = spu_slqwbyte(mat2, 4);
		mat6  = spu_slqwbyte(mat2, 8);
		mat8  = spu_slqwbyte(mat2, 12);
		mat12 = spu_slqwbyte(mat10, 4);
		mat14 = spu_slqwbyte(mat10, 8);
		mat16 = spu_slqwbyte(mat10, 12);
		mat1  = spu_rl(mat2, 16);
		mat3  = spu_rl(mat4, 16);
		mat5  = spu_rl(mat6, 16);
		mat7  = spu_rl(mat8, 16);
		mat9  = spu_rl(mat10, 16);
		mat11 = spu_rl(mat12, 16);
		mat13 = spu_rl(mat14, 16);
		mat15 = spu_rl(mat16, 16);
		
		ind1 = *indexesAndWeights;
		ind2 = *(indexesAndWeights + 1);
		indexesAndWeights = indexesAndWeights + 2;

		// Unpack the weights into four quadwords, and scale them back up to floats
		weights1 = spu_convtf( (vec_uint4)spu_shuffle(ind1, ind1, s_000A000C000E000G), 0);
		weights2 = spu_convtf( (vec_uint4)spu_shuffle(ind1, ind1, s_000I000K000M000O), 0);
		weights3 = spu_convtf( (vec_uint4)spu_shuffle(ind2, ind2, s_000A000C000E000G), 0);
		weights4 = spu_convtf( (vec_uint4)spu_shuffle(ind2, ind2, s_000I000K000M000O), 0);
		weights1 = spu_mul(weights1, weightScale);
		weights2 = spu_mul(weights2, weightScale);
		weights3 = spu_mul(weights3, weightScale);
		weights4 = spu_mul(weights4, weightScale);
		
		indexes1 = (vec_ushort8)spu_shuffle(ind1, ind1, s_0b0d0f0h0j0l0n0p);
		indexes2 = (vec_ushort8)spu_shuffle(ind2, ind2, s_0b0d0f0h0j0l0n0p);
		
		// Create final matrix M1
		const vec_float4 mat1_1 = spu_mul( w1, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat1));
		const vec_float4 mat1_2 = spu_mul( w1, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat1));
		const vec_float4 mat1_3 = spu_mul( w1, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat1));
		const vec_float4 mat2_1 = spu_madd(w2, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat2), mat1_1);
		const vec_float4 mat2_2 = spu_madd(w2, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat2), mat1_2);
		const vec_float4 mat2_3 = spu_madd(w2, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat2), mat1_3);
		const vec_float4 mat3_1 = spu_madd(w3, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat3), mat2_1);
		const vec_float4 mat3_2 = spu_madd(w3, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat3), mat2_2);
		const vec_float4 mat3_3 = spu_madd(w3, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat3), mat2_3);
		M1_1 = spu_madd(w4, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat4), mat3_1);
		M1_2 = spu_madd(w4, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat4), mat3_2);
		M1_3 = spu_madd(w4, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat4), mat3_3);
		M1_4 = spu_shuffle(M1_2, M1_3, s_0Dd0);
        M1_4 = spu_shuffle(M1_4, M1_1, s_dBC0);
        M1_1 = spu_sel(k1_0f, M1_1, mask_XXX0);
		M1_2 = spu_sel((vec_float4)mask_XXX0, M1_2, mask_XXX0); 
		M1_3 = spu_sel((vec_float4)mask_XXX0, M1_3, mask_XXX0); 
		
		// Create final matrix M2
		const vec_float4 mat5_1 = spu_mul( w5, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat5));
		const vec_float4 mat5_2 = spu_mul( w5, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat5));
		const vec_float4 mat5_3 = spu_mul( w5, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat5));
		const vec_float4 mat6_1 = spu_madd(w6, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat6), mat5_1);
		const vec_float4 mat6_2 = spu_madd(w6, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat6), mat5_2);
		const vec_float4 mat6_3 = spu_madd(w6, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat6), mat5_3);
		const vec_float4 mat7_1 = spu_madd(w7, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat7), mat6_1);
		const vec_float4 mat7_2 = spu_madd(w7, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat7), mat6_2);
		const vec_float4 mat7_3 = spu_madd(w7, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat7), mat6_3);
		M2_1 = spu_madd(w8, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat8), mat7_1);
		M2_2 = spu_madd(w8, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat8), mat7_2);
		M2_3 = spu_madd(w8, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat8), mat7_3);
		M2_4 = spu_shuffle(M2_2, M2_3, s_0Dd0);
		M2_4 = spu_shuffle(M2_4, M2_1, s_dBC0);
        M2_1 = spu_sel(k1_0f, M2_1, mask_XXX0);
		M2_2 = spu_sel((vec_float4)mask_XXX0, M2_2, mask_XXX0); 
		M2_3 = spu_sel((vec_float4)mask_XXX0, M2_3, mask_XXX0); 

		
		// Create final matrix M3
		const vec_float4 mat9_1  = spu_mul( w9,  (vec_float4)si_lqx(pMatRow0, (vec_char16)mat9 ));
		const vec_float4 mat9_2  = spu_mul( w9,  (vec_float4)si_lqx(pMatRow1, (vec_char16)mat9 ));
		const vec_float4 mat9_3  = spu_mul( w9,  (vec_float4)si_lqx(pMatRow2, (vec_char16)mat9 ));
		const vec_float4 mat10_1 = spu_madd(w10, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat10), mat9_1);
		const vec_float4 mat10_2 = spu_madd(w10, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat10), mat9_2);
		const vec_float4 mat10_3 = spu_madd(w10, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat10), mat9_3);
		const vec_float4 mat11_1 = spu_madd(w11, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat11), mat10_1);
		const vec_float4 mat11_2 = spu_madd(w11, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat11), mat10_2);
		const vec_float4 mat11_3 = spu_madd(w11, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat11), mat10_3);
		M3_1 = spu_madd(w12, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat12), mat11_1);
		M3_2 = spu_madd(w12, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat12), mat11_2);
		M3_3 = spu_madd(w12, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat12), mat11_3);
		M3_4 = spu_shuffle(M3_2, M3_3, s_0Dd0);
		M3_4 = spu_shuffle(M3_4, M3_1, s_dBC0);
        M3_1 = spu_sel(k1_0f, M3_1, mask_XXX0);
		M3_2 = spu_sel((vec_float4)mask_XXX0, M3_2, mask_XXX0); 
		M3_3 = spu_sel((vec_float4)mask_XXX0, M3_3, mask_XXX0); 

		
		// Create final matrix M4
		const vec_float4 mat13_1 = spu_mul( w13, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat13));
		const vec_float4 mat13_2 = spu_mul( w13, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat13));
		const vec_float4 mat13_3 = spu_mul( w13, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat13));
		const vec_float4 mat14_1 = spu_madd(w14, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat14), mat13_1);
		const vec_float4 mat14_2 = spu_madd(w14, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat14), mat13_2);
		const vec_float4 mat14_3 = spu_madd(w14, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat14), mat13_3);
		const vec_float4 mat15_1 = spu_madd(w15, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat15), mat14_1);
		const vec_float4 mat15_2 = spu_madd(w15, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat15), mat14_2);
		const vec_float4 mat15_3 = spu_madd(w15, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat15), mat14_3);
		M4_1 = spu_madd(w16, (vec_float4)si_lqx(pMatRow0, (vec_char16)mat16), mat15_1);
		M4_2 = spu_madd(w16, (vec_float4)si_lqx(pMatRow1, (vec_char16)mat16), mat15_2);
		M4_3 = spu_madd(w16, (vec_float4)si_lqx(pMatRow2, (vec_char16)mat16), mat15_3);
		M4_4 = spu_shuffle(M4_2, M4_3, s_0Dd0);
		M4_4 = spu_shuffle(M4_4, M4_1, s_dBC0);
        M4_1 = spu_sel(k1_0f, M4_1, mask_XXX0);
		M4_2 = spu_sel((vec_float4)mask_XXX0, M4_2, mask_XXX0); 
		M4_3 = spu_sel((vec_float4)mask_XXX0, M4_3, mask_XXX0); 



        si_stqd((qword)spu_madd(pos1__, M1_3, spu_madd(pos1_, M1_2, spu_madd(pos1, M1_1, M1_4))), pSrc, 0x00);
        si_stqd((qword)spu_madd(pos2__, M2_3, spu_madd(pos2_, M2_2, spu_madd(pos2, M2_1, M2_4))), pSrc, 0x10);
        si_stqd((qword)spu_madd(pos3__, M3_3, spu_madd(pos3_, M3_2, spu_madd(pos3, M3_1, M3_4))), pSrc, 0x20);
        si_stqd((qword)spu_madd(pos4__, M4_3, spu_madd(pos4_, M4_2, spu_madd(pos4, M4_1, M4_4))), pSrc, 0x30);

		pSrc = (qword)spu_add((vec_int4)pSrc, 0x40);
		
		pos1 = (vec_float4)si_lqd(pSrc, 0x00);
		pos2 = (vec_float4)si_lqd(pSrc, 0x10);
		pos3 = (vec_float4)si_lqd(pSrc, 0x20);
		pos4 = (vec_float4)si_lqd(pSrc, 0x30);
		pos1_ = spu_shuffle(pos1, pos1, s_BCAD);
		pos2_ = spu_shuffle(pos2, pos2, s_BCAD);
		pos3_ = spu_shuffle(pos3, pos3, s_BCAD);
		pos4_ = spu_shuffle(pos4, pos4, s_BCAD);
		pos1__ = spu_shuffle(pos1, pos1, s_CABD);
		pos2__ = spu_shuffle(pos2, pos2, s_CABD);
		pos3__ = spu_shuffle(pos3, pos3, s_CABD);
		pos4__ = spu_shuffle(pos4, pos4, s_CABD);

		--iterCount;
		if(__builtin_expect(iterCount == 0, false))
			break;
	}
	//asm volatile ("ESLB:");
}

