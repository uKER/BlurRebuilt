#include "stdafx.h"
#include "Graphics/Lighting/JobQueuePrepass.h"

#include "Core/Threads/Atomic.h"
#include "Spu/Texture/Tile.h"
#include "Spu/Buffer.h"
#include "Spu/Common.h"
#include "Spu/Maths.h"
#include "System/Scheduler/JobQueue.h"

namespace Prepass
{

const U32 c_MaximumVisibleLights = 12;

struct VisibleLights
{
	LightingData::PointLight	pointLights[c_MaximumVisibleLights];
	U32							pointLightCount;
};

#if defined(PLATFORM_SPU)

void Generate2dFrustumPlane(qword* __restrict__ nx, qword* __restrict__ nz, const qword x0, const qword z0, const qword x1, const qword z1) __attribute__((always_inline));
void Generate2dFrustumPlane(qword* __restrict__ nx, qword* __restrict__ nz, const qword x0, const qword z0, const qword x1, const qword z1)		// TODO: [Stove] change to ptrs and restrict.
{
	qword dx		= si_fs(x1, x0);
	qword dz		= si_fs(z0, z1);
	qword len		= si_fma(dx, dx, si_fm(dz, dz));
	qword len_rcp	= si_frsqest(len);
		  len_rcp	= si_fi(len, len_rcp);
		  *nx		= si_fm(dz, len_rcp);
		  *nz		= si_fm(dx, len_rcp);
	return;
}

void GatherLights(VisibleLights* __restrict__ pLights, 
	const LightingData::PointLight* __restrict__ pPointLights, 
	const uint32_t pointLightCount, 
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

	qword plane_left_nx, plane_left_nz;
	qword plane_right_nx, plane_right_nz;
	qword plane_up_ny, plane_up_nz;
	qword plane_down_ny, plane_down_nz;
	Generate2dFrustumPlane(&plane_left_nx, &plane_left_nz, x00, z_min, x10, z_max);
	Generate2dFrustumPlane(&plane_right_nx, &plane_right_nz, x11, z_max, x01, z_min);
	Generate2dFrustumPlane(&plane_up_ny, &plane_up_nz, y10, z_max, y00, z_min);
	Generate2dFrustumPlane(&plane_down_ny, &plane_down_nz, y01, z_min, y11, z_max);

	// gather point lights
	{
		qword light_ptr 			= (const qword)si_from_ptr(&pPointLights[0].f[0]);
		qword light_colour			= si_lqd(light_ptr, 16);
		qword light_position		= si_lqd(light_ptr, 0);
		qword light_position_x		= si_shufb(light_position, light_position, shuffle_xxxx);
		qword light_position_y		= si_shufb(light_position, light_position, shuffle_yyyy);
		qword light_position_z		= si_shufb(light_position, light_position, shuffle_zzzz);
		qword light_radius			= si_shufb(light_colour, light_colour, shuffle_wwww);

		pLights->pointLightCount = 0;
		for(S32 i = 0; i < pointLightCount && pLights->pointLightCount < c_MaximumVisibleLights; ++i)
		{
			pLights->pointLights[pLights->pointLightCount] = pPointLights[i];

			const qword plane_dot_left	= si_fma(light_position_x, plane_left_nx, si_fms(light_position_z, plane_left_nz, light_radius));
			const qword plane_dot_right	= si_fma(light_position_x, plane_right_nx, si_fms(light_position_z, plane_right_nz, light_radius));
			const qword plane_dot_up	= si_fma(light_position_y, plane_up_ny, si_fms(light_position_z, plane_up_nz, light_radius));
			const qword plane_dot_down	= si_fma(light_position_y, plane_down_ny, si_fms(light_position_z, plane_down_nz, light_radius));

			const qword cmp_near		= si_fcgt(z_max, si_fs(light_position_z, light_radius));
			const qword cmp_far			= si_fcgt(si_fa(light_position_z, light_radius), z_min);
			const qword cmp_left		= si_fcgt(zero_float, plane_dot_left);
			const qword cmp_right		= si_fcgt(zero_float, plane_dot_right);
			const qword cmp_up			= si_fcgt(zero_float, plane_dot_up);
			const qword cmp_down		= si_fcgt(zero_float, plane_dot_down);
			const qword in_tile_rslt	= si_and(si_and(si_and(cmp_left, cmp_right), si_and(cmp_up, cmp_down)), si_and(cmp_near, cmp_far));

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
}

U8	tiledNormalDepthBuffer[c_FBTileSize] ALIGN128;
U8	normalDepthBuffer[c_FBTileSize*2] ALIGN128;

const float __attribute__((aligned(16))) expTable[48] = 
{
	1.0f,			1.0f,			1.0f,			1.0f,					// exp(0)
	0.367879441f,	0.367879441f,	0.367879441f,	0.367879441f,			// exp(-1)
	0.135335283f,	0.135335283f,	0.135335283f,	0.135335283f,			// exp(-2)
	0.0497870684f,	0.0497870684f,	0.0497870684f,	0.0497870684f,			// exp(-3)
	0.0183156389f,  0.0183156389f,  0.0183156389f,  0.0183156389f,			// exp(-4)
	0.006737947f,	0.006737947f,	0.006737947f,	0.006737947f,			// exp(-5)
	0.00247875218f,	0.00247875218f,	0.00247875218f,	0.00247875218f,			// exp(-6)
	0.000911881966f,0.000911881966f,0.000911881966f,0.000911881966f,		// exp(-7)
	0.000335462628f,0.000335462628f,0.000335462628f,0.000335462628f,		// exp(-8)
	0.000123409804f,0.000123409804f,0.000123409804f,0.000123409804f,		// exp(-9)
	0.000045399930f,0.000045399930f,0.000045399930f,0.000045399930f,		// exp(-10)
	0.000016701700f,0.000016701700f,0.000016701700f,0.000016701700,			// exp(-11)
};

//! fog, apply fog calculations to a batch of 16 pixels
static inline void fog(const uint16_t x0, 
					   const uint16_t x1, 
					   const uint16_t x2, 
					   const uint16_t x3, 
					   const uint16_t x4, 
					   const uint16_t x5, 
					   const uint16_t x6, 
					   const uint16_t x7, 
					   const uint16_t x8, 
					   const uint16_t x9, 
					   const uint16_t x10, 
					   const uint16_t x11, 
					   const uint16_t x12, 
					   const uint16_t x13, 
					   const uint16_t x14, 
					   const uint16_t x15, 
					   const qword uv_y0y1y2y3_in,
					   const qword uv_z0z1z2z3,
					   const qword uv_z4z5z6z7,
					   const qword uv_z8z9z10z11,
					   const qword uv_z12z13z14z15,
					   const qword to_screen_x,
					   const qword to_screen_y,
					   const qword far_clip,
					   const qword depth_fog_scale,
					   const qword depth_fog_offset,
					   const qword height_fog_density,
					   const qword height_fog_falloff,
					   const qword height_fog_plane_xxxx,
					   const qword height_fog_plane_yyyy,
					   const qword height_fog_plane_zzzz,
					   const qword height_fog_plane_wwww,
					   const F32* const __restrict pMatrix,
					   uint32_t* __restrict__ pResult) __attribute__((always_inline));
static inline void fog(const uint16_t x0, 
					   const uint16_t x1, 
					   const uint16_t x2, 
					   const uint16_t x3, 
					   const uint16_t x4, 
					   const uint16_t x5, 
					   const uint16_t x6, 
					   const uint16_t x7, 
					   const uint16_t x8, 
					   const uint16_t x9, 
					   const uint16_t x10, 
					   const uint16_t x11, 
					   const uint16_t x12, 
					   const uint16_t x13, 
					   const uint16_t x14, 
					   const uint16_t x15, 
					   const qword uv_y0y1y2y3_in,
					   const qword uv_z0z1z2z3,
					   const qword uv_z4z5z6z7,
					   const qword uv_z8z9z10z11,
					   const qword uv_z12z13z14z15,
					   const qword to_screen_x,
					   const qword to_screen_y,
					   const qword far_clip,
					   const qword depth_fog_scale,
					   const qword depth_fog_offset,
					   const qword height_fog_density,
					   const qword height_fog_falloff,
					   const qword height_fog_plane_xxxx,
					   const qword height_fog_plane_yyyy,
					   const qword height_fog_plane_zzzz,
					   const qword height_fog_plane_wwww,
					   const F32* const __restrict pMatrix,
					   uint32_t* __restrict__ pResult)
{
	const vector unsigned char _mask_00ff =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	const vector unsigned char _mask_000000ff =
	{ 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff };
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

	qword addr								= si_from_ptr(pMatrix);
	qword matrix_0							= si_lqd(addr, 0x00);
	qword matrix_1							= si_lqd(addr, 0x10);
	qword z_bits							= si_ilhu(0xffff);											// load constants.
		  z_bits							= si_iohl(z_bits, 0xff00);
	const qword mask_000000ff				= (const qword)_mask_000000ff;
	const qword mask_00ff					= si_fsmbi(0x00ff);
	const qword shuf_xxxx					= si_ila(0x010203);
	qword shuf_yyyy							= si_ilhu(0x0405);
		  shuf_yyyy							= si_iohl(shuf_yyyy, 0x0607);
	qword shuf_zzzz							= si_ilhu(0x0809);
		  shuf_zzzz							= si_iohl(shuf_zzzz, 0x0a0b);
	qword shuf_wwww							= si_ilhu(0x0c0d);
		  shuf_wwww							= si_iohl(shuf_wwww, 0x0e0f);

	const qword zero						= si_il(0x0000);
	const qword one							= si_ilhu(0x3f80);
	const qword one_half					= si_ilhu(0x3f00);
	const qword one_third					= si_iohl(si_ilhu(0x3eaa), 0xaaab);
	const qword one_quarter					= si_ilhu(0x3e80);
	const qword one_fifth					= si_iohl(si_ilhu(0x3e4c), 0xcccd);
	const qword one_sixth					= si_iohl(si_ilhu(0x3e2a), 0xaaab);
	const qword one_seventh					= si_iohl(si_ilhu(0x3e12), 0x4925);
	const qword one_eighth					= si_ilhu(0x3e00);
	const qword minus_one					= si_ilhu(0xbf80);
	const qword minus_ten					= si_ilhu(0xc120);
	const qword pixel_scale					= si_ilhu(0x3f7f);

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
	matrix_0								= si_shufb(matrix_0, matrix_0, shuf_xxxx);
	matrix_1								= si_shufb(matrix_1, matrix_1, shuf_yyyy);
	qword uv_x0x1							= si_shufb(si_from_uint(x0), si_from_uint(x1), shuf_AaAa);
	qword uv_x2x3							= si_shufb(si_from_uint(x2), si_from_uint(x3), shuf_AaAa);
	qword uv_x4x5							= si_shufb(si_from_uint(x4), si_from_uint(x5), shuf_AaAa);
	qword uv_x6x7							= si_shufb(si_from_uint(x6), si_from_uint(x7), shuf_AaAa);
	qword uv_x8x9							= si_shufb(si_from_uint(x8), si_from_uint(x9), shuf_AaAa);
	qword uv_x10x11							= si_shufb(si_from_uint(x10), si_from_uint(x11), shuf_AaAa);
	qword uv_x12x13							= si_shufb(si_from_uint(x12), si_from_uint(x13), shuf_AaAa);
	qword uv_x14x15							= si_shufb(si_from_uint(x14), si_from_uint(x15), shuf_AaAa);
	qword uv_x0x1x2x3						= si_selb(uv_x0x1, uv_x2x3, mask_00ff);
	qword uv_x4x5x6x7						= si_selb(uv_x4x5, uv_x6x7, mask_00ff);
	qword uv_x8x9x10x11						= si_selb(uv_x8x9, uv_x10x11, mask_00ff);
	qword uv_x12x13x14x15					= si_selb(uv_x12x13, uv_x14x15, mask_00ff);
		  uv_x0x1x2x3						= si_cuflt(uv_x0x1x2x3, 0x0);
		  uv_x4x5x6x7						= si_cuflt(uv_x4x5x6x7, 0x0);
		  uv_x8x9x10x11						= si_cuflt(uv_x8x9x10x11, 0x0);
		  uv_x12x13x14x15					= si_cuflt(uv_x12x13x14x15, 0x0);
		  uv_x0x1x2x3						= si_fms(uv_x0x1x2x3, to_screen_x, one);
		  uv_x4x5x6x7						= si_fms(uv_x4x5x6x7, to_screen_x, one);
		  uv_x8x9x10x11						= si_fms(uv_x8x9x10x11, to_screen_x, one);
		  uv_x12x13x14x15					= si_fms(uv_x12x13x14x15, to_screen_x, one);
	qword uv_y0y1y2y3						= si_fma(uv_y0y1y2y3_in, to_screen_y, one);
		  uv_x0x1x2x3						= si_fm(uv_x0x1x2x3, matrix_0);
		  uv_x4x5x6x7						= si_fm(uv_x4x5x6x7, matrix_0);
		  uv_x8x9x10x11						= si_fm(uv_x8x9x10x11, matrix_0);
		  uv_x12x13x14x15					= si_fm(uv_x12x13x14x15, matrix_0);
		  uv_y0y1y2y3						= si_fm(uv_y0y1y2y3, matrix_1);
	qword view_z0z1z2z3						= get_view_z_value4_linear_16(uv_z0z1z2z3, far_clip);
	qword view_z4z5z6z7						= get_view_z_value4_linear_16(uv_z4z5z6z7, far_clip);
	qword view_z8z9z10z11					= get_view_z_value4_linear_16(uv_z8z9z10z11, far_clip);
	qword view_z12z13z14z15					= get_view_z_value4_linear_16(uv_z12z13z14z15, far_clip);
	qword view_x0x1x2x3						= si_fm(uv_x0x1x2x3, view_z0z1z2z3);
	qword view_x4x5x6x7						= si_fm(uv_x4x5x6x7, view_z4z5z6z7);
	qword view_x8x9x10x11					= si_fm(uv_x8x9x10x11, view_z8z9z10z11);
	qword view_x12x13x14x15					= si_fm(uv_x12x13x14x15, view_z12z13z14z15);
	qword view_y0y1y2y3						= si_fm(uv_y0y1y2y3, view_z0z1z2z3);
	qword view_y4y5y6y7						= si_fm(uv_y0y1y2y3, view_z4z5z6z7);
	qword view_y8y9y10y11					= si_fm(uv_y0y1y2y3, view_z8z9z10z11);
	qword view_y12y13y14y15					= si_fm(uv_y0y1y2y3, view_z12z13z14z15);

	qword depth_fog_0						= si_fma(view_z0z1z2z3, depth_fog_scale, depth_fog_offset);
	qword depth_fog_1						= si_fma(view_z4z5z6z7, depth_fog_scale, depth_fog_offset);
	qword depth_fog_2						= si_fma(view_z8z9z10z11, depth_fog_scale, depth_fog_offset);
	qword depth_fog_3						= si_fma(view_z12z13z14z15, depth_fog_scale, depth_fog_offset);
	qword depth_fog_cmp_0					= si_fcgt(depth_fog_0, zero);
	qword depth_fog_cmp_1					= si_fcgt(depth_fog_1, zero);
	qword depth_fog_cmp_2					= si_fcgt(depth_fog_2, zero);
	qword depth_fog_cmp_3					= si_fcgt(depth_fog_3, zero);
		  depth_fog_0						= si_selb(depth_fog_0, zero, depth_fog_cmp_0);
		  depth_fog_1						= si_selb(depth_fog_1, zero, depth_fog_cmp_1);
		  depth_fog_2						= si_selb(depth_fog_2, zero, depth_fog_cmp_2);
		  depth_fog_3						= si_selb(depth_fog_3, zero, depth_fog_cmp_3);

	qword height_fog_f0_0					= si_fma(view_x0x1x2x3, height_fog_plane_xxxx, height_fog_plane_wwww);
	qword height_fog_f0_1					= si_fma(view_x4x5x6x7, height_fog_plane_xxxx, height_fog_plane_wwww);
	qword height_fog_f0_2					= si_fma(view_x8x9x10x11, height_fog_plane_xxxx, height_fog_plane_wwww);
	qword height_fog_f0_3					= si_fma(view_x12x13x14x15, height_fog_plane_xxxx, height_fog_plane_wwww);
		  height_fog_f0_0					= si_fma(view_y0y1y2y3, height_fog_plane_yyyy, height_fog_f0_0);
		  height_fog_f0_1					= si_fma(view_y4y5y6y7, height_fog_plane_yyyy, height_fog_f0_1);
		  height_fog_f0_2					= si_fma(view_y8y9y10y11, height_fog_plane_yyyy, height_fog_f0_2);
		  height_fog_f0_3					= si_fma(view_y12y13y14y15, height_fog_plane_yyyy, height_fog_f0_3);
		  height_fog_f0_0					= si_fma(view_z0z1z2z3, height_fog_plane_zzzz, height_fog_f0_0);
		  height_fog_f0_1					= si_fma(view_z4z5z6z7, height_fog_plane_zzzz, height_fog_f0_1);
		  height_fog_f0_2					= si_fma(view_z8z9z10z11, height_fog_plane_zzzz, height_fog_f0_2);
		  height_fog_f0_3					= si_fma(view_z12z13z14z15, height_fog_plane_zzzz, height_fog_f0_3);
	qword height_fog_cmp_0					= si_fcgt(height_fog_f0_0, height_fog_plane_wwww);
	qword height_fog_cmp_1					= si_fcgt(height_fog_f0_1, height_fog_plane_wwww);
	qword height_fog_cmp_2					= si_fcgt(height_fog_f0_2, height_fog_plane_wwww);
	qword height_fog_cmp_3					= si_fcgt(height_fog_f0_3, height_fog_plane_wwww);
	qword height_fog_min_0					= si_selb(height_fog_f0_0, height_fog_plane_wwww, height_fog_cmp_0);
	qword height_fog_min_1					= si_selb(height_fog_f0_1, height_fog_plane_wwww, height_fog_cmp_1);
	qword height_fog_min_2					= si_selb(height_fog_f0_2, height_fog_plane_wwww, height_fog_cmp_2);
	qword height_fog_min_3					= si_selb(height_fog_f0_3, height_fog_plane_wwww, height_fog_cmp_3);
	qword height_fog_max_0					= si_selb(height_fog_plane_wwww, height_fog_f0_0, height_fog_cmp_0);
	qword height_fog_max_1					= si_selb(height_fog_plane_wwww, height_fog_f0_1, height_fog_cmp_1);
	qword height_fog_max_2					= si_selb(height_fog_plane_wwww, height_fog_f0_2, height_fog_cmp_2);
	qword height_fog_max_3					= si_selb(height_fog_plane_wwww, height_fog_f0_3, height_fog_cmp_3);
	qword height_fog_ratio_0				= si_fs(height_fog_min_0, height_fog_max_0);
	qword height_fog_ratio_1				= si_fs(height_fog_min_1, height_fog_max_1);
	qword height_fog_ratio_2				= si_fs(height_fog_min_2, height_fog_max_2);
	qword height_fog_ratio_3				= si_fs(height_fog_min_3, height_fog_max_3);
		  height_fog_ratio_0				= si_frest(height_fog_ratio_0);
		  height_fog_ratio_1				= si_frest(height_fog_ratio_1);
		  height_fog_ratio_2				= si_frest(height_fog_ratio_2);
		  height_fog_ratio_3				= si_frest(height_fog_ratio_3);
		  height_fog_ratio_0				= si_fm(height_fog_ratio_0, height_fog_min_0);
		  height_fog_ratio_1				= si_fm(height_fog_ratio_1, height_fog_min_1);
		  height_fog_ratio_2				= si_fm(height_fog_ratio_2, height_fog_min_2);
		  height_fog_ratio_3				= si_fm(height_fog_ratio_3, height_fog_min_3);
		  height_fog_ratio_0				= si_cfltu(height_fog_ratio_0, 0x20);
		  height_fog_ratio_1				= si_cfltu(height_fog_ratio_1, 0x20);
		  height_fog_ratio_2				= si_cfltu(height_fog_ratio_2, 0x20);
		  height_fog_ratio_3				= si_cfltu(height_fog_ratio_3, 0x20);
		  height_fog_ratio_0				= si_cuflt(height_fog_ratio_0, 0x20);
		  height_fog_ratio_1				= si_cuflt(height_fog_ratio_1, 0x20);
		  height_fog_ratio_2				= si_cuflt(height_fog_ratio_2, 0x20);
		  height_fog_ratio_3				= si_cuflt(height_fog_ratio_3, 0x20);	
	qword height_fog_distance_0				= si_fm(height_fog_min_0, height_fog_falloff);
	qword height_fog_distance_1				= si_fm(height_fog_min_0, height_fog_falloff);
	qword height_fog_distance_2				= si_fm(height_fog_min_0, height_fog_falloff);
	qword height_fog_distance_3				= si_fm(height_fog_min_0, height_fog_falloff);
		  height_fog_distance_0				= si_cfltu(height_fog_distance_0, 0x20);
		  height_fog_distance_1				= si_cfltu(height_fog_distance_1, 0x20);
		  height_fog_distance_2				= si_cfltu(height_fog_distance_2, 0x20);
		  height_fog_distance_3				= si_cfltu(height_fog_distance_3, 0x20);
		  height_fog_distance_0				= si_cuflt(height_fog_distance_0, 0x20);
		  height_fog_distance_1				= si_cuflt(height_fog_distance_1, 0x20);
		  height_fog_distance_2				= si_cuflt(height_fog_distance_2, 0x20);
		  height_fog_distance_3				= si_cuflt(height_fog_distance_3, 0x20);	
	qword height_fog_0						= si_fm(height_fog_ratio_0, height_fog_distance_0);
	qword height_fog_1						= si_fm(height_fog_ratio_1, height_fog_distance_1);
	qword height_fog_2						= si_fm(height_fog_ratio_2, height_fog_distance_2);
	qword height_fog_3						= si_fm(height_fog_ratio_3, height_fog_distance_3);
		  height_fog_0						= si_fm(height_fog_0, view_z0z1z2z3);
		  height_fog_1						= si_fm(height_fog_1, view_z4z5z6z7);
		  height_fog_2						= si_fm(height_fog_2, view_z8z9z10z11);
		  height_fog_3						= si_fm(height_fog_3, view_z12z13z14z15);

	qword fog_amount_0						= si_fma(height_fog_0, height_fog_density, depth_fog_0);
	qword fog_amount_1						= si_fma(height_fog_1, height_fog_density, depth_fog_1);
	qword fog_amount_2						= si_fma(height_fog_2, height_fog_density, depth_fog_2);
	qword fog_amount_3						= si_fma(height_fog_3, height_fog_density, depth_fog_3);
	qword fog_cmp_0							= si_fcgt(fog_amount_0, minus_ten);
	qword fog_cmp_1							= si_fcgt(fog_amount_1, minus_ten);
	qword fog_cmp_2							= si_fcgt(fog_amount_2, minus_ten);
	qword fog_cmp_3							= si_fcgt(fog_amount_3, minus_ten);
		  fog_amount_0						= si_selb(minus_ten, fog_amount_0, fog_cmp_0);
		  fog_amount_1						= si_selb(minus_ten, fog_amount_1, fog_cmp_1);
		  fog_amount_2						= si_selb(minus_ten, fog_amount_2, fog_cmp_2);
		  fog_amount_3						= si_selb(minus_ten, fog_amount_3, fog_cmp_3);

	// exp

	qword fog_amount_half_0					= si_fs(fog_amount_0, one_half);
	qword fog_amount_half_1					= si_fs(fog_amount_1, one_half);
	qword fog_amount_half_2					= si_fs(fog_amount_2, one_half);
	qword fog_amount_half_3					= si_fs(fog_amount_3, one_half);
	qword fog_amount_int_0					= si_cflts(fog_amount_half_0, 0x00);
	qword fog_amount_int_1					= si_cflts(fog_amount_half_1, 0x00);
	qword fog_amount_int_2					= si_cflts(fog_amount_half_2, 0x00);
	qword fog_amount_int_3					= si_cflts(fog_amount_half_3, 0x00);
	qword fog_amount_round_0				= si_csflt(fog_amount_int_0, 0x00);
	qword fog_amount_round_1				= si_csflt(fog_amount_int_1, 0x00);
	qword fog_amount_round_2				= si_csflt(fog_amount_int_2, 0x00);
	qword fog_amount_round_3				= si_csflt(fog_amount_int_3, 0x00);
	qword sign_bit_mask						= si_ilhu(0x7fff);
		  sign_bit_mask						= si_iohl(sign_bit_mask, 0xffff);
		  fog_amount_int_0					= si_mpyi(fog_amount_int_0, -1);
		  fog_amount_int_1					= si_mpyi(fog_amount_int_1, -1);
		  fog_amount_int_2					= si_mpyi(fog_amount_int_2, -1);
		  fog_amount_int_3					= si_mpyi(fog_amount_int_3, -1);

	qword fog_base_0						= si_fs(fog_amount_0, fog_amount_round_0);
	qword fog_base_1						= si_fs(fog_amount_1, fog_amount_round_1);
	qword fog_base_2						= si_fs(fog_amount_2, fog_amount_round_2);
	qword fog_base_3						= si_fs(fog_amount_3, fog_amount_round_3);

	qword fog_0								= si_fma(fog_base_0, one_eighth, one);
	qword fog_1								= si_fma(fog_base_1, one_eighth, one);
	qword fog_2								= si_fma(fog_base_2, one_eighth, one);
	qword fog_3								= si_fma(fog_base_3, one_eighth, one);
		  fog_0								= si_fm(fog_0, one_seventh);
		  fog_1								= si_fm(fog_1, one_seventh);
		  fog_2								= si_fm(fog_2, one_seventh);
		  fog_3								= si_fm(fog_3, one_seventh);
		  fog_0								= si_fma(fog_base_0, fog_0, one);
		  fog_1								= si_fma(fog_base_1, fog_1, one);
		  fog_2								= si_fma(fog_base_2, fog_2, one);
		  fog_3								= si_fma(fog_base_3, fog_3, one);
		  fog_0								= si_fm(fog_0, one_sixth);
		  fog_1								= si_fm(fog_1, one_sixth);
		  fog_2								= si_fm(fog_2, one_sixth);
		  fog_3								= si_fm(fog_3, one_sixth);
		  fog_0								= si_fma(fog_base_0, fog_0, one);
		  fog_1								= si_fma(fog_base_1, fog_1, one);
		  fog_2								= si_fma(fog_base_2, fog_2, one);
		  fog_3								= si_fma(fog_base_3, fog_3, one);
		  fog_0								= si_fm(fog_0, one_fifth);
		  fog_1								= si_fm(fog_1, one_fifth);
		  fog_2								= si_fm(fog_2, one_fifth);
		  fog_3								= si_fm(fog_3, one_fifth);
		  fog_0								= si_fma(fog_base_0, fog_0, one);
		  fog_1								= si_fma(fog_base_1, fog_1, one);
		  fog_2								= si_fma(fog_base_2, fog_2, one);
		  fog_3								= si_fma(fog_base_3, fog_3, one);
		  fog_0								= si_fm(fog_0, one_quarter);
		  fog_1								= si_fm(fog_1, one_quarter);
		  fog_2								= si_fm(fog_2, one_quarter);
		  fog_3								= si_fm(fog_3, one_quarter);
		  fog_0								= si_fma(fog_base_0, fog_0, one);
		  fog_1								= si_fma(fog_base_1, fog_1, one);
		  fog_2								= si_fma(fog_base_2, fog_2, one);
		  fog_3								= si_fma(fog_base_3, fog_3, one);
		  fog_0								= si_fm(fog_0, one_third);
		  fog_1								= si_fm(fog_1, one_third);
		  fog_2								= si_fm(fog_2, one_third);
		  fog_3								= si_fm(fog_3, one_third);
		  fog_0								= si_fma(fog_base_0, fog_0, one);
		  fog_1								= si_fma(fog_base_1, fog_1, one);
		  fog_2								= si_fma(fog_base_2, fog_2, one);
		  fog_3								= si_fma(fog_base_3, fog_3, one);
		  fog_0								= si_fm(fog_0, one_half);
		  fog_1								= si_fm(fog_1, one_half);
		  fog_2								= si_fm(fog_2, one_half);
		  fog_3								= si_fm(fog_3, one_half);
		  fog_0								= si_fma(fog_base_0, fog_0, one);
		  fog_1								= si_fma(fog_base_1, fog_1, one);
		  fog_2								= si_fma(fog_base_2, fog_2, one);
		  fog_3								= si_fma(fog_base_3, fog_3, one);
		  fog_0								= si_fma(fog_base_0, fog_0, one);
		  fog_1								= si_fma(fog_base_1, fog_1, one);
		  fog_2								= si_fma(fog_base_2, fog_2, one);
		  fog_3								= si_fma(fog_base_3, fog_3, one);

		  fog_amount_int_0					= si_shli(fog_amount_int_0, 0x04);
		  fog_amount_int_1					= si_shli(fog_amount_int_1, 0x04);
		  fog_amount_int_2					= si_shli(fog_amount_int_2, 0x04);
		  fog_amount_int_3					= si_shli(fog_amount_int_3, 0x04);

	qword fog_exp_z0_0						= si_lqx(si_from_ptr(&expTable[0]), fog_amount_int_0);
	qword fog_exp_z0_1						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_0, 0x04));
	qword fog_exp_z0_2						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_0, 0x08));
	qword fog_exp_z0_3						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_0, 0x0C));
	qword fog_exp_z0_4						= si_lqx(si_from_ptr(&expTable[0]), fog_amount_int_1);
	qword fog_exp_z0_5						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_1, 0x04));
	qword fog_exp_z0_6						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_1, 0x08));
	qword fog_exp_z0_7						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_1, 0x0C));
	qword fog_exp_z0_8						= si_lqx(si_from_ptr(&expTable[0]), fog_amount_int_2);
	qword fog_exp_z0_9						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_2, 0x04));
	qword fog_exp_z0_10						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_2, 0x08));
	qword fog_exp_z0_11						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_2, 0x0C));
	qword fog_exp_z0_12						= si_lqx(si_from_ptr(&expTable[0]), fog_amount_int_3);
	qword fog_exp_z0_13						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_3, 0x04));
	qword fog_exp_z0_14						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_3, 0x08));
	qword fog_exp_z0_15						= si_lqx(si_from_ptr(&expTable[0]), si_rotqbyi(fog_amount_int_3, 0x0C));

	const vector unsigned char _shuf_Aa00 = 
	 { 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
	const vector unsigned char _shuf_00Aa =
	 { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13 };
	
	qword shuf_Aa00							= (const qword)_shuf_Aa00;
	qword shuf_00Aa							= (const qword)_shuf_00Aa;
	qword fog_exp_z0_01						= si_shufb(fog_exp_z0_0, fog_exp_z0_1, shuf_Aa00);
	qword fog_exp_z0_23						= si_shufb(fog_exp_z0_2, fog_exp_z0_3, shuf_00Aa);
	qword fog_exp_z0_0123					= si_selb(fog_exp_z0_01, fog_exp_z0_23, mask_00ff);
	qword fog_exp_z0_45						= si_shufb(fog_exp_z0_4, fog_exp_z0_5, shuf_Aa00);
	qword fog_exp_z0_67						= si_shufb(fog_exp_z0_6, fog_exp_z0_7, shuf_00Aa);
	qword fog_exp_z0_4567					= si_selb(fog_exp_z0_45, fog_exp_z0_67, mask_00ff);
	qword fog_exp_z0_89						= si_shufb(fog_exp_z0_8, fog_exp_z0_9, shuf_Aa00);
	qword fog_exp_z0_1011					= si_shufb(fog_exp_z0_10, fog_exp_z0_11, shuf_00Aa);
	qword fog_exp_z0_891011					= si_selb(fog_exp_z0_89, fog_exp_z0_1011, mask_00ff);
	qword fog_exp_z0_1213					= si_shufb(fog_exp_z0_12, fog_exp_z0_13, shuf_Aa00);
	qword fog_exp_z0_1415					= si_shufb(fog_exp_z0_14, fog_exp_z0_15, shuf_00Aa);
	qword fog_exp_z0_12131415				= si_selb(fog_exp_z0_1213, fog_exp_z0_1415, mask_00ff);

		  fog_0								= si_fm(fog_0, fog_exp_z0_0123);
		  fog_1								= si_fm(fog_1, fog_exp_z0_4567);
		  fog_2								= si_fm(fog_2, fog_exp_z0_891011);
		  fog_3								= si_fm(fog_3, fog_exp_z0_12131415);

	const vector unsigned char _shuf_uint =
	{ 0x03, 0x80, 0x80, 0x80, 0x07, 0x80, 0x80, 0x80, 0x0B, 0x80, 0x80, 0x80, 0x0F, 0x80, 0x80, 0x80 };
	const qword shuf_uint					= (const qword)_shuf_uint;

		  fog_0								= si_fm(fog_0, pixel_scale);
		  fog_1								= si_fm(fog_1, pixel_scale);
		  fog_2								= si_fm(fog_2, pixel_scale);
		  fog_3								= si_fm(fog_3, pixel_scale);

	qword pixel_0 							= si_cfltu(fog_0, 0x08);
	qword pixel_1 							= si_cfltu(fog_1, 0x08);
	qword pixel_2 							= si_cfltu(fog_2, 0x08);
	qword pixel_3 							= si_cfltu(fog_3, 0x08);
		  pixel_0							= si_shufb(pixel_0, pixel_0, shuf_uint);
		  pixel_1							= si_shufb(pixel_1, pixel_1, shuf_uint);
		  pixel_2							= si_shufb(pixel_2, pixel_2, shuf_uint);
		  pixel_3							= si_shufb(pixel_3, pixel_3, shuf_uint);

	const qword base_addr					= si_from_ptr(pResult);
	si_stqd(pixel_0, base_addr, 0x00);		// Batch LS writes.
	si_stqd(pixel_1, base_addr, 0x10);
	si_stqd(pixel_2, base_addr, 0x20);
	si_stqd(pixel_3, base_addr, 0x30);
	return;
}

static inline void ProcessTileFog(LightingData::Fog* __restrict__ pFog,
								  LightingData::View* __restrict__ pView,
								  U8* pOutput, 
								  U8* pNormalDepthBuffer,
								  const uint32_t tileWidth, 
								  const uint32_t tileHeight, 
								  const uint32_t tileStride, 
								  const qword to_screen_x,
								  const qword to_screen_y,
								  const uint16_t x, 
								  const uint16_t y) __attribute__((always_inline));
static inline void ProcessTileFog(LightingData::Fog* __restrict__ pFog, 
								  LightingData::View* __restrict__ pView, 
								  U8* pOutput, 
								  U8* pNormalDepthBuffer,
								  const uint32_t tileWidth, 
								  const uint32_t tileHeight, 
								  const uint32_t tileStride, 
								  const qword to_screen_x,
								  const qword to_screen_y,
								  const uint16_t x, 
								  const uint16_t y)
{
	qword y_val							= si_cuflt(si_from_uint(y), 0x0);
	const qword shuf_xxxx				= si_ila(0x010203);
	const qword shuf_yyyy				= si_iohl(si_ilhu(0x0405), 0x0607);
	const qword shuf_zzzz				= si_iohl(si_ilhu(0x0809), 0x0A0B);
	const qword shuf_wwww				= si_iohl(si_ilhu(0x0C0D), 0x0E0F);
	const qword far_clip				= si_from_float(pView->fFarClip);
	const qword minus_one				= si_ilhu(0xbf80);
	uint32_t* pRowInput					= (uint32_t*)pNormalDepthBuffer;
	uint32_t* pRowOutput				= (uint32_t*)pOutput;
	uint16_t i							= 0;
	y_val								= si_shufb(y_val, y_val, shuf_xxxx);

	const qword	depth_fog_density			= si_shufb(si_from_float(pFog->fDepthFogDensity), si_from_float(pFog->fDepthFogDensity), shuf_xxxx);
	const qword	depth_fog_near_distance		= si_shufb(si_from_float(pFog->fDepthFogNearDistance), si_from_float(pFog->fDepthFogNearDistance), shuf_xxxx);
	const qword	height_fog_density			= si_fm(si_shufb(si_from_float(pFog->fHeightFogDensity), si_from_float(pFog->fHeightFogDensity), shuf_xxxx), minus_one);
	const qword	height_fog_falloff			= si_fm(si_shufb(si_from_float(pFog->fHeightFogFalloff), si_from_float(pFog->fHeightFogFalloff), shuf_xxxx), minus_one);
	const qword	height_fog_plane			= si_lqd(si_from_ptr(&pFog->fHeightFogPlane), 0x0);
	const qword depth_fog_scale				= si_fm(depth_fog_density, minus_one);
	const qword depth_fog_offset			= si_fm(depth_fog_density, depth_fog_near_distance);
	const qword height_fog_plane_xxxx		= si_shufb(height_fog_plane, height_fog_plane, shuf_xxxx);
	const qword height_fog_plane_yyyy		= si_shufb(height_fog_plane, height_fog_plane, shuf_yyyy);
	const qword height_fog_plane_zzzz		= si_shufb(height_fog_plane, height_fog_plane, shuf_zzzz);
	const qword height_fog_plane_wwww		= si_shufb(height_fog_plane, height_fog_plane, shuf_wwww);

	do
	{
		qword i_val						= si_shufb(si_from_uint(i), si_from_uint(i), shuf_xxxx);
		qword depth_addr				= si_from_ptr(pRowInput);
		qword depth0					= si_lqd(depth_addr, 0x0); 
		qword depth1					= si_lqd(depth_addr, 0x10);
		qword depth2 					= si_lqd(depth_addr, 0x20);
		i_val							= si_cuflt(i_val, 0x0);
		qword depth3 					= si_lqd(depth_addr, 0x30);
		qword depth4 					= si_lqd(depth_addr, 0x40);
		qword depth5 					= si_lqd(depth_addr, 0x50);
		qword depth6 					= si_lqd(depth_addr, 0x60);
		qword depth7 					= si_lqd(depth_addr, 0x70);
		qword y_vals					= si_fa(y_val, i_val);

		fog(x + 0, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7, x + 8, x + 9, x + 10, x + 11, x + 12, x + 13, x + 14, x + 15, y_vals, depth0, depth1, depth2, depth3, to_screen_x, to_screen_y, far_clip, depth_fog_scale, depth_fog_offset, height_fog_density, height_fog_falloff, height_fog_plane_xxxx, height_fog_plane_yyyy, height_fog_plane_zzzz, height_fog_plane_wwww, (const F32* const)&pView->fProjInverseMatrix, &pRowOutput[0]);
		fog(x + 16, x + 17, x + 18, x + 19, x + 20, x + 21, x + 22, x + 23, x + 24, x + 25, x + 26, x + 27, x + 28, x + 29, x + 30, x + 31, y_vals, depth4, depth5, depth6, depth7, to_screen_x, to_screen_y, far_clip, depth_fog_scale, depth_fog_offset, height_fog_density, height_fog_falloff, height_fog_plane_xxxx, height_fog_plane_yyyy, height_fog_plane_zzzz, height_fog_plane_wwww, (const F32* const)&pView->fProjInverseMatrix, &pRowOutput[16]);

		// Advance our buffer pointers to next row.
		uint32_t pixel_move = tileStride / sizeof(uint32_t);
		pRowOutput += pixel_move;
		pRowInput += pixel_move;
		++i;
	}
	while(i < tileHeight);
}

void ProcessTileUnlit(U8* __restrict__ pOutput, const U32 tileWidth, const U32 tileHeight, const U32 tileStride)
{
	U8* __restrict__ pRowOutput = pOutput;
	for(U32 y = 0; y < tileHeight; ++y)
	{
		memset(pRowOutput, 0xFF000000, tileWidth * sizeof(U32));
		pRowOutput += tileStride;
	}
}

void ProcessTileWhite(U8* __restrict__ pOutput, const U32 tileWidth, const U32 tileHeight, const U32 tileStride)
{
	U8* __restrict__ pRowOutput = pOutput;
	for(U32 y = 0; y < tileHeight; ++y)
	{
		memset(pRowOutput, 0xFFFFFFFF, tileWidth * sizeof(U32));
		pRowOutput += tileStride;
	}
}

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

	const qword to_screen_x,
	const qword to_screen_y,

	uint32_t* __restrict__ pResult,	

	VisibleLights* __restrict__ pVisibleLights, 

	const qword far_clip, 
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

	const qword to_screen_x,
	const qword to_screen_y,

	uint32_t* __restrict__ pResult,	

	VisibleLights* __restrict__ pVisibleLights, 

	const qword far_clip, 
	const F32* const __restrict pMatrix)
{
	const vector unsigned char _mask_00ff =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	const vector unsigned char _mask_000f =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff };
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

	qword addr								= si_from_ptr(pMatrix);
	qword matrix_0							= si_lqd(addr, 0x00);
	qword matrix_1							= si_lqd(addr, 0x10);
	qword z_bits							= si_ilhu(0xffff);											// load constants.
		  z_bits							= si_iohl(z_bits, 0xff00);
	const qword mask_00ff					= (const qword)_mask_00ff;
	const qword shuf_xxxx					= si_ila(0x010203);
	qword shuf_yyyy							= si_ilhu(0x0405);
		  shuf_yyyy							= si_iohl(shuf_yyyy, 0x0607);
	qword shuf_zzzz							= si_ilhu(0x0809);
		  shuf_zzzz							= si_iohl(shuf_zzzz, 0x0a0b);
	qword shuf_wwww							= si_ilhu(0x0c0d);
		  shuf_wwww							= si_iohl(shuf_wwww, 0x0e0f);
	const qword zero						= si_il(0x0000);
	const qword point_two_five				= si_ilhu(0x3e80);
	const qword point_seven_five			= si_ilhu(0x3f40);
	qword diffuse0							= zero;
	qword diffuse1							= zero;
	qword diffuse2							= zero;
	qword diffuse3							= zero;  
	qword diffuse4							= zero;
	qword diffuse5							= zero;
	qword diffuse6							= zero;
	qword diffuse7							= zero;  
	qword diffuse8							= zero;
	qword diffuse9							= zero;
	qword diffuse10							= zero;
	qword diffuse11							= zero;  
	qword diffuse12							= zero;
	qword diffuse13							= zero;
	qword diffuse14							= zero;
	qword diffuse15							= zero;  
	const qword one							= si_ilhu(0x3f80);
	const qword two							= si_ilhu(0x4000);
	const qword minus_one					= si_ilhu(0xbf80);
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
	matrix_0								= si_shufb(matrix_0, matrix_0, shuf_xxxx);
	matrix_1								= si_shufb(matrix_1, matrix_1, shuf_yyyy);
	qword uv_x0x1							= si_shufb(si_from_uint(x0), si_from_uint(x1), shuf_AaAa);
	qword uv_x2x3							= si_shufb(si_from_uint(x2), si_from_uint(x3), shuf_AaAa);
	qword uv_x4x5							= si_shufb(si_from_uint(x4), si_from_uint(x5), shuf_AaAa);
	qword uv_x6x7							= si_shufb(si_from_uint(x6), si_from_uint(x7), shuf_AaAa);
	qword uv_x8x9							= si_shufb(si_from_uint(x8), si_from_uint(x9), shuf_AaAa);
	qword uv_x10x11							= si_shufb(si_from_uint(x10), si_from_uint(x11), shuf_AaAa);
	qword uv_x12x13							= si_shufb(si_from_uint(x12), si_from_uint(x13), shuf_AaAa);
	qword uv_x14x15							= si_shufb(si_from_uint(x14), si_from_uint(x15), shuf_AaAa);
	qword uv_x0x1x2x3						= si_selb(uv_x0x1, uv_x2x3, mask_00ff);
	qword uv_x4x5x6x7						= si_selb(uv_x4x5, uv_x6x7, mask_00ff);
	qword uv_x8x9x10x11						= si_selb(uv_x8x9, uv_x10x11, mask_00ff);
	qword uv_x12x13x14x15					= si_selb(uv_x12x13, uv_x14x15, mask_00ff);
		  uv_x0x1x2x3						= si_cuflt(uv_x0x1x2x3, 0x0);
		  uv_x4x5x6x7						= si_cuflt(uv_x4x5x6x7, 0x0);
		  uv_x8x9x10x11						= si_cuflt(uv_x8x9x10x11, 0x0);
		  uv_x12x13x14x15					= si_cuflt(uv_x12x13x14x15, 0x0);
		  uv_x0x1x2x3						= si_fms(uv_x0x1x2x3, to_screen_x, one);
		  uv_x4x5x6x7						= si_fms(uv_x4x5x6x7, to_screen_x, one);
		  uv_x8x9x10x11						= si_fms(uv_x8x9x10x11, to_screen_x, one);
		  uv_x12x13x14x15					= si_fms(uv_x12x13x14x15, to_screen_x, one);
	qword uv_y0y1y2y3						= si_fma(uv_y0y1y2y3_in, to_screen_y, one);
		  uv_x0x1x2x3						= si_fm(uv_x0x1x2x3, matrix_0);
		  uv_x4x5x6x7						= si_fm(uv_x4x5x6x7, matrix_0);
		  uv_x8x9x10x11						= si_fm(uv_x8x9x10x11, matrix_0);
		  uv_x12x13x14x15					= si_fm(uv_x12x13x14x15, matrix_0);
		  uv_y0y1y2y3						= si_fm(uv_y0y1y2y3, matrix_1);
	qword view_z0z1z2z3						= get_view_z_value4_linear_16(uv_z0z1z2z3, far_clip);
	qword view_z4z5z6z7						= get_view_z_value4_linear_16(uv_z4z5z6z7, far_clip);
	qword view_z8z9z10z11					= get_view_z_value4_linear_16(uv_z8z9z10z11, far_clip);
	qword view_z12z13z14z15					= get_view_z_value4_linear_16(uv_z12z13z14z15, far_clip);
	qword view_x0x1x2x3						= si_fm(uv_x0x1x2x3, view_z0z1z2z3);
	qword view_x4x5x6x7						= si_fm(uv_x4x5x6x7, view_z4z5z6z7);
	qword view_x8x9x10x11					= si_fm(uv_x8x9x10x11, view_z8z9z10z11);
	qword view_x12x13x14x15					= si_fm(uv_x12x13x14x15, view_z12z13z14z15);
	qword view_y0y1y2y3						= si_fm(uv_y0y1y2y3, view_z0z1z2z3);
	qword view_y4y5y6y7						= si_fm(uv_y0y1y2y3, view_z4z5z6z7);
	qword view_y8y9y10y11					= si_fm(uv_y0y1y2y3, view_z8z9z10z11);
	qword view_y12y13y14y15					= si_fm(uv_y0y1y2y3, view_z12z13z14z15);
	const qword two_five_five				= si_ilhu(0x437f);
	qword n_x0x1y0y1						= si_shufb(normal0, normal1, shuf_AaBb);
	qword n_y2y3x2x3						= si_shufb(normal2, normal3, shuf_BbAa);
	qword n_x4x5y4y5						= si_shufb(normal4, normal5, shuf_AaBb);
	qword n_y6y7x6x7						= si_shufb(normal6, normal7, shuf_BbAa);
	qword n_x8x9y8y9						= si_shufb(normal8, normal9, shuf_AaBb);
	qword n_y10y11x10x11					= si_shufb(normal10, normal11, shuf_BbAa);
	qword n_x12x13y12y13					= si_shufb(normal12, normal13, shuf_AaBb);
	qword n_y14y15x14x15					= si_shufb(normal14, normal15, shuf_BbAa);
	qword view_normal_x0x1x2x3				= si_selb(n_x0x1y0y1, n_y2y3x2x3, mask_00ff);
	qword view_normal_y0y1y2y3				= si_shufb(n_x0x1y0y1, n_y2y3x2x3, shuf_CDab);
	qword view_normal_x4x5x6x7				= si_selb(n_x4x5y4y5, n_y6y7x6x7, mask_00ff);
	qword view_normal_y4y5y6y7				= si_shufb(n_x4x5y4y5, n_y6y7x6x7, shuf_CDab);
	qword view_normal_x8x9x10x11			= si_selb(n_x8x9y8y9, n_y10y11x10x11, mask_00ff);
	qword view_normal_y8y9y10y11			= si_shufb(n_x8x9y8y9, n_y10y11x10x11, shuf_CDab);
	qword view_normal_x12x13x14x15			= si_selb(n_x12x13y12y13, n_y14y15x14x15, mask_00ff);
	qword view_normal_y12y13y14y15			= si_shufb(n_x12x13y12y13, n_y14y15x14x15, shuf_CDab);
	qword view_normal_sq_x0x1x2x3			= si_fm(view_normal_x0x1x2x3, view_normal_x0x1x2x3);
	qword view_normal_sq_x4x5x6x7			= si_fm(view_normal_x4x5x6x7, view_normal_x4x5x6x7);
	qword view_normal_sq_x8x9x10x11			= si_fm(view_normal_x8x9x10x11, view_normal_x8x9x10x11);
	qword view_normal_sq_x12x13x14x15		= si_fm(view_normal_x12x13x14x15, view_normal_x12x13x14x15);
	qword view_normal_sq_xy0xy1xy2xy3		= si_fma(view_normal_y0y1y2y3, view_normal_y0y1y2y3, view_normal_sq_x0x1x2x3);
	qword view_normal_sq_xy4xy5xy6xy7		= si_fma(view_normal_y4y5y6y7, view_normal_y4y5y6y7, view_normal_sq_x4x5x6x7);
	qword view_normal_sq_xy8xy9xy10xy11		= si_fma(view_normal_y8y9y10y11, view_normal_y8y9y10y11, view_normal_sq_x8x9x10x11);
	qword view_normal_sq_xy12xy13xy14xy15	= si_fma(view_normal_y12y13y14y15, view_normal_y12y13y14y15, view_normal_sq_x12x13x14x15);
	qword view_normal_sq_z0z1z2z3			= si_fs(one, view_normal_sq_xy0xy1xy2xy3);
	qword view_normal_sq_z4z5z6z7			= si_fs(one, view_normal_sq_xy4xy5xy6xy7);
	qword view_normal_sq_z8z9z10z11			= si_fs(one, view_normal_sq_xy8xy9xy10xy11);
	qword view_normal_sq_z12z13z14z15		= si_fs(one, view_normal_sq_xy12xy13xy14xy15);
	qword view_normal_rcp_z0z1z2z3			= si_frsqest(view_normal_sq_z0z1z2z3);
	qword view_normal_rcp_z4z5z6z7			= si_frsqest(view_normal_sq_z4z5z6z7);
	qword view_normal_rcp_z8z9z10z11		= si_frsqest(view_normal_sq_z8z9z10z11);
	qword view_normal_rcp_z12z13z14z15		= si_frsqest(view_normal_sq_z12z13z14z15);
	qword view_normal_plus_z0z1z2z3			= si_frest(view_normal_rcp_z0z1z2z3);
	qword view_normal_plus_z4z5z6z7			= si_frest(view_normal_rcp_z4z5z6z7);
	qword view_normal_plus_z8z9z10z11		= si_frest(view_normal_rcp_z8z9z10z11);
	qword view_normal_plus_z12z13z14z15		= si_frest(view_normal_rcp_z12z13z14z15);
	qword view_normal_z0z1z2z3				= si_fm(minus_one, view_normal_plus_z0z1z2z3);
	qword view_normal_z4z5z6z7				= si_fm(minus_one, view_normal_plus_z4z5z6z7);
	qword view_normal_z8z9z10z11			= si_fm(minus_one, view_normal_plus_z8z9z10z11);
	qword view_normal_z12z13z14z15			= si_fm(minus_one, view_normal_plus_z12z13z14z15);
	qword light_ptr 						= (const qword)si_from_ptr(&pVisibleLights->pointLights[0].f[0]);
	qword light_position					= si_lqd(light_ptr, 0x00);
	qword light_position_x					= si_shufb(light_position, light_position, shuf_xxxx);
	qword light_position_y					= si_shufb(light_position, light_position, shuf_yyyy);
	qword light_position_z					= si_shufb(light_position, light_position, shuf_zzzz);
	qword light_colour						= si_lqd(light_ptr, 0x10);
	qword light_falloff						= si_lqd(light_ptr, 0x20);
	qword view_to_light_x0x1x2x3			= si_fs(light_position_x, view_x0x1x2x3);														// light_to_world = light_position - world_position;
	qword view_to_light_x4x5x6x7			= si_fs(light_position_x, view_x4x5x6x7);
	qword view_to_light_x8x9x10x11			= si_fs(light_position_x, view_x8x9x10x11);
	qword view_to_light_x12x13x14x15		= si_fs(light_position_x, view_x12x13x14x15);
	qword view_to_light_y0y1y2y3			= si_fs(light_position_y, view_y0y1y2y3);
	qword view_to_light_y4y5y6y7			= si_fs(light_position_y, view_y4y5y6y7);
	qword view_to_light_y8y9y10y11			= si_fs(light_position_y, view_y8y9y10y11);
	qword view_to_light_y12y13y14y15		= si_fs(light_position_y, view_y12y13y14y15);
	qword view_to_light_z0z1z2z3			= si_fs(light_position_z, view_z0z1z2z3);
	qword view_to_light_z4z5z6z7			= si_fs(light_position_z, view_z4z5z6z7);
	qword view_to_light_z8z9z10z11			= si_fs(light_position_z, view_z8z9z10z11);
	qword view_to_light_z12z13z14z15		= si_fs(light_position_z, view_z12z13z14z15);

	uint16_t	current_point_light			= 0;
	uint16_t	point_light_count			= pVisibleLights->pointLightCount;
	do 
	{
		qword light_distance_sq03			= si_fm(view_to_light_x0x1x2x3, view_to_light_x0x1x2x3);
		qword light_distance_sq47			= si_fm(view_to_light_x4x5x6x7, view_to_light_x4x5x6x7);
		qword light_distance_sq811			= si_fm(view_to_light_x8x9x10x11, view_to_light_x8x9x10x11);
		qword light_distance_sq1215			= si_fm(view_to_light_x12x13x14x15, view_to_light_x12x13x14x15);
			  light_distance_sq03			= si_fma(view_to_light_y0y1y2y3, view_to_light_y0y1y2y3, light_distance_sq03);
			  light_distance_sq47			= si_fma(view_to_light_y4y5y6y7, view_to_light_y4y5y6y7, light_distance_sq47);
			  light_distance_sq811			= si_fma(view_to_light_y8y9y10y11, view_to_light_y8y9y10y11, light_distance_sq811);
			  light_distance_sq1215			= si_fma(view_to_light_y12y13y14y15, view_to_light_y12y13y14y15, light_distance_sq1215);
			  light_distance_sq03			= si_fma(view_to_light_z0z1z2z3, view_to_light_z0z1z2z3, light_distance_sq03);
			  light_distance_sq47			= si_fma(view_to_light_z4z5z6z7, view_to_light_z4z5z6z7, light_distance_sq47);
			  light_distance_sq811			= si_fma(view_to_light_z8z9z10z11, view_to_light_z8z9z10z11, light_distance_sq811);
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
		qword diff_dot_47					= si_fm(view_to_light_unitx_47, view_normal_x4x5x6x7);
		qword diff_dot_811					= si_fm(view_to_light_unitx_811, view_normal_x8x9x10x11);
		qword diff_dot_1215					= si_fm(view_to_light_unitx_1215, view_normal_x12x13x14x15);
			  diff_dot_03					= si_fma(view_to_light_unity_03, view_normal_y0y1y2y3, diff_dot_03);
			  diff_dot_47					= si_fma(view_to_light_unity_47, view_normal_y4y5y6y7, diff_dot_47);
			  diff_dot_811					= si_fma(view_to_light_unity_811, view_normal_y8y9y10y11, diff_dot_811);
			  diff_dot_1215					= si_fma(view_to_light_unity_1215, view_normal_y12y13y14y15, diff_dot_1215);
			  diff_dot_03					= si_fma(view_to_light_unitz_03, view_normal_z0z1z2z3, diff_dot_03);
			  diff_dot_47					= si_fma(view_to_light_unitz_47, view_normal_z4z5z6z7, diff_dot_47);
			  diff_dot_811					= si_fma(view_to_light_unitz_811, view_normal_z8z9z10z11, diff_dot_811);
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
	
		// prepare for next loop:
		light_ptr 							= (const qword)si_from_ptr(&pVisibleLights->pointLights[current_point_light+1].f[0]);
		light_position						= si_lqd(light_ptr, 0x00);
		light_position_x					= si_shufb(light_position, light_position, shuf_xxxx);
		light_position_y					= si_shufb(light_position, light_position, shuf_yyyy);
		light_position_z					= si_shufb(light_position, light_position, shuf_zzzz);
		light_colour						= si_lqd(light_ptr, 0x10);
		light_falloff						= si_lqd(light_ptr, 0x20);
		view_to_light_x0x1x2x3				= si_fs(light_position_x, view_x0x1x2x3);														// light_to_world = light_position - world_position;
		view_to_light_x4x5x6x7				= si_fs(light_position_x, view_x4x5x6x7);
		view_to_light_x8x9x10x11			= si_fs(light_position_x, view_x8x9x10x11);
		view_to_light_x12x13x14x15			= si_fs(light_position_x, view_x12x13x14x15);
		view_to_light_y0y1y2y3				= si_fs(light_position_y, view_y0y1y2y3);
		view_to_light_y4y5y6y7				= si_fs(light_position_y, view_y4y5y6y7);
		view_to_light_y8y9y10y11			= si_fs(light_position_y, view_y8y9y10y11);
		view_to_light_y12y13y14y15			= si_fs(light_position_y, view_y12y13y14y15);
		view_to_light_z0z1z2z3				= si_fs(light_position_z, view_z0z1z2z3);
		view_to_light_z4z5z6z7				= si_fs(light_position_z, view_z4z5z6z7);
		view_to_light_z8z9z10z11			= si_fs(light_position_z, view_z8z9z10z11);
		view_to_light_z12z13z14z15			= si_fs(light_position_z, view_z12z13z14z15);
	} 
	while (++current_point_light<point_light_count);

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
	diffuse0								= si_ila(0x8000);
	diffuse1								= si_ila(0x8000);
	diffuse2								= si_ila(0x8000);
	diffuse3								= si_ila(0x8000);
	diffuse4								= si_ila(0x8000);
	diffuse5								= si_ila(0x8000);
	diffuse6								= si_ila(0x8000);
	diffuse7								= si_ila(0x8000);
	diffuse8								= si_ila(0x8000);
	diffuse9								= si_ila(0x8000);
	diffuse10								= si_ila(0x8000);
	diffuse11								= si_ila(0x8000);
	diffuse12								= si_ila(0x8000);
	diffuse13								= si_ila(0x8000);
	diffuse14								= si_ila(0x8000);
	diffuse15								= si_ila(0x8000);
	qword scale								= si_ilh(0xff00);		
	diffuse0 								= si_mpyhhau(clamp0, scale, diffuse0);			// Result is in upper byte of word - rounding should match scale
	diffuse1 								= si_mpyhhau(clamp1, scale, diffuse1);
	diffuse2 								= si_mpyhhau(clamp2, scale, diffuse2);
	diffuse3 								= si_mpyhhau(clamp3, scale, diffuse3);
	diffuse4 								= si_mpyhhau(clamp4, scale, diffuse4);
	diffuse5 								= si_mpyhhau(clamp5, scale, diffuse5);
	diffuse6 								= si_mpyhhau(clamp6, scale, diffuse6);
	diffuse7 								= si_mpyhhau(clamp7, scale, diffuse7);
	diffuse8 								= si_mpyhhau(clamp8, scale, diffuse8);
	diffuse9 								= si_mpyhhau(clamp9, scale, diffuse9);
	diffuse10 								= si_mpyhhau(clamp10, scale, diffuse10);
	diffuse11								= si_mpyhhau(clamp11, scale, diffuse11);
	diffuse12 								= si_mpyhhau(clamp12, scale, diffuse12);
	diffuse13 								= si_mpyhhau(clamp13, scale, diffuse13);
	diffuse14 								= si_mpyhhau(clamp14, scale, diffuse14);
	diffuse15 								= si_mpyhhau(clamp15, scale, diffuse15);

	const vector unsigned char _shuf_uint =
	{ 0xc0, 0x00, 0x04, 0x08, 0xc0, 0x10, 0x14, 0x18, 0xc0, 0x00, 0x04, 0x08, 0xc0, 0x10, 0x14, 0x18 };
	const qword shuf_uint					= (const qword)_shuf_uint;
	const qword base_addr					= si_from_ptr(pResult);
	qword pixel0_01							= si_shufb(diffuse0, diffuse1, shuf_uint);
	qword pixel0_02							= si_shufb(diffuse2, diffuse3, shuf_uint);
	qword pixel1_01							= si_shufb(diffuse4, diffuse5, shuf_uint);
	qword pixel1_02							= si_shufb(diffuse6, diffuse7, shuf_uint);
	qword pixel2_01							= si_shufb(diffuse8, diffuse9, shuf_uint);
	qword pixel2_02							= si_shufb(diffuse10, diffuse11, shuf_uint);
	qword pixel3_01							= si_shufb(diffuse12, diffuse13, shuf_uint);
	qword pixel3_02							= si_shufb(diffuse14, diffuse15, shuf_uint);
	qword pixel0							= si_selb(pixel0_01, pixel0_02, mask_00ff);
	qword pixel1							= si_selb(pixel1_01, pixel1_02, mask_00ff);
	qword pixel2							= si_selb(pixel2_01, pixel2_02, mask_00ff);
	qword pixel3							= si_selb(pixel3_01, pixel3_02, mask_00ff);
											  si_stqd(pixel0, base_addr, 0x00);		// Batch LS writes.
											  si_stqd(pixel1, base_addr, 0x10);
											  si_stqd(pixel2, base_addr, 0x20);
											  si_stqd(pixel3, base_addr, 0x30);
	return;
}

static inline void ProcessTileLit(VisibleLights* __restrict__ pVisibleLights, 
								  LightingData::View* __restrict__ pView, 
								  U8* pOutput, 
								  U8* pNormalDepthBuffer,
								  const uint32_t tileWidth, 
								  const uint32_t tileHeight, 
								  const uint32_t tileStride, 
								  const qword to_screen_x,
								  const qword to_screen_y,
								  const uint16_t x, 
								  const uint16_t y) __attribute__((always_inline));
static inline void ProcessTileLit(VisibleLights* __restrict__ pVisibleLights, 
								  LightingData::View* __restrict__ pView, 
								  U8* pOutput, 
								  U8* pNormalDepthBuffer,
								  const uint32_t tileWidth, 
								  const uint32_t tileHeight, 
								  const uint32_t tileStride, 
								  const qword to_screen_x,
								  const qword to_screen_y,
								  const uint16_t x, 
								  const uint16_t y)
{
	qword y_val							= si_cuflt(si_from_uint(y), 0x0);
	const qword shuf_xxxx				= si_ila(0x010203);
	const qword far_clip				= si_from_float(pView->fFarClip);
	uint32_t* pRowOutput				= (uint32_t*)pOutput;
	uint32_t* pNormalBuf				= (uint32_t*)pNormalDepthBuffer;
	uint32_t* pDepthBuf					= (uint32_t*)pNormalDepthBuffer;
	uint16_t i							= 0;
	y_val								= si_shufb(y_val, y_val, shuf_xxxx);

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
			to_screen_x,
			to_screen_y,
			&pRowOutput[0],
			pVisibleLights, 
			far_clip,
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
			to_screen_x,
			to_screen_y,
			&pRowOutput[16],
			pVisibleLights, 
			far_clip,
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
	LightingData::Fog* pFog = (LightingData::Fog*)Spu::FetchAllocAsync((void*)pInput->fogAddress, sizeof(LightingData::Fog), 2);
	LightingData::PointLight* pPointLights = (LightingData::PointLight*)Spu::FetchAllocAsync((void*)pInput->pointLightsAddress, pInput->pointLightsCount * sizeof(LightingData::PointLight), 2);

	// only want to do this the once per job!
	qword width_addr		= si_from_ptr(&pInput->textureWidth);
	qword height_addr		= si_from_ptr(&pInput->textureHeight);		// only have 4 byte align on this.
	qword width				= si_lqd(width_addr, 0x0);
	qword height			= si_lqd(height_addr, 0x0);
	qword width_offset		= si_andi(width_addr, 0xf);
	qword height_offset		= si_andi(height_addr, 0xf);
		  width				= si_rotqby(width, width_offset);
		  height			= si_rotqby(height, height_offset);			// shift into preferred word.
	qword shuf_xxxx			= si_ila(0x10203);
		  width				= si_shufb(width, width, shuf_xxxx);
		  height			= si_shufb(height, height, shuf_xxxx);
		  width				= si_cuflt(width, 0x0);
		  height			= si_cuflt(height, 0x0);
	qword two				= si_ilhu(0x4000);			// 2
	qword neg_two			= si_ilhu(0xc000);			// -2
	qword to_screen_x		= si_frest(width);
		  to_screen_x		= si_fi(width, to_screen_x);
		  to_screen_x		= si_fm(to_screen_x, two);
	qword to_screen_y		= si_frest(height);
		  to_screen_y		= si_fi(height, to_screen_y);
		  to_screen_y		= si_fm(to_screen_y, neg_two);

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

	const qword far_clip				= si_splat(pView->fFarClip);

	U8* normalDepthBuffer0 = &normalDepthBuffer[0];
	U8* normalDepthBuffer1 = &normalDepthBuffer[c_FBTileSize];

	S32 iJob = MT::AtomicIncrement(pInput->pJobIndex);
	{
		// calculate tile position
		const U32 ty	= iJob / iTilesX;
		const U32 tx	= iJob - ty * iTilesX;
		const U32 x		= tx * c_FBTileWidth;
		const U32 y		= ty * c_FBTileHeight;
		// read in normals plus depth
		const U32 tileReadOffset = (tx & ~0x03) + (ty * iTilesX);
		const U32 normalDepthBlockIndexIn = Spu::GetFBTileIndexForLinearTile(tx, ty, pInput->normalDepthBufferBankOffset);
		const U32 normalDepthOffset = (tileReadOffset + normalDepthBlockIndexIn) * c_FBTileSize;
		cellDmaGet(tiledNormalDepthBuffer, pInput->normalDepthBufferAddress + normalDepthOffset, c_FBTileSize, 0, 0, 0);
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
		vec_uint4 tileMinZ[8];
		vec_uint4 tileMaxZ[8];
		Spu::DetileNormalsAndDepthBufferRangeProcessor detileNormalDepthProcessor(tileMinZ, tileMaxZ);
		Spu::DetileBuffer<Spu::DetileNormalsAndDepthBufferRangeProcessor>(detileNormalDepthProcessor, tiledNormalDepthBuffer, normalDepthBuffer0, tx & 0x03);

		// pull in next job
		iJob = MT::AtomicIncrement(pInput->pJobIndex);
		if(iJob < pInput->jobCount)
		{
			const U32 ty	= iJob / iTilesX;
			const U32 tx	= iJob - ty * iTilesX;
			const U32 x		= tx * c_FBTileWidth;
			const U32 y		= ty * c_FBTileHeight;
			const U32 tileReadOffset = (tx & ~0x03) + (ty * iTilesX);
			const U32 normalDepthBlockIndexIn = Spu::GetFBTileIndexForLinearTile(tx, ty, pInput->normalDepthBufferBankOffset);
			const U32 normalDepthOffset = (tileReadOffset + normalDepthBlockIndexIn) * c_FBTileSize;
			cellDmaGet(tiledNormalDepthBuffer, pInput->normalDepthBufferAddress + normalDepthOffset, c_FBTileSize, 0, 0, 0);
		}

		U8* pLightBuffer = normalDepthBuffer0;
		// calculate lighting for each sub tile
		qword screen_tile_y0	= si_fms(plus_one, inv_proj_y, si_fm(screen_tile_dy, si_splat((F32)(ty*iSubTilesY))));
		qword screen_tile_y1	= si_fs(screen_tile_y0, screen_tile_dy);
		for(U32 sy = 0; sy < iSubTilesY; ++sy)
		{
			qword screen_tile_x0	= si_fma(minus_one, inv_proj_x, si_fm(screen_tile_dx, si_splat((F32)(tx*iSubTilesX))));
			qword screen_tile_x1	= si_fa(screen_tile_x0, screen_tile_dx);

			U8* pOutput = pLightBuffer + sy * iSubTilePixelsY * c_FBTileStride;
			U8* pNormalDepth = pOutput;

			for(U32 sx = 0; sx < iSubTilesX; ++sx)
			{
				qword min_z = (qword)tileMinZ[(sy << 1) + sx];
				qword max_z = (qword)tileMaxZ[(sy << 1) + sx];

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

				if(__builtin_expect(si_to_uint(max_z) == 0x0000, 0))
				{
					ProcessTileUnlit(pOutput, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride);
				}
				else
				{
					// convert the integer zbuffer values to view space z values
					min_z				= si_cuflt(min_z, 0x20);
					max_z				= si_cuflt(max_z, 0x20);
					min_z				= si_fm(min_z, far_clip);
					max_z				= si_fm(max_z, far_clip);

					VisibleLights visibleLights;
					GatherLights(&visibleLights, pPointLights, pInput->pointLightsCount, screen_tile_x0, screen_tile_x1, screen_tile_y0, screen_tile_y1, min_z, max_z);

					// Don't call the lighting code if there are no lights hitting the tile.
					if(!visibleLights.pointLightCount)
					{
						//ProcessTileFog(pFog, pView, pOutput, pNormalDepth, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride, to_screen_x, to_screen_y, x + sx * iSubTilePixelsX, y + sy * iSubTilePixelsY);
						ProcessTileUnlit(pOutput, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride);
					}
					else
					{
						//ProcessTileFog(pFog, pView, pOutput, pNormalDepth, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride, to_screen_x, to_screen_y, x + sx * iSubTilePixelsX, y + sy * iSubTilePixelsY);
						ProcessTileLit(&visibleLights, pView, pOutput, pNormalDepth, iSubTilePixelsX, iSubTilePixelsY, c_FBTileStride, to_screen_x, to_screen_y, x + sx * iSubTilePixelsX, y + sy * iSubTilePixelsY);
					}
				}

				pOutput			+= iSubTilePixelsX * sizeof(U32);
				pNormalDepth	+= iSubTilePixelsX * sizeof(U32);

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

			U8* normalDepthTemp = normalDepthBuffer0;
			normalDepthBuffer0 = normalDepthBuffer1;
			normalDepthBuffer1 = normalDepthTemp;
		}
	}

	cellDmaWaitTagStatusAll(1 << 1);	// Wait on lighting buffer DMAs.
	Spu::HardSync(pInput->lightingBufferAddress, 0);
	Spu::WriteWord(pInput->gateAddress, pInput->gateValue);
	MT::AtomicDecrement((volatile uint32_t*)pInput->freeAddress);
#endif
}

}; // namespace Prepass

JQ_DECLARE_SPU_MODULE(JobQueuePrepassWorkerModule, Prepass::LightingPrepassWorker, jobqueueprepassworker, "deferred.scheduler");
