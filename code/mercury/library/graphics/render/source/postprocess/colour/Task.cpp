#include "stdafx.h"
#include "Render/Postprocess/Colour/Task.h"
#include "Render/Postprocess/Colour/Functions.h"
#include "System/Scheduler.h"

#define APPLY_COLOUR_FUNCTION(func, ...) \
	vColour0 = func(vColour0, __VA_ARGS__); \
	vColour1 = func(vColour1, __VA_ARGS__); \
	vColour2 = func(vColour2, __VA_ARGS__); \
	vColour3 = func(vColour3, __VA_ARGS__); \
	vColour4 = func(vColour4, __VA_ARGS__); \
	vColour5 = func(vColour5, __VA_ARGS__); \
	vColour6 = func(vColour6, __VA_ARGS__); \
	vColour7 = func(vColour7, __VA_ARGS__)

namespace PostProcess
{

void ColourLookupTableBuild(ColourTaskData* pData)
{
	PERF_EVENT("LookupTable");

	pData = (ColourTaskData*)Spu::FetchAlloc(pData, sizeof(ColourTaskData));

	U32* const	pOutput		= pData->m_pData;	
	U32 const	iSize		= pData->m_iSize;
	U32 const	iRowPitch	= pData->m_iRowPitch;
	U32 const	iSlicePitch	= pData->m_iSlicePitch;

#if PLATFORM_SPU
	U32*	pWorking	= (U32*)Spu::Alloc(sizeof(U32) * iSize * iSize);
#else
	U32*	pWorking	= pData->m_pData;
#endif

	// set up maths
	Maths::VF32		fBrightness	= Maths::MakeVF32(pData->m_fBrightness);
	Maths::VF32		fContrast	= Maths::MakeVF32(pData->m_fContrast);
	Maths::VF32		fGamma		= Maths::MakeVF32(pData->m_fGamma);
	Maths::VF32		fSepia		= Maths::MakeVF32(pData->m_fSepia);
	Maths::Vec4F32	vAdd		= Maths::Load<Maths::Vec4F32>(&pData->m_vAdd);
	Maths::Vec4F32	vModulate	= Maths::Load<Maths::Vec4F32>(&pData->m_vModulate);
	Maths::Vec4F32	vDesaturate	= Maths::Load<Maths::Vec4F32>(&pData->m_vDesaturate);
	
	const F32 fScale = 1.0f / (F32) (iSize - 1);

	if (pWorking && pOutput)
	{
		volatile U32* pSlice = (volatile U32*)pWorking;
		for (U32 b = 0; b < iSize; ++b)
		{
			volatile U32* pRow = pSlice;
			for (U32 g = 0; g < iSize; ++g)
			{
				for (U32 r = 0; r < iSize; r += 8)
				{
					const F32 fBlue = b * fScale;
					const F32 fGreen = g * fScale;
		
					const F32 fRed0 = (r + 0) * fScale;
					const F32 fRed1 = (r + 1) * fScale;
					const F32 fRed2 = (r + 2) * fScale;
					const F32 fRed3 = (r + 3) * fScale;
					const F32 fRed4 = (r + 4) * fScale;
					const F32 fRed5 = (r + 5) * fScale;
					const F32 fRed6 = (r + 6) * fScale;
					const F32 fRed7 = (r + 7) * fScale;

					Maths::Vec4F32 vColour0 = Maths::MakeVec4F32(fRed0, fGreen, fBlue, 1.0f);
					Maths::Vec4F32 vColour1 = Maths::MakeVec4F32(fRed1, fGreen, fBlue, 1.0f);
					Maths::Vec4F32 vColour2 = Maths::MakeVec4F32(fRed2, fGreen, fBlue, 1.0f);
					Maths::Vec4F32 vColour3 = Maths::MakeVec4F32(fRed3, fGreen, fBlue, 1.0f);
					Maths::Vec4F32 vColour4 = Maths::MakeVec4F32(fRed4, fGreen, fBlue, 1.0f);
					Maths::Vec4F32 vColour5 = Maths::MakeVec4F32(fRed5, fGreen, fBlue, 1.0f);
					Maths::Vec4F32 vColour6 = Maths::MakeVec4F32(fRed6, fGreen, fBlue, 1.0f);
					Maths::Vec4F32 vColour7 = Maths::MakeVec4F32(fRed7, fGreen, fBlue, 1.0f);

					APPLY_COLOUR_FUNCTION(AdjustBrightness, fBrightness);
					APPLY_COLOUR_FUNCTION(AdjustContrast, fContrast);
					APPLY_COLOUR_FUNCTION(AdjustSaturation, vDesaturate);
					APPLY_COLOUR_FUNCTION(ApplySepia, fSepia);
					APPLY_COLOUR_FUNCTION(ApplyGamma, fGamma);
					APPLY_COLOUR_FUNCTION(ApplyModulateAdd, vModulate, vAdd);

					pRow[r+0] = Maths::RGBA(Maths::C_RGBA_F32(Maths::HVec4(vColour0)).Saturate());
					pRow[r+1] = Maths::RGBA(Maths::C_RGBA_F32(Maths::HVec4(vColour1)).Saturate());
					pRow[r+2] = Maths::RGBA(Maths::C_RGBA_F32(Maths::HVec4(vColour2)).Saturate());
					pRow[r+3] = Maths::RGBA(Maths::C_RGBA_F32(Maths::HVec4(vColour3)).Saturate());
					pRow[r+4] = Maths::RGBA(Maths::C_RGBA_F32(Maths::HVec4(vColour4)).Saturate());
					pRow[r+5] = Maths::RGBA(Maths::C_RGBA_F32(Maths::HVec4(vColour5)).Saturate());
					pRow[r+6] = Maths::RGBA(Maths::C_RGBA_F32(Maths::HVec4(vColour6)).Saturate());
					pRow[r+7] = Maths::RGBA(Maths::C_RGBA_F32(Maths::HVec4(vColour7)).Saturate());
				}
				pRow += iRowPitch / 4;
			}
#if PLATFORM_SPU
			Spu::Store((void*)(pOutput + b * iSlicePitch), (const void*)(pSlice), sizeof(U32) * iSize * iSize);
#endif
			pSlice += iSlicePitch / 4;
		}
	}

}

void ColourLookupTableTask(Scheduler::Task* pTask)
{
	ColourLookupTableBuild((ColourTaskData*)pTask->Output);
}

}; // namespace PostProcess

JQ_DECLARE_SPU_MODULE(ColourLookupTableModule, PostProcess::ColourLookupTableTask, ColourLookupTableModule, "postprocess");
