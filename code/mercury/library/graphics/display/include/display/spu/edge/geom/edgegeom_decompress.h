/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEGEOM_DECOMPRESS_H
#define EDGEGEOM_DECOMPRESS_H

/* from edgegeom_masks.cpp: */
/* all shuffle masks used in this code MUST be local variables. No external links allowed! */

// We expect many of these variable declarations to be unused and
// compiled out of existence; defining them this way will suppress the
// compiler's spurious warning messages.
#if !defined(EDGE_VAR)
#define EDGE_VAR __attribute__(__unused__)
#endif

//
// Processing macros
//

#define EDGE_GEOM_DECOMPRESS_STORE(attrNum)										\
	do {																\
		if (forceWToOne##attrNum)										\
		{																\
			attr0##attrNum = (vec_char16)spu_sel((vec_uint4)attr0##attrNum, kOne, mask_000F); \
			attr1##attrNum = (vec_char16)spu_sel((vec_uint4)attr1##attrNum, kOne, mask_000F); \
		}																\
		if (isBlendShapeStream)											\
		{																\
			const vec_float4 oldAttr0 = (vec_float4)si_lqx(pAttr0##attrNum, uniformTableOffsets[loopCycleCount##attrNum]); \
			const vec_float4 oldAttr1 = (vec_float4)si_lqx(pAttr1##attrNum, uniformTableOffsets[loopCycleCount##attrNum]); \
			attr0##attrNum = (vec_char16)spu_madd((vec_float4)attr0##attrNum, blendAlpha, oldAttr0); \
			attr1##attrNum = (vec_char16)spu_madd((vec_float4)attr1##attrNum, blendAlpha, oldAttr1); \
		}																\
		si_stqx(attr0##attrNum, pAttr0##attrNum, uniformTableOffsets[loopCycleCount##attrNum]); \
		si_stqx(attr1##attrNum, pAttr1##attrNum, uniformTableOffsets[loopCycleCount##attrNum]); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum)										\
	do {																\
		inAttr0##attrNum = useFastExtract0##attrNum ? inQuads[inQuad0##attrNum##0] : \
			spu_shuffle(inQuads[inQuad0##attrNum##0], inQuads[inQuad0##attrNum##1], shuf0##attrNum); \
		inAttr1##attrNum = useFastExtract1##attrNum ? inQuads[inQuad1##attrNum##0] : \
			spu_shuffle(inQuads[inQuad1##attrNum##0], inQuads[inQuad1##attrNum##1], shuf1##attrNum); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_FINALIZE_F32(attrNum)                        \
    do{                                                     \
    } while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_F16(attrNum)                        \
    do{                                                     \
    } while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_U8(attrNum)                         \
    do{                                                     \
    } while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_I16(attrNum)                        \
    do{                                                     \
    } while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_U8N(attrNum)                         \
    do{                                                     \
    } while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_I16N(attrNum)                        \
    do{                                                     \
    } while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(attrNum)                  \
    do{                                                     \
    } while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_FIXED_POINT(attrNum)                \
    do{														   \
    } while(0)                                              

#define EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(attrNum)					 \
    do{															 \
        EDGE_GEOM_DECOMPRESS_C_UNIT_VECTOR(attrNum);                 \
        EDGE_GEOM_DECOMPRESS_STORE(attrNum);							 \
    } while(0)                                      

#define EDGE_GEOM_DECOMPRESS_PROCESS_F32(attrNum)				\
	do {													\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);						\
		attr0##attrNum = inAttr0##attrNum;					\
		attr1##attrNum = inAttr1##attrNum;					\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_F16(attrNum)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);									\
		const vec_int4 fDataHole0 = spu_andc(spu_rlmaska((vec_int4)inAttr0##attrNum, -3), expHoleMask); \
		const vec_int4 fDataHole1 = spu_andc(spu_rlmaska((vec_int4)inAttr1##attrNum, -3), expHoleMask); \
		const vec_int4 expBias0 = spu_and((vec_int4)spu_cmpgt(spu_andc((vec_uint4)inAttr0##attrNum, signMask), 0), expRebias); \
		const vec_int4 expBias1 = spu_and((vec_int4)spu_cmpgt(spu_andc((vec_uint4)inAttr1##attrNum, signMask), 0), expRebias); \
		attr0##attrNum = (vec_char16)spu_add(fDataHole0, expBias0);		\
		attr1##attrNum = (vec_char16)spu_add(fDataHole1, expBias1); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_U8(attrNum)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);									\
		attr0##attrNum = (vec_char16)spu_convtf((vec_uint4)inAttr0##attrNum, 16); \
		attr1##attrNum = (vec_char16)spu_convtf((vec_uint4)inAttr1##attrNum, 16); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_I16(attrNum)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);									\
		attr0##attrNum = (vec_char16)spu_convtf((vec_int4)inAttr0##attrNum, 16); \
		attr1##attrNum = (vec_char16)spu_convtf((vec_int4)inAttr1##attrNum, 16); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_U8N(attrNum)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);									\
		attr0##attrNum = (vec_char16)spu_madd(spu_convtf((vec_uint4)inAttr0##attrNum, 16), u8nScale, u8nBias); \
		attr1##attrNum = (vec_char16)spu_madd(spu_convtf((vec_uint4)inAttr1##attrNum, 16), u8nScale, u8nBias); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_I16N(attrNum)							\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);									\
		attr0##attrNum = (vec_char16)spu_madd(spu_convtf((vec_int4)inAttr0##attrNum, 16), i16nScale, i16nBias); \
		attr1##attrNum = (vec_char16)spu_madd(spu_convtf((vec_int4)inAttr1##attrNum, 16), i16nScale, i16nBias); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_X11Y11Z10N(attrNum)					\
	do {																\
		EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);									\
		const vec_float4 fData0 = spu_convtf(spu_and(spu_sl((vec_int4)inAttr0##attrNum, shiftValues), bitMask), 31); \
		const vec_float4 fData1 = spu_convtf(spu_and(spu_sl((vec_int4)inAttr1##attrNum, shiftValues), bitMask), 31); \
		attr0##attrNum = (vec_char16)spu_madd(fData0, x11y11z10Scale, x11y11z10Bias); \
		attr1##attrNum = (vec_char16)spu_madd(fData1, x11y11z10Scale, x11y11z10Bias); \
	} while(0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_FIXED_POINT(attrNum)   \
    do{                                                                 \
        EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);                                    \
        attr0##attrNum = si_csflt((vec_char16)spu_add(spu_and(fixedPointMask##attrNum, spu_rlmask((vec_uint4)inAttr0##attrNum, fixedPointRotAmount##attrNum)),fixedPointOffset##attrNum),maxPrecision##attrNum);  \
        attr1##attrNum = si_csflt((vec_char16)spu_add(spu_and(fixedPointMask##attrNum, spu_rlmask((vec_uint4)inAttr1##attrNum, fixedPointRotAmount##attrNum)),fixedPointOffset##attrNum),maxPrecision##attrNum);  \
    } while(0)

#define EDGE_GEOM_DECOMPRESS_A_UNIT_VECTOR(attrNum)					\
    do {                                                                \
        EDGE_GEOM_DECOMPRESS_EXTRACT(attrNum);                                    \
        iData0##attrNum = spu_sel(formatMask, spu_rlmask((vec_uint4)inAttr0##attrNum,rotValues), formatSelect); \
        iData1##attrNum = spu_sel(formatMask, spu_rlmask((vec_uint4)inAttr1##attrNum,rotValues), formatSelect); \
        fData0##attrNum = spu_madd((vec_float4)spu_shuffle(iData0##attrNum, iData1##attrNum, s_BCbc),fSwUnitVectorScale, fSwUnitVectorBias); \
        fData0Sq##attrNum = spu_mul(fData0##attrNum, fData0##attrNum);                             \
    } while(0)                                                          


#define EDGE_GEOM_DECOMPRESS_B_UNIT_VECTOR(attrNum)					\
    do {                                                                \
        fToSqrt##attrNum = spu_sub(spu_splats(1.0f),spu_add(fData0Sq##attrNum, spu_slqwbyte(fData0Sq##attrNum, 4))); \
        complexMask0##attrNum = (vec_uchar16)si_lqx(pMaskTable,(vec_char16)iData0##attrNum); \
        complexMask1##attrNum = (vec_uchar16)si_lqx(pMaskTable,(vec_char16)iData1##attrNum); \
        fSqrt##attrNum = spu_mul(fToSqrt##attrNum,spu_rsqrte(fToSqrt##attrNum));					\
        fData0Holder##attrNum = fData0##attrNum;                                          \
    } while (0)                                                         

#define EDGE_GEOM_DECOMPRESS_C_UNIT_VECTOR(attrNum)					\
    do {                                                                \
        s_output0##attrNum = (vec_uchar16)spu_sel(outputMask, (vec_uint4)complexMask0##attrNum, outputSelect); \
        s_output1##attrNum = (vec_uchar16)spu_add(spu_sel(outputMask, (vec_uint4)complexMask1##attrNum, outputSelect), shuffleAdjust); \
        attr0##attrNum = (vec_char16)spu_shuffle(spu_sel((vec_uint4)fSqrt##attrNum, (vec_uint4)complexMask0##attrNum, swSelect), (vec_uint4)fData0Holder##attrNum, s_output0##attrNum);	\
        attr1##attrNum = (vec_char16)spu_shuffle(spu_sel((vec_uint4)fSqrt##attrNum, (vec_uint4)complexMask1##attrNum, swSelect), (vec_uint4)fData0Holder##attrNum, s_output1##attrNum);	\
    } while (0)

#define EDGE_GEOM_DECOMPRESS_PROCESS_UNIT_VECTOR(attrNum)				\
    do {                                                                \
        EDGE_GEOM_DECOMPRESS_A_UNIT_VECTOR(attrNum);					\
        EDGE_GEOM_DECOMPRESS_B_UNIT_VECTOR(attrNum);					\
    } while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_START()					\
	do {} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_END()											\
    do {																\
        uniformTableOffsets[4] = uniformTableOffsets[3];				\
        uniformTableOffsets[3] = (vec_char16)spu_add((vec_int4)uniformTableOffsets[3], 0x20); \
        EDGE_GEOM_DECOMPRESS_LOAD_COMMON();										\
    } while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_F32(attrNum)			\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrNum);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_F32(attrNum);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_F16(attrNum)			\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrNum);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_F16(attrNum);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_U8(attrNum)				\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrNum);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_U8(attrNum);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_I16(attrNum)			\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrNum);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_I16(attrNum);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_U8N(attrNum)				\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrNum);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_U8N(attrNum);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_I16N(attrNum)			\
	do {										\
		EDGE_GEOM_DECOMPRESS_STORE(attrNum);			\
		EDGE_GEOM_DECOMPRESS_PROCESS_I16N(attrNum);		\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(attrNum)			\
	do {											\
		EDGE_GEOM_DECOMPRESS_STORE(attrNum);				\
		EDGE_GEOM_DECOMPRESS_PROCESS_X11Y11Z10N(attrNum);	\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_FIXED_POINT(attrNum)        \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_STORE(attrNum);            \
        EDGE_GEOM_DECOMPRESS_PROCESS_FIXED_POINT(attrNum); \
    } while(0)

#define EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(attrNum)          \
    do {                                            \
        EDGE_GEOM_DECOMPRESS_C_UNIT_VECTOR(attrNum);         \
        EDGE_GEOM_DECOMPRESS_STORE(attrNum);               \
        EDGE_GEOM_DECOMPRESS_B_UNIT_VECTOR(attrNum);       \
        EDGE_GEOM_DECOMPRESS_A_UNIT_VECTOR(attrNum);     \
    } while (0) 

#define EDGE_GEOM_DECOMPRESS_PRIME_F32(attrNum)               \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_PROCESS_F32(attrNum);         \
    } while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_F16(attrNum)               \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_PROCESS_F16(attrNum);         \
    } while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_U8(attrNum)               \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_PROCESS_U8(attrNum);         \
    } while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_I16(attrNum)               \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_PROCESS_I16(attrNum);         \
    } while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_U8N(attrNum)               \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_PROCESS_U8N(attrNum);         \
    } while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_I16N(attrNum)               \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_PROCESS_I16N(attrNum);         \
    } while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_X11Y11Z10N(attrNum)               \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_PROCESS_X11Y11Z10N(attrNum);         \
    } while (0)

#define EDGE_GEOM_DECOMPRESS_PRIME_FIXED_POINT(attrNum)       \
    do{                                             \
        EDGE_GEOM_DECOMPRESS_PROCESS_FIXED_POINT(attrNum);  \
    } while(0)

#define EDGE_GEOM_DECOMPRESS_PRIME_UNIT_VECTOR(attrNum)          \
    do {                                            \
        EDGE_GEOM_DECOMPRESS_A_UNIT_VECTOR(attrNum);     \
    } while (0) 

#define EDGE_GEOM_DECOMPRESS_LOAD_COMMON()								\
	do {																\
		if (kStrideTimes2 % 16 == 0)									\
		{																\
			switch(kNumQuads)											\
			{															\
			case 16:													\
				inQuads[15] = si_lqd(si_from_ptr(pInput), 0xF0);		\
				/* fall through */										\
			case 15:													\
				inQuads[14] = si_lqd(si_from_ptr(pInput), 0xE0);		\
				/* fall through */										\
			case 14:													\
				inQuads[13] = si_lqd(si_from_ptr(pInput), 0xD0);		\
				/* fall through */										\
			case 13:													\
				inQuads[12] = si_lqd(si_from_ptr(pInput), 0xC0);		\
				/* fall through */										\
			case 12:													\
				inQuads[11] = si_lqd(si_from_ptr(pInput), 0xB0);		\
				/* fall through */										\
			case 11:													\
				inQuads[10] = si_lqd(si_from_ptr(pInput), 0xA0);		\
				/* fall through */										\
			case 10:													\
				inQuads[9] = si_lqd(si_from_ptr(pInput), 0x90);			\
				/* fall through */										\
			case 9:														\
				inQuads[8] = si_lqd(si_from_ptr(pInput), 0x80);			\
				/* fall through */										\
			case 8:														\
				inQuads[7] = si_lqd(si_from_ptr(pInput), 0x70);			\
				/* fall through */										\
			case 7:														\
				inQuads[6] = si_lqd(si_from_ptr(pInput), 0x60);			\
				/* fall through */										\
			case 6:														\
				inQuads[5] = si_lqd(si_from_ptr(pInput), 0x50);			\
				/* fall through */										\
			case 5:														\
				inQuads[4] = si_lqd(si_from_ptr(pInput), 0x40);			\
				/* fall through */										\
			case 4:														\
				inQuads[3] = si_lqd(si_from_ptr(pInput), 0x30);			\
				/* fall through */										\
			case 3:														\
				inQuads[2] = si_lqd(si_from_ptr(pInput), 0x20);			\
				/* fall through */										\
			case 2:														\
				inQuads[1] = si_lqd(si_from_ptr(pInput), 0x10);			\
				/* fall through */										\
			case 1:														\
				inQuads[0] = si_lqd(si_from_ptr(pInput), 0x00);			\
				/* fall through */										\
			}															\
		}																\
		else															\
		{																\
			vec_char16 inTemp[kNumQuads+1];								\
			const vec_char16 quadOff = spu_and((vec_char16)si_from_ptr(pInput), 0x00000000F); \
			const vec_uchar16 inShuf = (vec_uchar16)spu_add((vec_int4)s_ABCD, (vec_int4)spu_shuffle(quadOff, quadOff, s_DDDDDDDDDDDDDDDD));	\
			switch(kNumQuads)											\
			{															\
			case 16:													\
				inTemp[ 16] = si_lqd(si_from_ptr(pInput), 0x100);		\
				inTemp[ 15] = si_lqd(si_from_ptr(pInput), 0xF0);		\
				inQuads[15] = spu_shuffle(inTemp[15], inTemp[16], inShuf); \
				/* fall through */										\
			case 15:													\
				inTemp[ 15] = si_lqd(si_from_ptr(pInput), 0xF0);		\
				inTemp[ 14] = si_lqd(si_from_ptr(pInput), 0xE0);		\
				inQuads[14] = spu_shuffle(inTemp[14], inTemp[15], inShuf); \
				/* fall through */										\
			case 14:													\
				inTemp[ 14] = si_lqd(si_from_ptr(pInput), 0xE0);		\
				inTemp[ 13] = si_lqd(si_from_ptr(pInput), 0xD0);		\
				inQuads[13] = spu_shuffle(inTemp[13], inTemp[14], inShuf); \
				/* fall through */										\
			case 13:													\
				inTemp[ 13] = si_lqd(si_from_ptr(pInput), 0xD0);		\
				inTemp[ 12] = si_lqd(si_from_ptr(pInput), 0xC0);		\
				inQuads[12] = spu_shuffle(inTemp[12], inTemp[13], inShuf); \
				/* fall through */										\
			case 12:													\
				inTemp[ 12] = si_lqd(si_from_ptr(pInput), 0xC0);		\
				inTemp[ 11] = si_lqd(si_from_ptr(pInput), 0xB0);		\
				inQuads[11] = spu_shuffle(inTemp[11], inTemp[12], inShuf); \
				/* fall through */										\
			case 11:													\
				inTemp[ 11] = si_lqd(si_from_ptr(pInput), 0xB0);		\
				inTemp[ 10] = si_lqd(si_from_ptr(pInput), 0xA0);		\
				inQuads[10] = spu_shuffle(inTemp[10], inTemp[11], inShuf); \
				/* fall through */										\
			case 10:													\
				inTemp[10] = si_lqd(si_from_ptr(pInput), 0xA0);			\
				inTemp[ 9] = si_lqd(si_from_ptr(pInput), 0x90);			\
				inQuads[9] = spu_shuffle(inTemp[9], inTemp[10], inShuf); \
				/* fall through */										\
			case 9:														\
				inTemp[ 9] = si_lqd(si_from_ptr(pInput), 0x90);			\
				inTemp[ 8] = si_lqd(si_from_ptr(pInput), 0x80);			\
				inQuads[8] = spu_shuffle(inTemp[8], inTemp[9], inShuf);	\
				/* fall through */										\
			case 8:														\
				inTemp[ 8] = si_lqd(si_from_ptr(pInput), 0x80);			\
				inTemp[ 7] = si_lqd(si_from_ptr(pInput), 0x70);			\
				inQuads[7] = spu_shuffle(inTemp[7], inTemp[8], inShuf);	\
				/* fall through */										\
			case 7:														\
				inTemp[ 7] = si_lqd(si_from_ptr(pInput), 0x70);			\
				inTemp[ 6] = si_lqd(si_from_ptr(pInput), 0x60);			\
				inQuads[6] = spu_shuffle(inTemp[6], inTemp[7], inShuf);	\
				/* fall through */										\
			case 6:														\
				inTemp[ 6] = si_lqd(si_from_ptr(pInput), 0x60);			\
				inTemp[ 5] = si_lqd(si_from_ptr(pInput), 0x50);			\
				inQuads[5] = spu_shuffle(inTemp[5], inTemp[6], inShuf);	\
				/* fall through */										\
			case 5:														\
				inTemp[ 5] = si_lqd(si_from_ptr(pInput), 0x50);			\
				inTemp[ 4] = si_lqd(si_from_ptr(pInput), 0x40);			\
				inQuads[4] = spu_shuffle(inTemp[4], inTemp[5], inShuf);	\
				/* fall through */										\
			case 4:														\
				inTemp[ 4] = si_lqd(si_from_ptr(pInput), 0x40);			\
				inTemp[ 3] = si_lqd(si_from_ptr(pInput), 0x30);			\
				inQuads[3] = spu_shuffle(inTemp[3], inTemp[4], inShuf);	\
				/* fall through */										\
			case 3:														\
				inTemp[ 3] = si_lqd(si_from_ptr(pInput), 0x30);			\
				inTemp[ 2] = si_lqd(si_from_ptr(pInput), 0x20);			\
				inQuads[2] = spu_shuffle(inTemp[2], inTemp[3], inShuf);	\
				/* fall through */										\
			case 2:														\
				inTemp[ 2] = si_lqd(si_from_ptr(pInput), 0x20);			\
				inTemp[ 1] = si_lqd(si_from_ptr(pInput), 0x10);			\
				inQuads[1] = spu_shuffle(inTemp[1], inTemp[2], inShuf);	\
				/* fall through */										\
			case 1:														\
				inTemp[ 1] = si_lqd(si_from_ptr(pInput), 0x10);			\
				inTemp[ 0] = si_lqd(si_from_ptr(pInput), 0x00);			\
				inQuads[0] = spu_shuffle(inTemp[0], inTemp[1], inShuf);	\
				break;													\
			default:													\
				break;													\
			}															\
		}																\
		pInput += kStrideTimes2;										\
	} while(0)

#define EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(attrType, attrNum, attrUsage, attrOffset, attrComponentCount) \
	/* TODO: this code quietly assumes that positionTable and friends are \
	 * defined in the current scope.  This is extremely fragile.		\
	 */																	\
	if (!isBlendShapeStream)											\
	{																	\
		switch(attrUsage)												\
		{																\
		case EDGE_GEOM_ATTRIBUTE_USAGE_POSITION:						\
			*ppPositionTable = (void*)pUniforms[attrNum];				\
			break;														\
		case EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL:							\
			*ppNormalTable = (void*)pUniforms[attrNum];					\
			break;														\
		case EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT:							\
			*ppTangentTable = (void*)pUniforms[attrNum];				\
			break;														\
		case EDGE_GEOM_ATTRIBUTE_USAGE_BINORMAL:						\
			*ppBinormalTable = (void*)pUniforms[attrNum];				\
			break;														\
		case EDGE_GEOM_ATTRIBUTE_USAGE_GENERIC:							\
		default:														\
			break;														\
		}																\
	}																	\
	const bool forceWToOne##attrNum = (attrUsage == EDGE_GEOM_ATTRIBUTE_USAGE_POSITION && attrComponentCount < 4); \
	const int offset0##attrNum = attrOffset;							\
	const bool attribIsAligned0##attrNum = (offset0##attrNum % 16 == 0); \
	const bool useFastExtract0##attrNum = isF32##attrNum && attribIsAligned0##attrNum; \
	const int inQuad0##attrNum##0 = offset0##attrNum / 16;				\
	const int inQuad0##attrNum##1 = (inQuad0##attrNum##0 + 1 >= kNumQuads) ? kNumQuads-1 : inQuad0##attrNum##0 + 1; \
	vec_uchar16 shuf0##attrNum;											\
	if (useFastExtract0##attrNum)										\
	{																	\
		/* No shuffle masks necessary */								\
	}																	\
	else if (!attribIsAligned0##attrNum)								\
	{																	\
		const vec_int4 quadOff = (vec_int4)si_from_int(offset0##attrNum % 16); \
		const vec_uchar16 temp = (vec_uchar16)spu_add((vec_int4)s_ABCD, spu_shuffle(quadOff, quadOff, s_DDDDDDDDDDDDDDDD));	\
		shuf0##attrNum = spu_shuffle(temp, temp, shufSeed##attrNum);	\
	}																	\
	else																\
	{																	\
		shuf0##attrNum = spu_shuffle(s_ABCD, s_ABCD, shufSeed##attrNum); \
	}																	\
    if (doFixedPointSetW##attrNum) {                                        \
        shuf0##attrNum = (vec_uchar16)spu_sel((vec_uint4)shuf0##attrNum, spu_splats(0xc0c0c0c0), mask_000F);    \
    }                                                                   \
																		\
	const int offset1##attrNum = attrOffset + kStride;					\
	const bool attribIsAligned1##attrNum = (offset1##attrNum % 16 == 0); \
	const bool useFastExtract1##attrNum = isF32##attrNum && attribIsAligned1##attrNum; \
	const int inQuad1##attrNum##0 = offset1##attrNum / 16;				\
	const int inQuad1##attrNum##1 = (inQuad1##attrNum##0 + 1 >= kNumQuads) ? kNumQuads-1 : inQuad1##attrNum##0 + 1; \
	vec_uchar16 shuf1##attrNum;											\
	if (useFastExtract1##attrNum)										\
	{																	\
		/* No shuffle masks necessary */								\
	}																	\
	else if (!attribIsAligned1##attrNum)								\
	{																	\
		const vec_int4 quadOff = (vec_int4)si_from_int(offset1##attrNum % 16);		\
		const vec_uchar16 temp = (vec_uchar16)spu_add((vec_int4)s_ABCD, spu_shuffle(quadOff, quadOff, s_DDDDDDDDDDDDDDDD));	\
		shuf1##attrNum = spu_shuffle(temp, temp, shufSeed##attrNum);	\
	}																	\
	else																\
	{																	\
		shuf1##attrNum = spu_shuffle(s_ABCD, s_ABCD, shufSeed##attrNum); \
	}																	\
    if (doFixedPointSetW##attrNum) {                                        \
        shuf1##attrNum = (vec_uchar16)spu_sel((vec_uint4)shuf1##attrNum, spu_splats(0xc0c0c0c0), mask_000F);    \
    }                                                                   \
																		\
	vec_char16 attr0##attrNum;											\
	vec_char16 attr1##attrNum;											\
	vec_char16 inAttr0##attrNum;										\
	vec_char16 inAttr1##attrNum;										\
	const qword pAttr0##attrNum = si_from_ptr( (char*)(pUniforms[attrNum])+kVertCount*0x10); \
    const qword pAttr1##attrNum = si_from_ptr( (char*)(pUniforms[attrNum])+kVertCount*0x10+0x10) ;	\
    EDGE_GEOM_DECOMPRESS_PRIME_##attrType(attrNum);
	
#define EDGE_GEOM_DECOMPRESS_INIT_F32(attrNum, attrUsage, attrOffset, attrComponentCount)			\
	const vec_uchar16 shufSeed##attrNum = s_ABCD;						\
	const bool isF32##attrNum = true;									\
    const bool doFixedPointSetW##attrNum = false;                           \
	const uint32_t loopCycleCount##attrNum = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(F32, attrNum, attrUsage, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_F16(attrNum, attrUsage, attrOffset, attrComponentCount)			\
	const vec_uchar16 shufSeed##attrNum = s_A0B0C0D0;					\
	const bool isF32##attrNum = false;									\
    const bool doFixedPointSetW##attrNum = false;                           \
	const uint32_t loopCycleCount##attrNum = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(F16, attrNum, attrUsage, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_U8(attrNum, attrUsage, attrOffset, attrComponentCount)				\
	const vec_uchar16 shufSeed##attrNum = s_0A000B000C000D00;			\
	const bool isF32##attrNum = false;									\
    const bool doFixedPointSetW##attrNum = false;                           \
	const uint32_t loopCycleCount##attrNum = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(U8, attrNum, attrUsage, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_I16(attrNum, attrUsage, attrOffset, attrComponentCount)						\
	const vec_uchar16 shufSeed##attrNum = s_A0B0C0D0;					\
	const bool isF32##attrNum = false;									\
    const bool doFixedPointSetW##attrNum = false;                           \
	const uint32_t loopCycleCount##attrNum = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(I16, attrNum, attrUsage, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_U8N(attrNum, attrUsage, attrOffset, attrComponentCount)							\
	const vec_uchar16 shufSeed##attrNum = s_0A000B000C000D00;			\
	const bool isF32##attrNum = false;									\
    const bool doFixedPointSetW##attrNum = false;                           \
	const uint32_t loopCycleCount##attrNum = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(U8N, attrNum, attrUsage, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_I16N(attrNum, attrUsage, attrOffset, attrComponentCount)						\
	const vec_uchar16 shufSeed##attrNum = s_A0B0C0D0;					\
	const bool isF32##attrNum = false;									\
    const bool doFixedPointSetW##attrNum = false;                           \
	const uint32_t loopCycleCount##attrNum = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(I16N, attrNum, attrUsage, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(attrNum, attrUsage, attrOffset, attrComponentCount)					\
	const vec_uchar16 shufSeed##attrNum = s_CD00BC00AB000000;			\
	const bool isF32##attrNum = false;									\
    const bool doFixedPointSetW##attrNum = false;                           \
	const uint32_t loopCycleCount##attrNum = 3;								\
	EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(X11Y11Z10N, attrNum, attrUsage, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_FIXED_POINT(attrNum, attrUsage, attrOffset, attrComponentCount, numIntBits0, numPrecisionBits0, numIntBits1, numPrecisionBits1, numIntBits2, numPrecisionBits2, numIntBits3, numPrecisionBits3)  \
    const bool hasX##attrNum = (numIntBits0 + numPrecisionBits0) > 0;							\
    const bool hasY##attrNum = (numIntBits1 + numPrecisionBits1) > 0;							\
    const bool hasZ##attrNum = (numIntBits2 + numPrecisionBits2) > 0;							\
    const bool hasW##attrNum = (numIntBits3 + numPrecisionBits3) > 0;							\
    const vec_uchar16 shufSeedOffsetTmp##attrNum =                      \
        {   (hasX##attrNum ? 0 : 0xE0), (hasX##attrNum ? 0 : 0xE0), (hasX##attrNum ? 0 : 0xE0), (hasX##attrNum ? 0 : 0xE0), \
            (hasY##attrNum ? (numIntBits0 + numPrecisionBits0)/8 : 0xE0), (hasY##attrNum ? (numIntBits0 + numPrecisionBits0)/8 : 0xE0), (hasY##attrNum ? (numIntBits0 + numPrecisionBits0)/8 : 0xE0), (hasY##attrNum ? (numIntBits0 + numPrecisionBits0)/8 : 0xE0), \
            (hasZ##attrNum ? ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1))/8 : 0xE0), (hasZ##attrNum ? ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1))/8 : 0xE0), (hasZ##attrNum ? ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1))/8 : 0xE0),(hasZ##attrNum ? ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1))/8 : 0xE0), \
            (hasW##attrNum ? ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1)+(numIntBits2 + numPrecisionBits2))/8 : 0xc0), (hasW##attrNum ? ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1)+(numIntBits2 + numPrecisionBits2))/8 : 0xc0), (hasW##attrNum ? ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1)+(numIntBits2 + numPrecisionBits2))/8 : 0xc0), (hasW##attrNum ? ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1)+(numIntBits2 + numPrecisionBits2))/8 : 0xc0)}; \
    const vec_uchar16 shufSeed##attrNum = (vec_uchar16)spu_add((vec_uint4)s_AAAA, (vec_uint4)shufSeedOffsetTmp##attrNum);    \
    const int maxPrecisionTmp0##attrNum = (numPrecisionBits0) < (numPrecisionBits1) ? (numPrecisionBits1) : (numPrecisionBits0); \
    const int maxPrecisionTmp1##attrNum = (numPrecisionBits2) < (numPrecisionBits3) ? (numPrecisionBits3) : (numPrecisionBits2); \
    const int maxPrecision##attrNum = maxPrecisionTmp0##attrNum < maxPrecisionTmp1##attrNum ? maxPrecisionTmp1##attrNum : maxPrecisionTmp0##attrNum;   \
    const vec_int4 fixedPointRotAmount##attrNum =                     \
        {                                                               \
			-(32 - (numIntBits0 + numPrecisionBits0) - (maxPrecision##attrNum - (numPrecisionBits0))), \
			-(32 - (numIntBits1 + numPrecisionBits1) - (numIntBits0 + numPrecisionBits0)%8 - (maxPrecision##attrNum - (numPrecisionBits1))), \
			-(32 - (numIntBits2 + numPrecisionBits2) - ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1))%8 - (maxPrecision##attrNum - (numPrecisionBits2))), \
			(hasW##attrNum ? (-(32 - (numIntBits3 + numPrecisionBits3) - ((numIntBits0 + numPrecisionBits0)+(numIntBits1 + numPrecisionBits1)+(numIntBits2 + numPrecisionBits2))%8 - (maxPrecision##attrNum - (numPrecisionBits3)))): 0)}; \
    const vec_int4 fixedPointMaskRotAmount##attrNum = {               \
		((numIntBits0 + numPrecisionBits0) + (maxPrecision##attrNum - (numPrecisionBits0))),	\
		((numIntBits1 + numPrecisionBits1) + (maxPrecision##attrNum - (numPrecisionBits1))),	\
		((numIntBits2 + numPrecisionBits2) + (maxPrecision##attrNum - (numPrecisionBits2))),	\
		(hasW##attrNum ? ((numIntBits3 + numPrecisionBits3) + (maxPrecision##attrNum - (numPrecisionBits3))) : 0)}; \
    const vec_uint4 fixedPointMaskBase##attrNum = (hasW##attrNum ? (vec_uint4)spu_splats(1) : (vec_uint4)spu_sel(spu_splats(1), spu_splats((1<<maxPrecision##attrNum)+1), mask_000F));\
    const vec_uint4 fixedPointMask##attrNum = spu_sub(spu_sl(fixedPointMaskBase##attrNum,(vec_uint4)fixedPointMaskRotAmount##attrNum), (vec_uint4)spu_splats(1));   \
    const bool isF32##attrNum = false;                                  \
    const bool doFixedPointSetW##attrNum = !hasW##attrNum;            \
	const uint32_t loopCycleCount##attrNum = 3;							\
    const EDGE_VAR vec_uint4 fixedPointOffset##attrNum = (vec_uint4){pFixedOffsets[fixedOffsetNum*4+0], pFixedOffsets[fixedOffsetNum*4+1], pFixedOffsets[fixedOffsetNum*4+2], pFixedOffsets[fixedOffsetNum*4+3]}; \
    ++fixedOffsetNum;													\
    EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(FIXED_POINT, attrNum, attrUsage, attrOffset, attrComponentCount);

#define EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(attrNum, attrUsage, attrOffset, attrComponentCount)	\
    vec_float4 fToSqrt##attrNum            =  (vec_float4)spu_splats(0); \
    vec_uchar16 complexMask0##attrNum      = (vec_uchar16)spu_splats(0); \
    vec_uchar16 complexMask1##attrNum      = (vec_uchar16)spu_splats(0); \
    vec_float4 fData0##attrNum             =  (vec_float4)spu_splats(0); \
    vec_float4 fData0Holder##attrNum       =  (vec_float4)spu_splats(0); \
    vec_float4 fData0Sq##attrNum           =  (vec_float4)spu_splats(0); \
    vec_uint4 iData0##attrNum              =   (vec_uint4)spu_splats(0); \
    vec_uint4 iData1##attrNum              =   (vec_uint4)spu_splats(0); \
    vec_uchar16 s_output0##attrNum         = (vec_uchar16)spu_splats(0); \
    vec_uchar16 s_output1##attrNum         = (vec_uchar16)spu_splats(0); \
    vec_float4 fSqrt##attrNum              =  (vec_float4)spu_splats(0); \
    const vec_uchar16 shufSeed##attrNum    = s_00C000AB00BC0000;		\
	const bool isF32##attrNum              = false;						\
    const bool doFixedPointSetW##attrNum = false;						\
	const uint32_t loopCycleCount##attrNum = 4;							\
    EDGE_GEOM_DECOMPRESS_INIT_ATTRIBUTE_COMMON(UNIT_VECTOR, attrNum, attrUsage, attrOffset, attrComponentCount);      

#define EDGE_GEOM_DECOMPRESS_INIT_GLOBAL_COMMON(ctx, vertexes, vertCount, fixedOffsets)		\
	void ** const ppPositionTable = &ctx->positionTable;						\
	void ** const ppNormalTable   = &ctx->normalTable;							\
	void ** const ppTangentTable  = &ctx->tangentTable;						\
	void ** const ppBinormalTable = &ctx->binormalTable;						\
	float ** const pUniforms = (float **)ctx->uniformTables;									\
	const int kVertCount = ((vertCount)+1) & ~0x1;						\
	vec_char16 uniformTableOffsets[5] = {(vec_char16)si_from_int(0), (vec_char16)si_from_int(0), (vec_char16)si_from_int(0), (vec_char16)si_from_int(-kVertCount*0x10), (vec_char16)si_from_int(0) }; \
	char *pInput = (char*)(vertexes);									\
	const vec_uint4 kOne = (vec_uint4)spu_splats(1.0f);				\
    const EDGE_VAR vec_uint4 mask_000F           = (vec_uint4)spu_maskb(0x000F); \
	/* shuffle masks */													\
	const EDGE_VAR vec_uchar16 s_00C000AB00BC0000 = {0x80,0x80,0x02,0x80, 0x80,0x80,0x00,0x01, 0x80,0x80,0x01,0x02, 0x80,0x80,0x80,0x80};\
	const EDGE_VAR vec_uchar16 s_0A000B000C000D00 = {0xFF,0x00,0xFF,0xFF, 0xFF,0x01,0xFF,0xFF, 0xFF,0x02,0xFF,0xFF, 0xFF,0x03,0xFF,0xFF};\
	const EDGE_VAR vec_uchar16 s_A0B0C0D0         = {0x00,0x01,0xFF,0xFF, 0x02,0x03,0xFF,0xFF, 0x04,0x05,0xFF,0xFF, 0x06,0x07,0xFF,0xFF};\
	const EDGE_VAR vec_uchar16 s_BCbc             = {0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b};\
	const EDGE_VAR vec_uchar16 s_CD00BC00AB000000 = {0x02,0x03,0xFF,0xFF, 0x01,0x02,0xFF,0xFF, 0x00,0x01,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF};\
	const EDGE_VAR vec_uchar16 s_ABCD = (vec_uchar16)spu_and(si_cwd((vec_char16)spu_splats(0), 0), 0xF); \
	const EDGE_VAR vec_uchar16 s_DDDDDDDDDDDDDDDD = (vec_uchar16)spu_splats(0x03030303); \
	/* U8N variables */													\
	const EDGE_VAR vec_float4 u8nScale = (vec_float4)spu_splats(1.0f / 255.0f);	\
	const EDGE_VAR vec_float4 u8nBias  = (vec_float4)spu_splats(0.0f);	\
	/* I16N variables */													\
	const EDGE_VAR vec_float4 i16nScale = (vec_float4)spu_splats(2.0f / 65535.0f);	\
	const EDGE_VAR vec_float4 i16nBias  = (vec_float4)spu_splats(1.0f / 65535.0f);	\
	/* F16 variables */													\
	const EDGE_VAR vec_int4 expHoleMask = (vec_int4)spu_splats(0x70000000); \
	const EDGE_VAR vec_uint4 signMask   = (vec_uint4)spu_splats(0x80000000); \
	const EDGE_VAR vec_int4 expRebias   = (vec_int4)spu_splats(0x38000000); \
	/* X11Y11Z10N variables */											\
	const EDGE_VAR vec_int4 bitMask               =    (vec_int4){0xFFE00000, 0xFFE00000, 0xFFC00000, 0x00000000}; \
	const EDGE_VAR vec_uint4 shiftValues          =   (vec_uint4){5, 2, 0, 0}; \
	const EDGE_VAR vec_float4 x11y11z10Scale      =  (vec_float4)(vec_int4){0x3F801002, 0x3F801002, 0x3F802008, 0x00000000}; \
	const EDGE_VAR vec_float4 x11y11z10Bias       =  (vec_float4)(vec_int4){0x3A001002, 0x3A001002, 0x3A802008, 0x3F800000}; \
 	/* FIXED_POINT variables */											\
    const EDGE_VAR vec_uint4 s_AAAA = (vec_uint4)spu_splats(0x10203);	\
	/* UNIT_VECTOR variables */											\
    const EDGE_VAR vec_int4 rotValues            =   (vec_int4){-4, -6, -4, 0};	\
    const EDGE_VAR vec_uint4 formatSelect        =   (vec_uint4){0xf0, 0x3ff, 0x3ff, 0x0}; \
    const EDGE_VAR vec_uint4 formatMask          =   (vec_uint4){0x0, 0x3f800000, 0x3f800000, 0x0}; \
    const EDGE_VAR vec_float4 fSwUnitVectorScale =   (vec_float4)spu_splats(0x46352e7c); \
    const EDGE_VAR vec_float4 fSwUnitVectorBias  =   (vec_float4)spu_splats(0xc6353150); \
    const EDGE_VAR vec_uint4 outputMask          =   (vec_uint4){0x0, 0x0, 0x0, 0x0c0d0e0f}; \
    const EDGE_VAR vec_uint4 outputSelect        =   (vec_uint4){0x7fffffff, 0x7fffffff, 0x7fffffff, 0x0}; \
    const EDGE_VAR vec_uint4 swSelect            =   (vec_uint4){0x80000000, 0x80000000, 0x80000000, 0xffffffff}; \
    const EDGE_VAR vec_uint4 shuffleAdjust       =   (vec_uint4){0x08080808,0x08080808,0x08080808,0x0}; \
	const vec_uint4 unitVectorMaskTable[12] __attribute__((__aligned__(16))) = \
	{																	\
		{0x80010203, 0x10111213, 0x94151617, 0xbf800000},	\
		{0x00010203, 0x10111213,	0x14151617, 0xbf800000},	\
		{0x80010203, 0x10111213, 0x94151617, 0x3f800000},	\
		{0x00010203, 0x10111213, 0x14151617, 0x3f800000},	\
		{0x80111213, 0x00010203, 0x94151617, 0xbf800000},	\
		{0x10111213, 0x00010203, 0x14151617, 0xbf800000},	\
		{0x90111213, 0x00010203, 0x94151617, 0x3f800000},	\
		{0x10111213, 0x00010203, 0x14151617, 0x3f800000},	\
		{0x90111213, 0x14151617, 0x80010203, 0xbf800000},	\
		{0x10111213, 0x14151617, 0x00010203, 0xbf800000},	\
		{0x90111213, 0x14151617, 0x80010203, 0x3f800000},	\
		{0x10111213, 0x14151617, 0x00010203, 0x3f800000},	\
	};																	\
    const EDGE_VAR vec_char16 pMaskTable         = (vec_char16)si_from_ptr(unitVectorMaskTable); \
	EDGE_VAR uint32_t fixedOffsetNum = 0;								\
	EDGE_VAR uint32_t *pFixedOffsets = (uint32_t *)fixedOffsets;

#define EDGE_GEOM_DECOMPRESS_SET_STRIDE(stride) \
	const int kStride = stride;											\
	const int kStrideTimes2 = 2*kStride;								\
	const int kNumQuads = (kStrideTimes2+15) / 16;						\
	vec_char16 inQuads[kNumQuads];

#define EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(stride) \
	const bool isBlendShapeStream = false;								\
	const vec_float4 blendAlpha = spu_splats(0.f);						\
	EDGE_GEOM_DECOMPRESS_SET_STRIDE(stride);										\
    EDGE_GEOM_DECOMPRESS_LOAD_COMMON();

#define EDGE_GEOM_DECOMPRESS_INIT_BLEND_GLOBAL(stride, blendFactor) \
	const bool isBlendShapeStream = true;								\
	const vec_float4 blendAlpha = spu_splats((float)(blendFactor));		\
	EDGE_GEOM_DECOMPRESS_SET_STRIDE(stride);										\
    EDGE_GEOM_DECOMPRESS_LOAD_COMMON();

#define EDGE_GEOM_DECOMPRESS_LOOP_DONE() \
	(si_to_int(uniformTableOffsets[3]) == 0)

// This is a special-case macro that handles streams consisting of a
// single 3-element F32 attribute (such as shadow map position-only
// streams).  We can schedule this code roughly 2x faster if we handle
// it separately.
#define EDGE_GEOM_DECOMPRESS_SPECIALCASE_F32_3(ctx, vertexes, vertCount) \
	float ** const pUniforms = (float **)ctx->uniformTables;									\
	edgeGeomSetPositionUniformTable(ctx, (qword*)pUniforms[0]);				\
	const int kVertCount = ((vertCount)+1) & ~0x1;						\
	qword outOffset = si_from_int(-kVertCount*0x10);					\
																		\
	const qword pOutput0 = si_from_ptr((char*)(pUniforms[0]) + (kVertCount*0x10) + 0x00); \
	const qword pOutput1 = si_from_ptr((char*)(pUniforms[0]) + (kVertCount*0x10) + 0x10);\
	const qword pOutput2 = si_from_ptr((char*)(pUniforms[0]) + (kVertCount*0x10) + 0x20);\
	const qword pOutput3 = si_from_ptr((char*)(pUniforms[0]) + (kVertCount*0x10) + 0x30);\
	char *pInput = (char*)(vertexes);									\
	const vec_uchar16 s_ABCD = (vec_uchar16)spu_and(si_cwd((vec_char16)spu_splats(0), 0), 0xF); \
	const vec_uchar16 s_CDab = (vec_uchar16)spu_add((vec_int4)s_ABCD, spu_splats(0x08080808));\
	const vec_uchar16 s_Dabc = (vec_uchar16)spu_add((vec_int4)s_ABCD, spu_splats(0x0c0c0c0c));\
																		\
	qword inQuad0 = si_lqd(si_from_ptr(pInput), 0x00);					\
	qword inQuad1 = si_lqd(si_from_ptr(pInput), 0x10);					\
	qword inQuad2 = si_lqd(si_from_ptr(pInput), 0x20);					\
	pInput += 0x30;														\
	qword nextOffset;													\
	do																	\
	{																	\
		const qword outQuad0 = inQuad0;									\
		const qword outQuad1 = spu_shuffle(inQuad0, inQuad1, s_Dabc);	\
		const qword outQuad2 = spu_shuffle(inQuad1, inQuad2, s_CDab);	\
		const qword outQuad3 = spu_slqwbyte(inQuad2, 4);				\
		inQuad0 = si_lqd(si_from_ptr(pInput), 0x00);					\
		inQuad1 = si_lqd(si_from_ptr(pInput), 0x10);					\
		inQuad2 = si_lqd(si_from_ptr(pInput), 0x20);					\
		pInput += 0x30;													\
		nextOffset = (qword)spu_add((vec_int4)outOffset, 0x40);			\
		si_stqx(outQuad0, pOutput0, outOffset);							\
		si_stqx(outQuad1, pOutput1, outOffset);							\
		si_stqx(outQuad2, pOutput2, outOffset);							\
		si_stqx(outQuad3, pOutput3, outOffset);							\
		outOffset = nextOffset;											\
	} while(si_to_int(nextOffset) != 0);								\
	



#endif // EDGEGEOM_DECOMPRESS_H
