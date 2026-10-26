#include "stdafx.h"
#include <Graphics/TonemapSPU.h>

#include "Spu/Maths.h"
#include "System/Scheduler/JobQueue.h"

#if defined(PLATFORM_SPU) || defined(PLATFORM_PS3) 

#ifdef PLATFORM_SPU

const int TagRead = 1, TagWrite[] = { 3, 4 };
char BufIn[32*32*32*4] __attribute__((aligned(128)));
char BufOut[2][32*32*4] __attribute__((aligned(128)));

vector float GetValue(Display::C_LockedBox& lockedBoxSrc, S32 u, S32 v, S32 w)
{
	U32* Slice = (U32*)(&BufIn[w * lockedBoxSrc.slicePitch]);

	unsigned int src = Slice[u + v * lockedBoxSrc.rowPitch / 4];
	vector unsigned int src_u4 = spu_splats(src);
	src_u4 = spu_shuffle(src_u4, src_u4, ((vector unsigned char){ 2, 0x80, 0x80, 0x80, 1, 0x80, 0x80, 0x80, 0, 0x80, 0x80, 0x80, 3, 0x80, 0x80, 0x80 }));

	return spu_convtf(src_u4, 32);
}

vector float SampleLUT(Display::C_LockedBox& lockedBoxSrc, float r, float g, float b)
{
	F32 x = (F32)r * (32-1);
	F32 y = (F32)g * (32-1);
	F32 z = (F32)b * (32-1);

	S32 ix = (S32)floor(x);
	S32 iy = (S32)floor(y);
	S32 iz = (S32)floor(z);

	F32 fx = x - (F32)ix;
	F32 fy = y - (F32)iy;
	F32 fz = z - (F32)iz;

	F32 omfx = 1.0f - fx;
	F32 omfy = 1.0f - fy;
	F32 omfz = 1.0f - fz;

	vector float result = {0.0f, 0.0f, 0.0f, 0.0f};

	result = spu_madd(GetValue(lockedBoxSrc, ix,   iy,   iz  ), spu_splats(omfx * omfy * omfz), result);
	result = spu_madd(GetValue(lockedBoxSrc, ix+1, iy,   iz  ), spu_splats(fx   * omfy * omfz), result);
	result = spu_madd(GetValue(lockedBoxSrc, ix,   iy+1, iz  ), spu_splats(omfx * fy   * omfz), result);
	result = spu_madd(GetValue(lockedBoxSrc, ix+1, iy+1, iz  ), spu_splats(fx   * fy   * omfz), result);

	result = spu_madd(GetValue(lockedBoxSrc, ix,   iy,   iz+1), spu_splats(omfx * omfy * fz), result);
	result = spu_madd(GetValue(lockedBoxSrc, ix+1, iy,   iz+1), spu_splats(fx   * omfy * fz), result);
	result = spu_madd(GetValue(lockedBoxSrc, ix,   iy+1, iz+1), spu_splats(omfx * fy   * fz), result);
	result = spu_madd(GetValue(lockedBoxSrc, ix+1, iy+1, iz+1), spu_splats(fx   * fy   * fz), result);

	return result;
}

void UpdateGamma(Tonemap::TaskData* Data)
{
	Display::C_LockedBox& lockedBoxSrc = Data->lockedBoxSrc;
	Display::C_LockedBox& lockedBoxGamma = Data->lockedBoxGamma;

	Spu::FetchAsync(BufIn, lockedBoxSrc.pBits, sizeof(BufIn), TagRead);

	float table[32];
	for (int i = 0; i < 32; i++)
		table[i] = pow((float)i / 31.0f, Data->fGamma);

	Spu::Wait(TagRead);
	int CurOut = 0;

	for(U32 w = 0; w < 32; ++w)
	{
		// Wait for previous DMA to complete.
		Spu::Wait(TagWrite[CurOut]);
		U32* pBitsDst = (U32*)BufOut[CurOut];

		float z = table[w];

		for(U32 v = 0; v < 32; ++v)
		{
			float y = table[v];

			for(U32 u = 0; u < 32; ++u)
			{
				float x = table[u];

				vector float color = SampleLUT(lockedBoxSrc, x, y, z);

				// Pack it back into an u32.
				vector unsigned int out = spu_convtu(color, 32);
				out = spu_shuffle(out, out, ((vector unsigned char) { 8, 4, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}) );
				unsigned int dest = spu_extract(out, 0);

				pBitsDst[u + v * lockedBoxGamma.rowPitch / 4] = dest;
			}
		}

		Spu::StoreAsync(lockedBoxGamma.pBits + w * lockedBoxGamma.slicePitch, BufOut[CurOut], sizeof(BufOut[CurOut]), TagWrite[CurOut]);
		CurOut ^= 1;
	}

	Spu::WaitMultiple(0xffff);
}

#endif

void TonemapUpdate(Scheduler::Task* pTask)
{
#ifdef PLATFORM_SPU
	Tonemap::TaskData* Data = (Tonemap::TaskData*)pTask->ParamData;

	// If the gamma changed, update the gamma texture.
	if (Data->UpdateGamma)
		UpdateGamma(Data);

	Display::C_LockedBox& lockedBoxGamma = Data->lockedBoxGamma;
	Display::C_LockedBox& lockedBoxDst = Data->lockedBoxDst;
	Maths::Mat44 mColour;

	int CurOut = 0;

	Spu::FetchAsync(&mColour, Data->mColour, sizeof(mColour), TagRead);
	Spu::FetchAsync(BufIn, lockedBoxGamma.pBits, sizeof(BufIn), TagRead);

	Spu::Wait(TagRead);

	const vector unsigned char shuffle0 = ((vector unsigned char) {0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03});
	const vector unsigned char shuffle1 = ((vector unsigned char) {0x04, 0x05, 0x06, 0x07, 0x04, 0x05, 0x06, 0x07, 0x04, 0x05, 0x06, 0x07, 0x04, 0x05, 0x06, 0x07});
	const vector unsigned char shuffle2 = ((vector unsigned char) {0x08, 0x09, 0x0a, 0x0b, 0x08, 0x09, 0x0a, 0x0b, 0x08, 0x09, 0x0a, 0x0b, 0x08, 0x09, 0x0a, 0x0b});

	for(U32 w = 0; w < 32; ++w)
	{
		// Wait for previous DMA to complete.
		Spu::Wait(TagWrite[CurOut]);
		U32* pBitsDst = (U32*)BufOut[CurOut];
		U32* pBitsGamma = (U32*)(&BufIn[w * lockedBoxGamma.slicePitch]);

		for(U32 v = 0; v < 32; ++v)
		{
			for(U32 u = 0; u < 32; ++u)
			{
				unsigned int src = pBitsGamma[u + v * lockedBoxGamma.rowPitch / 4];
				vector unsigned int src_u4 = spu_splats(src);
				src_u4 = spu_shuffle(src_u4, src_u4, ((vector unsigned char){ 2, 0x80, 0x80, 0x80, 1, 0x80, 0x80, 0x80, 0, 0x80, 0x80, 0x80, 3, 0x80, 0x80, 0x80 }));

				vector float color = spu_convtf(src_u4, 32);

				vector float vecr = spu_shuffle(color, color, shuffle0);
				vector float vecg = spu_shuffle(color, color, shuffle1);
				vector float vecb = spu_shuffle(color, color, shuffle2);

				color = spu_madd(vecr, mColour.m_RowX, spu_madd(vecg, mColour.m_RowY, spu_madd(vecb, mColour.m_RowZ, mColour.m_RowW)));

				// Pack it back into an u32.
				vector unsigned int out = spu_convtu(color, 32);
				out = spu_shuffle(out, out, ((vector unsigned char) { 12, 0, 4, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}) );
				unsigned int dest = spu_extract(out, 0);

				pBitsDst[u + v * lockedBoxDst.rowPitch / 4] = dest;
			}
		}

		Spu::StoreAsync(lockedBoxDst.pBits + w * lockedBoxDst.slicePitch, BufOut[CurOut], sizeof(BufOut[CurOut]), TagWrite[CurOut]);
		CurOut ^= 1;
	}

	Spu::WaitMultiple(0xffff);
#endif
}

JQ_DECLARE_SPU_MODULE(TonemapModule, TonemapUpdate, TonemapSpu, "Tonemap");

#endif
