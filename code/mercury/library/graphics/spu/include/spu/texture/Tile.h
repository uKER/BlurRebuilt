#ifndef GRAPHICS_SPU_TEXTURE_TILE_H
#define GRAPHICS_SPU_TEXTURE_TILE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Spu/Common.h"

#if defined(PLATFORM_SPU)

const U32	c_RopTileWidth		= 8;
const U32	c_RopTileStride		= c_RopTileWidth * 4;
const U32	c_RopTileHeight		= 4;
const U32	c_RopTileSize		= c_RopTileStride * c_RopTileHeight;
const U32	c_FBTilesX			= 8;
const U32	c_FBTilesY			= 16;
const U32	c_FBTileWidth		= c_RopTileWidth * c_FBTilesX;
const U32	c_FBTileHeight		= c_RopTileHeight * c_FBTilesY;
const U32	c_FBTileStride		= c_RopTileStride * c_FBTilesX;
const U32	c_FBTileSize		= c_FBTileWidth * c_FBTileHeight * 4;

namespace Spu
{

#define ROP_TILE_PIXELS_X		8
#define ROP_TILE_STRIDE			(ROP_TILE_PIXELS_X*4)
#define ROP_TILE_ROWS			4
#define ROP_TILE_SIZE			(ROP_TILE_STRIDE*ROP_TILE_ROWS)
#define FB_NUM_TILES_X			8
#define FB_NUM_TILES_Y			16
#define FB_TILE_STRIDE			(ROP_TILE_SIZE * FB_NUM_TILES_X)
#define ODD_TILE_OFFSET			((ROP_TILE_ROWS*2)*(ROP_TILE_STRIDE))
#define ODD_TILE_OFFSET_UCHAR16	((ODD_TILE_OFFSET/16)-2)
#define ROP_TILE_SIZE2			(ROP_TILE_SIZE*2)
#define ROP_TILE_SIZE_UCHAR16	(ROP_TILE_SIZE/16)
#define ROP_TILE_SIZE2_UCHAR16	(ROP_TILE_SIZE2/16)
#define FB_TILE_NUM_ROWS		(FB_NUM_TILES_Y*ROP_TILE_ROWS)
#define FB_TILE_NUM_PIXELS_X	(FB_NUM_TILES_X*ROP_TILE_PIXELS_X)
#define FB_TILE_SIZE			(FB_TILE_NUM_PIXELS_X * FB_TILE_NUM_ROWS * 4)

#define DEPTH_DETILE_BIAS		0x0

static const vec_uint4 DEPTH_DETILE_UNBIAS = { DEPTH_DETILE_BIAS << 8, DEPTH_DETILE_BIAS << 8, DEPTH_DETILE_BIAS << 8, DEPTH_DETILE_BIAS << 8 };

class DetileBufferProcessor
{
public:

	INLINE void BeginProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void BeginProcessBlock16(const S32 tileYIndex)
	{
	}

	INLINE void EndProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void EndProcessBlock16(const S32 tileYIndex)
	{
	}

	INLINE void ProcessROPTile(vec_uchar16*& __restrict pOut,
									 const vec_uchar16 r00, const vec_uchar16 r01, const vec_uchar16 r02, const vec_uchar16 r03,
									 const vec_uchar16 r10, const vec_uchar16 r11, const vec_uchar16 r12, const vec_uchar16 r13,
									 const vec_uchar16 r20, const vec_uchar16 r21, const vec_uchar16 r22, const vec_uchar16 r23,
									 const vec_uchar16 r30, const vec_uchar16 r31, const vec_uchar16 r32, const vec_uchar16 r33)
	{
		// Output the 64 pixel row (as 16 uchar16s) linearly.
		pOut[0] = r00;
		pOut[1] = r01;
		pOut[2] = r02;
		pOut[3] = r03;
		pOut[4] = r10;
		pOut[5] = r11;
		pOut[6] = r12;
		pOut[7] = r13;
		pOut[8] = r20;
		pOut[9] = r21;
		pOut[10] = r22;
		pOut[11] = r23;
		pOut[12] = r30;
		pOut[13] = r31;
		pOut[14] = r32;
		pOut[15] = r33;
		pOut += 16;
	}

};

class DetileBufferMSAACheckProcessor
{
public:

	INLINE DetileBufferMSAACheckProcessor(vec_uint4* pNoMSAA, const U32 msaaComparisonThreshold)
		: m_pNoMSAA(pNoMSAA)
	{
		m_MSAAComparisonThreshold = spu_splats((U8)msaaComparisonThreshold);
	}

	INLINE void BeginProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void BeginProcessBlock16(const S32 tileYIndex)
	{
		m_NoMSAA0	= spu_splats((U32)0xFFFFFFFF);
		m_NoMSAA1	= spu_splats((U32)0xFFFFFFFF);
	}

	INLINE void EndProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void EndProcessBlock16(const S32 tileYIndex)
	{
		m_pNoMSAA[tileYIndex]	= m_NoMSAA0;
		m_pNoMSAA[tileYIndex+1]	= m_NoMSAA1;
	}

	INLINE void ProcessROPTile(vec_uchar16*& __restrict pOut,
								const vec_uchar16 r00, const vec_uchar16 r01, const vec_uchar16 r02, const vec_uchar16 r03,
								const vec_uchar16 r10, const vec_uchar16 r11, const vec_uchar16 r12, const vec_uchar16 r13,
								const vec_uchar16 r20, const vec_uchar16 r21, const vec_uchar16 r22, const vec_uchar16 r23,
								const vec_uchar16 r30, const vec_uchar16 r31, const vec_uchar16 r32, const vec_uchar16 r33)
	{
		const vec_uchar16 threshold = m_MSAAComparisonThreshold;
		const vec_uint4 cmp0 = (vec_uint4)spu_cmpgt(threshold, spu_absd((vec_uchar16)spu_shuffle(r00, r01, SHUFFLE_XZAC), (vec_uchar16)spu_shuffle(r00, r01, SHUFFLE_YWBD)));
		const vec_uint4 cmp1 = (vec_uint4)spu_cmpgt(threshold, spu_absd((vec_uchar16)spu_shuffle(r02, r03, SHUFFLE_XZAC), (vec_uchar16)spu_shuffle(r02, r03, SHUFFLE_YWBD)));
		const vec_uint4 cmp2 = (vec_uint4)spu_cmpgt(threshold, spu_absd((vec_uchar16)spu_shuffle(r10, r11, SHUFFLE_XZAC), (vec_uchar16)spu_shuffle(r10, r11, SHUFFLE_YWBD)));
		const vec_uint4 cmp3 = (vec_uint4)spu_cmpgt(threshold, spu_absd((vec_uchar16)spu_shuffle(r12, r13, SHUFFLE_XZAC), (vec_uchar16)spu_shuffle(r12, r13, SHUFFLE_YWBD)));
		const vec_uint4 cmp4 = (vec_uint4)spu_cmpgt(threshold, spu_absd((vec_uchar16)spu_shuffle(r20, r21, SHUFFLE_XZAC), (vec_uchar16)spu_shuffle(r20, r21, SHUFFLE_YWBD)));
		const vec_uint4 cmp5 = (vec_uint4)spu_cmpgt(threshold, spu_absd((vec_uchar16)spu_shuffle(r22, r23, SHUFFLE_XZAC), (vec_uchar16)spu_shuffle(r22, r23, SHUFFLE_YWBD)));
		const vec_uint4 cmp6 = (vec_uint4)spu_cmpgt(threshold, spu_absd((vec_uchar16)spu_shuffle(r30, r31, SHUFFLE_XZAC), (vec_uchar16)spu_shuffle(r30, r31, SHUFFLE_YWBD)));
		const vec_uint4 cmp7 = (vec_uint4)spu_cmpgt(threshold, spu_absd((vec_uchar16)spu_shuffle(r32, r33, SHUFFLE_XZAC), (vec_uchar16)spu_shuffle(r32, r33, SHUFFLE_YWBD)));

		const vec_uint4 cmpA = spu_and(spu_and(cmp0, cmp1), spu_and(cmp2,cmp3));
		const vec_uint4 cmpB = spu_and(spu_and(cmp4, cmp5), spu_and(cmp6,cmp7));

		m_NoMSAA0 = spu_and(m_NoMSAA0, cmpA);
		m_NoMSAA1 = spu_and(m_NoMSAA1, cmpB);

		pOut[0] = r00;
		pOut[1] = r01;
		pOut[2] = r02;
		pOut[3] = r03;
		pOut[4] = r10;
		pOut[5] = r11;
		pOut[6] = r12;
		pOut[7] = r13;
		pOut[8] = r20;
		pOut[9] = r21;
		pOut[10] = r22;
		pOut[11] = r23;
		pOut[12] = r30;
		pOut[13] = r31;
		pOut[14] = r32;
		pOut[15] = r33;
		pOut += 16;
	}

private:

	vec_uint4	m_NoMSAA0;
	vec_uint4	m_NoMSAA1;
	vec_uint4*	m_pNoMSAA;
	vec_uchar16	m_MSAAComparisonThreshold;

};

class DetileDepthBufferProcessor
{
public:
	INLINE DetileDepthBufferProcessor()
	{
	}

	INLINE void BeginProcessBlock8(const S32 tileYIndex)
	{
	}
	
	INLINE void BeginProcessBlock16(const S32 tileYIndex)
	{
	}
	
	INLINE void EndProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void EndProcessBlock16(const S32 tileYCounter)
	{
	}

	INLINE void ProcessROPTile(vec_uchar16*& __restrict pOut,
									vec_uchar16 r00, vec_uchar16 r01, vec_uchar16 r02, vec_uchar16 r03,
									 vec_uchar16 r10, vec_uchar16 r11, vec_uchar16 r12, vec_uchar16 r13,
									 vec_uchar16 r20, vec_uchar16 r21, vec_uchar16 r22, vec_uchar16 r23,
									 vec_uchar16 r30, vec_uchar16 r31, vec_uchar16 r32, vec_uchar16 r33)
	{
		pOut[0] = r00;
		pOut[1] = r01;
		pOut[2] = r02;
		pOut[3] = r03;
		pOut[4] = r10;
		pOut[5] = r11;
		pOut[6] = r12;
		pOut[7] = r13;
		pOut[8] = r20;
		pOut[9] = r21;
		pOut[10] = r22;
		pOut[11] = r23;
		pOut[12] = r30;
		pOut[13] = r31;
		pOut[14] = r32;
		pOut[15] = r33;
		pOut += 16;
	}

private:

	vec_uchar16 m_ZShuffleMask;

};

class DetileDepthBufferRangeProcessor
{
public:
	INLINE DetileDepthBufferRangeProcessor(vec_uint4* pCurrentMinZin, vec_uint4* pCurrentMaxZin, vec_uchar16* pStencilMinIn, vec_uchar16* pStencilMaxIn)
		: m_pCurrentMinZin(pCurrentMinZin)
		, m_pCurrentMaxZin(pCurrentMaxZin)
		, m_pStencilMinIn(pStencilMinIn)
		, m_pStencilMaxIn(pStencilMaxIn)
	{
		const vec_uchar16 initialStencilMin = spu_splats((U8)0xFF);
		const vec_uchar16 initialStencilMax = spu_splats((U8)0x00);
		const vec_ushort8 initialMinZ = spu_splats((unsigned short)0xFFFF);
		const vec_ushort8 initialMaxZ = spu_splats((unsigned short)0x0000);

		m_MinZ0 = initialMinZ;
		m_MinZ1 = initialMinZ;
		m_MinZ2 = initialMinZ;
		m_MinZ3 = initialMinZ;
		m_MinZ4 = initialMinZ;
		m_MinZ5 = initialMinZ;
		m_MinZ6 = initialMinZ;
		m_MinZ7 = initialMinZ;

		m_MaxZ0 = initialMaxZ;
		m_MaxZ1 = initialMaxZ;
		m_MaxZ2 = initialMaxZ;
		m_MaxZ3 = initialMaxZ;
		m_MaxZ4 = initialMaxZ;
		m_MaxZ5 = initialMaxZ;
		m_MaxZ6 = initialMaxZ;
		m_MaxZ7 = initialMaxZ;

		m_StencilMin0 = initialStencilMin;
		m_StencilMax0 = initialStencilMax;
		m_StencilMin1 = initialStencilMin;
		m_StencilMax1 = initialStencilMax;
	}

	INLINE void BeginProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void BeginProcessBlock16(const S32 tileYIndex)
	{
	}

	INLINE void EndProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void EndProcessBlock16(const S32 tileYCounter)
	{
		const vec_ushort8 minZ0Cmp = spu_cmpgt(m_MinZ0, m_MinZ2);
		const vec_ushort8 minZ1Cmp = spu_cmpgt(m_MinZ1, m_MinZ3);
		const vec_ushort8 minZ2Cmp = spu_cmpgt(m_MinZ4, m_MinZ6);
		const vec_ushort8 minZ3Cmp = spu_cmpgt(m_MinZ5, m_MinZ7);

		m_MinZ0 = spu_sel(m_MinZ0, m_MinZ2, minZ0Cmp);
		m_MinZ1 = spu_sel(m_MinZ1, m_MinZ3, minZ1Cmp);
		m_MinZ4 = spu_sel(m_MinZ4, m_MinZ6, minZ2Cmp);
		m_MinZ5 = spu_sel(m_MinZ5, m_MinZ7, minZ3Cmp);

		const vec_ushort8 maxZ0Cmp = spu_cmpgt(m_MaxZ0, m_MaxZ2);
		const vec_ushort8 maxZ1Cmp = spu_cmpgt(m_MaxZ1, m_MaxZ3);
		const vec_ushort8 maxZ2Cmp = spu_cmpgt(m_MaxZ4, m_MaxZ6);
		const vec_ushort8 maxZ3Cmp = spu_cmpgt(m_MaxZ5, m_MaxZ7);

		m_MaxZ0 = spu_sel(m_MaxZ2, m_MaxZ0, maxZ0Cmp);
		m_MaxZ1 = spu_sel(m_MaxZ3, m_MaxZ1, maxZ1Cmp);
		m_MaxZ4 = spu_sel(m_MaxZ6, m_MaxZ4, maxZ2Cmp);
		m_MaxZ5 = spu_sel(m_MaxZ7, m_MaxZ5, maxZ3Cmp);

		const vec_ushort8 minZ4Cmp = spu_cmpgt(m_MinZ0, m_MinZ1);
		const vec_ushort8 minZ5Cmp = spu_cmpgt(m_MinZ4, m_MinZ5);

		m_MinZ0 = spu_sel(m_MinZ0, m_MinZ1, minZ4Cmp);
		m_MinZ4 = spu_sel(m_MinZ4, m_MinZ5, minZ5Cmp);
		vec_uint4 miniz0 = (vec_uint4)spu_shuffle(m_MinZ0, m_MinZ0, SHUFFLE_Z_TOINT0);
		vec_uint4 miniz1 = (vec_uint4)spu_shuffle(m_MinZ0, m_MinZ0, SHUFFLE_Z_TOINT1);
		vec_uint4 miniz2 = (vec_uint4)spu_shuffle(m_MinZ4, m_MinZ4, SHUFFLE_Z_TOINT0);
		vec_uint4 miniz3 = (vec_uint4)spu_shuffle(m_MinZ4, m_MinZ4, SHUFFLE_Z_TOINT1);

		const vec_ushort8 maxZ4Cmp = spu_cmpgt(m_MaxZ0, m_MaxZ1);
		const vec_ushort8 maxZ5Cmp = spu_cmpgt(m_MaxZ4, m_MaxZ5);

		m_MaxZ0 = spu_sel(m_MaxZ1, m_MaxZ0, maxZ4Cmp);
		m_MaxZ4 = spu_sel(m_MaxZ5, m_MaxZ4, maxZ5Cmp);
		vec_uint4 maxiz0 = (vec_uint4)spu_shuffle(m_MaxZ0, m_MaxZ0, SHUFFLE_Z_TOINT0);
		vec_uint4 maxiz1 = (vec_uint4)spu_shuffle(m_MaxZ0, m_MaxZ0, SHUFFLE_Z_TOINT1);
		vec_uint4 maxiz2 = (vec_uint4)spu_shuffle(m_MaxZ4, m_MaxZ4, SHUFFLE_Z_TOINT0);
		vec_uint4 maxiz3 = (vec_uint4)spu_shuffle(m_MaxZ4, m_MaxZ4, SHUFFLE_Z_TOINT1);

		const vec_uint4 miniz01Cmp = spu_cmpgt(miniz0, miniz1);
		const vec_uint4 miniz23Cmp = spu_cmpgt(miniz2, miniz3);
		const vec_uint4 maxiz01Cmp = spu_cmpgt(maxiz0, maxiz1);
		const vec_uint4 maxiz23Cmp = spu_cmpgt(maxiz2, maxiz3);

		m_pCurrentMinZin[tileYCounter]		= spu_sel(miniz0, miniz1, miniz01Cmp);
		m_pCurrentMinZin[tileYCounter+1]	= spu_sel(miniz2, miniz3, miniz23Cmp);
		m_pCurrentMaxZin[tileYCounter]		= spu_sel(maxiz1, maxiz0, maxiz01Cmp);
		m_pCurrentMaxZin[tileYCounter+1]	= spu_sel(maxiz3, maxiz2, maxiz23Cmp);

		m_pStencilMinIn[tileYCounter]		= m_StencilMin0;
		m_pStencilMaxIn[tileYCounter]		= m_StencilMax0;
		m_pStencilMinIn[tileYCounter+1]		= m_StencilMin1;
		m_pStencilMaxIn[tileYCounter+1]		= m_StencilMax1;

		const vec_uchar16 initialStencilMin = spu_splats((U8)0xFF);
		const vec_uchar16 initialStencilMax = spu_splats((U8)0x00);
		const vec_ushort8 initialMinZ = spu_splats((unsigned short)0xFFFF);
		const vec_ushort8 initialMaxZ = spu_splats((unsigned short)0x00);

		m_MinZ0 = initialMinZ;
		m_MinZ1 = initialMinZ;
		m_MinZ2 = initialMinZ;
		m_MinZ3 = initialMinZ;
		m_MinZ4 = initialMinZ;
		m_MinZ5 = initialMinZ;
		m_MinZ6 = initialMinZ;
		m_MinZ7 = initialMinZ;

		m_MaxZ0 = initialMaxZ;
		m_MaxZ1 = initialMaxZ;
		m_MaxZ2 = initialMaxZ;
		m_MaxZ3 = initialMaxZ;
		m_MaxZ4 = initialMaxZ;
		m_MaxZ5 = initialMaxZ;
		m_MaxZ6 = initialMaxZ;
		m_MaxZ7 = initialMaxZ;

		m_StencilMin0 = initialStencilMin;
		m_StencilMax0 = initialStencilMax;
		m_StencilMin1 = initialStencilMin;
		m_StencilMax1 = initialStencilMax;
	}

	INLINE void ProcessROPTile(vec_uchar16*& __restrict__ pOut,
									vec_uchar16 r00, vec_uchar16 r01, vec_uchar16 r02, vec_uchar16 r03,
									 vec_uchar16 r10, vec_uchar16 r11, vec_uchar16 r12, vec_uchar16 r13,
									 vec_uchar16 r20, vec_uchar16 r21, vec_uchar16 r22, vec_uchar16 r23,
									 vec_uchar16 r30, vec_uchar16 r31, vec_uchar16 r32, vec_uchar16 r33)
	{
		const vec_ushort8 z0 = (vec_ushort8)spu_shuffle(r00, r01, SHUFFLE_Z_TOSHORT);
		const vec_ushort8 z1 = (vec_ushort8)spu_shuffle(r02, r03, SHUFFLE_Z_TOSHORT);
		const vec_ushort8 z2 = (vec_ushort8)spu_shuffle(r10, r11, SHUFFLE_Z_TOSHORT);
		const vec_ushort8 z3 = (vec_ushort8)spu_shuffle(r12, r13, SHUFFLE_Z_TOSHORT);
		const vec_ushort8 z4 = (vec_ushort8)spu_shuffle(r20, r21, SHUFFLE_Z_TOSHORT);
		const vec_ushort8 z5 = (vec_ushort8)spu_shuffle(r22, r23, SHUFFLE_Z_TOSHORT);
		const vec_ushort8 z6 = (vec_ushort8)spu_shuffle(r30, r31, SHUFFLE_Z_TOSHORT);
		const vec_ushort8 z7 = (vec_ushort8)spu_shuffle(r32, r33, SHUFFLE_Z_TOSHORT);

		const vec_uchar16 s0 = spu_shuffle(r00, r01, SHUFFLE_STENCILS_0);
		const vec_uchar16 s1 = spu_shuffle(r02, r03, SHUFFLE_STENCILS_1);
		const vec_uchar16 s2 = spu_shuffle(r10, r11, SHUFFLE_STENCILS_0);
		const vec_uchar16 s3 = spu_shuffle(r12, r13, SHUFFLE_STENCILS_1);
		const vec_uchar16 s4 = spu_shuffle(r20, r21, SHUFFLE_STENCILS_0);
		const vec_uchar16 s5 = spu_shuffle(r22, r23, SHUFFLE_STENCILS_1);
		const vec_uchar16 s6 = spu_shuffle(r30, r31, SHUFFLE_STENCILS_0);
		const vec_uchar16 s7 = spu_shuffle(r32, r33, SHUFFLE_STENCILS_1);

		const vec_uchar16 stencils0 = (vec_uchar16)spu_or(s0, s1);
		const vec_uchar16 stencils1 = (vec_uchar16)spu_or(s2, s3);
		const vec_uchar16 stencils2 = (vec_uchar16)spu_or(s4, s5);
		const vec_uchar16 stencils3 = (vec_uchar16)spu_or(s6, s7);

		const vec_uchar16 stencilMin0Cmp0 = spu_cmpgt(stencils0, m_StencilMin0);
		const vec_uchar16 stencilMax0Cmp0 = spu_cmpgt(stencils0, m_StencilMax0);
		m_StencilMin0 = spu_sel(stencils0, m_StencilMin0, stencilMin0Cmp0);
		m_StencilMax0 = spu_sel(m_StencilMax0, stencils0, stencilMax0Cmp0);
		const vec_uchar16 stencilMin0Cmp1 = spu_cmpgt(stencils1, m_StencilMin0);
		const vec_uchar16 stencilMax0Cmp1 = spu_cmpgt(stencils1, m_StencilMax0);
		m_StencilMin0 = spu_sel(stencils1, m_StencilMin0, stencilMin0Cmp1);
		m_StencilMax0 = spu_sel(m_StencilMax0, stencils1, stencilMax0Cmp1);

		const vec_uchar16 stencilMin1Cmp0 = spu_cmpgt(stencils2, m_StencilMin0);
		const vec_uchar16 stencilMax1Cmp0 = spu_cmpgt(stencils2, m_StencilMax0);
		m_StencilMin1 = spu_sel(stencils2, m_StencilMin0, stencilMin1Cmp0);
		m_StencilMax1 = spu_sel(m_StencilMax0, stencils2, stencilMax1Cmp0);
		const vec_uchar16 stencilMin1Cmp1 = spu_cmpgt(stencils3, m_StencilMin0);
		const vec_uchar16 stencilMax1Cmp1 = spu_cmpgt(stencils3, m_StencilMax0);
		m_StencilMin1 = spu_sel(stencils3, m_StencilMin0, stencilMin1Cmp1);
		m_StencilMax1 = spu_sel(m_StencilMax0, stencils3, stencilMax1Cmp1);

		const vec_ushort8 minZ0Cmp = spu_cmpgt(m_MinZ0, z0);
		const vec_ushort8 minZ1Cmp = spu_cmpgt(m_MinZ1, z1);
		const vec_ushort8 minZ2Cmp = spu_cmpgt(m_MinZ2, z2);
		const vec_ushort8 minZ3Cmp = spu_cmpgt(m_MinZ3, z3);
		const vec_ushort8 minZ4Cmp = spu_cmpgt(m_MinZ4, z4);
		const vec_ushort8 minZ5Cmp = spu_cmpgt(m_MinZ5, z5);
		const vec_ushort8 minZ6Cmp = spu_cmpgt(m_MinZ6, z6);
		const vec_ushort8 minZ7Cmp = spu_cmpgt(m_MinZ7, z7);
		m_MinZ0 = spu_sel(m_MinZ0, z0, minZ0Cmp);
		m_MinZ1 = spu_sel(m_MinZ1, z1, minZ1Cmp);
		m_MinZ2 = spu_sel(m_MinZ2, z2, minZ2Cmp);
		m_MinZ3 = spu_sel(m_MinZ3, z3, minZ3Cmp);
		m_MinZ4 = spu_sel(m_MinZ4, z4, minZ4Cmp);
		m_MinZ5 = spu_sel(m_MinZ5, z5, minZ5Cmp);
		m_MinZ6 = spu_sel(m_MinZ6, z6, minZ6Cmp);
		m_MinZ7 = spu_sel(m_MinZ7, z7, minZ7Cmp);

		const vec_ushort8 maxZ0Cmp = spu_cmpgt(m_MaxZ0, z0);
		const vec_ushort8 maxZ1Cmp = spu_cmpgt(m_MaxZ1, z1);
		const vec_ushort8 maxZ2Cmp = spu_cmpgt(m_MaxZ2, z2);
		const vec_ushort8 maxZ3Cmp = spu_cmpgt(m_MaxZ3, z3);
		const vec_ushort8 maxZ4Cmp = spu_cmpgt(m_MaxZ4, z4);
		const vec_ushort8 maxZ5Cmp = spu_cmpgt(m_MaxZ5, z5);
		const vec_ushort8 maxZ6Cmp = spu_cmpgt(m_MaxZ6, z6);
		const vec_ushort8 maxZ7Cmp = spu_cmpgt(m_MaxZ7, z7);
		m_MaxZ0 = spu_sel(z0, m_MaxZ0, maxZ0Cmp);
		m_MaxZ1 = spu_sel(z1, m_MaxZ1, maxZ1Cmp);
		m_MaxZ2 = spu_sel(z2, m_MaxZ2, maxZ2Cmp);
		m_MaxZ3 = spu_sel(z3, m_MaxZ3, maxZ3Cmp);
		m_MaxZ4 = spu_sel(z4, m_MaxZ4, maxZ4Cmp);
		m_MaxZ5 = spu_sel(z5, m_MaxZ5, maxZ5Cmp);
		m_MaxZ6 = spu_sel(z6, m_MaxZ6, maxZ6Cmp);
		m_MaxZ7 = spu_sel(z7, m_MaxZ7, maxZ7Cmp);

		pOut[0] = r00;
		pOut[1] = r01;
		pOut[2] = r02;
		pOut[3] = r03;
		pOut[4] = r10;
		pOut[5] = r11;
		pOut[6] = r12;
		pOut[7] = r13;
		pOut[8] = r20;
		pOut[9] = r21;
		pOut[10] = r22;
		pOut[11] = r23;
		pOut[12] = r30;
		pOut[13] = r31;
		pOut[14] = r32;
		pOut[15] = r33;
		pOut += 16;
	}

private:

	vec_ushort8 m_MinZ0;
	vec_ushort8 m_MinZ1;
	vec_ushort8 m_MinZ2;
	vec_ushort8 m_MinZ3;
	vec_ushort8 m_MinZ4;
	vec_ushort8 m_MinZ5;
	vec_ushort8 m_MinZ6;
	vec_ushort8 m_MinZ7;

	vec_ushort8 m_MaxZ0;
	vec_ushort8 m_MaxZ1;
	vec_ushort8 m_MaxZ2;
	vec_ushort8 m_MaxZ3;
	vec_ushort8 m_MaxZ4;
	vec_ushort8 m_MaxZ5;
	vec_ushort8 m_MaxZ6;
	vec_ushort8 m_MaxZ7;

	vec_uchar16 m_StencilMin0;
	vec_uchar16 m_StencilMax0;
	vec_uchar16 m_StencilMin1;
	vec_uchar16 m_StencilMax1;

	vec_uint4* m_pCurrentMinZin;
	vec_uint4* m_pCurrentMaxZin;
	vec_uchar16* m_pStencilMinIn;
	vec_uchar16* m_pStencilMaxIn;

};

class DetileNormalsAndDepthBufferRangeProcessor
{
public:
	INLINE DetileNormalsAndDepthBufferRangeProcessor(vec_uint4* pCurrentMinZin, vec_uint4* pCurrentMaxZin)
		: m_pCurrentMinZin(pCurrentMinZin)
		, m_pCurrentMaxZin(pCurrentMaxZin)
	{
		const vec_ushort8 initialMinZ = spu_splats((unsigned short)0xFFFF);
		const vec_ushort8 initialMaxZ = spu_splats((unsigned short)0x0000);

		m_MinZ0 = initialMinZ;
		m_MinZ1 = initialMinZ;
		m_MinZ2 = initialMinZ;
		m_MinZ3 = initialMinZ;
		m_MinZ4 = initialMinZ;
		m_MinZ5 = initialMinZ;
		m_MinZ6 = initialMinZ;
		m_MinZ7 = initialMinZ;

		m_MaxZ0 = initialMaxZ;
		m_MaxZ1 = initialMaxZ;
		m_MaxZ2 = initialMaxZ;
		m_MaxZ3 = initialMaxZ;
		m_MaxZ4 = initialMaxZ;
		m_MaxZ5 = initialMaxZ;
		m_MaxZ6 = initialMaxZ;
		m_MaxZ7 = initialMaxZ;
	}

	INLINE void BeginProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void BeginProcessBlock16(const S32 tileYIndex)
	{
	}

	INLINE void EndProcessBlock8(const S32 tileYIndex)
	{
	}

	INLINE void EndProcessBlock16(const S32 tileYCounter)
	{
		const vec_ushort8 minZ0Cmp = spu_cmpgt(m_MinZ0, m_MinZ2);
		const vec_ushort8 minZ1Cmp = spu_cmpgt(m_MinZ1, m_MinZ3);
		const vec_ushort8 minZ2Cmp = spu_cmpgt(m_MinZ4, m_MinZ6);
		const vec_ushort8 minZ3Cmp = spu_cmpgt(m_MinZ5, m_MinZ7);

		m_MinZ0 = spu_sel(m_MinZ0, m_MinZ2, minZ0Cmp);
		m_MinZ1 = spu_sel(m_MinZ1, m_MinZ3, minZ1Cmp);
		m_MinZ4 = spu_sel(m_MinZ4, m_MinZ6, minZ2Cmp);
		m_MinZ5 = spu_sel(m_MinZ5, m_MinZ7, minZ3Cmp);

		const vec_ushort8 maxZ0Cmp = spu_cmpgt(m_MaxZ0, m_MaxZ2);
		const vec_ushort8 maxZ1Cmp = spu_cmpgt(m_MaxZ1, m_MaxZ3);
		const vec_ushort8 maxZ2Cmp = spu_cmpgt(m_MaxZ4, m_MaxZ6);
		const vec_ushort8 maxZ3Cmp = spu_cmpgt(m_MaxZ5, m_MaxZ7);

		m_MaxZ0 = spu_sel(m_MaxZ2, m_MaxZ0, maxZ0Cmp);
		m_MaxZ1 = spu_sel(m_MaxZ3, m_MaxZ1, maxZ1Cmp);
		m_MaxZ4 = spu_sel(m_MaxZ6, m_MaxZ4, maxZ2Cmp);
		m_MaxZ5 = spu_sel(m_MaxZ7, m_MaxZ5, maxZ3Cmp);

		const vec_ushort8 minZ4Cmp = spu_cmpgt(m_MinZ0, m_MinZ1);
		const vec_ushort8 minZ5Cmp = spu_cmpgt(m_MinZ4, m_MinZ5);

		m_MinZ0 = spu_sel(m_MinZ0, m_MinZ1, minZ4Cmp);
		m_MinZ4 = spu_sel(m_MinZ4, m_MinZ5, minZ5Cmp);
		vec_uint4 miniz0 = (vec_uint4)spu_shuffle(m_MinZ0, m_MinZ0, SHUFFLE_Z_TOINT0);
		vec_uint4 miniz1 = (vec_uint4)spu_shuffle(m_MinZ0, m_MinZ0, SHUFFLE_Z_TOINT1);
		vec_uint4 miniz2 = (vec_uint4)spu_shuffle(m_MinZ4, m_MinZ4, SHUFFLE_Z_TOINT0);
		vec_uint4 miniz3 = (vec_uint4)spu_shuffle(m_MinZ4, m_MinZ4, SHUFFLE_Z_TOINT1);

		const vec_ushort8 maxZ4Cmp = spu_cmpgt(m_MaxZ0, m_MaxZ1);
		const vec_ushort8 maxZ5Cmp = spu_cmpgt(m_MaxZ4, m_MaxZ5);

		m_MaxZ0 = spu_sel(m_MaxZ1, m_MaxZ0, maxZ4Cmp);
		m_MaxZ4 = spu_sel(m_MaxZ5, m_MaxZ4, maxZ5Cmp);
		vec_uint4 maxiz0 = (vec_uint4)spu_shuffle(m_MaxZ0, m_MaxZ0, SHUFFLE_Z_TOINT0);
		vec_uint4 maxiz1 = (vec_uint4)spu_shuffle(m_MaxZ0, m_MaxZ0, SHUFFLE_Z_TOINT1);
		vec_uint4 maxiz2 = (vec_uint4)spu_shuffle(m_MaxZ4, m_MaxZ4, SHUFFLE_Z_TOINT0);
		vec_uint4 maxiz3 = (vec_uint4)spu_shuffle(m_MaxZ4, m_MaxZ4, SHUFFLE_Z_TOINT1);

		const vec_uint4 miniz01Cmp = spu_cmpgt(miniz0, miniz1);
		const vec_uint4 miniz23Cmp = spu_cmpgt(miniz2, miniz3);
		const vec_uint4 maxiz01Cmp = spu_cmpgt(maxiz0, maxiz1);
		const vec_uint4 maxiz23Cmp = spu_cmpgt(maxiz2, maxiz3);

		m_pCurrentMinZin[tileYCounter]		= spu_sel(miniz0, miniz1, miniz01Cmp);
		m_pCurrentMinZin[tileYCounter+1]	= spu_sel(miniz2, miniz3, miniz23Cmp);
		m_pCurrentMaxZin[tileYCounter]		= spu_sel(maxiz1, maxiz0, maxiz01Cmp);
		m_pCurrentMaxZin[tileYCounter+1]	= spu_sel(maxiz3, maxiz2, maxiz23Cmp);

		const vec_ushort8 initialMinZ = spu_splats((unsigned short)0xFFFF);
		const vec_ushort8 initialMaxZ = spu_splats((unsigned short)0x0000);

		m_MinZ0 = initialMinZ;
		m_MinZ1 = initialMinZ;
		m_MinZ2 = initialMinZ;
		m_MinZ3 = initialMinZ;
		m_MinZ4 = initialMinZ;
		m_MinZ5 = initialMinZ;
		m_MinZ6 = initialMinZ;
		m_MinZ7 = initialMinZ;

		m_MaxZ0 = initialMaxZ;
		m_MaxZ1 = initialMaxZ;
		m_MaxZ2 = initialMaxZ;
		m_MaxZ3 = initialMaxZ;
		m_MaxZ4 = initialMaxZ;
		m_MaxZ5 = initialMaxZ;
		m_MaxZ6 = initialMaxZ;
		m_MaxZ7 = initialMaxZ;
	}

	INLINE void ProcessROPTile(vec_uchar16*& __restrict__ pOut,
		vec_uchar16 r00, vec_uchar16 r01, vec_uchar16 r02, vec_uchar16 r03,
		vec_uchar16 r10, vec_uchar16 r11, vec_uchar16 r12, vec_uchar16 r13,
		vec_uchar16 r20, vec_uchar16 r21, vec_uchar16 r22, vec_uchar16 r23,
		vec_uchar16 r30, vec_uchar16 r31, vec_uchar16 r32, vec_uchar16 r33)
	{
		static const vec_uchar16 pack_depth_16s = { 0x03, 0x00, 0x07, 0x04, 0x0B, 0x08, 0x0F, 0x0C, 0x13, 0x10, 0x17, 0x14, 0x1B, 0x18, 0x1F, 0x1C };

		const vec_ushort8 z0 = (vec_ushort8)spu_shuffle(r00, r01, pack_depth_16s);
		const vec_ushort8 z1 = (vec_ushort8)spu_shuffle(r02, r03, pack_depth_16s);
		const vec_ushort8 z2 = (vec_ushort8)spu_shuffle(r10, r11, pack_depth_16s);
		const vec_ushort8 z3 = (vec_ushort8)spu_shuffle(r12, r13, pack_depth_16s);
		const vec_ushort8 z4 = (vec_ushort8)spu_shuffle(r20, r21, pack_depth_16s);
		const vec_ushort8 z5 = (vec_ushort8)spu_shuffle(r22, r23, pack_depth_16s);
		const vec_ushort8 z6 = (vec_ushort8)spu_shuffle(r30, r31, pack_depth_16s);
		const vec_ushort8 z7 = (vec_ushort8)spu_shuffle(r32, r33, pack_depth_16s);

		const vec_ushort8 minZ0Cmp = spu_cmpgt(m_MinZ0, z0);
		const vec_ushort8 minZ1Cmp = spu_cmpgt(m_MinZ1, z1);
		const vec_ushort8 minZ2Cmp = spu_cmpgt(m_MinZ2, z2);
		const vec_ushort8 minZ3Cmp = spu_cmpgt(m_MinZ3, z3);
		const vec_ushort8 minZ4Cmp = spu_cmpgt(m_MinZ4, z4);
		const vec_ushort8 minZ5Cmp = spu_cmpgt(m_MinZ5, z5);
		const vec_ushort8 minZ6Cmp = spu_cmpgt(m_MinZ6, z6);
		const vec_ushort8 minZ7Cmp = spu_cmpgt(m_MinZ7, z7);
		m_MinZ0 = spu_sel(m_MinZ0, z0, minZ0Cmp);
		m_MinZ1 = spu_sel(m_MinZ1, z1, minZ1Cmp);
		m_MinZ2 = spu_sel(m_MinZ2, z2, minZ2Cmp);
		m_MinZ3 = spu_sel(m_MinZ3, z3, minZ3Cmp);
		m_MinZ4 = spu_sel(m_MinZ4, z4, minZ4Cmp);
		m_MinZ5 = spu_sel(m_MinZ5, z5, minZ5Cmp);
		m_MinZ6 = spu_sel(m_MinZ6, z6, minZ6Cmp);
		m_MinZ7 = spu_sel(m_MinZ7, z7, minZ7Cmp);

		const vec_ushort8 maxZ0Cmp = spu_cmpgt(m_MaxZ0, z0);
		const vec_ushort8 maxZ1Cmp = spu_cmpgt(m_MaxZ1, z1);
		const vec_ushort8 maxZ2Cmp = spu_cmpgt(m_MaxZ2, z2);
		const vec_ushort8 maxZ3Cmp = spu_cmpgt(m_MaxZ3, z3);
		const vec_ushort8 maxZ4Cmp = spu_cmpgt(m_MaxZ4, z4);
		const vec_ushort8 maxZ5Cmp = spu_cmpgt(m_MaxZ5, z5);
		const vec_ushort8 maxZ6Cmp = spu_cmpgt(m_MaxZ6, z6);
		const vec_ushort8 maxZ7Cmp = spu_cmpgt(m_MaxZ7, z7);
		m_MaxZ0 = spu_sel(z0, m_MaxZ0, maxZ0Cmp);
		m_MaxZ1 = spu_sel(z1, m_MaxZ1, maxZ1Cmp);
		m_MaxZ2 = spu_sel(z2, m_MaxZ2, maxZ2Cmp);
		m_MaxZ3 = spu_sel(z3, m_MaxZ3, maxZ3Cmp);
		m_MaxZ4 = spu_sel(z4, m_MaxZ4, maxZ4Cmp);
		m_MaxZ5 = spu_sel(z5, m_MaxZ5, maxZ5Cmp);
		m_MaxZ6 = spu_sel(z6, m_MaxZ6, maxZ6Cmp);
		m_MaxZ7 = spu_sel(z7, m_MaxZ7, maxZ7Cmp);

		pOut[0] = r00;
		pOut[1] = r01;
		pOut[2] = r02;
		pOut[3] = r03;
		pOut[4] = r10;
		pOut[5] = r11;
		pOut[6] = r12;
		pOut[7] = r13;
		pOut[8] = r20;
		pOut[9] = r21;
		pOut[10] = r22;
		pOut[11] = r23;
		pOut[12] = r30;
		pOut[13] = r31;
		pOut[14] = r32;
		pOut[15] = r33;
		pOut += 16;
	}

private:

	vec_ushort8 m_MinZ0;
	vec_ushort8 m_MinZ1;
	vec_ushort8 m_MinZ2;
	vec_ushort8 m_MinZ3;
	vec_ushort8 m_MinZ4;
	vec_ushort8 m_MinZ5;
	vec_ushort8 m_MinZ6;
	vec_ushort8 m_MinZ7;

	vec_ushort8 m_MaxZ0;
	vec_ushort8 m_MaxZ1;
	vec_ushort8 m_MaxZ2;
	vec_ushort8 m_MaxZ3;
	vec_ushort8 m_MaxZ4;
	vec_ushort8 m_MaxZ5;
	vec_ushort8 m_MaxZ6;
	vec_ushort8 m_MaxZ7;

	vec_uint4* m_pCurrentMinZin;
	vec_uint4* m_pCurrentMaxZin;

};

// DetileBuffer
// Take an FB tile as input, detile it and output it to a linear buffer.
template <class Processor> INLINE void DetileBuffer(Processor& processor, const void* pInput, void* __restrict pOutput, const S32 blockIndex)
{
	// An FP tile in main memory contains 8x16 rop tiles. (A vram FB tile contains 8x8 rop tiles).
	// A ROP tile contains 8x4 pixels arranged linearly like so:
	//  0  1  2  3  4  5  6  7
	//  8  9 10 11 12 13 14 15
	// 16 17 18 19 20 21 22 23
	// 24 25 26 27 28 29 30 31

	// The ROP tiles are not arranged linearly, but are offset in a repeating pattern, in pairs, in the X axis.
	// The block arrangement follows this pattern every two rows of rop tiles:
	// 0,1,2,3
	// 2,3,0,1
	// 1,0,3,2
	// 3,2,1,0
	// So, for the second arrangement as an example, the first pixels in x for the linear output must be read from the ROP tile pair 2
	// (ROP tiles 4 and 5). The second pixels are from pair 3 (ROP tiles 6 and 7); the third are from pair 0 (ROP tiles 0 and 1) and
	// the fourth are from pair 1 (ROP tiles 2 and 3).
	// This arrangement only changes every 2 rows of ROP tiles - i.e. every 8 rows of pixels.
	// For pairs 0 and 2, the tiles read sequentially. But, for pairs 1 and 3, the tile on the second row is read first, and the tile
	// on the first row is read 2nd.
	// For example, the complete tile layout for the second block arrangement is as follows:
	//  4  5  14 15  0  1 10 11
	// 12 13   6  7  8  9  2  3

	const U8* pIn = (U8*)pInput;
	vec_uchar16* pOut = (vec_uchar16*)pOutput;

	S32 blockIdx = (blockIndex&1)<<1;
	const vec_uint4 arrangement0 = (vec_uint4){0,1,2,3};
	const vec_uint4 arrangement1 = (vec_uint4){1,0,3,2};

	S32 y = 0;
	for(S32 tileYCounter = 0; tileYCounter < 8; tileYCounter += 2)
	{
		processor.BeginProcessBlock16(tileYCounter);

		for(S32 y2 = 0; y2 < 2; ++y2, ++blockIdx, y += ROP_TILE_ROWS*2 )
		{
			blockIdx = blockIdx & 3;

			// Find the correct block arrangement.
			vec_uint4 blockArrangement = spu_sel(arrangement0, arrangement1, (vec_uint4)spu_splats(blockIdx >= 2));
			blockArrangement = spu_rlqwbyte(blockArrangement, (blockIdx & 1) << 3);

			// Get the offsets for the 4 pairs of ROP tiles.
			const U8* rop0 = pIn + (ROP_TILE_SIZE2 * spu_extract(blockArrangement, 0) * 2);
			const U8* rop1 = pIn + (ROP_TILE_SIZE2 * spu_extract(blockArrangement, 1) * 2);
			const U8* rop2 = pIn + (ROP_TILE_SIZE2 * spu_extract(blockArrangement, 2) * 2);
			const U8* rop3 = pIn + (ROP_TILE_SIZE2 * spu_extract(blockArrangement, 3) * 2);
			const vec_uchar16* ropPtr0 = (vec_uchar16*)rop0;
			const vec_uchar16* ropPtr1 = (vec_uchar16*)rop1;
			const vec_uchar16* ropPtr2 = (vec_uchar16*)rop2;
			const vec_uchar16* ropPtr3 = (vec_uchar16*)rop3;

			// For odd numbered tiles, tile row 2 is read first.
			ropPtr1 += ROP_TILE_SIZE_UCHAR16;
			ropPtr3 += ROP_TILE_SIZE_UCHAR16;

			processor.BeginProcessBlock8(tileYCounter);

			for(S32 r2 = 0; r2 < 2; ++r2)
			{
				for(S32 r = 0; r < ROP_TILE_ROWS*2; r += 2)
				{
					// Read a whole row of each tile at once.
					// A row is 8 pixels, so can be loaded into two uchar16s.

					// Pair 0:
					// First read a row from the first tile in the pair.
					const vec_uchar16 r00 = ropPtr0[0];
					const vec_uchar16 r01 = ropPtr0[1];
					// Now read a row from the second tile in the pair.
					const vec_uchar16 r02 = ropPtr0[0 + ROP_TILE_SIZE2_UCHAR16];
					const vec_uchar16 r03 = ropPtr0[1 + ROP_TILE_SIZE2_UCHAR16];

					// Pair 1
					const vec_uchar16 r10 = ropPtr1[0];
					const vec_uchar16 r11 = ropPtr1[1];
					const vec_uchar16 r12 = ropPtr1[0 + ROP_TILE_SIZE2_UCHAR16];
					const vec_uchar16 r13 = ropPtr1[1 + ROP_TILE_SIZE2_UCHAR16];

					// Pair 2
					const vec_uchar16 r20 = ropPtr2[0];
					const vec_uchar16 r21 = ropPtr2[1];
					const vec_uchar16 r22 = ropPtr2[0 + ROP_TILE_SIZE2_UCHAR16];
					const vec_uchar16 r23 = ropPtr2[1 + ROP_TILE_SIZE2_UCHAR16];

					// Pair 3
					const vec_uchar16 r30 = ropPtr3[0];
					const vec_uchar16 r31 = ropPtr3[1];
					const vec_uchar16 r32 = ropPtr3[0 + ROP_TILE_SIZE2_UCHAR16];
					const vec_uchar16 r33 = ropPtr3[1 + ROP_TILE_SIZE2_UCHAR16];

					processor.ProcessROPTile(pOut, r00, r01, r02, r03, r10, r11, r12, r13, r20, r21, r22, r23, r30, r31, r32, r33);

					ropPtr0 += 2;
					ropPtr1 += 2;
					ropPtr2 += 2;
					ropPtr3 += 2;
				}

				// For odd numbered tiles, tile row 2 is read first. Reset to read tile row 1 here.
				ropPtr1 = ((vec_uchar16*)rop1);
				ropPtr3 = ((vec_uchar16*)rop3);
			}

			processor.EndProcessBlock8(tileYCounter);
			pIn += (FB_TILE_STRIDE * 2);
		}
		processor.EndProcessBlock16(tileYCounter);
	}
}

// Given a tile location in linear space (x,y) and a bank offset for the buffer, locate the framebuffer tile offset.
INLINE U32 GetFBTileIndexForLinearTile(const U32 tileX, const U32 tileY, const U32 bankOffset)
{
	const vec_uint4 blockIndicesVector = (vec_uint4){0,2,1,3};
	const S32 fbyBanked = (tileY+bankOffset)&3;

	vec_uint4 blockIndices = spu_rlqwbyte(blockIndicesVector, fbyBanked*-4);
	vec_uint4 blockIndicesOut = spu_splats((U32)0x00000000);

	blockIndicesOut = spu_insert(0, blockIndicesOut, spu_extract(blockIndices,0));
	blockIndicesOut = spu_insert(1, blockIndicesOut, spu_extract(blockIndices,1));
	blockIndicesOut = spu_insert(2, blockIndicesOut, spu_extract(blockIndices,2));
	blockIndicesOut = spu_insert(3, blockIndicesOut, spu_extract(blockIndices,3));

	U32 blockIndexOut = spu_extract(blockIndicesOut, tileX&3);
	return blockIndexOut;
}

INLINE U32 GetTileLocation(const S32 tx, const S32 ty, const S32 numTilesX, const S32 bankOffset)
{
	U32 tileReadOffset = (tx & ~3) + (ty * numTilesX);
	U32 bankOff = GetFBTileIndexForLinearTile(tx, ty, bankOffset);
	return tileReadOffset + bankOff;
}

INLINE U32 GetNextTileLocation(const S32 tx, const S32 ty, const S32 numTilesX, const S32 bankOffset)
{
	S32 nextTx = tx + 1;
	S32 nextTy = ty;
	if(nextTx == numTilesX)
	{
		nextTx = 0;
		++nextTy;
	}
	return GetTileLocation(nextTx, nextTy, numTilesX, bankOffset);
}

}; // namespace Spu

#endif // PLATFORM_SPU

#endif // GRAPHICS_SPU_TEXTURE_TILE_H
