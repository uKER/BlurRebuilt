#include "stdafx.h"
#include "Deferred/Prepass/SPU/JobQueue.h"

#include "Core/Threads/Atomic.h"
#include "Spu/Texture/Tile.h"
#include "Spu/Buffer.h"
#include "Spu/Common.h"
#include "Spu/Maths.h"
#include "System/Scheduler/JobQueue.h"


//#define NO_CULLING TRUE



namespace Deferred
{

const U32 c_MaximumVisibleLights = 8;

struct VisibleLights
{
	LightingData::PointLight	pointLights[c_MaximumVisibleLights];
	LightingData::SpotLight		spotLights[c_MaximumVisibleLights];
	U32							pointLightCount;
	U32							spotLightCount;
};

#if defined(PLATFORM_SPU)

void Generate2dFrustumPlane(qword& nx, qword& nz, qword& nw, const qword x0, const qword z0, const qword x1, const qword z1) __attribute__((always_inline));
void Generate2dFrustumPlane(qword& nx, qword& nz, qword& nw, const qword x0, const qword z0, const qword x1, const qword z1)		// TODO: [Stove] change to ptrs and restrict.
{
	qword dx		= si_fs(x1, x0);
	qword dz		= si_fs(z1, z0);
	qword len		= si_fma(dx, dx, si_fm(dz, dz));
	qword len_rcp	= si_frsqest(len);
		  len_rcp	= si_fi(len, len_rcp);
		  dx		= si_fm(dx, len_rcp);
		  dz		= si_fm(dz, len_rcp);
		  nx		= si_fs(zero_float, dz);
		  nz		= dx;
		  nw		= si_fma(x0, nx, si_fm(z0, nz));
	return;
}

void GatherLights(VisibleLights* __restrict__ pLights, 
				  const LightingData::PointLight* __restrict__ pPointLights, 
				  const uint32_t pointLightCount, 
				  LightingData::SpotLight* __restrict__ pSpotLights, 
				  const uint32_t spotLightCount, 
				  const qword x0, 
				  const qword x1, 
				  const qword y0, 
				  const qword y1, 
				  const qword z_min, 
				  const qword z_max)
{
	const qword x00	= si_fm(x0, z_min);
	const qword x01	= si_fm(x1, z_min);
	const qword x10	= si_fm(x0, z_max);
	const qword x11	= si_fm(x1, z_max);
	const qword y00	= si_fm(y0, z_min);
	const qword y01	= si_fm(y1, z_min);
	const qword y10	= si_fm(y0, z_max);
	const qword y11	= si_fm(y1, z_max);
	const vector unsigned char _mask_000f = 
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff };
	const qword mask_000f = (const qword)_mask_000f;
	
	qword plane_left_nx, plane_left_nz, plane_left_nw;
	qword plane_right_nx, plane_right_nz, plane_right_nw;
	qword plane_up_ny, plane_up_nz, plane_up_nw;
	qword plane_down_ny, plane_down_nz, plane_down_nw;
	Generate2dFrustumPlane(plane_left_nx, plane_left_nz, plane_left_nw, x00, z_min, x10, z_max);
	Generate2dFrustumPlane(plane_right_nx, plane_right_nz, plane_right_nw, x11, z_max, x01, z_min);
	Generate2dFrustumPlane(plane_up_ny, plane_up_nz, plane_up_nw, y10, z_max, y00, z_min);
	Generate2dFrustumPlane(plane_down_ny, plane_down_nz, plane_down_nw, y01, z_min, y11, z_max);

	// gather point lights
	{
		qword light_ptr 			= (const qword)si_from_ptr(&pPointLights[0].f[0]);
		qword light_colour			= si_lqd(light_ptr, 16);
		qword light_position		= si_lqd(light_ptr, 0);
		qword light_position_x		= si_shufb(light_position, light_position, shuffle_xxxx);
		qword light_position_y		= si_shufb(light_position, light_position, shuffle_yyyy);
		qword light_position_z		= si_shufb(light_position, light_position, shuffle_zzzz);
		qword light_radius			= si_shufb(light_colour, light_colour, shuffle_wwww);
		qword light_intensity		= si_shufb(light_position, light_position, shuffle_wwww);
		qword cmp_intensity			= si_fcgt(light_intensity, zero_float);
			  
		pLights->pointLightCount = 0;
		for(S32 i = 0; i < pointLightCount && pLights->pointLightCount < c_MaximumVisibleLights; ++i)
		{
			pLights->pointLights[pLights->pointLightCount] = pPointLights[i];
	
			const qword plane_dot_left	= si_fma(light_position_x, plane_left_nx, si_fma(light_position_z, plane_left_nz, si_fs(plane_left_nw, light_radius)));
			const qword plane_dot_right	= si_fma(light_position_x, plane_right_nx, si_fma(light_position_z, plane_right_nz, si_fs(plane_right_nw, light_radius)));
			const qword plane_dot_up	= si_fma(light_position_y, plane_up_ny, si_fma(light_position_z, plane_up_nz, si_fs(plane_up_nw, light_radius)));
			const qword plane_dot_down	= si_fma(light_position_y, plane_down_ny, si_fma(light_position_z, plane_down_nz, si_fs(plane_down_nw, light_radius)));

			const qword cmp_near		= si_fcgt(z_max, si_fs(light_position_z, light_radius));
			const qword cmp_far			= si_fcgt(si_fa(light_position_z, light_radius), z_min);
			const qword cmp_left		= si_fcgt(zero_float, plane_dot_left);
			const qword cmp_right		= si_fcgt(zero_float, plane_dot_right);
			const qword cmp_up			= si_fcgt(zero_float, plane_dot_up);
			const qword cmp_down		= si_fcgt(zero_float, plane_dot_down);
			const qword in_tile_rslt	= si_and(cmp_intensity, si_and(si_and(si_and(cmp_left, cmp_right), si_and(cmp_up, cmp_down)), si_and(cmp_near, cmp_far)));

			light_ptr 					= (const qword)si_from_ptr(&pPointLights[i+1].f[0]);
			light_colour				= si_lqd(light_ptr, 16);
			light_position				= si_lqd(light_ptr, 0);
			light_position_x			= si_shufb(light_position, light_position, shuffle_xxxx);
			light_position_y			= si_shufb(light_position, light_position, shuffle_yyyy);
			light_position_z			= si_shufb(light_position, light_position, shuffle_zzzz);
			light_radius				= si_shufb(light_colour, light_colour, shuffle_wwww);
										  
			pLights->pointLightCount += si_to_uint(in_tile_rslt) & 0x01;
		}
	}

	// gather spot lights
	{
		qword light_ptr 			= (const qword)si_from_ptr(&pSpotLights[0].f[0]);
		qword light_colour			= si_lqd(light_ptr, 0x80);
		qword light_position_x		= si_lqd(light_ptr, 0x00);
		qword light_position_y		= si_lqd(light_ptr, 0x10);
		qword light_position_z		= si_lqd(light_ptr, 0x20);
		qword light_radius			= si_shufb(light_colour, light_colour, shuffle_wwww);
		
		pLights->spotLightCount = 0;
		for(S32 i = 0; i < spotLightCount && pLights->spotLightCount < c_MaximumVisibleLights; ++i)
		{
			pLights->spotLights[pLights->spotLightCount] = pSpotLights[i];
	
			const qword plane_dot_left	= si_fma(light_position_x, plane_left_nx, si_fma(light_position_z, plane_left_nz, si_fs(plane_left_nw, light_radius)));
			const qword plane_dot_right	= si_fma(light_position_x, plane_right_nx, si_fma(light_position_z, plane_right_nz, si_fs(plane_right_nw, light_radius)));
			const qword plane_dot_up	= si_fma(light_position_y, plane_up_ny, si_fma(light_position_z, plane_up_nz, si_fs(plane_up_nw, light_radius)));
			const qword plane_dot_down	= si_fma(light_position_y, plane_down_ny, si_fma(light_position_z, plane_down_nz, si_fs(plane_down_nw, light_radius)));

			const qword cmp_near		= si_fcgt(z_max, si_fs(light_position_z, light_radius));
			const qword cmp_far			= si_fcgt(si_fa(light_position_z, light_radius), z_min);
			const qword cmp_left		= si_fcgt(zero_float, plane_dot_left);
			const qword cmp_right		= si_fcgt(zero_float, plane_dot_right);
			const qword cmp_up			= si_fcgt(zero_float, plane_dot_up);
			const qword cmp_down		= si_fcgt(zero_float, plane_dot_down);
			const qword in_tile_rslt	= si_and(si_and(si_and(cmp_left, cmp_right), si_and(cmp_up, cmp_down)), si_and(cmp_near, cmp_far));
		
			light_ptr 					= (const qword)si_from_ptr(&pSpotLights[i+1].f[0]);
			qword light_position_x		= si_lqd(light_ptr, 0x00);
			qword light_position_y		= si_lqd(light_ptr, 0x10);
			qword light_position_z		= si_lqd(light_ptr, 0x20);
			qword light_colour			= si_lqd(light_ptr, 0x80);
			light_radius				= si_shufb(light_colour, light_colour, shuffle_wwww);
			
#ifndef NO_CULLING
			pLights->spotLightCount += si_to_uint(in_tile_rslt) & 0x01;
#else
			pLights->spotLightCount += 1;
#endif
		}
	}
}

void ProcessTileUnlit(U8* __restrict__ pOutput, const U32 tileWidth, const U32 tileHeight, const U32 tileStride)
{
	U8* __restrict__ pRowOutput = pOutput;
	for(U32 y = 0; y < tileHeight; ++y)
	{
		memset(pRowOutput, 0x00000000, tileWidth * sizeof(U32));
		pRowOutput += tileStride;
	}
}

void ProcessTileLitLod(U8* pOutput, const U32 tileWidth, const U32 tileHeight, const U32 tileStride)
{
	U8* pRowOutput = pOutput;
	for(U32 y = 0; y < tileHeight; ++y)
	{		
		memset(pRowOutput, 0xffffffff, tileWidth * sizeof(U32));
		pRowOutput += tileStride;
	}
	
	return;
}

U8	tiledNormalBuffer[c_FBTileSize] ALIGN128;
U8	tiledDepthBuffer[c_FBTileSize] ALIGN128;
U8	normalBuffer[c_FBTileSize*2] ALIGN128;
U8	depthBuffer[c_FBTileSize*2] ALIGN128;

static inline const uint32_t qword_to_u32(const qword q) __attribute__((pure, always_inline, const));
static inline const uint32_t qword_to_u32(const qword q)
{
	qword f						= si_cfltu(q, 0x00);
	qword shuf_uint				= si_ilhu(0x0f03);
		  shuf_uint				= si_iohl(shuf_uint, 0x070b);
	qword uint					= si_shufb(f, f, shuf_uint);
	return si_to_uint(uint);
}

static inline const qword frac(const qword x) __attribute__((pure, const, always_inline));
static inline const qword frac(const qword  x)
{
	qword frac				= si_cfltu(x, 0x0);
	qword frac_f			= si_cuflt(frac, 0x0);
	qword result			= si_fs(x, frac_f);
	return result;
}
	
//! Convert a U32 to qword no range decompression.
//@param u U32 value to convert.
//@return qword representation of value.
static inline const qword u32_to_qword(const uint32_t u) __attribute__((pure, always_inline, const));
static inline const qword u32_to_qword(const uint32_t u)
{
	const vector unsigned char _shuf_qword = 
	{
		0x80, 0x80, 0x80, 0x00,
		0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x02,
		0x80, 0x80, 0x80, 0x03
	};
	qword q						= si_from_uint(u);
	qword shuf_qword			= (const qword)_shuf_qword;
	qword rcp_255				= si_ilhu(0x3b80);
		  rcp_255				= si_iohl(rcp_255, 0x8080);
	qword q_shuf				= si_shufb(q, q, shuf_qword);
	qword f						= si_cuflt(q_shuf, 0x00);
	qword fm					= si_fm(f, rcp_255);
	return fm;
}

//! Convert a U32 to qword no range decompression, extra 24bit float from U32.
//@param u U32 value to convert.
//@return qword representation of value.
static inline const qword u32_to_qword_depth(const uint32_t u, const qword rcp_farminusnear, const qword rcp_far) __attribute__((pure, always_inline, const));
static inline const qword u32_to_qword_depth(const uint32_t u, const qword rcp_farminusnear, const qword rcp_far)
{
	qword q						= si_from_uint(u);
	qword shuf_pat				= si_ila(0x010203);
	qword q_shuf				= si_shufb(q, q, shuf_pat);
		  q_shuf				= si_andi(q_shuf, -0x0100);
	qword depth_val				= si_cuflt(q_shuf, 0x20);	// Saturate.
	return depth_val;
}
	
//! Convert a U32 to qword with range decompression.
//@param u U32 value to convert.
//@return qword representation of value.
static inline const qword u32_to_qword_rd(const uint32_t u) __attribute__((pure, always_inline, const));
static inline const qword u32_to_qword_rd(const uint32_t u)
{
	const vector unsigned char _shuf_qword = 
	{
		0x80, 0x80, 0x80, 0x01,
		0x80, 0x80, 0x80, 0x02,
		0x80, 0x80, 0x80, 0x03,
		0x80, 0x80, 0x80, 0x00
	};
	qword q						= si_from_uint(u);
	qword shuf_qword			= (const qword)_shuf_qword;
	qword rcp_127				= si_ilhu(0x3C00);
		  rcp_127				= si_iohl(rcp_127, 0x8080);
	qword one					= si_ilhu(0x3f80);
	qword q_shuf				= si_shufb(q, q, shuf_qword);
	qword f						= si_cuflt(q_shuf, 0x00);
	qword fm					= si_fm(f, rcp_127);	// Folded the 2* from range decompression.
		  fm					= si_fs(fm, one);
	return fm;
}


//! lit4, apply lighting calculations to a batch of 4 pixels -- 4 cycle stall outside of loop, 14 cycles inside of loop.
static inline void lit(const qword normal0, const uint16_t x0, 
				 	   const qword normal1, const uint16_t x1, 
					   const qword normal2, const uint16_t x2, 
					   const qword normal3, const uint16_t x3, 
					   const qword normal4, const uint16_t x4, 
					   const qword normal5, const uint16_t x5, 
					   const qword normal6, const uint16_t x6, 
					   const qword normal7, const uint16_t x7, 
					   const qword normal8, const uint16_t x8, 
					   const qword normal9, const uint16_t x9, 
					   const qword normal10, const uint16_t x10, 
					   const qword normal11, const uint16_t x11, 
					   const qword normal12, const uint16_t x12, 
					   const qword normal13, const uint16_t x13, 
					   const qword normal15, const uint16_t x14, 
					   const qword normal15, const uint16_t x15, 

					   const qword uv_y0y1y2y3_in,
					   
					   const qword uv_z0z1z2z3,
					   const qword uv_z4z5z6z7,
					   const qword uv_z8z9z10z11,
					   const qword uv_z12z13z14z15,

					   uint32_t* __restrict__ pResult,	

					   VisibleLights* __restrict__ pVisibleLights, 
					   
					   const qword inv_far_minus_inv_near, 
					   const qword inv_near, 
					   const F32* const __restrict pMatrix) __attribute__((always_inline));
static inline void lit(const qword normal0, const uint16_t x0, 
					   const qword normal1, const uint16_t x1, 
					   const qword normal2, const uint16_t x2, 
					   const qword normal3, const uint16_t x3, 
					   const qword normal4, const uint16_t x4, 
					   const qword normal5, const uint16_t x5, 
					   const qword normal6, const uint16_t x6, 
					   const qword normal7, const uint16_t x7, 
					   const qword normal8, const uint16_t x8, 
					   const qword normal9, const uint16_t x9, 
					   const qword normal10, const uint16_t x10, 
					   const qword normal11, const uint16_t x11, 
					   const qword normal12, const uint16_t x12, 
					   const qword normal13, const uint16_t x13, 
					   const qword normal14, const uint16_t x14, 
					   const qword normal15, const uint16_t x15, 

					   const qword uv_y0y1y2y3_in,
					   
					   const qword uv_z0z1z2z3,
					   const qword uv_z4z5z6z7,
					   const qword uv_z8z9z10z11,
					   const qword uv_z12z13z14z15,

					   uint32_t* __restrict__ pResult,	

					   VisibleLights* __restrict__ pVisibleLights, 

					   const qword inv_far_minus_inv_near, 
					   const qword inv_near, 
					   const F32* const __restrict pMatrix)
{
	const vector unsigned char _mask_00ff =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	const vector unsigned char _mask_000f =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff };
	const vector unsigned char _to_screen_x =			// 1/1280 * 2
	{ 0x3A, 0xCC, 0xCC, 0xCD, 0x3A, 0xCC, 0xCC, 0xCD, 0x3A, 0xCC, 0xCC, 0xCD, 0x3A, 0xCC, 0xCC, 0xCD };
	const vector unsigned char _to_screen_y =			// 1/720 * -2
	{ 0xBB, 0x36, 0x0B, 0x61, 0xBB, 0x36, 0x0B, 0x61, 0xBB, 0x36, 0x0B, 0x61, 0xBB, 0x36, 0x0B, 0x61 };
	const vector unsigned char _shuf_AaBb = 
	{ 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x04, 0x05, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17 };
	const vector unsigned char _shuf_CcDd = 
	{ 0x08, 0x09, 0x0a, 0x0b, 0x18, 0x19, 0x1a, 0x1b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1c, 0x1d, 0x1e, 0x1f };
	const vector unsigned char _shuf_ABa0 = 
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80 };
	const vector unsigned char _shuf_CDb0 = 
	{ 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x14, 0x15, 0x16, 0x17, 0x80, 0x80, 0x80, 0x80 };
	const vector unsigned char _shuf_ABc0 = 
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x18, 0x19, 0x1a, 0x1b, 0x80, 0x80, 0x80, 0x80 };
	const vector unsigned char _shuf_CDd0 = 
	{ 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1c, 0x1d, 0x1e, 0x1f, 0x80, 0x80, 0x80, 0x80 };
	const vector unsigned char _shuf_AaAa = 
	{ 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13 };
	const vector unsigned char _shuf_CcCc = 
	{ 0x08, 0x09, 0x0a, 0x0b, 0x18, 0x19, 0x1a, 0x1b, 0x08, 0x09, 0x0a, 0x0b, 0x18, 0x19, 0x1a, 0x1b };
	const vector unsigned char _shuf_CDab = 
	{ 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 };
	const vector unsigned char _shuf_BbAa = 
	{ 0x04, 0x05, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17, 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13 };

		  qword addr						= si_from_ptr(pMatrix);
		  qword matrix_0					= si_lqd(addr, 0x00);
		  qword matrix_1					= si_lqd(addr, 0x10);
		  qword z_bits						= si_ilhu(0xffff);											// load constants.
				z_bits						= si_iohl(z_bits, 0xff00);
	const qword mask_00ff					= (const qword)_mask_00ff;
	const qword shuf_xxxx					= si_ila(0x010203);
		  qword shuf_yyyy					= si_ilhu(0x0405);
			    shuf_yyyy					= si_iohl(shuf_yyyy, 0x0607);
		  qword shuf_zzzz					= si_ilhu(0x0809);
				shuf_zzzz					= si_iohl(shuf_zzzz, 0x0a0b);
		  qword shuf_wwww					= si_ilhu(0x0c0d);
				shuf_wwww					= si_iohl(shuf_wwww, 0x0e0f);
	const qword zero						= si_il(0x0000);
	const qword point_two_five				= si_ilhu(0x3e80);
	const qword point_seven_five			= si_ilhu(0x3f40);
		  qword diffuse0					= zero;
		  qword diffuse1					= zero;
		  qword diffuse2					= zero;
		  qword diffuse3					= zero;  
		  qword diffuse4					= zero;
		  qword diffuse5					= zero;
		  qword diffuse6					= zero;
		  qword diffuse7					= zero;  
		  qword diffuse8					= zero;
		  qword diffuse9					= zero;
		  qword diffuse10					= zero;
		  qword diffuse11					= zero;  
		  qword diffuse12					= zero;
		  qword diffuse13					= zero;
		  qword diffuse14					= zero;
		  qword diffuse15					= zero;  
	const qword one							= si_ilhu(0x3f80);
	const qword two							= si_ilhu(0x4000);
	const qword shuf_AaAa					= (const qword)_shuf_AaAa;
	const qword shuf_CcCc					= (const qword)_shuf_CcCc;
	const qword shuf_AaBb					= (const qword)_shuf_AaBb;
	const qword shuf_BbAa					= (const qword)_shuf_BbAa;
	const qword shuf_CcDd					= (const qword)_shuf_CcDd;
	const qword shuf_ABa0					= (const qword)_shuf_ABa0;
	const qword shuf_CDb0					= (const qword)_shuf_CDb0;
	const qword shuf_ABc0					= (const qword)_shuf_ABc0;
	const qword shuf_CDd0					= (const qword)_shuf_CDd0;
	const qword shuf_CDab					= (const qword)_shuf_CDab;
	const qword to_screen_x					= (const qword)_to_screen_x;
	const qword to_screen_y					= (const qword)_to_screen_y;
			    matrix_0					= si_shufb(matrix_0, matrix_0, shuf_xxxx);
			    matrix_1					= si_shufb(matrix_1, matrix_1, shuf_yyyy);
		  qword uv_x0x1						= si_shufb(si_from_uint(x0), si_from_uint(x1), shuf_AaAa);
		  qword uv_x2x3						= si_shufb(si_from_uint(x2), si_from_uint(x3), shuf_AaAa);
		  qword uv_x4x5						= si_shufb(si_from_uint(x4), si_from_uint(x5), shuf_AaAa);
		  qword uv_x6x7						= si_shufb(si_from_uint(x6), si_from_uint(x7), shuf_AaAa);
		  qword uv_x8x9						= si_shufb(si_from_uint(x8), si_from_uint(x9), shuf_AaAa);
		  qword uv_x10x11					= si_shufb(si_from_uint(x10), si_from_uint(x11), shuf_AaAa);
		  qword uv_x12x13					= si_shufb(si_from_uint(x12), si_from_uint(x13), shuf_AaAa);
		  qword uv_x14x15					= si_shufb(si_from_uint(x14), si_from_uint(x15), shuf_AaAa);
		  qword uv_x0x1x2x3					= si_selb(uv_x0x1, uv_x2x3, mask_00ff);
		  qword uv_x4x5x6x7					= si_selb(uv_x4x5, uv_x6x7, mask_00ff);
		  qword uv_x8x9x10x11				= si_selb(uv_x8x9, uv_x10x11, mask_00ff);
		  qword uv_x12x13x14x15				= si_selb(uv_x12x13, uv_x14x15, mask_00ff);
				uv_x0x1x2x3					= si_cuflt(uv_x0x1x2x3, 0x0);
				uv_x4x5x6x7					= si_cuflt(uv_x4x5x6x7, 0x0);
				uv_x8x9x10x11				= si_cuflt(uv_x8x9x10x11, 0x0);
				uv_x12x13x14x15				= si_cuflt(uv_x12x13x14x15, 0x0);
		  		uv_x0x1x2x3					= si_fms(uv_x0x1x2x3, to_screen_x, one);
				uv_x4x5x6x7					= si_fms(uv_x4x5x6x7, to_screen_x, one);
				uv_x8x9x10x11				= si_fms(uv_x8x9x10x11, to_screen_x, one);
				uv_x12x13x14x15				= si_fms(uv_x12x13x14x15, to_screen_x, one);
		  qword uv_y0y1y2y3					= si_fma(uv_y0y1y2y3_in, to_screen_y, one);
		  qword uv_y4y5y6y7					= si_fma(uv_y0y1y2y3_in, to_screen_y, one);
		  qword uv_y8y9y10y11				= si_fma(uv_y0y1y2y3_in, to_screen_y, one);
		  qword uv_y12y13y14y15				= si_fma(uv_y0y1y2y3_in, to_screen_y, one);
				uv_x0x1x2x3					= si_fm(uv_x0x1x2x3, matrix_0);
				uv_x4x5x6x7					= si_fm(uv_x4x5x6x7, matrix_0);
				uv_x8x9x10x11				= si_fm(uv_x8x9x10x11, matrix_0);
				uv_x12x13x14x15				= si_fm(uv_x12x13x14x15, matrix_0);
				uv_y0y1y2y3					= si_fm(uv_y0y1y2y3, matrix_1);
				uv_y4y5y6y7					= si_fm(uv_y4y5y6y7, matrix_1);
				uv_y8y9y10y11				= si_fm(uv_y8y9y10y11, matrix_1);
				uv_y12y13y14y15				= si_fm(uv_y12y13y14y15, matrix_1);
		  qword view_z0z1z2z3				= get_view_z_value4(uv_z0z1z2z3, inv_far_minus_inv_near, inv_near);
		  qword view_z4z5z6z7				= get_view_z_value4(uv_z4z5z6z7, inv_far_minus_inv_near, inv_near);
		  qword view_z8z9z10z11				= get_view_z_value4(uv_z8z9z10z11, inv_far_minus_inv_near, inv_near);
		  qword view_z12z13z14z15			= get_view_z_value4(uv_z12z13z14z15, inv_far_minus_inv_near, inv_near);
		  qword view_x0x1x2x3				= si_fm(uv_x0x1x2x3, view_z0z1z2z3);
		  qword view_x4x5x6x7				= si_fm(uv_x4x5x6x7, view_z4z5z6z7);
		  qword view_x8x9x10x11				= si_fm(uv_x8x9x10x11, view_z8z9z10z11);
		  qword view_x12x13x14x15			= si_fm(uv_x12x13x14x15, view_z12z13z14z15);
		  qword view_y0y1y2y3				= si_fm(uv_y0y1y2y3, view_z0z1z2z3);
		  qword view_y4y5y6y7				= si_fm(uv_y4y5y6y7, view_z4z5z6z7);
		  qword view_y8y9y10y11				= si_fm(uv_y8y9y10y11, view_z8z9z10z11);
		  qword view_y12y13y14y15			= si_fm(uv_y12y13y14y15, view_z12z13z14z15);
	const qword two_five_five				= si_ilhu(0x437f);
		  qword n_x0x1y0y1					= si_shufb(normal0, normal1, shuf_AaBb);
		  qword n_y2y3x2x3					= si_shufb(normal2, normal3, shuf_BbAa);
		  qword n_z0z1z0z1					= si_shufb(normal0, normal1, shuf_CcCc);
		  qword n_z2z3z2z3					= si_shufb(normal2, normal3, shuf_CcCc);
		  qword n_x4x5y4y5					= si_shufb(normal4, normal5, shuf_AaBb);
		  qword n_y6y7x6x7					= si_shufb(normal6, normal7, shuf_BbAa);
		  qword n_z4z5z4z5					= si_shufb(normal4, normal5, shuf_CcCc);
		  qword n_z6z7z6z7					= si_shufb(normal6, normal7, shuf_CcCc);
		  qword n_x8x9y8y9					= si_shufb(normal8, normal9, shuf_AaBb);
		  qword n_y10y11x10x11				= si_shufb(normal10, normal11, shuf_BbAa);
		  qword n_z8z9z8z9					= si_shufb(normal8, normal9, shuf_CcCc);
		  qword n_z10z11z10z11				= si_shufb(normal10, normal11, shuf_CcCc);
		  qword n_x12x13y12y13				= si_shufb(normal12, normal13, shuf_AaBb);
		  qword n_y14y15x14x15				= si_shufb(normal14, normal15, shuf_BbAa);
		  qword n_z12z13z12z13				= si_shufb(normal12, normal13, shuf_CcCc);
		  qword n_z14z15z14z15				= si_shufb(normal14, normal15, shuf_CcCc);
		  qword view_normal_x0x1x2x3		= si_selb(n_x0x1y0y1, n_y2y3x2x3, mask_00ff);
		  qword view_normal_y0y1y2y3		= si_shufb(n_x0x1y0y1, n_y2y3x2x3, shuf_CDab);
		  qword view_normal_z0z1z2z3		= si_selb(n_z0z1z0z1, n_z2z3z2z3, mask_00ff);
		  qword view_normal_x4x5x6x7		= si_selb(n_x4x5y4y5, n_y6y7x6x7, mask_00ff);
		  qword view_normal_y4y5y6y7		= si_shufb(n_x4x5y4y5, n_y6y7x6x7, shuf_CDab);
		  qword view_normal_z4z5z6z7		= si_selb(n_z4z5z4z5, n_z6z7z6z7, mask_00ff);
		  qword view_normal_x8x9x10x11		= si_selb(n_x8x9y8y9, n_y10y11x10x11, mask_00ff);
		  qword view_normal_y8y9y10y11		= si_shufb(n_x8x9y8y9, n_y10y11x10x11, shuf_CDab);
		  qword view_normal_z8z9z10z11		= si_selb(n_z8z9z8z9, n_z10z11z10z11, mask_00ff);
		  qword view_normal_x12x13x14x15	= si_selb(n_x12x13y12y13, n_y14y15x14x15, mask_00ff);
		  qword view_normal_y12y13y14y15	= si_shufb(n_x12x13y12y13, n_y14y15x14x15, shuf_CDab);
		  qword view_normal_z12z13z14z15	= si_selb(n_z12z13z12z13, n_z14z15z14z15, mask_00ff);
	
	uint16_t	current_point_light			= 0;
	uint16_t	point_light_count			= pVisibleLights->pointLightCount;
	do 
	{
		qword light_ptr 					= (const qword)si_from_ptr(&pVisibleLights->pointLights[current_point_light].f[0]);
		qword light_position				= si_lqd(light_ptr, 0x00);
		qword light_position_x				= si_shufb(light_position, light_position, shuf_xxxx);
		qword light_position_y				= si_shufb(light_position, light_position, shuf_yyyy);
		qword light_position_z				= si_shufb(light_position, light_position, shuf_zzzz);
		qword light_colour					= si_lqd(light_ptr, 0x10);
		qword light_falloff					= si_lqd(light_ptr, 0x20);
		qword view_to_light_x0x1x2x3		= si_fs(light_position_x, view_x0x1x2x3);														// light_to_world = light_position - world_position;
		qword view_to_light_x4x5x6x7		= si_fs(light_position_x, view_x4x5x6x7);
		qword view_to_light_x8x9x10x11		= si_fs(light_position_x, view_x8x9x10x11);
		qword view_to_light_x12x13x14x15	= si_fs(light_position_x, view_x12x13x14x15);
		qword view_to_light_y0y1y2y3		= si_fs(light_position_y, view_y0y1y2y3);
		qword view_to_light_y4y5y6y7		= si_fs(light_position_y, view_y4y5y6y7);
		qword view_to_light_y8y9y10y11		= si_fs(light_position_y, view_y8y9y10y11);
		qword view_to_light_y12y13y14y15	= si_fs(light_position_y, view_y12y13y14y15);
		qword view_to_light_z0z1z2z3		= si_fs(light_position_z, view_z0z1z2z3);
		qword view_to_light_z4z5z6z7		= si_fs(light_position_z, view_z4z5z6z7);
		qword view_to_light_z8z9z10z11		= si_fs(light_position_z, view_z8z9z10z11);
		qword view_to_light_z12z13z14z15	= si_fs(light_position_z, view_z12z13z14z15);
		qword light_distance_sq03			= si_fm(view_to_light_x0x1x2x3, view_to_light_x0x1x2x3);
			  light_distance_sq03			= si_fma(view_to_light_y0y1y2y3, view_to_light_y0y1y2y3, light_distance_sq03);
			  light_distance_sq03			= si_fma(view_to_light_z0z1z2z3, view_to_light_z0z1z2z3, light_distance_sq03);
		qword light_distance_sq47			= si_fm(view_to_light_x4x5x6x7, view_to_light_x4x5x6x7);
			  light_distance_sq47			= si_fma(view_to_light_y4y5y6y7, view_to_light_y4y5y6y7, light_distance_sq47);
			  light_distance_sq47			= si_fma(view_to_light_z4z5z6z7, view_to_light_z4z5z6z7, light_distance_sq47);
		qword light_distance_sq811			= si_fm(view_to_light_x8x9x10x11, view_to_light_x8x9x10x11);
			  light_distance_sq811			= si_fma(view_to_light_y8y9y10y11, view_to_light_y8y9y10y11, light_distance_sq811);
			  light_distance_sq811			= si_fma(view_to_light_z8z9z10z11, view_to_light_z8z9z10z11, light_distance_sq811);
		qword light_distance_sq1215			= si_fm(view_to_light_x12x13x14x15, view_to_light_x12x13x14x15);
			  light_distance_sq1215			= si_fma(view_to_light_y12y13y14y15, view_to_light_y12y13y14y15, light_distance_sq1215);
			  light_distance_sq1215			= si_fma(view_to_light_z12z13z14z15, view_to_light_z12z13z14z15, light_distance_sq1215);		
		qword light_atten03					= si_fm(light_distance_sq03, light_falloff);
		qword light_atten47					= si_fm(light_distance_sq47, light_falloff);
		qword light_atten811				= si_fm(light_distance_sq811, light_falloff);
		qword light_atten1215				= si_fm(light_distance_sq1215, light_falloff);
 		qword light_atten_sat_a03			= si_cfltu(light_atten03, 0x20);
		qword light_atten_sat_a47			= si_cfltu(light_atten47, 0x20);
		qword light_atten_sat_a811			= si_cfltu(light_atten811, 0x20);
		qword light_atten_sat_a1215			= si_cfltu(light_atten1215, 0x20);
 		qword light_atten_sat03				= si_cuflt(light_atten_sat_a03, 0x20);
		qword light_atten_sat47				= si_cuflt(light_atten_sat_a47, 0x20);
		qword light_atten_sat811			= si_cuflt(light_atten_sat_a811, 0x20);
		qword light_atten_sat1215			= si_cuflt(light_atten_sat_a1215, 0x20);
		qword light_factor03				= si_fs(one, light_atten_sat03);																// atten = 1 - saturate(sq_distance * falloff);	
		qword light_factor47				= si_fs(one, light_atten_sat47);
		qword light_factor811				= si_fs(one, light_atten_sat811);	
		qword light_factor1215				= si_fs(one, light_atten_sat1215);
		qword view_to_light_dot03			= si_frsqest(light_distance_sq03);
		qword view_to_light_dot47			= si_frsqest(light_distance_sq47);
		qword view_to_light_dot811			= si_frsqest(light_distance_sq811);
		qword view_to_light_dot1215			= si_frsqest(light_distance_sq1215);
		qword view_to_light_unitx_03		= si_fm(view_to_light_x0x1x2x3, view_to_light_dot03);
		qword view_to_light_unity_03		= si_fm(view_to_light_y0y1y2y3, view_to_light_dot03);
		qword view_to_light_unitz_03		= si_fm(view_to_light_z0z1z2z3, view_to_light_dot03);
		qword view_to_light_unitx_47		= si_fm(view_to_light_x4x5x6x7, view_to_light_dot47);
		qword view_to_light_unity_47		= si_fm(view_to_light_y4y5y6y7, view_to_light_dot47);
		qword view_to_light_unitz_47		= si_fm(view_to_light_z4z5z6z7, view_to_light_dot47);
		qword view_to_light_unitx_811		= si_fm(view_to_light_x8x9x10x11, view_to_light_dot811);
		qword view_to_light_unity_811		= si_fm(view_to_light_y8y9y10y11, view_to_light_dot811);
		qword view_to_light_unitz_811		= si_fm(view_to_light_z8z9z10z11, view_to_light_dot811);
		qword view_to_light_unitx_1215		= si_fm(view_to_light_x12x13x14x15, view_to_light_dot1215);
		qword view_to_light_unity_1215		= si_fm(view_to_light_y12y13y14y15, view_to_light_dot1215);
		qword view_to_light_unitz_1215		= si_fm(view_to_light_z12z13z14z15, view_to_light_dot1215);
		qword diff_dot_03					= si_fm(view_to_light_unitx_03, view_normal_x0x1x2x3);
			  diff_dot_03					= si_fma(view_to_light_unity_03, view_normal_y0y1y2y3, diff_dot_03);
			  diff_dot_03					= si_fma(view_to_light_unitz_03, view_normal_z0z1z2z3, diff_dot_03);
	    qword diff_dot_47					= si_fm(view_to_light_unitx_47, view_normal_x4x5x6x7);
			  diff_dot_47					= si_fma(view_to_light_unity_47, view_normal_y4y5y6y7, diff_dot_47);
			  diff_dot_47					= si_fma(view_to_light_unitz_47, view_normal_z4z5z6z7, diff_dot_47);
		qword diff_dot_811					= si_fm(view_to_light_unitx_811, view_normal_x8x9x10x11);
			  diff_dot_811					= si_fma(view_to_light_unity_811, view_normal_y8y9y10y11, diff_dot_811);
			  diff_dot_811					= si_fma(view_to_light_unitz_811, view_normal_z8z9z10z11, diff_dot_811);
	    qword diff_dot_1215					= si_fm(view_to_light_unitx_1215, view_normal_x12x13x14x15);
			  diff_dot_1215					= si_fma(view_to_light_unity_1215, view_normal_y12y13y14y15, diff_dot_1215);
			  diff_dot_1215					= si_fma(view_to_light_unitz_1215, view_normal_z12z13z14z15, diff_dot_1215);
 		qword diff_dot03_atten				= si_fm(light_factor03, diff_dot_03);
		qword diff_dot47_atten				= si_fm(light_factor47, diff_dot_47);
		qword diff_dot811_atten				= si_fm(light_factor811, diff_dot_811);
		qword diff_dot1215_atten			= si_fm(light_factor1215, diff_dot_1215);														// dot *= atten;
 		qword dot0							= si_shufb(diff_dot03_atten, diff_dot03_atten, shuf_xxxx);
		qword dot1							= si_shufb(diff_dot03_atten, diff_dot03_atten, shuf_yyyy);
		qword dot2							= si_shufb(diff_dot03_atten, diff_dot03_atten, shuf_zzzz);
		qword dot3							= si_shufb(diff_dot03_atten, diff_dot03_atten, shuf_wwww);
		qword dot4							= si_shufb(diff_dot47_atten, diff_dot47_atten, shuf_xxxx);
		qword dot5							= si_shufb(diff_dot47_atten, diff_dot47_atten, shuf_yyyy);
		qword dot6							= si_shufb(diff_dot47_atten, diff_dot47_atten, shuf_zzzz);
		qword dot7							= si_shufb(diff_dot47_atten, diff_dot47_atten, shuf_wwww);
		qword dot8							= si_shufb(diff_dot811_atten, diff_dot811_atten, shuf_xxxx);
		qword dot9							= si_shufb(diff_dot811_atten, diff_dot811_atten, shuf_yyyy);
		qword dot10							= si_shufb(diff_dot811_atten, diff_dot811_atten, shuf_zzzz);
		qword dot11							= si_shufb(diff_dot811_atten, diff_dot811_atten, shuf_wwww);
		qword dot12							= si_shufb(diff_dot1215_atten, diff_dot1215_atten, shuf_xxxx);
		qword dot13							= si_shufb(diff_dot1215_atten, diff_dot1215_atten, shuf_yyyy);
		qword dot14							= si_shufb(diff_dot1215_atten, diff_dot1215_atten, shuf_zzzz);
		qword dot15							= si_shufb(diff_dot1215_atten, diff_dot1215_atten, shuf_wwww);
 			  diffuse0						= si_fma(dot0, light_colour, diffuse0);
			  diffuse1						= si_fma(dot1, light_colour, diffuse1);
			  diffuse2						= si_fma(dot2, light_colour, diffuse2);
			  diffuse3						= si_fma(dot3, light_colour, diffuse3);
			  diffuse4						= si_fma(dot4, light_colour, diffuse4);
			  diffuse5						= si_fma(dot5, light_colour, diffuse5);
			  diffuse6						= si_fma(dot6, light_colour, diffuse6);
			  diffuse7						= si_fma(dot7, light_colour, diffuse7);
			  diffuse8						= si_fma(dot8, light_colour, diffuse8);
			  diffuse9						= si_fma(dot8, light_colour, diffuse9);
			  diffuse10						= si_fma(dot10, light_colour, diffuse10);
			  diffuse11						= si_fma(dot11, light_colour, diffuse11);
			  diffuse12						= si_fma(dot12, light_colour, diffuse12);
			  diffuse13						= si_fma(dot13, light_colour, diffuse13);
			  diffuse14						= si_fma(dot14, light_colour, diffuse14);
			  diffuse15						= si_fma(dot15, light_colour, diffuse15);														// dot *= colour;
	} 
	while (++current_point_light<point_light_count);
	

	uint16_t	current_spot_light			= 0;
	uint16_t	spot_light_count			= pVisibleLights->spotLightCount;
	do 
	{
		qword light_ptr						= si_from_ptr(&pVisibleLights->spotLights[current_spot_light].f[0]);
		qword light_position_x				= si_lqd(light_ptr, 0x00);
		qword light_position_y				= si_lqd(light_ptr, 0x10);
		qword light_position_z				= si_lqd(light_ptr, 0x20);
		qword direction_x					= si_lqd(light_ptr, 0x30);
		qword direction_y					= si_lqd(light_ptr, 0x40);
		qword direction_z					= si_lqd(light_ptr, 0x50);
		qword falloff_x						= si_lqd(light_ptr, 0x60);
		qword falloff_y						= si_lqd(light_ptr, 0x70);
		qword light_colour					= si_lqd(light_ptr, 0x80);
		qword inv_falloff_y					= si_fs(one, falloff_y);
		qword inv_falloff_y_rcp				= si_frest(inv_falloff_y);
			  inv_falloff_y_rcp				= si_fi(inv_falloff_y, inv_falloff_y_rcp);
		qword view_vector_x_0				= si_fs(view_x0x1x2x3, light_position_x);
		qword view_vector_y_0				= si_fs(view_y0y1y2y3, light_position_y);
		qword view_vector_z_0				= si_fs(view_z0z1z2z3, light_position_z);
		qword view_vector_x_1				= si_fs(view_x4x5x6x7, light_position_x);
		qword view_vector_y_1				= si_fs(view_y4y5y6y7, light_position_y);
		qword view_vector_z_1				= si_fs(view_z4z5z6z7, light_position_z);
		qword view_vector_x_2				= si_fs(view_x8x9x10x11, light_position_x);
		qword view_vector_y_2				= si_fs(view_y8y9y10y11, light_position_y);
		qword view_vector_z_2				= si_fs(view_z8z9z10z11, light_position_z);
		qword view_vector_x_3				= si_fs(view_x12x13x14x15, light_position_x);
		qword view_vector_y_3				= si_fs(view_y12y13y14y15, light_position_y);
		qword view_vector_z_3				= si_fs(view_z12z13z14z15, light_position_z);
		qword view_dot_view_0				= si_fm(view_vector_x_0, view_vector_x_0);
			  view_dot_view_0				= si_fma(view_vector_y_0, view_vector_y_0, view_dot_view_0);
			  view_dot_view_0				= si_fma(view_vector_z_0, view_vector_z_0, view_dot_view_0);
		qword view_dot_view_1				= si_fm(view_vector_x_1, view_vector_x_1);
			  view_dot_view_1				= si_fma(view_vector_y_1, view_vector_y_1, view_dot_view_1);
			  view_dot_view_1				= si_fma(view_vector_z_1, view_vector_z_1, view_dot_view_1);
		qword view_dot_view_2				= si_fm(view_vector_x_2, view_vector_x_2);
			  view_dot_view_2				= si_fma(view_vector_y_2, view_vector_y_2, view_dot_view_2);
			  view_dot_view_2				= si_fma(view_vector_z_2, view_vector_z_2, view_dot_view_2);
		qword view_dot_view_3				= si_fm(view_vector_x_3, view_vector_x_3);
			  view_dot_view_3				= si_fma(view_vector_y_3, view_vector_y_3, view_dot_view_3);
			  view_dot_view_3				= si_fma(view_vector_z_3, view_vector_z_3, view_dot_view_3);	
		qword view_dot_view_sqrt_rcp_0		= si_frsqest(view_dot_view_0);
			  view_dot_view_sqrt_rcp_0		= si_fi(view_dot_view_0, view_dot_view_sqrt_rcp_0);
		qword view_dot_view_sqrt_rcp_1		= si_frsqest(view_dot_view_1);
			  view_dot_view_sqrt_rcp_1		= si_fi(view_dot_view_1, view_dot_view_sqrt_rcp_1);
		qword view_dot_view_sqrt_rcp_2		= si_frsqest(view_dot_view_2);
			  view_dot_view_sqrt_rcp_2		= si_fi(view_dot_view_2, view_dot_view_sqrt_rcp_2);
		qword view_dot_view_sqrt_rcp_3		= si_frsqest(view_dot_view_3);
			  view_dot_view_sqrt_rcp_3		= si_fi(view_dot_view_3, view_dot_view_sqrt_rcp_3);
		qword length_view_0					= si_frest(view_dot_view_sqrt_rcp_0);
			  length_view_0					= si_fi(view_dot_view_sqrt_rcp_0, length_view_0);
		qword length_view_1					= si_frest(view_dot_view_sqrt_rcp_1);
			  length_view_1					= si_fi(view_dot_view_sqrt_rcp_1, length_view_1);
		qword length_view_2					= si_frest(view_dot_view_sqrt_rcp_2);
			  length_view_2					= si_fi(view_dot_view_sqrt_rcp_2, length_view_2);
		qword length_view_3					= si_frest(view_dot_view_sqrt_rcp_3);
			  length_view_3					= si_fi(view_dot_view_sqrt_rcp_3, length_view_3);
		qword view_direction_x_0			= si_fm(view_vector_x_0, view_dot_view_sqrt_rcp_0);
		qword view_direction_y_0			= si_fm(view_vector_y_0, view_dot_view_sqrt_rcp_0);
		qword view_direction_z_0			= si_fm(view_vector_z_0, view_dot_view_sqrt_rcp_0);
		qword view_direction_x_1			= si_fm(view_vector_x_1, view_dot_view_sqrt_rcp_1);
		qword view_direction_y_1			= si_fm(view_vector_y_1, view_dot_view_sqrt_rcp_1);
		qword view_direction_z_1			= si_fm(view_vector_z_1, view_dot_view_sqrt_rcp_1);
		qword view_direction_x_2			= si_fm(view_vector_x_2, view_dot_view_sqrt_rcp_2);
		qword view_direction_y_2			= si_fm(view_vector_y_2, view_dot_view_sqrt_rcp_2);
		qword view_direction_z_2			= si_fm(view_vector_z_2, view_dot_view_sqrt_rcp_2);
		qword view_direction_x_3			= si_fm(view_vector_x_3, view_dot_view_sqrt_rcp_3);
		qword view_direction_y_3			= si_fm(view_vector_y_3, view_dot_view_sqrt_rcp_3);
		qword view_direction_z_3			= si_fm(view_vector_z_3, view_dot_view_sqrt_rcp_3);
		qword inv_view_direction_x_0		= si_fs(one, view_direction_x_0);
		qword inv_view_direction_y_0		= si_fs(one, view_direction_y_0);
		qword inv_view_direction_z_0		= si_fs(one, view_direction_z_0);		
		qword inv_view_direction_x_1		= si_fs(one, view_direction_x_1);
		qword inv_view_direction_y_1		= si_fs(one, view_direction_y_1);
		qword inv_view_direction_z_1		= si_fs(one, view_direction_z_1);		
		qword inv_view_direction_x_2		= si_fs(one, view_direction_x_2);
		qword inv_view_direction_y_2		= si_fs(one, view_direction_y_2);
		qword inv_view_direction_z_2		= si_fs(one, view_direction_z_2);		
		qword inv_view_direction_x_3		= si_fs(one, view_direction_x_3);
		qword inv_view_direction_y_3		= si_fs(one, view_direction_y_3);
		qword inv_view_direction_z_3		= si_fs(one, view_direction_z_3);
		qword normal_dot_inv_world_0		= si_fm(view_normal_x0x1x2x3, inv_view_direction_x_0);
			  normal_dot_inv_world_0		= si_fma(view_normal_y0y1y2y3, inv_view_direction_y_0, normal_dot_inv_world_0);
			  normal_dot_inv_world_0		= si_fma(view_normal_z0z1z2z3, inv_view_direction_z_0, normal_dot_inv_world_0);
		qword normal_dot_inv_world_1		= si_fm(view_normal_x4x5x6x7, inv_view_direction_x_1);
			  normal_dot_inv_world_1		= si_fma(view_normal_y4y5y6y7, inv_view_direction_y_1, normal_dot_inv_world_1);
			  normal_dot_inv_world_1		= si_fma(view_normal_z4z5z6z7, inv_view_direction_z_1, normal_dot_inv_world_1);
		qword normal_dot_inv_world_2		= si_fm(view_normal_x8x9x10x11, inv_view_direction_x_2);
			  normal_dot_inv_world_2		= si_fma(view_normal_y8y9y10y11, inv_view_direction_y_2, normal_dot_inv_world_2);
			  normal_dot_inv_world_2		= si_fma(view_normal_z8z9z10z11, inv_view_direction_z_2, normal_dot_inv_world_2);
		qword normal_dot_inv_world_3		= si_fm(view_normal_x12x13x14x15, inv_view_direction_x_3);
			  normal_dot_inv_world_3		= si_fma(view_normal_y12y13y14y15, inv_view_direction_y_3, normal_dot_inv_world_3);
			  normal_dot_inv_world_3		= si_fma(view_normal_z12z13z14z15, inv_view_direction_z_3, normal_dot_inv_world_3);
		qword n_dot_l_0						= si_fma(normal_dot_inv_world_0, point_seven_five, point_two_five);
			  n_dot_l_0						= si_cfltu(n_dot_l_0, 0x20);
			  n_dot_l_0						= si_cuflt(n_dot_l_0, 0x20);	// Saturate.
		qword n_dot_l_1						= si_fma(normal_dot_inv_world_1, point_seven_five, point_two_five);
			  n_dot_l_1						= si_cfltu(n_dot_l_1, 0x20);
			  n_dot_l_1						= si_cuflt(n_dot_l_1, 0x20);	// Saturate.
		qword n_dot_l_2						= si_fma(normal_dot_inv_world_2, point_seven_five, point_two_five);
			  n_dot_l_2						= si_cfltu(n_dot_l_2, 0x20);
			  n_dot_l_2						= si_cuflt(n_dot_l_2, 0x20);	// Saturate.
		qword n_dot_l_3						= si_fma(normal_dot_inv_world_3, point_seven_five, point_two_five);
			  n_dot_l_3						= si_cfltu(n_dot_l_3, 0x20);
			  n_dot_l_3						= si_cuflt(n_dot_l_3, 0x20);	// Saturate.
		qword d_dot_l_0						= si_fm(view_direction_x_0, direction_x);
			  d_dot_l_0						= si_fma(view_direction_y_0, direction_y, d_dot_l_0);
			  d_dot_l_0						= si_fma(view_direction_z_0, direction_z, d_dot_l_0);
			  d_dot_l_0						= si_cfltu(d_dot_l_0, 0x20);
			  d_dot_l_0						= si_cuflt(d_dot_l_0, 0x20);	// Saturate.
		qword d_dot_l_1						= si_fm(view_direction_x_1, direction_x);
			  d_dot_l_1						= si_fma(view_direction_y_1, direction_y, d_dot_l_1);
			  d_dot_l_1						= si_fma(view_direction_z_1, direction_z, d_dot_l_1);
			  d_dot_l_1						= si_cfltu(d_dot_l_1, 0x20);
			  d_dot_l_1						= si_cuflt(d_dot_l_1, 0x20);	// Saturate.
		qword d_dot_l_2						= si_fm(view_direction_x_2, direction_x);
			  d_dot_l_2						= si_fma(view_direction_y_2, direction_y, d_dot_l_2);
			  d_dot_l_2						= si_fma(view_direction_z_2, direction_z, d_dot_l_2);
			  d_dot_l_2						= si_cfltu(d_dot_l_2, 0x20);
			  d_dot_l_2						= si_cuflt(d_dot_l_2, 0x20);	// Saturate.
	    qword d_dot_l_3						= si_fm(view_direction_x_3, direction_x);
			  d_dot_l_3						= si_fma(view_direction_y_3, direction_y, d_dot_l_3);
			  d_dot_l_3						= si_fma(view_direction_z_3, direction_z, d_dot_l_3);
			  d_dot_l_3						= si_cfltu(d_dot_l_3, 0x20);
			  d_dot_l_3						= si_cuflt(d_dot_l_3, 0x20);	// Saturate.
		qword d_dot_l_0_minus_fall_y_0 		= si_fs(d_dot_l_0, falloff_y);
		qword d_dot_l_1_minus_fall_y_1 		= si_fs(d_dot_l_1, falloff_y);
		qword d_dot_l_2_minus_fall_y_2 		= si_fs(d_dot_l_2, falloff_y);
		qword d_dot_l_3_minus_fall_y_3 		= si_fs(d_dot_l_3, falloff_y);
		qword outer_radius_0				= si_fm(d_dot_l_0_minus_fall_y_0, inv_falloff_y_rcp);
			  outer_radius_0				= si_cfltu(outer_radius_0, 0x20);
			  outer_radius_0				= si_cuflt(outer_radius_0, 0x20);
		qword outer_radius_1				= si_fm(d_dot_l_1_minus_fall_y_1, inv_falloff_y_rcp);
			  outer_radius_1				= si_cfltu(outer_radius_1, 0x20);
			  outer_radius_1				= si_cuflt(outer_radius_1, 0x20);
		qword outer_radius_2				= si_fm(d_dot_l_2_minus_fall_y_2, inv_falloff_y_rcp);
			  outer_radius_2				= si_cfltu(outer_radius_2, 0x20);
			  outer_radius_2				= si_cuflt(outer_radius_2, 0x20);
	    qword outer_radius_3				= si_fm(d_dot_l_3_minus_fall_y_3, inv_falloff_y_rcp);
			  outer_radius_3				= si_cfltu(outer_radius_3, 0x20);
			  outer_radius_3				= si_cuflt(outer_radius_3, 0x20);
		qword length_view_sq_0				= si_fm(length_view_0, length_view_0);
			  length_view_sq_0				= si_fm(length_view_0, falloff_x);
		qword length_view_sq_1				= si_fm(length_view_1, length_view_1);
			  length_view_sq_1				= si_fm(length_view_1, falloff_x);
		qword length_view_sq_2				= si_fm(length_view_2, length_view_2);
			  length_view_sq_2				= si_fm(length_view_2, falloff_x);
		qword length_view_sq_3				= si_fm(length_view_3, length_view_3);
			  length_view_sq_3				= si_fm(length_view_3, falloff_x);
		qword inv_len_view_sq_0				= si_fs(one, length_view_sq_0);
			  inv_len_view_sq_0				= si_cfltu(inv_len_view_sq_0, 0x20);
			  inv_len_view_sq_0				= si_cuflt(inv_len_view_sq_0, 0x20);
		qword inv_len_view_sq_1				= si_fs(one, length_view_sq_1);
			  inv_len_view_sq_1				= si_cfltu(inv_len_view_sq_1, 0x20);
			  inv_len_view_sq_1				= si_cuflt(inv_len_view_sq_1, 0x20);
	    qword inv_len_view_sq_2				= si_fs(one, length_view_sq_2);
			  inv_len_view_sq_2				= si_cfltu(inv_len_view_sq_2, 0x20);
			  inv_len_view_sq_2				= si_cuflt(inv_len_view_sq_2, 0x20);
	    qword inv_len_view_sq_3				= si_fs(one, length_view_sq_3);
			  inv_len_view_sq_3				= si_cfltu(inv_len_view_sq_3, 0x20);
			  inv_len_view_sq_3				= si_cuflt(inv_len_view_sq_3, 0x20);
		qword falloff_0						= si_fm(inv_len_view_sq_0, outer_radius_0); 
		qword falloff_1						= si_fm(inv_len_view_sq_1, outer_radius_1); 
		qword falloff_2						= si_fm(inv_len_view_sq_2, outer_radius_2); 
		qword falloff_3						= si_fm(inv_len_view_sq_3, outer_radius_3); 
		qword light_diffuse_0				= si_fm(falloff_0, n_dot_l_0);
		qword light_diffuse_1				= si_fm(falloff_1, n_dot_l_1);
		qword light_diffuse_2				= si_fm(falloff_2, n_dot_l_2);
		qword light_diffuse_3				= si_fm(falloff_3, n_dot_l_3);		// TODO: Use falloff_1 through _3 and n_dot_l_1 through _3.
		qword light_diff_0					= si_shufb(light_diffuse_0, light_diffuse_0, shuf_xxxx);
		qword light_diff_1					= si_shufb(light_diffuse_0, light_diffuse_0, shuf_yyyy);
		qword light_diff_2					= si_shufb(light_diffuse_0, light_diffuse_0, shuf_zzzz);
		qword light_diff_3					= si_shufb(light_diffuse_0, light_diffuse_0, shuf_wwww);
		qword light_diff_4					= si_shufb(light_diffuse_1, light_diffuse_1, shuf_xxxx);
		qword light_diff_5					= si_shufb(light_diffuse_1, light_diffuse_1, shuf_yyyy);
		qword light_diff_6					= si_shufb(light_diffuse_1, light_diffuse_1, shuf_zzzz);
		qword light_diff_7					= si_shufb(light_diffuse_1, light_diffuse_1, shuf_wwww);
		qword light_diff_8					= si_shufb(light_diffuse_2, light_diffuse_2, shuf_xxxx);
		qword light_diff_9					= si_shufb(light_diffuse_2, light_diffuse_2, shuf_yyyy);
		qword light_diff_10					= si_shufb(light_diffuse_2, light_diffuse_2, shuf_zzzz);
		qword light_diff_11					= si_shufb(light_diffuse_2, light_diffuse_2, shuf_wwww);
		qword light_diff_12					= si_shufb(light_diffuse_3, light_diffuse_3, shuf_xxxx);
		qword light_diff_13					= si_shufb(light_diffuse_3, light_diffuse_3, shuf_yyyy);
		qword light_diff_14					= si_shufb(light_diffuse_3, light_diffuse_3, shuf_zzzz);
		qword light_diff_15					= si_shufb(light_diffuse_3, light_diffuse_3, shuf_wwww);
			  diffuse0						= si_fma(light_diff_0, light_colour, diffuse0);
			  diffuse1						= si_fma(light_diff_1, light_colour, diffuse1);
			  diffuse2						= si_fma(light_diff_2, light_colour, diffuse2);
			  diffuse3						= si_fma(light_diff_3, light_colour, diffuse3);	
			  diffuse4						= si_fma(light_diff_4, light_colour, diffuse4);
			  diffuse5						= si_fma(light_diff_5, light_colour, diffuse5);
			  diffuse6						= si_fma(light_diff_6, light_colour, diffuse6);
			  diffuse7						= si_fma(light_diff_7, light_colour, diffuse7);
			  diffuse8						= si_fma(light_diff_8, light_colour, diffuse8);
			  diffuse9						= si_fma(light_diff_9, light_colour, diffuse9);
			  diffuse10						= si_fma(light_diff_10, light_colour, diffuse10);
			  diffuse11						= si_fma(light_diff_11, light_colour, diffuse11);
			  diffuse12						= si_fma(light_diff_12, light_colour, diffuse12);
			  diffuse13						= si_fma(light_diff_13, light_colour, diffuse13);
			  diffuse14						= si_fma(light_diff_14, light_colour, diffuse14);
			  diffuse15						= si_fma(light_diff_15, light_colour, diffuse15);
	} 
	while (++current_spot_light<spot_light_count);

	
	qword clamp0 							= si_cfltu(diffuse0, 0x20);						// 1.0 is clamped to 0xFFFFFFFF , not 0xFF000000
	qword clamp1 							= si_cfltu(diffuse1, 0x20);
	qword clamp2 							= si_cfltu(diffuse2, 0x20);
	qword clamp3 							= si_cfltu(diffuse3, 0x20);
	qword clamp4 							= si_cfltu(diffuse4, 0x20);
	qword clamp5 							= si_cfltu(diffuse5, 0x20);
	qword clamp6 							= si_cfltu(diffuse6, 0x20);
	qword clamp7 							= si_cfltu(diffuse7, 0x20);
	qword clamp8 							= si_cfltu(diffuse8, 0x20);
	qword clamp9 							= si_cfltu(diffuse9, 0x20);
	qword clamp10 							= si_cfltu(diffuse10, 0x20);
	qword clamp11 							= si_cfltu(diffuse11, 0x20);
	qword clamp12 							= si_cfltu(diffuse12, 0x20);
	qword clamp13 							= si_cfltu(diffuse13, 0x20);
	qword clamp14 							= si_cfltu(diffuse14, 0x20);
	qword clamp15 							= si_cfltu(diffuse15, 0x20);
		  diffuse0							= si_ila(0x8000);
		  diffuse1							= si_ila(0x8000);
		  diffuse2							= si_ila(0x8000);
		  diffuse3							= si_ila(0x8000);
		  diffuse4							= si_ila(0x8000);
		  diffuse5							= si_ila(0x8000);
		  diffuse6							= si_ila(0x8000);
		  diffuse7							= si_ila(0x8000);
		  diffuse8							= si_ila(0x8000);
		  diffuse9							= si_ila(0x8000);
		  diffuse10							= si_ila(0x8000);
		  diffuse11							= si_ila(0x8000);
		  diffuse12							= si_ila(0x8000);
		  diffuse13							= si_ila(0x8000);
		  diffuse14							= si_ila(0x8000);
		  diffuse15							= si_ila(0x8000);
	qword scale								= si_ilh(0xff00);		
		  diffuse0 							= si_mpyhhau(clamp0, scale, diffuse0);			// Result is in upper byte of word - rounding should match scale
		  diffuse1 							= si_mpyhhau(clamp1, scale, diffuse1);
		  diffuse2 							= si_mpyhhau(clamp2, scale, diffuse2);
		  diffuse3 							= si_mpyhhau(clamp3, scale, diffuse3);
		  diffuse4 							= si_mpyhhau(clamp4, scale, diffuse4);
		  diffuse5 							= si_mpyhhau(clamp5, scale, diffuse5);
		  diffuse6 							= si_mpyhhau(clamp6, scale, diffuse6);
		  diffuse7 							= si_mpyhhau(clamp7, scale, diffuse7);
		  diffuse8 							= si_mpyhhau(clamp8, scale, diffuse8);
		  diffuse9 							= si_mpyhhau(clamp9, scale, diffuse9);
		  diffuse10 						= si_mpyhhau(clamp10, scale, diffuse10);
		  diffuse11							= si_mpyhhau(clamp11, scale, diffuse11);
		  diffuse12 						= si_mpyhhau(clamp12, scale, diffuse12);
		  diffuse13 						= si_mpyhhau(clamp13, scale, diffuse13);
		  diffuse14 						= si_mpyhhau(clamp14, scale, diffuse14);
		  diffuse15 						= si_mpyhhau(clamp15, scale, diffuse15);
	
	const vector unsigned char _shuf_uint =
	{ 0xc0, 0x00, 0x04, 0x08, 0xc0, 0x10, 0x14, 0x18, 0xc0, 0x00, 0x04, 0x08, 0xc0, 0x10, 0x14, 0x18 };
	const qword shuf_uint					= (const qword)_shuf_uint;
	const qword base_addr					= si_from_ptr(pResult);
		  qword pixel0_01					= si_shufb(diffuse0, diffuse1, shuf_uint);
		  qword pixel0_02					= si_shufb(diffuse2, diffuse3, shuf_uint);
		  qword pixel1_01					= si_shufb(diffuse4, diffuse5, shuf_uint);
		  qword pixel1_02					= si_shufb(diffuse6, diffuse7, shuf_uint);
		  qword pixel2_01					= si_shufb(diffuse8, diffuse9, shuf_uint);
		  qword pixel2_02					= si_shufb(diffuse10, diffuse11, shuf_uint);
		  qword pixel3_01					= si_shufb(diffuse12, diffuse13, shuf_uint);
		  qword pixel3_02					= si_shufb(diffuse14, diffuse15, shuf_uint);
		  qword pixel0						= si_selb(pixel0_01, pixel0_02, mask_00ff);
		  qword pixel1						= si_selb(pixel1_01, pixel1_02, mask_00ff);
		  qword pixel2						= si_selb(pixel2_01, pixel2_02, mask_00ff);
		  qword pixel3						= si_selb(pixel3_01, pixel3_02, mask_00ff);
											  si_stqd(pixel0, base_addr, 0x00);			// Batch LS writes.
											  si_stqd(pixel1, base_addr, 0x10);
											  si_stqd(pixel2, base_addr, 0x20);
											  si_stqd(pixel3, base_addr, 0x30);
	return;
}

void ProcessTileLit(VisibleLights* __restrict__ pVisibleLights, 
					LightingData::View* __restrict__ pView, 
					U8* pOutput, 
					U8* pNormalBuffer,
					U8* __restrict__ pDepthBuffer,
					const uint32_t tileWidth, 
					const uint32_t tileHeight, 
					const uint32_t tileStride, 
					const uint16_t textureWidth, 
					const uint16_t textureHeight, 
					const uint16_t x, 
					const uint16_t y)
{
	PERF_EVENT("SPU lighting.process tile lit");	
	      qword y_val					= si_cuflt(si_from_uint(y), 0x0);
	const qword shuf_xxxx				= si_ila(0x010203);
	const qword inv_far_clip_a			= si_frest(si_from_float(pView->fFarClip));
	const qword inv_far_clip			= si_shufb(inv_far_clip_a, inv_far_clip_a, shuf_xxxx);
	const qword inv_near_clip_a			= si_frest(si_from_float(pView->fNearClip));
	const qword inv_near				= si_shufb(inv_near_clip_a, inv_near_clip_a, shuf_xxxx);
	const qword inv_far_minus_inv_near	= si_fs(inv_far_clip, inv_near);
	uint32_t* pRowOutput				= (uint32_t*)pOutput;
	uint32_t* pNormalBuf				= (uint32_t*)pNormalBuffer;
	uint32_t* __restrict pDepthBuf		= (uint32_t*)pDepthBuffer;
	uint16_t	i						= 0;
	      y_val							= si_shufb(y_val, y_val, shuf_xxxx);
	  
	do
	{
		qword i_val						= si_shufb(si_from_uint(i), si_from_uint(i), shuf_xxxx);
		qword depth_addr				= si_from_ptr(pDepthBuf);
		qword depth0					= si_lqd(depth_addr, 0x0); 
		qword depth1					= si_lqd(depth_addr, 0x10);
		qword depth2 					= si_lqd(depth_addr, 0x20);
			  i_val						= si_cuflt(i_val, 0x0);
		qword depth3 					= si_lqd(depth_addr, 0x30);
		qword depth4 					= si_lqd(depth_addr, 0x40);
		qword depth5 					= si_lqd(depth_addr, 0x50);
		qword depth6 					= si_lqd(depth_addr, 0x60);
		qword depth7 					= si_lqd(depth_addr, 0x70);
		qword y_vals					= si_fa(y_val, i_val);
		
		lit(u32_to_qword_rd(pNormalBuf[ 0]), x + 0,					// todo: sort this out, get normals in AOS mode here.
			u32_to_qword_rd(pNormalBuf[ 1]), x + 1, 
			u32_to_qword_rd(pNormalBuf[ 2]), x + 2, 
			u32_to_qword_rd(pNormalBuf[ 3]), x + 3, 
			u32_to_qword_rd(pNormalBuf[ 4]), x + 4, 
			u32_to_qword_rd(pNormalBuf[ 5]), x + 5, 
			u32_to_qword_rd(pNormalBuf[ 6]), x + 6, 
			u32_to_qword_rd(pNormalBuf[ 7]), x + 7, 
			u32_to_qword_rd(pNormalBuf[ 8]), x + 8, 
			u32_to_qword_rd(pNormalBuf[ 9]), x + 9, 
			u32_to_qword_rd(pNormalBuf[10]), x + 10, 
			u32_to_qword_rd(pNormalBuf[11]), x + 11, 
			u32_to_qword_rd(pNormalBuf[12]), x + 12, 
			u32_to_qword_rd(pNormalBuf[13]), x + 13, 
			u32_to_qword_rd(pNormalBuf[14]), x + 14, 
			u32_to_qword_rd(pNormalBuf[15]), x + 15, 
			y_vals,
			depth0,
			depth1,
			depth2,
			depth3,
			&pRowOutput[0],
			pVisibleLights, 
			inv_far_minus_inv_near,
			inv_near,
			(const F32* const)&pView->fProjInverseMatrix);
		lit(u32_to_qword_rd(pNormalBuf[16]), x + 16, 
			u32_to_qword_rd(pNormalBuf[17]), x + 17, 
			u32_to_qword_rd(pNormalBuf[18]), x + 18, 
			u32_to_qword_rd(pNormalBuf[19]), x + 19, 
			u32_to_qword_rd(pNormalBuf[20]), x + 20, 
			u32_to_qword_rd(pNormalBuf[21]), x + 21, 
			u32_to_qword_rd(pNormalBuf[22]), x + 22, 
			u32_to_qword_rd(pNormalBuf[23]), x + 23, 
			u32_to_qword_rd(pNormalBuf[24]), x + 24, 
			u32_to_qword_rd(pNormalBuf[25]), x + 25, 
			u32_to_qword_rd(pNormalBuf[26]), x + 26, 
			u32_to_qword_rd(pNormalBuf[27]), x + 27, 
			u32_to_qword_rd(pNormalBuf[28]), x + 28, 
			u32_to_qword_rd(pNormalBuf[29]), x + 29, 
			u32_to_qword_rd(pNormalBuf[30]), x + 30, 
			u32_to_qword_rd(pNormalBuf[31]), x + 31, 
			y_vals,
			depth4,
			depth5,
			depth6,
			depth7,
			&pRowOutput[16],
			pVisibleLights, 
			inv_far_minus_inv_near,
			inv_near,
			(const F32* const)&pView->fProjInverseMatrix);
		
		// Advance our buffer pointers to next row.
		uint32_t pixel_move = tileStride / sizeof(uint32_t);
		pRowOutput += pixel_move;
		pNormalBuf += pixel_move;
		pDepthBuf  += pixel_move;
		++i;
	}
	while(i < tileHeight);	// TODO: [Stove] Is this a constant in reality?


}
#endif

void LightingPrepassWorker(Scheduler::Task* pTask)
{
#if defined(PLATFORM_SPU)
	PERF_EVENT("LightingPrepassWorker");

	LightingData::WorkerInput* pInput = (LightingData::WorkerInput*)Spu::FetchAlloc(pTask->Input, sizeof(LightingData::WorkerInput));

	// load light list & view information
	LightingData::View* pView = (LightingData::View*)Spu::FetchAllocAsync((void*)pInput->viewAddress, sizeof(LightingData::View), 2);
	LightingData::PointLight* pPointLights = (LightingData::PointLight*)Spu::FetchAllocAsync((void*)pInput->pointLightsAddress, pInput->pointLightsCount * sizeof(LightingData::PointLight), 2);
	LightingData::SpotLight* pSpotLights = (LightingData::SpotLight*)Spu::FetchAllocAsync((void*)pInput->spotLightsAddress, pInput->spotLightsCount * sizeof(LightingData::SpotLight), 2);

	const U32 iTilesX = (pInput->textureWidth + c_FBTileWidth - 1) / c_FBTileWidth;
	const U32 iTilesY = (pInput->textureHeight + c_FBTileHeight - 1) / c_FBTileHeight;

	const U32 iSubTilePixelsX = 32;
	const U32 iSubTilePixelsY = 16;
	const U32 iSubTilesX = c_FBTileWidth / iSubTilePixelsX;
	const U32 iSubTilesY = c_FBTileHeight / iSubTilePixelsY;

	const F32 fScreenDx	= 2.0f / pInput->textureWidth;
	const F32 fScreenDy = 2.0f / pInput->textureHeight;
	
	Spu::Wait(2);		// Spin on tag group.

	const qword inv_proj_x	= si_splat(1.0f / pView->fProjMatrix[0]);
	const qword inv_proj_y	= si_splat(1.0f / pView->fProjMatrix[5]);
	const qword minus_one	= si_splat(-1.0f);
	const qword plus_one	= si_splat(+1.0f);

	const qword screen_tile_dx	= si_fm(si_fm(si_splat(fScreenDx), si_splat((F32)iSubTilePixelsX)), inv_proj_x);
	const qword screen_tile_dy	= si_fm(si_fm(si_splat(fScreenDy), si_splat((F32)iSubTilePixelsY)), inv_proj_y);

	const qword inv_far_clip			= si_splat(1.0f / pView->fFarClip);
	const qword inv_near_clip			= si_splat(1.0f / pView->fNearClip);
	const qword inv_far_minus_inv_near	= si_fs(inv_far_clip, inv_near_clip);

	U8* normalBuffer0 = &normalBuffer[0];
	U8* normalBuffer1 = &normalBuffer[c_FBTileSize];
	U8* depthBuffer0 = &depthBuffer[0];
	U8* depthBuffer1 = &depthBuffer[c_FBTileSize];

	S32 iJob = MT::AtomicIncrement(pInput->pJobIndex);
	{
		// calculate tile position
		const U32 ty	= iJob / iTilesX;
		const U32 tx	= iJob - ty * iTilesX;
		const U32 x		= tx * c_FBTileWidth;
		const U32 y		= ty * c_FBTileHeight;
		// read in normals plus depth
		const U32 tileReadOffset = (tx & ~0x03) + (ty * iTilesX);
		const U32 normalBlockIndexIn = Spu::GetFBTileIndexForLinearTile(tx, ty, pInput->normalBufferBankOffset);
		const U32 depthBlockIndexIn = Spu::GetFBTileIndexForLinearTile(tx, ty, pInput->depthBufferBankOffset);
		const U32 normalOffset = (tileReadOffset + normalBlockIndexIn) * c_FBTileSize;
		const U32 depthOffset = (tileReadOffset + depthBlockIndexIn) * c_FBTileSize;
		cellDmaGet(tiledNormalBuffer, pInput->normalBufferAddress + normalOffset, c_FBTileSize, 0, 0, 0);
		cellDmaGet(tiledDepthBuffer, pInput->depthBufferAddress + depthOffset, c_FBTileSize, 0, 0, 0);
	}

	while(iJob < pInput->jobCount)
	{
		// calculate tile position
		const U32 ty	= iJob / iTilesX;
		const U32 tx	= iJob - ty * iTilesX;
		U32 x		= tx * c_FBTileWidth;
		U32 y		= ty * c_FBTileHeight;
		
		// wait for dma
		cellDmaWaitTagStatusAll(1 << 0);

		// detile buffers
		Spu::DetileBufferProcessor detileColourProcessor;
		Spu::DetileBuffer<Spu::DetileBufferProcessor>(detileColourProcessor, tiledNormalBuffer, normalBuffer0, tx & 0x03);
		
		vec_uint4 tileMinZ[8];
		vec_uint4 tileMaxZ[8];
		vec_uchar16 tileStencilMin[8];
		vec_uchar16 tileStencilMax[8];
		Spu::DetileDepthBufferRangeProcessor detileDepthProcessor(tileMinZ, tileMaxZ, tileStencilMin, tileStencilMax);
		Spu::DetileBuffer<Spu::DetileDepthBufferRangeProcessor>(detileDepthProcessor, tiledDepthBuffer, depthBuffer0, tx & 0x03);

		// pull in next job
		iJob = MT::AtomicIncrement(pInput->pJobIndex);
		if(iJob < pInput->jobCount)
		{
			const U32 ty	= iJob / iTilesX;
			const U32 tx	= iJob - ty * iTilesX;
			const U32 x		= tx * c_FBTileWidth;
			const U32 y		= ty * c_FBTileHeight;
			const U32 tileReadOffset = (tx & ~0x03) + (ty * iTilesX);
			const U32 normalBlockIndexIn = Spu::GetFBTileIndexForLinearTile(tx, ty, pInput->normalBufferBankOffset);
			const U32 depthBlockIndexIn = Spu::GetFBTileIndexForLinearTile(tx, ty, pInput->depthBufferBankOffset);
			const U32 normalOffset = (tileReadOffset + normalBlockIndexIn) * c_FBTileSize;
			const U32 depthOffset = (tileReadOffset + depthBlockIndexIn) * c_FBTileSize;
			cellDmaGet(tiledNormalBuffer, pInput->normalBufferAddress + normalOffset, c_FBTileSize, 0, 0, 0);
			cellDmaGet(tiledDepthBuffer, pInput->depthBufferAddress + depthOffset, c_FBTileSize, 0, 0, 0);
		}

		U8* pLightBuffer = normalBuffer0;
		// calculate lighting for each sub tile
		qword screen_tile_y0	= si_fms(plus_one, inv_proj_y, si_fm(screen_tile_dy, si_splat((F32)(ty*iSubTilesY))));
		qword screen_tile_y1	= si_fs(screen_tile_y0, screen_tile_dy);
		for(U32 sy = 0; sy < iSubTilesY; ++sy)
		{
			qword screen_tile_x0	= si_fma(minus_one, inv_proj_x, si_fm(screen_tile_dx, si_splat((F32)(tx*iSubTilesX))));
			qword screen_tile_x1	= si_fa(screen_tile_x0, screen_tile_dx);

			U8* pOutput = pLightBuffer + sy * iSubTilePixelsY * c_FBTileStride;
			U8* pNormal = pOutput;
			U8* pDepth	= depthBuffer0 + sy * iSubTilePixelsY * c_FBTileStride;

			for(U32 sx = 0; sx < iSubTilesX; ++sx)
			{
				qword min_z = (qword)tileMinZ[(sy << 1) + sx];
				qword max_z = (qword)tileMaxZ[(sy << 1) + sx];
				qword min_stencil = (qword)tileStencilMin[(sy << 1) + sx];
				qword max_stencil = (qword)tileStencilMax[(sy << 1) + sx];

				// evaluate the stencil min and max values
				min_stencil = min4_u8(si_shufb(min_stencil, min_stencil, shuffle_xxxx), si_shufb(min_stencil, min_stencil, shuffle_yyyy), si_shufb(min_stencil, min_stencil, shuffle_zzzz), si_shufb(min_stencil, min_stencil, shuffle_wwww));
				max_stencil = max4_u8(si_shufb(max_stencil, max_stencil, shuffle_xxxx), si_shufb(max_stencil, max_stencil, shuffle_yyyy), si_shufb(max_stencil, max_stencil, shuffle_zzzz), si_shufb(max_stencil, max_stencil, shuffle_wwww));
				min_stencil = min4_u8(si_shufb(min_stencil, min_stencil, shuffle_bytes_xxxx), si_shufb(min_stencil, min_stencil, shuffle_bytes_yyyy), si_shufb(min_stencil, min_stencil, shuffle_bytes_zzzz), si_shufb(min_stencil, min_stencil, shuffle_bytes_wwww));
				max_stencil = max4_u8(si_shufb(max_stencil, max_stencil, shuffle_bytes_xxxx), si_shufb(max_stencil, max_stencil, shuffle_bytes_yyyy), si_shufb(max_stencil, max_stencil, shuffle_bytes_zzzz), si_shufb(max_stencil, max_stencil, shuffle_bytes_wwww));

				const bool bStencilAllZero = (si_to_uchar(max_stencil) == 0);
				if(bStencilAllZero)
				{
					ProcessTileUnlit(pOutput, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride);
				}
				else
				{
					// evaluate depth min and max
					qword min_z_xy		= si_shufb(min_z, min_z, shuffle_xy00);
					qword min_z_zw		= si_shufb(min_z, min_z, shuffle_zw00);
					qword min_z_xy_zw	= si_selb(min_z_xy, min_z_zw, si_clgt(min_z_xy, min_z_zw));
					qword min_z_x		= si_shufb(min_z_xy_zw, min_z_xy_zw, shuffle_xxxx);
					qword min_z_y		= si_shufb(min_z_xy_zw, min_z_xy_zw, shuffle_yyyy);
					min_z				= si_selb(min_z_x, min_z_y, si_clgt(min_z_x, min_z_y));

					qword max_z_xy		= si_shufb(max_z, max_z, shuffle_xy00);
					qword max_z_zw		= si_shufb(max_z, max_z, shuffle_zw00);
					qword max_z_xy_zw	= si_selb(max_z_zw, max_z_xy, si_clgt(max_z_xy, max_z_zw));
					qword max_z_x		= si_shufb(max_z_xy_zw, max_z_xy_zw, shuffle_xxxx);
					qword max_z_y		= si_shufb(max_z_xy_zw, max_z_xy_zw, shuffle_yyyy);
					max_z				= si_selb(max_z_y, max_z_x, si_clgt(max_z_x, max_z_y));

					// convert the integer zbuffer values to view space z values
					min_z				= get_view_z_value(min_z, inv_far_minus_inv_near, inv_near_clip);
					max_z				= get_view_z_value(max_z, inv_far_minus_inv_near, inv_near_clip);

					VisibleLights visibleLights;
					GatherLights(&visibleLights, pPointLights, pInput->pointLightsCount, pSpotLights, pInput->spotLightsCount, screen_tile_x0, screen_tile_x1, screen_tile_y0, screen_tile_y1, max_z, min_z);

					// Don't call the lighting code if there are no lights hitting the tile.
					if(!visibleLights.pointLightCount && !visibleLights.spotLightCount)
						ProcessTileUnlit(pOutput, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride);
					else
					{
// TODO: [Stove] try out lodding the lights in the distance :)
// 						if(si_to_float(max_z) > 100.f && si_to_float(min_z) > 100.f)
// 							ProcessTileLitLod(pOutput, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride);
// 						else
							ProcessTileLit(&visibleLights, pView, pOutput, pNormal, pDepth, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride, (uint16_t)pInput->textureWidth, (uint16_t)pInput->textureHeight, x + sx * iSubTilePixelsX, y + sy * iSubTilePixelsY);
					}
				}

				pOutput += iSubTilePixelsX * sizeof(U32);
				pDepth	+= iSubTilePixelsX * sizeof(U32);
				pNormal += iSubTilePixelsX * sizeof(U32);
				
				screen_tile_x0 = screen_tile_x1;
				screen_tile_x1 = si_fa(screen_tile_x0, screen_tile_dx);
			}

			screen_tile_y0 = screen_tile_y1;
			screen_tile_y1 = si_fs(screen_tile_y0, screen_tile_dy);
		}

		// write tile
		{
			cellDmaWaitTagStatusAll(1 << 1);	// -- TODO: [Stove] Move this to before the de-tiling, alternate ls dma source so alternate tag group for spin/dma.
			for(U32 i = 0; i < c_FBTileHeight && (y + i) < pInput->textureHeight; ++i)
			{
				const U32 outputSize	= c_FBTilesX * c_RopTileStride;
				const U8* pOutput		= pLightBuffer + i * outputSize;
				const U32 outputAddress	= pInput->lightingBufferAddress + (y + i) * pInput->textureWidth * 4 + tx * outputSize;
				cellDmaPut(pOutput, outputAddress, outputSize, 1, 0, 0);
			}
						 
			U8* normalTemp = normalBuffer0;			// TODO: [Stove] xor trick ;-)
			normalBuffer0 = normalBuffer1;
			normalBuffer1 = normalTemp;
			U8* depthTemp = depthBuffer0;
			depthBuffer0 = depthBuffer1;
			depthBuffer1 = depthTemp;
		}
	}

	cellDmaWaitTagStatusAll(1 << 1);	// Wait on lighting buffer DMAs.
	Spu::HardSync(pInput->lightingBufferAddress, 0);
	Spu::WriteWord(pInput->gateAddress, pInput->gateValue);
	MT::AtomicSet((volatile uint32_t*)pInput->freeAddress, pInput->freeValue);
#endif
}

}; // namespace Deferred

JQ_DECLARE_SPU_MODULE(LightingPrepassWorkerModule, Deferred::LightingPrepassWorker, lightingprepassworker, "deferred.scheduler");
