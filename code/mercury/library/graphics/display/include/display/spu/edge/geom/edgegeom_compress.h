/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEGEOM_COMPRESS_H
#define EDGEGEOM_COMPRESS_H

/* from edgegeom_masks.cpp: */
/* all shuffle masks used in this code MUST be local variables. No external links allowed! */

// We expect many of these variable declarations to be unused and
// compiled out of existence; defining them this way will suppress the
// compiler issuing spurious warnings.
#if !defined(EDGE_VAR)
#define EDGE_VAR __attribute__(__unused__)
#endif

// The best way to learn to use this code is by example: look in
// edgegeom_compress.cpp for working examples of compression code built
// from these macros.

//
// Processing macros
//

#define EDGE_GEOM_COMPRESS_LOAD(attrNum)											\
	do {																\
		attr0##attrNum = si_lqd(pAttr##attrNum, 0x00);					\
		attr1##attrNum = si_lqd(pAttr##attrNum, 0x10);					\
		pAttr##attrNum = (qword)spu_add((vec_int4)pAttr##attrNum, 0x20); \
	} while(0)

#define EDGE_GEOM_COMPRESS_INSERT(attrNum)										\
	do {																\
		switch(shufCount0##attrNum)										\
		{																\
		case 2:															\
			outQuads[outQuad0##attrNum##1] = spu_shuffle(outAttr0##attrNum, outQuads[outQuad0##attrNum##1], shuf0##attrNum##1); \
			/* fall through */											\
		case 1:															\
			outQuads[outQuad0##attrNum##0] = spu_shuffle(outAttr0##attrNum, outQuads[outQuad0##attrNum##0], shuf0##attrNum##0); \
			break;														\
		case 0:															\
			outQuads[outQuad0##attrNum##0] = spu_sel(outQuads[outQuad0##attrNum##0], outAttr0##attrNum, fastInsertMask##attrNum); \
			break;														\
		default:														\
			break;														\
		}																\
		switch(shufCount1##attrNum)										\
		{																\
		case 2:															\
			outQuads[outQuad1##attrNum##1] = spu_shuffle(outAttr1##attrNum, outQuads[outQuad1##attrNum##1], shuf1##attrNum##1); \
			/* fall through */											\
		case 1:															\
			outQuads[outQuad1##attrNum##0] = spu_shuffle(outAttr1##attrNum, outQuads[outQuad1##attrNum##0], shuf1##attrNum##0); \
			break;														\
		case 0:															\
			outQuads[outQuad1##attrNum##0] = spu_sel(outQuads[outQuad1##attrNum##0], outAttr1##attrNum, fastInsertMask##attrNum); \
			break;														\
		default:														\
			break;														\
		}																\
	} while(0)

#define EDGE_GEOM_COMPRESS_PROCESS_F32(attrNum)			\
	do {										\
		outAttr0##attrNum = attr0##attrNum;		\
		outAttr1##attrNum = attr1##attrNum;		\
	} while(0)

#define EDGE_GEOM_COMPRESS_PROCESS_F16(attrNum)									\
	do {																\
		const vec_ushort8 in01  = (vec_ushort8)spu_shuffle(attr0##attrNum, attr1##attrNum, s_ABEFIJMNabefijmn); \
		const vec_ushort8 nz01  = spu_andc(in01, signMask);				\
		const vec_ushort8 m01   = spu_cmpgt(expMin, nz01);				\
		const vec_ushort8 x01   = spu_cmpgt(expMax, nz01);				\
		const vec_ushort8 r0    = (vec_ushort8)spu_rlqw(spu_sub((vec_int4)attr0##attrNum, expRebias), 3); \
		const vec_ushort8 r1    = (vec_ushort8)spu_rlqw(spu_sub((vec_int4)attr1##attrNum, expRebias), 3); \
		const vec_ushort8 r01   = spu_sel(maxHalf, (vec_ushort8)spu_shuffle(r0, r1, s_ABEFIJMNabefijmn), x01); \
		const vec_ushort8 n01   = spu_sel(r01, in01, signMask);			\
		outAttr0##attrNum = (qword)spu_andc(n01, m01);					\
		outAttr1##attrNum = spu_rlqwbyte(outAttr0##attrNum, 8);			\
	} while(0)

#define EDGE_GEOM_COMPRESS_PROCESS_U8(attrNum)									\
	do {																\
		outAttr0##attrNum = (qword)spu_convtu((vec_float4)attr0##attrNum, 24); \
		outAttr1##attrNum = (qword)spu_convtu((vec_float4)attr1##attrNum, 24); \
	} while(0)

#define EDGE_GEOM_COMPRESS_PROCESS_I16(attrNum)									\
	do {																\
		outAttr0##attrNum = (qword)spu_convts((vec_float4)attr0##attrNum, 16); \
		outAttr1##attrNum = (qword)spu_convts((vec_float4)attr1##attrNum, 16); \
	} while(0)

#define EDGE_GEOM_COMPRESS_PROCESS_U8N(attrNum)									\
	do {																\
		outAttr0##attrNum = (qword)spu_convtu(spu_madd((vec_float4)attr0##attrNum, u8nScale, u8nBias), 24); \
		outAttr1##attrNum = (qword)spu_convtu(spu_madd((vec_float4)attr1##attrNum, u8nScale, u8nBias), 24); \
	} while(0)

#define EDGE_GEOM_COMPRESS_PROCESS_I16N(attrNum)									\
	do {																\
		outAttr0##attrNum = (qword)spu_convts(spu_madd((vec_float4)attr0##attrNum, i16nScale, i16nBias), 16); \
		outAttr1##attrNum = (qword)spu_convts(spu_madd((vec_float4)attr1##attrNum, i16nScale, i16nBias), 16); \
	} while(0)

#define EDGE_GEOM_COMPRESS_PROCESS_X11Y11Z10N(attrNum)							\
	do {																\
		const vec_int4 temp0 = spu_rlmask(spu_convts(spu_madd((vec_float4)attr0##attrNum, x11y11z10Scale, x11y11z10Bias), 31), bitShift); \
		const vec_int4 temp1 = spu_rlmask(spu_convts(spu_madd((vec_float4)attr1##attrNum, x11y11z10Scale, x11y11z10Bias), 31), bitShift); \
		outAttr0##attrNum = (qword)spu_sel(spu_shuffle(temp0, temp1, s_xzMask), spu_shuffle(temp0, temp1, s_yMask), sel_bitMask); \
		outAttr1##attrNum = spu_rlqwbyte(outAttr0##attrNum, 4);			\
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_F32(attrNum)				\
	do {										\
		EDGE_GEOM_COMPRESS_INSERT(attrNum);				\
		EDGE_GEOM_COMPRESS_PROCESS_F32(attrNum);			\
		EDGE_GEOM_COMPRESS_LOAD(attrNum);					\
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_F16(attrNum)				\
	do {										\
		EDGE_GEOM_COMPRESS_INSERT(attrNum);				\
		EDGE_GEOM_COMPRESS_PROCESS_F16(attrNum);			\
		EDGE_GEOM_COMPRESS_LOAD(attrNum);					\
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_U8(attrNum)				\
	do {										\
		EDGE_GEOM_COMPRESS_INSERT(attrNum);				\
		EDGE_GEOM_COMPRESS_PROCESS_U8(attrNum);			\
		EDGE_GEOM_COMPRESS_LOAD(attrNum);					\
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_I16(attrNum)				\
	do {										\
		EDGE_GEOM_COMPRESS_INSERT(attrNum);				\
		EDGE_GEOM_COMPRESS_PROCESS_I16(attrNum);			\
		EDGE_GEOM_COMPRESS_LOAD(attrNum);					\
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_U8N(attrNum)				\
	do {										\
		EDGE_GEOM_COMPRESS_INSERT(attrNum);				\
		EDGE_GEOM_COMPRESS_PROCESS_U8N(attrNum);			\
		EDGE_GEOM_COMPRESS_LOAD(attrNum);					\
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_I16N(attrNum)				\
	do {										\
		EDGE_GEOM_COMPRESS_INSERT(attrNum);				\
		EDGE_GEOM_COMPRESS_PROCESS_I16N(attrNum);			\
		EDGE_GEOM_COMPRESS_LOAD(attrNum);					\
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_X11Y11Z10N(attrNum)		\
	do {										\
		EDGE_GEOM_COMPRESS_INSERT(attrNum);				\
		EDGE_GEOM_COMPRESS_PROCESS_X11Y11Z10N(attrNum);	\
		EDGE_GEOM_COMPRESS_LOAD(attrNum);					\
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_START()									\
	do {																\
		if (kStrideTimes2 % 16 == 0)									\
		{																\
			switch(kNumQuads)											\
			{															\
			case 16:													\
				si_stqx(outQuads[15], pOutput[15], outOffset);			\
				/* fall through */										\
			case 15:													\
				si_stqx(outQuads[14], pOutput[14], outOffset);			\
				/* fall through */										\
			case 14:													\
				si_stqx(outQuads[13], pOutput[13], outOffset);			\
				/* fall through */										\
			case 13:													\
				si_stqx(outQuads[12], pOutput[12], outOffset);			\
				/* fall through */										\
			case 12:													\
				si_stqx(outQuads[11], pOutput[11], outOffset);			\
				/* fall through */										\
			case 11:													\
				si_stqx(outQuads[10], pOutput[10], outOffset);			\
				/* fall through */										\
			case 10:													\
				si_stqx(outQuads[9], pOutput[9], outOffset);			\
				/* fall through */										\
			case 9:														\
				si_stqx(outQuads[8], pOutput[8], outOffset);			\
				/* fall through */										\
			case 8:														\
				si_stqx(outQuads[7], pOutput[7], outOffset);			\
				/* fall through */										\
			case 7:														\
				si_stqx(outQuads[6], pOutput[6], outOffset);			\
				/* fall through */										\
			case 6:														\
				si_stqx(outQuads[5], pOutput[5], outOffset);			\
				/* fall through */										\
			case 5:														\
				si_stqx(outQuads[4], pOutput[4], outOffset);			\
				/* fall through */										\
			case 4:														\
				si_stqx(outQuads[3], pOutput[3], outOffset);			\
				/* fall through */										\
			case 3:														\
				si_stqx(outQuads[2], pOutput[2], outOffset);			\
				/* fall through */										\
			case 2:														\
				si_stqx(outQuads[1], pOutput[1], outOffset);			\
				/* fall through */										\
			case 1:														\
				si_stqx(outQuads[0], pOutput[0], outOffset);			\
				break;													\
			default:													\
				break;													\
			}															\
		}																\
		else															\
		{																\
			qword outTemp[18];											\
			const qword quadOff = spu_and((qword)spu_add((vec_int4)pOutput[0], (vec_int4)outOffset), 0x0F); \
			const vec_uchar16 outShuf = (vec_uchar16)spu_sub((vec_int4)s_ABCDx, (vec_int4)spu_shuffle(quadOff, quadOff, s_DDDDDDDDDDDDDDDD)); \
			const qword oldLastRot = spu_rlqwbyte(oldLast, si_to_int(quadOff)); \
																		\
			switch(kNumQuads)											\
			{															\
			case 16:													\
				outTemp[15] = spu_shuffle(outQuads[15], outQuads[14], outShuf); \
				si_stqx(outTemp[15], pOutput[15], outOffset);			\
				/* fall through */										\
			case 15:													\
				outTemp[14] = spu_shuffle(outQuads[14], outQuads[13], outShuf); \
				si_stqx(outTemp[14], pOutput[14], outOffset);			\
				/* fall through */										\
			case 14:													\
				outTemp[13] = spu_shuffle(outQuads[13], outQuads[12], outShuf); \
				si_stqx(outTemp[13], pOutput[13], outOffset);			\
				/* fall through */										\
			case 13:													\
				outTemp[12] = spu_shuffle(outQuads[12], outQuads[11], outShuf); \
				si_stqx(outTemp[12], pOutput[12], outOffset);			\
				/* fall through */										\
			case 12:													\
				outTemp[11] = spu_shuffle(outQuads[11], outQuads[10], outShuf); \
				si_stqx(outTemp[11], pOutput[11], outOffset);			\
				/* fall through */										\
			case 11:													\
				outTemp[10] = spu_shuffle(outQuads[10], outQuads[9], outShuf); \
				si_stqx(outTemp[10], pOutput[10], outOffset);			\
				/* fall through */										\
			case 10:													\
				outTemp[9] = spu_shuffle(outQuads[9], outQuads[8], outShuf); \
				si_stqx(outTemp[9], pOutput[9], outOffset);				\
				/* fall through */										\
			case 9:														\
				outTemp[8] = spu_shuffle(outQuads[8], outQuads[7], outShuf); \
				si_stqx(outTemp[8], pOutput[8], outOffset);				\
				/* fall through */										\
			case 8:														\
				outTemp[7] = spu_shuffle(outQuads[7], outQuads[6], outShuf); \
				si_stqx(outTemp[7], pOutput[7], outOffset);				\
				/* fall through */										\
			case 7:														\
				outTemp[6] = spu_shuffle(outQuads[6], outQuads[5], outShuf); \
				si_stqx(outTemp[6], pOutput[6], outOffset);				\
				/* fall through */										\
			case 6:														\
				outTemp[5] = spu_shuffle(outQuads[5], outQuads[4], outShuf); \
				si_stqx(outTemp[5], pOutput[5], outOffset);				\
				/* fall through */										\
			case 5:														\
				outTemp[4] = spu_shuffle(outQuads[4], outQuads[3], outShuf); \
				si_stqx(outTemp[4], pOutput[4], outOffset);				\
				/* fall through */										\
			case 4:														\
				outTemp[3] = spu_shuffle(outQuads[3], outQuads[2], outShuf); \
				si_stqx(outTemp[3], pOutput[3], outOffset);				\
				/* fall through */										\
			case 3:														\
				outTemp[2] = spu_shuffle(outQuads[2], outQuads[1], outShuf); \
				si_stqx(outTemp[2], pOutput[2], outOffset);				\
				/* fall through */										\
			case 2:														\
				outTemp[1] = spu_shuffle(outQuads[1], outQuads[0], outShuf); \
				si_stqx(outTemp[1], pOutput[1], outOffset);				\
				/* fall through */										\
			case 1:														\
				outTemp[0] = spu_shuffle(outQuads[0], oldLastRot, outShuf); \
				si_stqx(outTemp[0], pOutput[0], outOffset);				\
				break;													\
			default:													\
				break;													\
			}															\
			if (kStrideTimes2 % 16 != 1)								\
			{															\
				outTemp[kNumQuads] = spu_shuffle(outQuads[kNumQuads-1], outQuads[kNumQuads-1], outShuf); \
				si_stqx(outTemp[kNumQuads], pOutputAfterLast, outOffset); \
				oldLast = si_lqx(pOutputLast, outOffset);				\
			}															\
			else														\
			{															\
				oldLast = outTemp[kNumQuads-1];							\
			}															\
		}																\
		outOffset = (qword)spu_add((vec_int4)outOffset, kStrideTimes2); \
	} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_END()						\
	do {} while(0)

//
// Initialization macros.  These must not be wrapped in do...while(0)
// blocks, since we want the variables they declare to be at the
// function-level scope.
//

#define EDGE_GEOM_COMPRESS_INIT_COMMON(attrType, attrNum, attrOffset, attrSize)	\
	const uint32_t sizeMask##attrNum = (0xFFFF0000 >> attrSize) & 0xFFFF; \
	const vec_uchar16 fastInsertMask##attrNum = spu_maskb(sizeMask##attrNum); \
																		\
	const uint32_t offset0##attrNum = attrOffset;						\
	const bool attribIsAligned0##attrNum = (offset0##attrNum % 16 == 0); \
	const bool attribStraddlesQuad0##attrNum = ( (offset0##attrNum % 16) + attrSize > 16 ); \
	const uint32_t shufCount0##attrNum = (attribIsAligned0##attrNum && useFastInsert##attrNum) ? 0 : \
		attribStraddlesQuad0##attrNum ? 2 : 1;							\
	const uint32_t outQuad0##attrNum##0 = offset0##attrNum / 16;				\
	const uint32_t outQuad0##attrNum##1 = outQuad0##attrNum##0 + 1;			\
	vec_uchar16 shuf0##attrNum##0, shuf0##attrNum##1;					\
	switch(shufCount0##attrNum)											\
	{																	\
	case 2:																\
		{																\
			const uint16_t maskGen = (sizeMask##attrNum << (16-(offset0##attrNum % 16))) & 0xFFFF; \
			const vec_uchar16 mask = spu_maskb(maskGen);				\
			shuf0##attrNum##1 = spu_sel(s_abcd, (vec_uchar16)spu_slqwbyte(shufSeed##attrNum, 16-(offset0##attrNum % 16)), mask); \
		}																\
		/* fall through */												\
	case 1:																\
		{																\
			const uint16_t maskGen = sizeMask##attrNum >> (offset0##attrNum % 16); \
			const vec_uchar16 mask = spu_maskb(maskGen);				\
			shuf0##attrNum##0 = spu_sel(s_abcd, (vec_uchar16)spu_rlmaskqwbyte(shufSeed##attrNum, -(offset0##attrNum % 16)), mask); \
		}																\
		break;															\
	case 0:																\
		break;															\
	default:															\
		break;															\
	}																	\
																		\
	const uint32_t offset1##attrNum = attrOffset + kStride;					\
	const bool attribIsAligned1##attrNum = (offset1##attrNum % 16 == 0); \
	const bool attribStraddlesQuad1##attrNum = ( (offset1##attrNum % 16) + attrSize > 16 ); \
	const uint32_t shufCount1##attrNum = (attribIsAligned1##attrNum && useFastInsert##attrNum) ? 0 : \
		attribStraddlesQuad1##attrNum ? 2 : 1;							\
	const uint32_t outQuad1##attrNum##0 = offset1##attrNum / 16;				\
	const uint32_t outQuad1##attrNum##1 = outQuad1##attrNum##0 + 1;			\
	vec_uchar16 shuf1##attrNum##0, shuf1##attrNum##1;					\
	switch(shufCount1##attrNum)											\
	{																	\
	case 2:																\
		{																\
			const uint16_t maskGen = (sizeMask##attrNum << (16-(offset1##attrNum % 16))) & 0xFFFF; \
			const vec_uchar16 mask = spu_maskb(maskGen);				\
			shuf1##attrNum##1 = spu_sel(s_abcd, (vec_uchar16)spu_slqwbyte(shufSeed##attrNum, 16-(offset1##attrNum % 16)), mask); \
		}																\
		/* fall through */												\
	case 1:																\
		{																\
			const uint16_t maskGen = sizeMask##attrNum >> (offset1##attrNum % 16); \
			const vec_uchar16 mask = spu_maskb(maskGen);				\
			shuf1##attrNum##0 = spu_sel(s_abcd, (vec_uchar16)spu_rlmaskqwbyte(shufSeed##attrNum, -(offset1##attrNum % 16)), mask); \
		}																\
		break;															\
	case 0:																\
		break;															\
	default:															\
		break;															\
	}																	\
																		\
	qword attr0##attrNum;												\
	qword attr1##attrNum;												\
	qword outAttr0##attrNum;											\
	qword outAttr1##attrNum;											\
	qword pAttr##attrNum = si_from_ptr(pUniforms[attrNum]);				\
																		\
	EDGE_GEOM_COMPRESS_LOAD(attrNum);												\
	EDGE_GEOM_COMPRESS_PROCESS_##attrType(attrNum);								\
	EDGE_GEOM_COMPRESS_INSERT(attrNum);											\
	EDGE_GEOM_COMPRESS_LOAD(attrNum);												\
	EDGE_GEOM_COMPRESS_PROCESS_##attrType(attrNum);								\
	EDGE_GEOM_COMPRESS_LOAD(attrNum);												\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_INIT_F32(attrNum, attrOffset, attrSizeBytes)		\
	const vec_uchar16 shufSeed##attrNum = s_ABCDx;					\
	const bool useFastInsert##attrNum = true;						\
	EDGE_GEOM_COMPRESS_INIT_COMMON(F32, attrNum, attrOffset, attrSizeBytes);	\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_INIT_F16(attrNum, attrOffset, attrSizeBytes)		\
	const vec_uchar16 shufSeed##attrNum = s_ABCDx;					\
	const bool useFastInsert##attrNum = true;						\
	EDGE_GEOM_COMPRESS_INIT_COMMON(F16, attrNum, attrOffset, attrSizeBytes);	\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_INIT_U8(attrNum, attrOffset, attrSizeBytes)		\
	const vec_uchar16 shufSeed##attrNum = s_AEIM000000000000;		\
	const bool useFastInsert##attrNum = false;						\
	EDGE_GEOM_COMPRESS_INIT_COMMON(U8, attrNum, attrOffset, attrSizeBytes);	\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_INIT_I16(attrNum, attrOffset, attrSizeBytes)		\
	const vec_uchar16 shufSeed##attrNum = s_ACEG0000;				\
	const bool useFastInsert##attrNum = false;						\
	EDGE_GEOM_COMPRESS_INIT_COMMON(I16, attrNum, attrOffset, attrSizeBytes);	\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_INIT_U8N(attrNum, attrOffset, attrSizeBytes)		\
	const vec_uchar16 shufSeed##attrNum = s_AEIM000000000000;		\
	const bool useFastInsert##attrNum = false;						\
	EDGE_GEOM_COMPRESS_INIT_COMMON(U8N, attrNum, attrOffset, attrSizeBytes);	\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_INIT_I16N(attrNum, attrOffset, attrSizeBytes)		\
	const vec_uchar16 shufSeed##attrNum = s_ACEG0000;				\
	const bool useFastInsert##attrNum = false;						\
	EDGE_GEOM_COMPRESS_INIT_COMMON(I16N, attrNum, attrOffset, attrSizeBytes);	\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_INIT_X11Y11Z10N(attrNum, attrOffset, attrSizeBytes)	\
	const vec_uchar16 shufSeed##attrNum = s_ABCDx;						\
	const bool useFastInsert##attrNum = true;							\
	EDGE_GEOM_COMPRESS_INIT_COMMON(X11Y11Z10N, attrNum, attrOffset, 4);			\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_INIT_GLOBAL_COMMON(ctx, vertCount, outVertexes) \
	const float **pUniforms = (const float **)ctx->uniformTables; 				\
	const uint32_t kVertCount = ((vertCount)+1) & ~0x1;						\
	char *pOutVertexes = (char*)(outVertexes);							\
	const EDGE_VAR vec_uchar16 s_ABCDx            = {0x20,0x21,0x22,0x23, 0x24,0x25,0x26,0x27, 0x28,0x29,0x2a,0x2b, 0x2c,0x2d,0x2e,0x2f}; \
	const EDGE_VAR vec_uchar16 s_abcd             = {0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b, 0x1c,0x1d,0x1e,0x1f}; \
	const EDGE_VAR vec_uchar16 s_ABEFIJMNabefijmn = {0x00,0x01,0x04,0x05, 0x08,0x09,0x0C,0x0D, 0x10,0x11,0x14,0x15, 0x18,0x19,0x1C,0x1D}; \
	const EDGE_VAR vec_uchar16 s_ACEG0000         = {0x00,0x01,0x04,0x05, 0x08,0x09,0x0C,0x0D, 0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80}; \
	const EDGE_VAR vec_uchar16 s_AEIM000000000000 = {0x00,0x04,0x08,0x0C, 0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80}; \
	const EDGE_VAR vec_uchar16 s_EAeaEAea         = {0x08,0x09,0x00,0x01, 0x18,0x19,0x10,0x11, 0x08,0x09,0x00,0x01, 0x18,0x19,0x10,0x11}; \
	const EDGE_VAR vec_uchar16 s_DDDDDDDDDDDDDDDD = (vec_uchar16)spu_splats(0x03030303); \
	/* U8N variables */													\
	const EDGE_VAR vec_float4  u8nScale = (vec_float4)spu_splats(255.0f); \
	const EDGE_VAR vec_float4  u8nBias  = (vec_float4)spu_splats(0.5f); \
	/* I16N variables */												\
	const EDGE_VAR vec_float4  i16nScale = (vec_float4)spu_splats(65535.0f / 2.0f); \
	const EDGE_VAR vec_float4  i16nBias  = (vec_float4)spu_splats(-0.5f); \
	/* F16 variables */													\
	const EDGE_VAR vec_int4    expRebias          =    (vec_int4)spu_splats(0x38000000); \
	const EDGE_VAR vec_ushort8 signMask           = (vec_ushort8)spu_splats(0x80008000); \
	const EDGE_VAR vec_ushort8 maxHalf            = (vec_ushort8)spu_splats(0x7BFF7BFF); \
	const EDGE_VAR vec_ushort8 expMin             = (vec_ushort8)spu_splats(0x38003800); \
	const EDGE_VAR vec_ushort8 expMax             = (vec_ushort8)spu_splats(0x47804780); \
	/* X11Y11Z10N variables */											\
	const EDGE_VAR vec_uchar16 s_xzMask       = s_EAeaEAea;				\
	const EDGE_VAR vec_uchar16 s_yMask        = spu_or((vec_uchar16)spu_rlqwbyte(s_xzMask,1), 0x04); \
	const EDGE_VAR vec_int4    bitShift       = (vec_int4){-5, -2,  0,  0};	\
	const EDGE_VAR vec_uint4   sel_bitMask    = (vec_uint4)spu_splats(0x003FF800); \
	const EDGE_VAR vec_float4  x11y11z10Scale = (vec_float4)(vec_int4){0x3F7FE000, 0x3F7FE000, 0x3F7FC000, 0x00000000}; \
	const EDGE_VAR vec_float4  x11y11z10Bias  = (vec_float4)(vec_int4){0xBA000000, 0xBA000000, 0xBA800000, 0x00000000}; \
	do {} while(0)


#define EDGE_GEOM_COMPRESS_INIT_GLOBAL(stride)									\
	const uint32_t kStride = stride;											\
	const uint32_t kStrideTimes2 = 2*kStride;								\
	const uint32_t kNumQuads = (kStrideTimes2+15) / 16;						\
	qword outQuads[17] = { {0},{0},{0},{0}, {0},{0},{0},{0}, {0},{0},{0},{0}, {0},{0},{0},{0}, {0}, }; \
																		\
	const qword pOutput[16] = {											\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x00),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x10),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x20),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x30),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x40),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x50),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x60),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x70),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x80),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0x90),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0xA0),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0xB0),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0xC0),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0xD0),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0xE0),			\
		si_from_ptr(pOutVertexes + kStride*kVertCount + 0xF0),			\
	};																	\
	const qword pOutputLast      = si_from_ptr(pOutVertexes + kStride*kVertCount + kStrideTimes2); \
	const qword pOutputAfterLast = si_from_ptr(pOutVertexes + kStride*kVertCount + kNumQuads*0x10); \
	qword outOffset = si_from_int(-kVertCount*kStride);					\
	qword oldLast = (qword)si_lqx(pOutput[0], outOffset);				\
	do {} while(0)

#define EDGE_GEOM_COMPRESS_LOOP_DONE()					\
	(si_to_int(outOffset) == 0)

#endif // EDGEGEOM_COMPRESS_H
