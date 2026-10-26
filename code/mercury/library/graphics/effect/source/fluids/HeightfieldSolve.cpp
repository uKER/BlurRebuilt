#include "stdafx.h"
#include "Effect/Fluids/HeightfieldTasks.h"
#include "System/Scheduler/JobQueue.h"

namespace Effect
{

#if defined(PLATFORM_SPU)

void HeightfieldSolve(Scheduler::Task* pTask)
{
	Spu::MemBaseMarker marker;
	
	HeightfieldSolveData* pData = (HeightfieldSolveData*)Spu::FetchAlloc(pTask->Output, sizeof(HeightfieldSolveData));

	const U32 iRowSize	= sizeof(F32) * pData->m_iWidth;
	const U32 iWidth	= pData->m_iWidth;
	const U32 iHeight	= pData->m_iHeight;
	const U32 iPitch	= pData->m_iPitch;

	const bool isTop	= (pData->m_pTop != 0);
	const bool isBottom	= (pData->m_pBottom != 0);
	const bool isLeft	= (pData->m_pLeft != 0);
	const bool isRight	= (pData->m_pRight != 0);

	vector float* pOutput		= (vector float*) Spu::Alloc(sizeof(F32) * iWidth);
	vector float* pOutputSave	= (vector float*) Spu::Alloc(sizeof(F32) * iWidth);
	vector float* pPrevious		= (vector float*) Spu::Alloc(sizeof(F32) * iWidth);
	vector float* pPreviousLoad	= (vector float*) Spu::Alloc(sizeof(F32) * iWidth);
	vector float* pCurrent0		= (vector float*) Spu::Alloc(sizeof(F32) * iWidth + 32);
	vector float* pCurrent1		= (vector float*) Spu::Alloc(sizeof(F32) * iWidth + 32);
	vector float* pCurrent2		= (vector float*) Spu::Alloc(sizeof(F32) * iWidth + 32);
	vector float* pCurrent3		= (vector float*) Spu::Alloc(sizeof(F32) * iWidth + 32);

	const U8* pTopAddress = pData->m_pCurrent;
	const U8* pBottomAddress = pData->m_pCurrent + iPitch * (iHeight - 1);
	if (isTop)
		pTopAddress = pData->m_pTop + iPitch * (iHeight - 1);
	if (isBottom)
		pBottomAddress = pData->m_pBottom;

	Spu::FetchAsync(pCurrent0 + 1, pTopAddress, iRowSize, 0);
	Spu::FetchAsync(pCurrent1 + 1, pData->m_pCurrent, iRowSize, 0);
	Spu::FetchAsync(pCurrent2 + 1, pData->m_pCurrent + iPitch, iRowSize, 0);
	Spu::FetchAsync(pPrevious, pData->m_pPrevious, iRowSize, 0);

	for (U32 y = 0; y < iHeight; ++y)
	{
		Spu::Wait(0);

		const U32 yNext			= (y + 1 < iHeight) ? y + 1 : y;
		const U8* pNextAddress	= (yNext + 1 < iHeight) ? pData->m_pCurrent + iPitch * (yNext + 1) : pBottomAddress;

		Spu::FetchAsync(pCurrent3 + 1, pNextAddress, iRowSize, 0);
		Spu::FetchAsync(pPreviousLoad, pData->m_pPrevious + iPitch * yNext, iRowSize, 0);

		// clamp left and right edges
		pCurrent2[0]				= spu_splats(spu_extract(pCurrent2[1], 0));
		pCurrent2[iWidth / 4 + 1]	= spu_splats(spu_extract(pCurrent2[iWidth / 4], 3));
		
		// wrap if need be
		if (isLeft)
			Spu::FetchAsync(pCurrent2, pData->m_pLeft + pData->m_iPitch * yNext + iWidth * sizeof(F32) - sizeof(vector float), sizeof(vector float), 0);
		if (isRight)
			Spu::FetchAsync(pCurrent2 + iWidth / 4 + 1, pData->m_pRight + iPitch * yNext, sizeof(vector float), 0);
	
		// loop
		for (U32 x = 0; x < iWidth / 4; x += 8)
		{
			vector float	vQuarter	= spu_splats(0.25f);
			vector float	vScale		= spu_splats(pData->m_fScale);
			vector float	vDamping	= spu_splats(pData->m_fDamping);

			vector float	vCurrent0	= *((vector float*)&pCurrent1[x + 0]);
			vector float	vCurrent1	= *((vector float*)&pCurrent1[x + 1]);
			vector float	vCurrent2	= *((vector float*)&pCurrent1[x + 2]);
			vector float	vCurrent3	= *((vector float*)&pCurrent1[x + 3]);
			vector float	vCurrent4	= *((vector float*)&pCurrent1[x + 4]);
			vector float	vCurrent5	= *((vector float*)&pCurrent1[x + 5]);
			vector float	vCurrent6	= *((vector float*)&pCurrent1[x + 6]);
			vector float	vCurrent7	= *((vector float*)&pCurrent1[x + 7]);
			vector float	vCurrent8	= *((vector float*)&pCurrent1[x + 8]);
			vector float	vCurrent9	= *((vector float*)&pCurrent1[x + 9]);

			vector float	vPrevious0	= *((vector float*)&pPrevious[x + 0]);
			vector float	vPrevious1	= *((vector float*)&pPrevious[x + 1]);
			vector float	vPrevious2	= *((vector float*)&pPrevious[x + 2]);
			vector float	vPrevious3	= *((vector float*)&pPrevious[x + 3]);
			vector float	vPrevious4	= *((vector float*)&pPrevious[x + 4]);
			vector float	vPrevious5	= *((vector float*)&pPrevious[x + 5]);
			vector float	vPrevious6	= *((vector float*)&pPrevious[x + 6]);
			vector float	vPrevious7	= *((vector float*)&pPrevious[x + 7]);

			vector float	vShiftL0	= spu_sel(vCurrent1, vCurrent0, (vector unsigned int){0, 0, 0, 0xFFFFFFFF});
			vector float	vShiftL1	= spu_sel(vCurrent2, vCurrent1, (vector unsigned int){0, 0, 0, 0xFFFFFFFF});
			vector float	vShiftL2	= spu_sel(vCurrent3, vCurrent2, (vector unsigned int){0, 0, 0, 0xFFFFFFFF});
			vector float	vShiftL3	= spu_sel(vCurrent4, vCurrent3, (vector unsigned int){0, 0, 0, 0xFFFFFFFF});
			vector float	vShiftL4	= spu_sel(vCurrent5, vCurrent4, (vector unsigned int){0, 0, 0, 0xFFFFFFFF});
			vector float	vShiftL5	= spu_sel(vCurrent6, vCurrent5, (vector unsigned int){0, 0, 0, 0xFFFFFFFF});
			vector float	vShiftL6	= spu_sel(vCurrent7, vCurrent6, (vector unsigned int){0, 0, 0, 0xFFFFFFFF});
			vector float	vShiftL7	= spu_sel(vCurrent8, vCurrent7, (vector unsigned int){0, 0, 0, 0xFFFFFFFF});

			vector float	vShiftR0	= spu_sel(vCurrent1, vCurrent2, (vector unsigned int){0xFFFFFFFF, 0, 0, 0});
			vector float	vShiftR1	= spu_sel(vCurrent2, vCurrent3, (vector unsigned int){0xFFFFFFFF, 0, 0, 0});
			vector float	vShiftR2	= spu_sel(vCurrent3, vCurrent4, (vector unsigned int){0xFFFFFFFF, 0, 0, 0});
			vector float	vShiftR3	= spu_sel(vCurrent4, vCurrent5, (vector unsigned int){0xFFFFFFFF, 0, 0, 0});
			vector float	vShiftR4	= spu_sel(vCurrent5, vCurrent6, (vector unsigned int){0xFFFFFFFF, 0, 0, 0});
			vector float	vShiftR5	= spu_sel(vCurrent6, vCurrent7, (vector unsigned int){0xFFFFFFFF, 0, 0, 0});
			vector float	vShiftR6	= spu_sel(vCurrent7, vCurrent8, (vector unsigned int){0xFFFFFFFF, 0, 0, 0});
			vector float	vShiftR7	= spu_sel(vCurrent8, vCurrent9, (vector unsigned int){0xFFFFFFFF, 0, 0, 0});

							vShiftL0	= spu_rlqwbyte(vShiftL0, 12);
							vShiftL1	= spu_rlqwbyte(vShiftL1, 12);
							vShiftL2	= spu_rlqwbyte(vShiftL2, 12);
							vShiftL3	= spu_rlqwbyte(vShiftL3, 12);
							vShiftL4	= spu_rlqwbyte(vShiftL4, 12);
							vShiftL5	= spu_rlqwbyte(vShiftL5, 12);
							vShiftL6	= spu_rlqwbyte(vShiftL6, 12);
							vShiftL7	= spu_rlqwbyte(vShiftL7, 12);

							vShiftR0	= spu_rlqwbyte(vShiftR0, 4);
							vShiftR1	= spu_rlqwbyte(vShiftR1, 4);
							vShiftR2	= spu_rlqwbyte(vShiftR2, 4);
							vShiftR3	= spu_rlqwbyte(vShiftR3, 4);
							vShiftR4	= spu_rlqwbyte(vShiftR4, 4);
							vShiftR5	= spu_rlqwbyte(vShiftR5, 4);
							vShiftR6	= spu_rlqwbyte(vShiftR6, 4);
							vShiftR7	= spu_rlqwbyte(vShiftR7, 4);

			vector float	vTop0		= *((vector float*)&pCurrent0[x + 1]);
			vector float	vTop1		= *((vector float*)&pCurrent0[x + 2]);
			vector float	vTop2		= *((vector float*)&pCurrent0[x + 3]);
			vector float	vTop3		= *((vector float*)&pCurrent0[x + 4]);
			vector float	vTop4		= *((vector float*)&pCurrent0[x + 5]);
			vector float	vTop5		= *((vector float*)&pCurrent0[x + 6]);
			vector float	vTop6		= *((vector float*)&pCurrent0[x + 7]);
			vector float	vTop7		= *((vector float*)&pCurrent0[x + 8]);

			vector float	vBottom0	= *((vector float*)&pCurrent2[x + 1]);
			vector float	vBottom1	= *((vector float*)&pCurrent2[x + 2]);
			vector float	vBottom2	= *((vector float*)&pCurrent2[x + 3]);
			vector float	vBottom3	= *((vector float*)&pCurrent2[x + 4]);
			vector float	vBottom4	= *((vector float*)&pCurrent2[x + 5]);
			vector float	vBottom5	= *((vector float*)&pCurrent2[x + 6]);
			vector float	vBottom6	= *((vector float*)&pCurrent2[x + 7]);
			vector float	vBottom7	= *((vector float*)&pCurrent2[x + 8]);
			
			vector float	vOffset0	= spu_mul(vTop0, vQuarter);
			vector float	vOffset1	= spu_mul(vTop1, vQuarter);
			vector float	vOffset2	= spu_mul(vTop2, vQuarter);
			vector float	vOffset3	= spu_mul(vTop3, vQuarter);
			vector float	vOffset4	= spu_mul(vTop4, vQuarter);
			vector float	vOffset5	= spu_mul(vTop5, vQuarter);
			vector float	vOffset6	= spu_mul(vTop6, vQuarter);
			vector float	vOffset7	= spu_mul(vTop7, vQuarter);
							
							vOffset0	= spu_madd(vBottom0, vQuarter, vOffset0);
							vOffset1	= spu_madd(vBottom1, vQuarter, vOffset1);
							vOffset2	= spu_madd(vBottom2, vQuarter, vOffset2);
							vOffset3	= spu_madd(vBottom3, vQuarter, vOffset3);
							vOffset4	= spu_madd(vBottom4, vQuarter, vOffset4);
							vOffset5	= spu_madd(vBottom5, vQuarter, vOffset5);
							vOffset6	= spu_madd(vBottom6, vQuarter, vOffset6);
							vOffset7	= spu_madd(vBottom7, vQuarter, vOffset7);
			
							vOffset0	= spu_madd(vShiftL0, vQuarter, vOffset0);
							vOffset1	= spu_madd(vShiftL1, vQuarter, vOffset1);
							vOffset2	= spu_madd(vShiftL2, vQuarter, vOffset2);
							vOffset3	= spu_madd(vShiftL3, vQuarter, vOffset3);
							vOffset4	= spu_madd(vShiftL4, vQuarter, vOffset4);
							vOffset5	= spu_madd(vShiftL5, vQuarter, vOffset5);
							vOffset6	= spu_madd(vShiftL6, vQuarter, vOffset6);
							vOffset7	= spu_madd(vShiftL7, vQuarter, vOffset7);

							vOffset0	= spu_madd(vShiftR0, vQuarter, vOffset0);
							vOffset1	= spu_madd(vShiftR1, vQuarter, vOffset1);
							vOffset2	= spu_madd(vShiftR2, vQuarter, vOffset2);
							vOffset3	= spu_madd(vShiftR3, vQuarter, vOffset3);
							vOffset4	= spu_madd(vShiftR4, vQuarter, vOffset4);
							vOffset5	= spu_madd(vShiftR5, vQuarter, vOffset5);
							vOffset6	= spu_madd(vShiftR6, vQuarter, vOffset6);
							vOffset7	= spu_madd(vShiftR7, vQuarter, vOffset7);

							vOffset0	= spu_sub(vOffset0, vCurrent1);
							vOffset1	= spu_sub(vOffset1, vCurrent2);
							vOffset2	= spu_sub(vOffset2, vCurrent3);
							vOffset3	= spu_sub(vOffset3, vCurrent4);
							vOffset4	= spu_sub(vOffset4, vCurrent5);
							vOffset5	= spu_sub(vOffset5, vCurrent6);
							vOffset6	= spu_sub(vOffset6, vCurrent7);
							vOffset7	= spu_sub(vOffset7, vCurrent8);

			vector float	vAccel0		= spu_mul(vOffset0, vScale);
			vector float	vAccel1		= spu_mul(vOffset1, vScale);
			vector float	vAccel2		= spu_mul(vOffset2, vScale);
			vector float	vAccel3		= spu_mul(vOffset3, vScale);
			vector float	vAccel4		= spu_mul(vOffset4, vScale);
			vector float	vAccel5		= spu_mul(vOffset5, vScale);
			vector float	vAccel6		= spu_mul(vOffset6, vScale);
			vector float	vAccel7		= spu_mul(vOffset7, vScale);

			vector float	vVelocity0	= spu_sub(vCurrent1, vPrevious0);
			vector float	vVelocity1	= spu_sub(vCurrent2, vPrevious1);
			vector float	vVelocity2	= spu_sub(vCurrent3, vPrevious2);
			vector float	vVelocity3	= spu_sub(vCurrent4, vPrevious3);
			vector float	vVelocity4	= spu_sub(vCurrent5, vPrevious4);
			vector float	vVelocity5	= spu_sub(vCurrent6, vPrevious5);
			vector float	vVelocity6	= spu_sub(vCurrent7, vPrevious6);
			vector float	vVelocity7	= spu_sub(vCurrent8, vPrevious7);

							vVelocity0	= spu_madd(vVelocity0, vDamping, vAccel0);
							vVelocity1	= spu_madd(vVelocity1, vDamping, vAccel1);
							vVelocity2	= spu_madd(vVelocity2, vDamping, vAccel2);
							vVelocity3	= spu_madd(vVelocity3, vDamping, vAccel3);
							vVelocity4	= spu_madd(vVelocity4, vDamping, vAccel4);
							vVelocity5	= spu_madd(vVelocity5, vDamping, vAccel5);
							vVelocity6	= spu_madd(vVelocity6, vDamping, vAccel6);
							vVelocity7	= spu_madd(vVelocity7, vDamping, vAccel7);

			pOutput[x+0] = spu_add(vCurrent1, vVelocity0);
			pOutput[x+1] = spu_add(vCurrent2, vVelocity1);
			pOutput[x+2] = spu_add(vCurrent3, vVelocity2);
			pOutput[x+3] = spu_add(vCurrent4, vVelocity3);
			pOutput[x+4] = spu_add(vCurrent5, vVelocity4);
			pOutput[x+5] = spu_add(vCurrent6, vVelocity5);
			pOutput[x+6] = spu_add(vCurrent7, vVelocity6);
			pOutput[x+7] = spu_add(vCurrent8, vVelocity7);
		}

		Spu::Wait(1);
		Spu::StoreAsync(pData->m_pNext + pData->m_iPitch * y, pOutput, iRowSize, 1);
		
		Ut::Swap(pOutput, pOutputSave);
		Ut::Swap(pPrevious, pPreviousLoad);
		Ut::Rotate(pCurrent0, pCurrent1, pCurrent2, pCurrent3);
	}
	Spu::Wait(1);
}

#else

void HeightfieldSolve(Scheduler::Task* pTask)
{
	HeightfieldSolveData* pData = (HeightfieldSolveData*)Spu::FetchAlloc(pTask->Output, sizeof(HeightfieldSolveData));

	const U32 iRowSize = sizeof(F32) * pData->m_iWidth;

	const bool isTop	= (pData->m_pTop != 0);
	const bool isBottom	= (pData->m_pBottom != 0);
	const bool isLeft	= (pData->m_pLeft != 0);
	const bool isRight	= (pData->m_pRight != 0);

	for (U32 y = 0; y < pData->m_iHeight; ++y)
	{
		const S32 yPrevious		= (y > 0) ? y - 1 : 0;
		const S32 yNext			= (y < pData->m_iHeight - 1) ? y + 1 : pData->m_iHeight - 1;

		volatile F32* pNext		= (F32*)(pData->m_pNext + pData->m_iPitch * y);
		const F32* pPrevious	= (F32*)(pData->m_pPrevious + pData->m_iPitch * y);
		const F32* pUp			= (F32*)(pData->m_pCurrent + pData->m_iPitch * yPrevious);
		const F32* pDown		= (F32*)(pData->m_pCurrent + pData->m_iPitch * yNext);
		const F32* pCentre		= (F32*)(pData->m_pCurrent + pData->m_iPitch * y);
		const F32* pLeft		= (F32*)(pData->m_pLeft + pData->m_iPitch * y);
		const F32* pRight		= (F32*)(pData->m_pRight + pData->m_iPitch * y);

		if (isTop && y == 0)
			pUp = (F32*)(pData->m_pTop + pData->m_iPitch * (pData->m_iHeight - 1));
		if (isBottom && y == pData->m_iHeight - 1)
			pDown = (F32*)(pData->m_pBottom);

		{
			const F32 fTop		= pUp[0];
			const F32 fBottom	= pDown[0];
			const F32 fLeft		= isLeft ? pLeft[pData->m_iWidth - 1] : pCentre[0];
			const F32 fRight	= pCentre[1];
			const F32 fCurrent	= pCentre[0];
			const F32 fPrevious	= *pPrevious++;

			const F32 fOffset	= 0.25f * fTop + 0.25f * fBottom + 0.25f * fLeft + 0.25f * fRight - fCurrent;
			const F32 fAccel	= pData->m_fScale * fOffset;
			const F32 fVelocity	= (fCurrent - fPrevious) * pData->m_fDamping;
			const F32 fHeight	= fCurrent + fVelocity + fAccel;

			*pNext++ = fHeight;
		}

		for (U32 x = 1; x < pData->m_iWidth - 1; ++x)
		{
			const F32 fTop		= pUp[x];
			const F32 fBottom	= pDown[x];
			const F32 fLeft		= pCentre[x - 1];
			const F32 fRight	= pCentre[x + 1];
			const F32 fCurrent	= pCentre[x];
			const F32 fPrevious	= *pPrevious++;

			const F32 fOffset	= 0.25f * fTop + 0.25f * fBottom + 0.25f * fLeft + 0.25f * fRight - fCurrent;
			const F32 fAccel	= pData->m_fScale * fOffset;
			const F32 fVelocity	= (fCurrent - fPrevious) * pData->m_fDamping;
			const F32 fHeight	= fCurrent + fVelocity + fAccel;

			*pNext++ = fHeight;
		}

		{
			const F32 fTop		= pUp[pData->m_iWidth - 1];
			const F32 fBottom	= pDown[pData->m_iWidth - 1];
			const F32 fLeft		= pCentre[pData->m_iWidth - 2];
			const F32 fRight	= isRight ? pRight[0] : pCentre[pData->m_iWidth - 1];
			const F32 fCurrent	= pCentre[pData->m_iWidth - 1];
			const F32 fPrevious	= *pPrevious++;

			const F32 fOffset	= 0.25f * fTop + 0.25f * fBottom + 0.25f * fLeft + 0.25f * fRight - fCurrent;
			const F32 fAccel	= pData->m_fScale * fOffset;
			const F32 fVelocity	= (fCurrent - fPrevious) * pData->m_fDamping;
			const F32 fHeight	= fCurrent + fVelocity + fAccel;

			*pNext++ = fHeight;
		}
	}
}

#endif

}; // namespace Effect

JQ_DECLARE_SPU_MODULE(HeightfieldSolveModule, Effect::HeightfieldSolve, HeightfieldSolve, "effect.fluids");
