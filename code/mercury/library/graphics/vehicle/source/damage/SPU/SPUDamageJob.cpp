#include "VehiclePCH.h"

#include "System/Scheduler/JobQueue.h"
#include "Vehicle/damage/spu/SPUDamageJob.h"
#include "Vehicle/damage/spu/SPUDamageContext.h"
#include "Vehicle/damage/NodeLayer.h"
#if defined(PLATFORM_SPU)
#include <spu_intrinsics.h>
#include <Core/maths/hvec/VectorConstGlobals.inl>
#include <core/maths/hvec/new/IncludeGeneral.h>
#include <core/maths/hvec/new/IncludeGeneral.inl>
#endif // #if defined(PLATFORM_SPU)

//#define USE_FAST_DEFORMATION


namespace SpuDamage
{

#ifdef PLATFORM_SPU
static uint8_t	__attribute__((aligned(128)))	damageLayers[((sizeof(Vehicle::C_NodeLayer)*3)+0xf)&~0xf];	//!< ~32KiB for node layer.
static uint8_t	__attribute__((aligned(128)))	buffer[0x10 << 0xb];										//!< 32KiB for VB input buffer.
static uint8_t	__attribute__((aligned(128)))	outputBuffer[0x10 << 0xb];									//!< 32KiB for VB output buffer.
static uint8_t	__attribute__((aligned(128)))	contextBuffer[(sizeof(C_DamageContext)+0xf)&~0xf];			//!< a few KiB for damage context.
static uint8_t*									vertexBuffer0;												//!< Pointer to in-use vertex buffer.
static uint8_t*									vertexBuffer1;												//!< Pointer to DMA target vertex buffer.
static float									g_VehicleBoundsInfo[4];										//!< Vehicle bounds.
static Vehicle::C_NodeLayer*					g_pLayers = NULL;
static float*									g_pHeightProfile = NULL;



//! Load float16_4 into qword.
//!       15 (msb)
//!       | 14  10 9        0 (lsb)
//!       X XXXXX  XXXXXXXXXX
//!       s e      m
//!
//!   If e is between 1 and 30, h is a normalized number:
//!               s    e-15
//!       h = (-1)  * 2     * 1.m
//!
//!   If e is 0, and m is not zero, h is a denormalized number:
//!               S    -14
//!       h = (-1)  * 2     * 0.m
//!
//!   If e and m are both zero, h is zero:
//!       h = 0.0
//!
//!   If e is 31, h is an "infinity" or "not a number" (NAN),
//!   depending on whether m is zero or not.
//!
//!   Examples:
//!       0 00000 0000000000 = 0.0
//!       0 01110 0000000000 = 0.5
//!       0 01111 0000000000 = 1.0
//!       0 10000 0000000000 = 2.0
//!       0 10000 1000000000 = 3.0
//!       1 10101 1111000001 = -124.0625
//!       0 11111 0000000000 = +infinity
//!       1 11111 0000000000 = -infinity
//!       0 11111 1000000000 = NAN
//!       1 11111 1111111111 = NAN
//@param addr Address to load.
//@return qword containing float16_4 as float32_4.
static inline const qword load_unaligned_float16_4(void* __restrict__ addr) __attribute__((always_inline));
static inline const qword load_unaligned_float16_4(void* __restrict__ addr)
{
	const vector unsigned char _loader = 
	 { 0x80, 0x80, 0x00, 0x01, 0x80, 0x80, 0x02, 0x03, 0x80, 0x80, 0x04, 0x05, 0x80, 0x80, 0x06, 0x07 };
	const vector unsigned char _shft   =
	 { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	
	qword target					= si_from_ptr(addr);
	qword val_lo					= si_lqd(target, 0x00);
	qword val_hi					= si_lqd(target, 0x10);						// might straddle boundary.
	qword sign_bit_mask				= si_ilhu(0x0);
		  sign_bit_mask				= si_iohl(sign_bit_mask, 0x8000);
	qword mant_bit_mask 			= si_ilhu(0x0);
		  mant_bit_mask				= si_iohl(mant_bit_mask, 0x7fff);
	qword expo_bias					= si_ilhu(0x3800);
	qword loader					= (const qword)_loader;						// 6 cycle load, best we can do here, due to format of the shuffle pattern :(
	qword shft						= (const qword)_shft;						// ...and again here.
	qword offset					= si_andi(target, 0x0f);					// mask off bottom 4 bits => a measurement of how much we suck in bytes.
	qword lo_byte_pattern 			= si_ilh(0x0303); 
	qword offset_pattern 			= si_shufb(offset, offset, lo_byte_pattern);
	qword mod_shuf					= si_a(shft, offset_pattern);
	qword val						= si_shufb(val_lo, val_hi, mod_shuf);	
	qword result					= si_shufb(val, val, loader);				// format now gaurnteed :)
	
	// de-code into IEEE754-ish float32 (meh):
	qword sign_bit					= si_and(result, sign_bit_mask);
		  sign_bit					= si_shli(sign_bit, 0x10);					// move 16 bits into correct place.
	qword significand				= si_and(result, mant_bit_mask);
		  significand				= si_shli(significand, 0xd);
	qword is_zero_mask				= si_cgti(significand, 0x0);				// all bits set if non-zero.
		  expo_bias					= si_and(is_zero_mask, expo_bias);
	qword exponent_bias				= si_a(significand, expo_bias);			// move expo up range, 0x07800000 => 0x3f800000.
		  exponent_bias 			= si_or(exponent_bias, sign_bit);
	return exponent_bias;
}

//! Convert two IEEE-754 qwords into a NVID GPU float-16 (1:5:8) single qword.
//@param x first vector of floats.
//@param y second vector of floats.
//@return 2 floats packed into 16bit floats.
static inline const qword float16_to_float32(const qword x, const qword y) __attribute__((always_inline, const, pure));
static inline const qword float16_to_float32(const qword x, const qword y)
{
	qword sign_bit_mask				= si_ilhu(0x8000);
	qword exp_mant_bit_mask			= si_ilhu(0x0fff);
		  exp_mant_bit_mask			= si_iohl(exp_mant_bit_mask, 0xe000);		// bottom 5 bits of exp -- top 10 bits of mantissa.
	qword sel_mask					= si_fsmbi(0x3333);
	qword sign_bits_x				= si_and(x, sign_bit_mask);
	qword sign_bits_y				= si_and(y, sign_bit_mask);
	qword expmant_bits_x			= si_and(x, exp_mant_bit_mask);
	qword expmant_bits_y			= si_and(y, exp_mant_bit_mask);
		  expmant_bits_x			= si_shli(expmant_bits_x, 0x3);
		  expmant_bits_y			= si_shli(expmant_bits_y, -0xd);			// get exp and mant into correct place -- contig. 15 bits.
		  sign_bits_y				= si_rotqbyi(sign_bits_y, -0x2);			// move the sign.
	qword result_x					= si_or(sign_bits_x, expmant_bits_x);
	qword result_y					= si_or(sign_bits_y, expmant_bits_y);
		  result_y					= si_rotqbyi(result_y, 0x2);
	qword res						= si_selb(result_x, result_y, sel_mask);	// interleave into 16 byte chunk.
	return res;
}

//! convert F32 to F16. pinched from Edge
const vec_uchar16 s_ABEFIJMNabefijmn = {0x00,0x01,0x04,0x05, 0x08,0x09,0x0C,0x0D, 0x10,0x11,0x14,0x15, 0x18,0x19,0x1C,0x1D};
/* F16 variables */
const vec_int4    expRebias          =    (vec_int4)spu_splats(0x38000000);
const vec_ushort8 signMask           = (vec_ushort8)spu_splats(0x80008000);
const vec_ushort8 maxHalf            = (vec_ushort8)spu_splats(0x7BFF7BFF);
const vec_ushort8 expMin             = (vec_ushort8)spu_splats(0x38003800);
const vec_ushort8 expMax             = (vec_ushort8)spu_splats(0x47804780);
static inline const qword float32_to_float16(const qword attr0, const qword attr1)
{
	const vec_ushort8 in01  = (vec_ushort8)spu_shuffle(attr0, attr1, s_ABEFIJMNabefijmn);
	const vec_ushort8 nz01  = spu_andc(in01, signMask);
	const vec_ushort8 m01   = spu_cmpgt(expMin, nz01);
	const vec_ushort8 x01   = spu_cmpgt(expMax, nz01);
	const vec_ushort8 r0    = (vec_ushort8)spu_rlqw(spu_sub((vec_int4)attr0, expRebias), 3);
	const vec_ushort8 r1    = (vec_ushort8)spu_rlqw(spu_sub((vec_int4)attr1, expRebias), 3);
	const vec_ushort8 r01   = spu_sel(maxHalf, (vec_ushort8)spu_shuffle(r0, r1, s_ABEFIJMNabefijmn), x01);
	const vec_ushort8 n01   = spu_sel(r01, in01, signMask);
	return (qword)spu_andc(n01, m01);
}

//! Write an un-aligned compressed vertex.
//@param val Value to write.
//@param addr Address to write to.
static inline void store_unaligned_cmp(const qword val, void* __restrict addr) __attribute__((always_inline));
static inline void store_unaligned_cmp(const qword val, void* __restrict addr)
{
	// TODO: [Stove] implement this.
	return;
}

static inline void store_aligned_cmp(const qword val, void* __restrict addr) __attribute__((always_inline));
static inline void store_aligned_cmp(const qword val, void* __restrict addr)
{
	// TODO: [Stove] implement this.
	return;
}

//! Saturate a value.
static inline const float saturate(const float f) __attribute__((always_inline));
static inline const float saturate(const float f)
{
	qword v 					= si_from_float(f);
		  v 					= si_cfltu(v, 0x20);
		  v 					= si_cuflt(v, 0x20);
	return si_to_float(v);
}


Maths::HVec2 GetDamageUV(Maths::HVec3 position)
{
	using namespace Maths;
	Maths::HVec2 uv(0.0f, 0.0f);
	uv.X(Maths::Clamp(((Maths::VF32)position.X() + Load<VF32>(&g_VehicleBoundsInfo[0]) / Maths::Const<Maths::VF32>::Two()) / Load<VF32>(&g_VehicleBoundsInfo[0]), Maths::Const<Maths::VF32>::Zero(), Maths::Const<Maths::VF32>::One()));
	uv.Y(Maths::Clamp(((Maths::VF32)position.Z() + Load<VF32>(&g_VehicleBoundsInfo[2]) / Maths::Const<Maths::VF32>::Two()) / Load<VF32>(&g_VehicleBoundsInfo[2]), Maths::Const<Maths::VF32>::Zero(), Maths::Const<Maths::VF32>::One()));
	return uv;
}

Maths::HVec4 EvaluateDamageInfoTexture(Maths::HVec2 uv, Maths::HF32 yPos)
{
	uint32_t u = (int)((F32)uv.X() * 7.f);
	uint32_t v = (int)((F32)uv.Y() * 7.f);
	uint32_t i = (v << 3) + u;
	uint32_t j = (v << 3) + ((u+1)%7);
	
	BIZ_ASSERT(i<(8*8));
	BIZ_ASSERT(j<(8*8));
	float yVal = (uv.Y()-((float)(v)/7.f)) / (((float)((v+1)%7)/7.f)-((float)(v)/7.f));
	Maths::VF32 maxDeform = Maths::Const<Maths::VF32>::Zero();
	for (uint16_t iLayer = 0; iLayer < 0x3; iLayer++)
	{
		Maths::VF32 deformLevel_a = Maths::Load<Maths::VF32>(&g_pLayers[iLayer].m_Nodes[i].m_DeformLevel);
		Maths::VF32 deformLevel_b = Maths::Load<Maths::VF32>(&g_pLayers[iLayer].m_Nodes[j].m_DeformLevel);
		Maths::VF32 deformLevel   = deformLevel_a * (1-yVal) + deformLevel_b * (yVal);
		if (deformLevel > maxDeform)
			maxDeform = deformLevel;
	}
	
	Maths::HVec4 damageInfo(0.0f, 0.0f, 0.0f, 0.0f);
	damageInfo.Y(maxDeform);
	damageInfo.X(Maths::Clamp(((Maths::VF32)yPos / (g_pHeightProfile[v] * Maths::Load<Maths::VF32>(&g_VehicleBoundsInfo[1]))), Maths::Const<Maths::VF32>::Zero(), Maths::Const<Maths::VF32>::One()));
	damageInfo.Z(0.0f);
	damageInfo.W(0.0f);
	return damageInfo;
}

//! Load cmp into qword.
//static inline const qword load_unaligned_SSHD_3N(const void *_addr) __attribute__((always_inline));
const vec_int4 bitMask               =  (vec_int4){0xFFE00000, 0xFFE00000, 0xFFC00000, 0x00000000};
const vec_uint4 shiftValues           =  (vec_uint4){21, 10, 0, 0};
const vec_float4 x11y11z10Scale      =  (vec_float4)(vec_int4){0x3F801002, 0x3F801002, 0x3F802008, 0x00000000};
const vec_float4 x11y11z10Bias       =  (vec_float4)(vec_int4){0x3A001002, 0x3A001002, 0x3A802008, 0x3F800000};
const qword load_unaligned_SSHD_3N(const void *_addr)
{
	const vector unsigned char _shft   =
     { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
       
	   qword addr = si_from_ptr(_addr);
       qword val_lo                             = si_lqd(addr, 0x00);
       qword val_hi                             = si_lqd(addr, 0x10);                                         // might straddle boundary.
       qword shft                               = (const qword)_shft;                                         // ...and again here.
       qword offset                             = si_andi(addr, 0x0f);                                        // mask off bottom 4 bits => a measurement of how much we suck in bytes.
       qword lo_byte_pattern                    = si_ilh(0x0303); 
       qword mask_00ff                          = si_fsmbi(0x00ff);
       qword mask_0fff                          = si_fsmbi(0x0fff);
       qword offset_pattern                     = si_shufb(offset, offset, lo_byte_pattern);
       qword mod_shuf                           = si_a(shft, offset_pattern);
       qword shuf_pat                           = si_ila(0x010203);
       qword val                                = si_shufb(val_lo, val_hi, mod_shuf);    
       qword result                             = si_shufb(val, val, shuf_pat);
	   vec_int4 v0 = spu_sl((vec_int4)result, shiftValues);
	   vec_int4 v1 = spu_and(v0, bitMask);
	   const vec_float4 fData0 = spu_convtf(v1, 31);
	   qword rtn = (vec_char16)spu_madd(fData0, x11y11z10Scale, x11y11z10Bias);
	   return (qword)rtn;
}


Maths::HVec4 ControlPointDamage(Maths::HVec3 vPos, U8 *pControlPoints, F32 *pControlPointData)
{
	using namespace Maths;
	HVec4 blendTgt1 = Load<HVec4, MathsStorage::Float4>((MathsStorage::Float4*)&pControlPointData[pControlPoints[0]*4]);
	HVec4 blendTgt2 = Load<HVec4, MathsStorage::Float4>((MathsStorage::Float4*)&pControlPointData[pControlPoints[1]*4]);
	HF32 blendWeight = (F32)pControlPoints[2] / HF32(254.0f);
	HVec4 blendTgt = blendTgt1 + (blendTgt2-blendTgt1)*blendWeight;
	blendTgt.W(0);

	return blendTgt;
}

const float g_DebugRearLight = 0.6f;

U32 CalculateRearLightAttenuation(Maths::HVec4 vPos, Maths::HVec4 vNormal, U8 iAO)
{
	float rearLightFactor = saturate(saturate(-vNormal.Z()) - g_DebugRearLight) / (1.f - g_DebugRearLight);
	float rearLightFactor2 = rearLightFactor * rearLightFactor * saturate(-(vPos.Z() * 0.5f));
	return (U32)(rearLightFactor2 * (float)iAO);
}


const Maths::HVec3 GetValueOffset(S32 x, S32 y, S32 z)
{
	return g_pLayers[z].m_Nodes[(y<<3) + x].m_OffsetVector;
}

const float GetValueScratch(S32 x, S32 y, S32 z)
{
	return g_pLayers[z].m_Nodes[(y<<3) + x].m_ScratchLevel;
}


Maths::HVec4 EvaluateVertexDamage(Maths::HVec3 uvw)
{
	const uint32_t x_dim = 7;
	const uint32_t y_dim = 7;
	const uint32_t z_dim = 2;
	F32 x = uvw.X() * (float)x_dim;
	F32 y = uvw.Y() * (float)y_dim;
	F32 z = uvw.Z() * (float)z_dim;
	S32 ix = (S32)floor(x);
	S32 iy = (S32)floor(y);
	S32 iz = (S32)floor(z);
	F32 fx = x - (F32)ix;
	F32 fy = y - (F32)iy;
	F32 fz = z - (F32)iz;
	F32 omfx = 1.0f - fx;
	F32 omfy = 1.0f - fy;
	F32 omfz = 1.0f - fz;

	Maths::HVec3 result(0,0,0);
	float		 scratchResult =  0.f;

	ix = (ix<0)?0:((ix>x_dim)?x_dim:ix);
	iy = (iy<0)?0:((iy>y_dim)?y_dim:iy);
	iz = (iz<0)?0:((iz>z_dim)?z_dim:iz);
	uint32_t ixp1 = ((ix+1)<0)?0:(((ix+1)>x_dim)?x_dim:(ix+1));
	uint32_t iyp1 = ((iy+1)<0)?0:(((iy+1)>y_dim)?y_dim:(iy+1));
	uint32_t izp1 = ((iz+1)<0)?0:(((iz+1)>z_dim)?z_dim:(iz+1));
	
	result = result + GetValueOffset(ix,   iy,   iz  ) * (omfx * omfy * omfz);
	result = result + GetValueOffset(ixp1, iy,   iz  ) * (fx   * omfy * omfz);
	result = result + GetValueOffset(ix,   iyp1, iz  ) * (omfx * fy   * omfz);
	result = result + GetValueOffset(ixp1, iyp1, iz  ) * (fx   * fy   * omfz);
	result = result + GetValueOffset(ix,   iy,   izp1) * (omfx * omfy * fz);
	result = result + GetValueOffset(ixp1, iy,   izp1) * (fx   * omfy * fz);
	result = result + GetValueOffset(ix,   iyp1, izp1) * (omfx * fy   * fz);
	result = result + GetValueOffset(ixp1, iyp1, izp1) * (fx   * fy   * fz);
	
	scratchResult = scratchResult + GetValueScratch(ix,   iy,   iz  ) * (omfx * omfy * omfz);
	scratchResult = scratchResult + GetValueScratch(ixp1, iy,   iz  ) * (fx   * omfy * omfz);
	scratchResult = scratchResult + GetValueScratch(ix,   iyp1, iz  ) * (omfx * fy   * omfz);
	scratchResult = scratchResult + GetValueScratch(ixp1, iyp1, iz  ) * (fx   * fy   * omfz);
	scratchResult = scratchResult + GetValueScratch(ix,   iy,   izp1) * (omfx * omfy * fz);
	scratchResult = scratchResult + GetValueScratch(ixp1, iy,   izp1) * (fx   * omfy * fz);
	scratchResult = scratchResult + GetValueScratch(ix,   iyp1, izp1) * (omfx * fy   * fz);
	scratchResult = scratchResult + GetValueScratch(ixp1, iyp1, izp1) * (fx   * fy   * fz);
	
	return Maths::HVec4(result.X(), result.Y(), result.Z(), scratchResult );
}

// 	uint32_t u = (int)(floor(uvw.X()) * 7.f);
// 	uint32_t v = (int)(floor(uvw.Y()) * 7.f);
// 	uint32_t w = (int)(floor(uvw.Z()) * 2.f);
// 	uint32_t i = (v << 3) + u;
// 	uint32_t j = (v << 3) + ((u+1) % 7);
// 	uint32_t k = (((v+1)%7) << 3) + u;
// 	uint32_t l = (((v+1)%7) << 3) + ((u+1) % 7);
// 	BIZ_ASSERT(i<(8*8));
// 	BIZ_ASSERT(j<(8*8));
// 	float xVal = (uvw.X()-((float)(u)/7.f)) / (((float)((u+1)%7)/7.f)-((float)(u)/7.f));			// lerp val for blend.
// 	float yVal = (uvw.Y()-((float)(v)/7.f)) / (((float)((v+1)%7)/7.f)-((float)(v)/7.f));			// lerp val for blend.
// 	float zVal = (uvw.Z()-((float)(w)/2.f)) / (((float)((w+1)%2)/2.f)-((float)(w)/2.f));			// lerp val for blend.
// 	
// 	uint32_t w_offset = ((w+1)>0x2)?0x2:w+1;
// 
// 	Maths::HVec3 vector_a = (g_pLayers[w].m_Nodes[i].m_OffsetVector);
// 	Maths::HVec3 vector_b = (g_pLayers[w].m_Nodes[j].m_OffsetVector);
// 	Maths::HVec3 vector_c = (g_pLayers[w_offset].m_Nodes[i].m_OffsetVector);				// clamp the z
// 	Maths::HVec3 vector_d = (g_pLayers[w_offset].m_Nodes[j].m_OffsetVector);
// 	Maths::HVec3 vector_e = (g_pLayers[w].m_Nodes[k].m_OffsetVector);
// 	Maths::HVec3 vector_f = (g_pLayers[w].m_Nodes[l].m_OffsetVector);
// 	Maths::HVec3 vector_g = (g_pLayers[w_offset].m_Nodes[k].m_OffsetVector);				// clamp the z
// 	Maths::HVec3 vector_h = (g_pLayers[w_offset].m_Nodes[l].m_OffsetVector);
// 	
// 	Maths::HVec3 vector_1  = Maths::Lerp(vector_a, vector_b, xVal);	
// 	Maths::HVec3 vector_2  = Maths::Lerp(vector_c, vector_d, xVal);	
// 	Maths::HVec3 vector_3  = Maths::Lerp(vector_e, vector_f, yVal);	
// 	Maths::HVec3 vector_4  = Maths::Lerp(vector_g, vector_h, yVal);
// 
// 	Maths::HVec3 vector_5  = Maths::Lerp(vector_1, vector_2, zVal);	
// 	Maths::HVec3 vector_6  = Maths::Lerp(vector_3, vector_4, zVal);
// 	Maths::HVec3 vector	   = Maths::Lper(vector_4, vector_5, 0.5f);
// 
// 	float scratchLevel_a   = Maths::Lerp(g_pLayers[w].m_Nodes[i].m_ScratchLevel, g_pLayers[w].m_Nodes[j].m_ScratchLevel, xVal);
// 	float scratchLevel_b   = Maths::Lerp(g_pLayers[((w+1)>0x2)?0x2:w+1].m_Nodes[i].m_ScratchLevel, g_pLayers[((w+1)>0x2)?0x2:w+1].m_Nodes[j].m_ScratchLevel, xVal);
// 	float scratchLevel	   = Maths::Lerp(scratchLevel_a, scratchLevel_b, zVal);
// 	return Maths::HVec4(vector.X(), vector.Y(), vector.Z(), scratchLevel);	
//}

//! Read an un-aligned float3.
//@param addr Address to read in.
//@return Float3, 1.0f in w component.
/*static inline*/ const qword load_unaligned_float3(void* __restrict__ addr) /*__attribute__((always_inline))*/;
/*static inline*/ const qword load_unaligned_float3(void* __restrict__ addr)
{
	const vector unsigned char _shift_left = 
	 { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }; 
	const vector unsigned char _mask_000f =
	 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff };

	qword source				= si_from_ptr(addr);
	qword ld_lo					= si_lqd(source, 0x00);
	qword ld_hi					= si_lqd(source, 0x10);
	qword offset 				= si_andi(source, 0x0f);
	qword lo_byte_pattern 		= si_ilh(0x0303); 
	qword offset_pattern 		= si_shufb(offset, offset, lo_byte_pattern);
	qword shift_left			= (const qword)_shift_left;
	qword mask_000f				= (const qword)_mask_000f;
	qword zero					= si_ilh(0x8080);
	qword shuf_pat 				= si_a(offset_pattern, shift_left);
		  shuf_pat				= si_selb(shuf_pat, zero, mask_000f);
	qword result				= si_shufb(ld_lo, ld_hi, shuf_pat);
	return result;
}

//! Store an un-aligned float3.
//@param addr Address to read in.
//@param val Value to store.
/*static inline*/ void store_unaligned_float3(void* __restrict__ addr, const qword value) /*__attribute__((always_inline))*/;
/*static inline*/ void store_unaligned_float3(void* __restrict__ addr, const qword value)
{
	const vector unsigned char _shift_left = 
	 { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }; 
	 
	intptr_t source_addr_u      = (uintptr_t)addr;
    qword source_addr           = si_from_uint( source_addr_u );
    qword source_lo             = si_lqd( source_addr, 0x00 );
    qword source_hi             = si_lqd( source_addr, 0x10 );
    qword shift_offset          = si_andi( source_addr, 0x0f );
    qword lo_byte_pattern       = si_ilh( 0x0303 );
    qword shift_offset_pattern_lo= si_shufb( shift_offset, shift_offset, lo_byte_pattern );
    qword shift_adjust_hi       = si_ilh( 0x1010 );
    qword shift_offset_pattern_hi= si_sf( shift_adjust_hi, shift_offset_pattern_lo );
    qword vector_shift_left     = (qword)_shift_left;
    qword shift_pattern_lo      = si_sf( shift_offset_pattern_lo, vector_shift_left );
    qword shift_pattern_hi      = si_sf( shift_offset_pattern_hi, vector_shift_left );
    qword source_bits_mask_lo   = si_clgtbi( shift_pattern_lo, 0x07 );
    qword source_bits_mask_hi   = si_clgtbi( shift_pattern_hi, 0x07 );
    qword value_lo              = si_shufb( value, value, shift_pattern_lo );
    qword value_hi              = si_shufb( value, value, shift_pattern_hi );
    qword prepped_source_lo     = si_and( source_lo, source_bits_mask_lo );
    qword prepped_source_hi     = si_and( source_hi, source_bits_mask_hi );
    qword prepped_value_lo      = si_andc( value_lo, source_bits_mask_lo );
    qword prepped_value_hi      = si_andc( value_hi, source_bits_mask_hi );
    qword result_lo             = si_or( prepped_source_lo, prepped_value_lo );
    qword result_hi             = si_or( prepped_source_hi, prepped_value_hi );
								  si_stqd( result_lo, source_addr, 0x00 );
								  si_stqd( result_hi, source_addr, 0x10 );
	return;
}

void Deform(Vehicle::C_NodeLayer* pLayer, SpuDamage::C_DamageContext* pContext, SpuDamage::C_LightWeightImpact* pImpact)
{
	float fRadius = (float)1.5f * pImpact->impact_size;
	fRadius = Maths::Max(fRadius, (float)0.4f);
	fRadius = fRadius * fRadius;
	Maths::HVec3 impactPosition = Maths::HVec3(pImpact->local_position[0], pImpact->local_position[1], pImpact->local_position[2]);
	Maths::HVec3 vOffsetVec		= Maths::HVec3(pImpact->local_normal[0], pImpact->local_normal[1], pImpact->local_normal[2]) * pImpact->impact_size * pImpact->filter_deform_amount_code * 0.75f;

	float fArtDeformRadiusScale		= fRadius;
	float fArtScratchRadiusScale	= fRadius;
	float fScratchAdditionScale		= pImpact->impact_size * pImpact->filter_deform_amount_art * 10.0f;
	float fDeformAdditionScale		= pImpact->impact_size * pImpact->filter_deform_amount_art * 10.0f;

	for (int iLayer = 0; iLayer < 0x3; iLayer++)
	{
		for (int iColumn = 0; iColumn < pLayer[iLayer].GetWidth(); iColumn++)
		{
			for (int iRow = 0; iRow < pLayer[iLayer].GetLength(); iRow++)
			{
				Vehicle::C_Node* pNode = pLayer[iLayer].GetNode(iRow, iColumn);

				float distToNode = (pNode->m_Position - impactPosition).Length2();
				float radius = fRadius;

				pNode->m_CachedPosition	= pNode->m_Position;
				pNode->m_TargetPosition = pNode->m_Position;

				if (distToNode < radius)
				{
					float distScale = 1.0f - Maths::Clamp(distToNode / radius, 0.0f, 1.0f);
					Maths::HVec3 newPosition = pNode->m_Position -  vOffsetVec * distScale;

					if (!pContext->transitioning)
					{
						MathsStorage::Float3 pos;

						if (pContext->fatalDamage)
						{
							pos.x = Maths::Clamp((float)newPosition.X(), pNode->m_DeformRange.m_MinX, pNode->m_DeformRange.m_MaxX);
							pos.y = Maths::Clamp((float)newPosition.Y(), pNode->m_DeformRange.m_MinY, pNode->m_DeformRange.m_MaxY);
							pos.z = Maths::Clamp((float)newPosition.Z(), pNode->m_DeformRange.m_MinZ, pNode->m_DeformRange.m_MaxZ);
						}
						else
						{
							pos.x = Maths::Clamp((float)newPosition.X(), pNode->m_DriveableDeformRange.m_MinX, pNode->m_DriveableDeformRange.m_MaxX);
							pos.y = Maths::Clamp((float)newPosition.Y(), pNode->m_DriveableDeformRange.m_MinY, pNode->m_DriveableDeformRange.m_MaxY);
							pos.z = Maths::Clamp((float)newPosition.Z(), pNode->m_DriveableDeformRange.m_MinZ, pNode->m_DriveableDeformRange.m_MaxZ);		
						}

						pNode->m_Position = Maths::Load<Maths::HVec3, MathsStorage::Float3>(&pos);
						pNode->m_OffsetVector = pNode->m_Position - pNode->m_OriginalPosition;
						pNode->m_OffsetLength = pNode->m_OffsetVector.Length();	
					}
					else
						pNode->m_TargetPosition = newPosition;
				}

				float scratchAmount = 0.0f;
				float deformAmount = 0.0f;
				float baseLevel = Maths::Clamp(pNode->m_OffsetLength / 0.05f, 0.0f, 1.0f);
				if (pImpact->scratch_amount)
				{					
					float scratchDistScale = 1.0f - Maths::Clamp(distToNode / fArtScratchRadiusScale, 0.0f, 1.0f);
					float scratchAddition = Maths::Max(2.0f * scratchDistScale, fScratchAdditionScale * scratchDistScale);
					scratchAmount = baseLevel + scratchAddition;
				}
				if (pImpact->deform_parts)
				{					
					float distScale = 1.0f - Maths::Clamp(distToNode / fArtDeformRadiusScale, 0.0f, 1.0f);
					float deformAddition = Maths::Max(0.5f * distScale, fDeformAdditionScale * distScale);
					deformAmount = baseLevel + deformAddition;
				}				

				if (pContext->transitioning)
				{
					pNode->m_CachedDeformLevel	= pNode->m_DeformLevel;					
					if (deformAmount > 0.0f)
						pNode->m_TargetDeformLevel = Maths::Clamp(pNode->m_DeformLevel + deformAmount, 0.0f, 1.0f);

					pNode->m_CachedScratchLevel	= pNode->m_ScratchLevel;
					if (scratchAmount > 0.0f)
						pNode->m_TargetScratchLevel = Maths::Clamp(pNode->m_ScratchLevel + scratchAmount, 0.0f, 1.0f);
				}
				else
				{
					if (deformAmount > 0.0f)
						pNode->m_DeformLevel = Maths::Clamp(pNode->m_DeformLevel + deformAmount, 0.0f, 1.0f);
					if (scratchAmount > 0.0f)
						pNode->m_ScratchLevel = Maths::Clamp(pNode->m_ScratchLevel + scratchAmount, 0.0f, 1.0f);
				}
			}
		}
	}

	return;
}

//! Repair damage.
void Repair(Vehicle::C_NodeLayer* pLayer, SpuDamage::C_DamageContext* pContext)
{
	float p_Amount = pContext->amount;
	float p_CurrentDamagePercentage = pContext->currentDamagePercentage;
	float p_Duration = pContext->duration;
	float fRawTargetAmount = Maths::Clamp(p_CurrentDamagePercentage - p_Amount, 0.0f, 1.0f);
	float fTargetAmount = Maths::Clamp(p_CurrentDamagePercentage - (p_Amount * 2.0f), 0.0f, 1.0f);
	float fRawLerp = fRawTargetAmount == 0.0f ? 1.0f : fRawTargetAmount;
	float fLerp = fTargetAmount == 0.0f ? 1.0f : fTargetAmount;

	for (int iLayer = 0; iLayer < 0x3; iLayer++)
	{
		for (int iColumn = 0; iColumn < pLayer[iLayer].GetWidth(); iColumn++)
		{
			for (int iRow = 0; iRow < pLayer[iLayer].GetLength(); iRow++)
			{
				Vehicle::C_Node* pNode = pLayer[iLayer].GetNode(iRow, iColumn);

				if (pContext->transitioning)
				{
					pNode->m_CachedPosition		= pNode->m_Position;
					pNode->m_TargetPosition		= Maths::Lerp(pNode->m_Position, pNode->m_OriginalPosition, fLerp);
					
					pNode->m_CachedScratchLevel	= pNode->m_ScratchLevel;	// never repair scratch.
					pNode->m_TargetScratchLevel	= pNode->m_ScratchLevel;	
					pNode->m_CachedDeformLevel	= pNode->m_DeformLevel;		// always repair morph.
					pNode->m_TargetDeformLevel	= Maths::Lerp(pNode->m_DeformLevel, pNode->m_OriginalDeformLevel, fRawLerp);
				}
				else
				{
					pNode->m_Position		= Maths::Lerp(pNode->m_Position, pNode->m_OriginalPosition, fLerp);
					pNode->m_DeformLevel	= Maths::Lerp(pNode->m_DeformLevel, pNode->m_OriginalDeformLevel, fLerp);
				}
			}
		}
	}

	return;
}

//! Clamp a value to an arbitrary range.
//@param x Value to clamp.
//@param min Minimum value.
//@param max Maximum value.
//@return Clamped value.
static inline const qword clamp(const qword x, const qword min, const qword max) __attribute__((pure, const, always_inline));
static inline const qword clamp(const qword x, const qword min, const qword max)
{
	qword a_mask				= si_fcgt(x, min);
	qword a						= si_selb(x, min, a_mask);
	qword b_mask				= si_fcgt(a, max);
	qword b						= si_selb(a, max, b_mask);
	return b;
}

//! Return the dot product of two vectors.
//@param x 1st vector.
//@param y 2nd vector.
//@return The dot product of two vectors.
static inline const qword dot(const qword x, const qword y) __attribute__((pure, const, always_inline));
static inline const qword dot(const qword x, const qword y)
{
	qword dot_xy				= si_fm(x, y);
	qword dot_xy_r4				= si_rotqbyi(dot_xy, 4);
		  dot_xy				= si_fa(dot_xy, dot_xy_r4);
	qword dot_xy_r8				= si_rotqbyi(dot_xy_r4, 4);
		  dot_xy				= si_fa(dot_xy, dot_xy_r8);
	return dot_xy;
}

//! Return the length of the vector.
//@param x 1st vector.
//@return The length of the vector.
static inline const qword length(const qword x) __attribute__((pure, const, always_inline));
static inline const qword length(const qword  x)
{
	qword dot_xy				= si_fm(x, x);								// EVEN, 6
	qword dot_xy_r4				= si_rotqbyi(dot_xy, 4);					// ODD, 4
		  dot_xy				= si_fa(dot_xy, dot_xy_r4);					// EVEN, 6
	qword dot_xy_r8				= si_rotqbyi(dot_xy_r4, 4);					// ODD, 4
		  dot_xy				= si_fa(dot_xy, dot_xy_r8);					// EVEN, 6
	qword dot_xy_sqrt			= si_frsqest(dot_xy);						// ODD, 4
		  dot_xy_sqrt			= si_frest(dot_xy_sqrt);					// ODD, 4
	return dot_xy_sqrt;
}

//! Compute the max of x and y.
//@param x First vector.
//@param y Second vector.
//@return Either x or y depending on which is larger.
static inline const qword max(const qword x, const qword y) __attribute__((pure, const, always_inline));
static inline const qword max(const qword x, const qword y)
{
	qword mask 					= si_fcgt(x, y);							// EVEN, 2
	qword result				= si_selb(x, y, mask);						// EVEN, 2
	return result;
}

//! Compute the min of x and y.
//@param x First vector.
//@param y Second vector.
//@return Either x or y depending on which is smaller.
static inline const qword min(const qword x, const qword y) __attribute__((pure, const, always_inline));
static inline const qword min(const qword x, const qword y)
{
	qword mask 					= si_fcgt(x, y);							// EVEN, 2
	qword result				= si_selb(y, x, mask);						// EVEN, 2
	return result;
}

#endif // #ifdef PLATFORM_SPU



//! Main entry point for SPU task.
void DamageTask(Scheduler::Task* __restrict pTask)
{
#if defined(PLATFORM_SPU)
	PERF_EVENT("CarDamage");
													  cellDmaGet((uint32_t)&contextBuffer[0x0], (uint64_t)pTask->Input, (sizeof(C_DamageContext)+0xf)&~0xf, 0x0, 0x0, 0x0);
	C_DamageContext* pContext						= (C_DamageContext*)&contextBuffer[0x0];
													  cellDmaWaitTagStatusAll(1<<0);
	Vehicle::C_NodeLayer* pLayers					= 0x0;
													  cellDmaLargeGet((uint32_t)&damageLayers[0x0], (uint64_t)pContext->damageLayerPtr, ((sizeof(Vehicle::C_NodeLayer)*3)+0xf)&~0xf, 0x0, 0x0, 0x0);
	pLayers											= (Vehicle::C_NodeLayer*)&damageLayers[0x0];	// Damage layer ready to rock! :)
	g_pLayers										= pLayers;
	g_pHeightProfile								= (float*)Spu::FetchAllocUnaligned((void*)pContext->heightProfilePtr, 0x8*sizeof(float));
													  memcpy(&g_VehicleBoundsInfo, &pContext->boundsInfo[0], 4 * sizeof(float));
													  cellDmaLargeGet((uint32_t)&buffer[0], (uint64_t)pContext->inputVertexBuffers[0], (0x10<<0x0b), 0x4, 0x0, 0x0);	
													  cellDmaWaitTagStatusAll(1<<0);

	

	// get control points
	F32 *pControlPoints = (F32*)Spu::FetchAlloc(pContext->pControlPointData, sizeof(F32)*6*3*4);
	
	if(!pContext->noDeformation)	// light weight job with no deformation.
	{
		PERF_EVENT("Vertex Processing");
													  cellDmaWaitTagStatusAll(1<<4);					
		uint32_t debugVertexCount					= 0;
		uint16_t currentBuffer						= 0;
		uint16_t vertexBufferCount					= pContext->vertexBufferCount;
		do
		{
			// Skip NULL vertex buffers.
			if(!pContext->inputVertexBuffers[currentBuffer] ||
			   !pContext->outputVertexBuffers[currentBuffer])
				continue;
		
			uint16_t currentVertex					= 0;
			uint16_t vertexCount					= pContext->inputBufferSize[currentBuffer] / pContext->inputStride[currentBuffer];
			uint16_t vertexSize						= pContext->inputStride[currentBuffer];
			uint8_t* addr							= &buffer[0];
			uint16_t copySize						= 0;														// Assume buffer is aligned at start.
			uint16_t tagGroup						= 0;														// Wait on tag group 4 first time round.
			uint32_t currentBufferEA				= (uint32_t)pContext->inputVertexBuffers[currentBuffer];
			uint32_t numberOfVertsInFullBuffer		= (0x10 << 0xb) / vertexSize;
			uint32_t bufferCountMask				= -(pContext->inputBufferSize[currentBuffer]<(0x10<<0x0b));	// will all fit in one buffer!
					 numberOfVertsInFullBuffer		= ((numberOfVertsInFullBuffer^(pContext->inputBufferSize[currentBuffer]/vertexSize))&bufferCountMask)^numberOfVertsInFullBuffer;
			uint16_t offset							= 0;
			uint16_t outputOffset					= 0;
			uint8_t* outputAddr						= &outputBuffer[0];
			uint32_t outputUsed						= 0;											// number of bytes used in output.
			uint16_t outputVertexSize				= pContext->outputStride[currentBuffer];
			uint32_t currentOutputBufferEA			= pContext->outputVertexBuffers[currentBuffer];
			uint32_t startOutputBufferEA			= currentOutputBufferEA;

			U32 controlPointOffset = pContext->controlPointOffset[currentBuffer];
			U32 positionOffsetOffset = pContext->positionOffsetOffset[currentBuffer];
			U32 normalOffsetOffset = pContext->normalOffsetOffset[currentBuffer];
			U32 colourOffset = pContext->colourOffset[currentBuffer];

			do	// Stream through every vertex.
			{
				// Need to take into account number of verts in chunk of vertex buffer < the size of the buffer in LS.
				for(uint32_t currentVertexInBuffer=0; currentVertexInBuffer<numberOfVertsInFullBuffer; ++currentVertexInBuffer)
				{
					// vertex shader work.
					qword raw_pos					= load_unaligned_float16_4(addr);
					float raw_pos_x					= si_to_float(raw_pos);
					float raw_pos_y					= si_to_float(si_rotqbyi(raw_pos, 0x4));
					float raw_pos_z					= si_to_float(si_rotqbyi(raw_pos, 0x8));
					Maths::HVec4	localPos		= Maths::HVec4(raw_pos_x, raw_pos_y, raw_pos_z, 0);

					qword raw_normal				= load_unaligned_SSHD_3N(addr+8);
					float raw_normal_x				= si_to_float(raw_normal);
					float raw_normal_y				= si_to_float(si_rotqbyi(raw_normal, 0x4));
					float raw_normal_z				= si_to_float(si_rotqbyi(raw_normal, 0x8));
					Maths::HVec4	localNormal		= Maths::HVec4(raw_normal_x, raw_normal_y, raw_normal_z, 0);

					Maths::HVec2	damageUV		= GetDamageUV((Maths::HVec3)localPos);
					Maths::HVec4	damageInfo		= EvaluateDamageInfoTexture(damageUV, localPos.Y());
					Maths::HVec4	damage			= EvaluateVertexDamage(Maths::HVec3(damageUV.X(), damageUV.Y(), damageInfo.X()));

					// apply control point deformation
					if (controlPointOffset != UINT_MAX)
						damage						+= ControlPointDamage((Maths::HVec3)localPos, addr+controlPointOffset, pControlPoints);

					// apply morph target deformation
					if (positionOffsetOffset != UINT_MAX)
					{
						Maths::HVec4 posOffset, normalOffset;
						posOffset.m_V = (vector float)load_unaligned_SSHD_3N(addr + positionOffsetOffset);
						posOffset.W(0);
						normalOffset.m_V = (vector float)load_unaligned_SSHD_3N(addr + normalOffsetOffset);
						normalOffset.W(0);

						damage += posOffset * damageInfo.Y();
						localNormal = localNormal + normalOffset*damageInfo.Y();
						localNormal = localNormal.Normalised();
					}

					localPos += damage;

					localPos.W(localPos.W()*Maths::HF32(2.f));

					// calculate rear light attenuation
					U32 iRearLightAttenuation = CalculateRearLightAttenuation(localPos, localNormal, addr[colourOffset]);
					// scratch scale goes into the frac and rear light attenuation goes into the int part of the F16
					localNormal.W(Maths::Min(0.996f, damageInfo.Y()) + (F32)iRearLightAttenuation);

					*(qword*)outputAddr = float32_to_float16((qword)localPos.m_V, (qword)localNormal.m_V);
					
					// increment pointers.
					outputAddr += outputVertexSize;
					outputUsed += outputVertexSize;
					addr += vertexSize;
					currentVertex += 1;
					currentBufferEA += vertexSize;
					currentOutputBufferEA += outputVertexSize;
					debugVertexCount += 1;
				}
				
				if(currentVertex < vertexCount)		// replace with conditional dma.
				{
					// Fill the buffer again.
					offset = currentBufferEA - (currentBufferEA & ~0xf);
					cellDmaLargeGet((uint32_t)&buffer[0], (uint64_t)currentBufferEA & ~0xf, (0x10<<0x0b), 0x5, 0x0, 0x0);
					addr = (uint8_t*)(&buffer[0] + offset);
					numberOfVertsInFullBuffer = ( ((((0x10<<0x0b)-offset)/vertexSize) ^ (vertexCount-currentVertex)) & -((vertexCount-currentVertex) < (((0x10<<0x0b)-offset)/vertexSize))) ^ (((0x10<<0x0b)-offset)/vertexSize);
					
				}
	 			
 				outputOffset = currentOutputBufferEA - (currentOutputBufferEA & ~0xf);
 				cellDmaLargePut((uint32_t)&outputBuffer[0], (uint64_t)startOutputBufferEA, (outputUsed & ~0xf), 0x0, 0x0, 0x0);
 				cellDmaWaitTagStatusAll(1<<0);
 				
 				
 				memcpy(&outputBuffer[0], (const void*)((uint32_t)outputAddr & ~0xf), outputOffset);
 				startOutputBufferEA = (startOutputBufferEA + outputUsed) & ~0xf;
 				outputUsed = outputOffset;
 				outputAddr = &outputBuffer[0] + outputOffset;	// ready to go at the new offset.
 				cellDmaWaitTagStatusAll(1<<0);
 				cellDmaWaitTagStatusAll(1<<5);
 																// TODO: [Stove] needs a double buffer... like srsly!
			} while (currentVertex < vertexCount);
	 		
 			// Fetch first 32KiB of next vertex buffer.
 			if(currentBuffer+1 < vertexBufferCount && pContext->inputVertexBuffers[currentBuffer+1])
 				cellDmaLargeGet((uint32_t)&buffer[0], (uint64_t)pContext->inputVertexBuffers[currentBuffer+1], (0x10<<0x0b), 0x4, 0x0, 0x0);
 			cellDmaWaitTagStatusAll(1<<4);
	 		
 		} while(++currentBuffer < vertexBufferCount);
 	}
 	else
 		cellDmaWaitTagStatusAll(1<<4);
 	
 												  cellDmaWaitTagStatusAll(1<<0);
												  cellDmaWaitTagStatusAll(1<<1);	// Wait for output data -- deformed layer stuff.
												  
												  // Atomic put the value.
 												  MT::AtomicSet((volatile U32*)pContext->damageStateEA, pContext->contextLabelValue);
												  
#endif // #if defined(PLATFORM_SPU)
	return;
}

};

JQ_DECLARE_SPU_MODULE(CarDamageJob, SpuDamage::DamageTask, spudamage, "cardamage");
