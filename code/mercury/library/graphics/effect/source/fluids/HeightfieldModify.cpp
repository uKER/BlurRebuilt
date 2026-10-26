#include "stdafx.h"
#include "Effect/Fluids/HeightfieldTasks.h"
#include "System/Scheduler/JobQueue.h"

namespace Effect
{

#if PLATFORM_SPU

void HeightfieldModify(Scheduler::Task* pTask)
{
	Spu::MemBaseMarker marker;

	HeightfieldModifyData* pData = (HeightfieldModifyData*)Spu::FetchAlloc(pTask->Output, sizeof(HeightfieldModifyData));

	F32*			pTexture		= (F32*) Spu::FetchAllocAsync(pData->m_pModify, pData->m_iModifyPitch * pData->m_iModifyHeight, 0);
	vector float*	pOutput			= (vector float*) Spu::Alloc(sizeof(F32) * pData->m_iSurfaceWidth);
	vector float*	pOutputSave		= (vector float*) Spu::Alloc(sizeof(F32) * pData->m_iSurfaceWidth);
	vector float*	pCurrent		= (vector float*) Spu::Alloc(sizeof(F32) * pData->m_iSurfaceWidth);
	vector float*	pCurrentLoad	= (vector float*) Spu::Alloc(sizeof(F32) * pData->m_iSurfaceWidth);

	Spu::FetchAsync(pCurrent, pData->m_pCurrent, pData->m_iSurfaceWidth * sizeof(F32), 0);

	vector signed int	vZero			= (vector signed int) { 0, 0, 0, 0 };
	vector float		vZeroF			= (vector float) { 0.0f, 0.0f, 0.0f, 0.0f };
	vector signed int	vMinusOne		= (vector signed int) { -1, -1, -1, -1 };
	vector signed int	vSurfaceSize	= (vector signed int) { pData->m_iSurfaceWidth, pData->m_iSurfaceHeight, 1, 1 };
	vector signed int	vModifySize		= (vector signed int) { pData->m_iModifyWidth, pData->m_iModifyHeight, 0, 0 };
	
	vector float		vRcpSurfaceSize	= spu_re(spu_convtf(vSurfaceSize, 0));
	
	vector float		vModifyScale	= spu_convtf(vModifySize, 0);

	vector float		vTransformU		= (vector float) { pData->m_vTransformU.x, pData->m_vTransformU.y, pData->m_vTransformU.z, pData->m_vTransformU.w };
	vector float		vTransformV		= (vector float) { pData->m_vTransformV.x, pData->m_vTransformV.y, pData->m_vTransformV.z, pData->m_vTransformV.w };
	vector signed int	vIndexScale		= (vector signed int) { 1, 0, pData->m_iModifyPitch / sizeof(F32), 0 };
	vector signed int	vIndexMax		= (vector signed int) { pData->m_iModifyPitch * pData->m_iModifyHeight, pData->m_iModifyPitch * pData->m_iModifyHeight, pData->m_iModifyPitch * pData->m_iModifyHeight, pData->m_iModifyPitch * pData->m_iModifyHeight };
	vector float		vScale			= spu_splats(pData->m_fScale);

	vector signed int	vModifyWidth		= (vector signed int) { pData->m_iModifyWidth, pData->m_iModifyWidth, pData->m_iModifyWidth, pData->m_iModifyWidth };
	vector signed int	vModifyHeight		= (vector signed int) { pData->m_iModifyHeight, pData->m_iModifyHeight, pData->m_iModifyHeight, pData->m_iModifyHeight };
	vector signed int	vModifyWidthIn		= (vector signed int) { pData->m_iModifyWidth - 1, pData->m_iModifyWidth - 1, pData->m_iModifyWidth - 1, pData->m_iModifyWidth - 1 };
	vector signed int	vModifyHeightIn		= (vector signed int) { pData->m_iModifyHeight - 1, pData->m_iModifyHeight - 1, pData->m_iModifyHeight - 1, pData->m_iModifyHeight - 1 };
	vector float		vModifyScaleX		= spu_convtf(vModifyWidth, 0);
	vector float		vModifyScaleY		= spu_convtf(vModifyHeight, 0);
	vector float		vRcpSurfaceWidth	= spu_splats(spu_extract(vRcpSurfaceSize, 0));
	vector float		vRcpSurfaceHeight	= spu_splats(spu_extract(vRcpSurfaceSize, 1));
	vector float		vTransformU_X		= spu_splats(spu_extract(vTransformU, 0));
	vector float		vTransformU_Y		= spu_splats(spu_extract(vTransformU, 1));
	vector float		vTransformU_Z		= spu_splats(spu_extract(vTransformU, 2));
	vector float		vTransformV_X		= spu_splats(spu_extract(vTransformV, 0));
	vector float		vTransformV_Y		= spu_splats(spu_extract(vTransformV, 1));
	vector float		vTransformV_Z		= spu_splats(spu_extract(vTransformV, 2));

	for (U32 y = 0; y < pData->m_iSurfaceHeight; ++y)
	{
		Spu::Wait(0);

		const U32 yNext = Maths::Min(y + 1, pData->m_iSurfaceHeight - 1);
		Spu::FetchAsync(pCurrentLoad, pData->m_pCurrent + yNext * pData->m_iSurfacePitch, pData->m_iSurfaceWidth * sizeof(F32), 0);

		for (U32 x = 0; x < pData->m_iSurfaceWidth * sizeof(F32) / sizeof(vector float); x += 8)
		{
			vector unsigned int vYi			= (vector unsigned int) { y, y, y, y };
			vector unsigned int vX0i		= (vector unsigned int) { x * 4 + 0, x * 4 + 1, x * 4 + 2, x * 4 + 3 };
			vector unsigned int vX1i		= (vector unsigned int) { x * 4 + 4, x * 4 + 5, x * 4 + 6, x * 4 + 7 };
			vector unsigned int vX2i		= (vector unsigned int) { x * 4 + 8, x * 4 + 9, x * 4 + 10, x * 4 + 11 };
			vector unsigned int vX3i		= (vector unsigned int) { x * 4 + 12, x * 4 + 13, x * 4 + 14, x * 4 + 15 };
			vector unsigned int vX4i		= (vector unsigned int) { x * 4 + 16, x * 4 + 17, x * 4 + 18, x * 4 + 19 };
			vector unsigned int vX5i		= (vector unsigned int) { x * 4 + 20, x * 4 + 21, x * 4 + 22, x * 4 + 23 };
			vector unsigned int vX6i		= (vector unsigned int) { x * 4 + 24, x * 4 + 25, x * 4 + 26, x * 4 + 27 };
			vector unsigned int vX7i		= (vector unsigned int) { x * 4 + 28, x * 4 + 29, x * 4 + 30, x * 4 + 31 };

			vector float		vY			= spu_convtf(vYi, 0);
			vector float		vX0			= spu_convtf(vX0i, 0);
			vector float		vX1			= spu_convtf(vX1i, 0);
			vector float		vX2			= spu_convtf(vX2i, 0);
			vector float		vX3			= spu_convtf(vX3i, 0);
			vector float		vX4			= spu_convtf(vX4i, 0);
			vector float		vX5			= spu_convtf(vX5i, 0);
			vector float		vX6			= spu_convtf(vX6i, 0);
			vector float		vX7			= spu_convtf(vX7i, 0);

								vY			= spu_mul(vY, vRcpSurfaceHeight);
								vX0			= spu_mul(vX0, vRcpSurfaceWidth);
								vX1			= spu_mul(vX1, vRcpSurfaceWidth);
								vX2			= spu_mul(vX2, vRcpSurfaceWidth);
								vX3			= spu_mul(vX3, vRcpSurfaceWidth);
								vX4			= spu_mul(vX4, vRcpSurfaceWidth);
								vX5			= spu_mul(vX5, vRcpSurfaceWidth);
								vX6			= spu_mul(vX6, vRcpSurfaceWidth);
								vX7			= spu_mul(vX7, vRcpSurfaceWidth);

			vector float		vY_YZ		= spu_madd(vY, vTransformV_Y, vTransformV_Z);
			vector float		vX_YZ		= spu_madd(vY, vTransformU_Y, vTransformU_Z);

			vector float		vY0			= spu_madd(vX0, vTransformV_X, vY_YZ);			
			vector float		vY1			= spu_madd(vX1, vTransformV_X, vY_YZ);			
			vector float		vY2			= spu_madd(vX2, vTransformV_X, vY_YZ);			
			vector float		vY3			= spu_madd(vX3, vTransformV_X, vY_YZ);			
			vector float		vY4			= spu_madd(vX4, vTransformV_X, vY_YZ);			
			vector float		vY5			= spu_madd(vX5, vTransformV_X, vY_YZ);			
			vector float		vY6			= spu_madd(vX6, vTransformV_X, vY_YZ);			
			vector float		vY7			= spu_madd(vX7, vTransformV_X, vY_YZ);			

								vX0			= spu_madd(vX0, vTransformU_X, vX_YZ);			
								vX1			= spu_madd(vX1, vTransformU_X, vX_YZ);			
								vX2			= spu_madd(vX2, vTransformU_X, vX_YZ);			
								vX3			= spu_madd(vX3, vTransformU_X, vX_YZ);			
								vX4			= spu_madd(vX4, vTransformU_X, vX_YZ);			
								vX5			= spu_madd(vX5, vTransformU_X, vX_YZ);			
								vX6			= spu_madd(vX6, vTransformU_X, vX_YZ);			
								vX7			= spu_madd(vX7, vTransformU_X, vX_YZ);			

								vX0			= spu_mul(vX0, vModifyScaleX);
								vX1			= spu_mul(vX1, vModifyScaleX);
								vX2			= spu_mul(vX2, vModifyScaleX);
								vX3			= spu_mul(vX3, vModifyScaleX);
								vX4			= spu_mul(vX4, vModifyScaleX);
								vX5			= spu_mul(vX5, vModifyScaleX);
								vX6			= spu_mul(vX6, vModifyScaleX);
								vX7			= spu_mul(vX7, vModifyScaleX);
								vY0			= spu_mul(vY0, vModifyScaleY);
								vY1			= spu_mul(vY1, vModifyScaleY);
								vY2			= spu_mul(vY2, vModifyScaleY);
								vY3			= spu_mul(vY3, vModifyScaleY);
								vY4			= spu_mul(vY4, vModifyScaleY);
								vY5			= spu_mul(vY5, vModifyScaleY);
								vY6			= spu_mul(vY6, vModifyScaleY);
								vY7			= spu_mul(vY7, vModifyScaleY);

			vector signed int	vU0			= spu_convts(vX0, 0);
			vector signed int	vV0			= spu_convts(vY0, 0);
			vector signed int	vU1			= spu_convts(vX1, 0);
			vector signed int	vV1			= spu_convts(vY1, 0);
			vector signed int	vU2			= spu_convts(vX2, 0);
			vector signed int	vV2			= spu_convts(vY2, 0);
			vector signed int	vU3			= spu_convts(vX3, 0);
			vector signed int	vV3			= spu_convts(vY3, 0);
			vector signed int	vU4			= spu_convts(vX4, 0);
			vector signed int	vV4			= spu_convts(vY4, 0);
			vector signed int	vU5			= spu_convts(vX5, 0);
			vector signed int	vV5			= spu_convts(vY5, 0);
			vector signed int	vU6			= spu_convts(vX6, 0);
			vector signed int	vV6			= spu_convts(vY6, 0);
			vector signed int	vU7			= spu_convts(vX7, 0);
			vector signed int	vV7			= spu_convts(vY7, 0);

			vector unsigned int	vGTX0		= spu_cmpgt(vU0, vMinusOne);
			vector unsigned int	vGTY0		= spu_cmpgt(vV0, vMinusOne);
			vector unsigned int	vLTX0		= spu_cmpgt(vModifyWidth, vU0);
			vector unsigned int	vLTY0		= spu_cmpgt(vModifyHeight, vV0);
			vector unsigned int	vGTX1		= spu_cmpgt(vU1, vMinusOne);
			vector unsigned int	vGTY1		= spu_cmpgt(vV1, vMinusOne);
			vector unsigned int	vLTX1		= spu_cmpgt(vModifyWidth, vU1);
			vector unsigned int	vLTY1		= spu_cmpgt(vModifyHeight, vV1);
			vector unsigned int	vGTX2		= spu_cmpgt(vU2, vMinusOne);
			vector unsigned int	vGTY2		= spu_cmpgt(vV2, vMinusOne);
			vector unsigned int	vLTX2		= spu_cmpgt(vModifyWidth, vU2);
			vector unsigned int	vLTY2		= spu_cmpgt(vModifyHeight, vV2);
			vector unsigned int	vGTX3		= spu_cmpgt(vU3, vMinusOne);
			vector unsigned int	vGTY3		= spu_cmpgt(vV3, vMinusOne);
			vector unsigned int	vLTX3		= spu_cmpgt(vModifyWidth, vU3);
			vector unsigned int	vLTY3		= spu_cmpgt(vModifyHeight, vV3);
			vector unsigned int	vGTX4		= spu_cmpgt(vU4, vMinusOne);
			vector unsigned int	vGTY4		= spu_cmpgt(vV4, vMinusOne);
			vector unsigned int	vLTX4		= spu_cmpgt(vModifyWidth, vU4);
			vector unsigned int	vLTY4		= spu_cmpgt(vModifyHeight, vV4);
			vector unsigned int	vGTX5		= spu_cmpgt(vU5, vMinusOne);
			vector unsigned int	vGTY5		= spu_cmpgt(vV5, vMinusOne);
			vector unsigned int	vLTX5		= spu_cmpgt(vModifyWidth, vU5);
			vector unsigned int	vLTY5		= spu_cmpgt(vModifyHeight, vV5);
			vector unsigned int	vGTX6		= spu_cmpgt(vU6, vMinusOne);
			vector unsigned int	vGTY6		= spu_cmpgt(vV6, vMinusOne);
			vector unsigned int	vLTX6		= spu_cmpgt(vModifyWidth, vU6);
			vector unsigned int	vLTY6		= spu_cmpgt(vModifyHeight, vV6);
			vector unsigned int	vGTX7		= spu_cmpgt(vU7, vMinusOne);
			vector unsigned int	vGTY7		= spu_cmpgt(vV7, vMinusOne);
			vector unsigned int	vLTX7		= spu_cmpgt(vModifyWidth, vU7);
			vector unsigned int	vLTY7		= spu_cmpgt(vModifyHeight, vV7);

								vU0			= spu_sel(vZero, vU0, vGTX0);
								vV0			= spu_sel(vZero, vV0, vGTY0);
								vU1			= spu_sel(vZero, vU1, vGTX1);
								vV1			= spu_sel(vZero, vV1, vGTY1);
								vU2			= spu_sel(vZero, vU2, vGTX2);
								vV2			= spu_sel(vZero, vV2, vGTY2);
								vU3			= spu_sel(vZero, vU3, vGTX3);
								vV3			= spu_sel(vZero, vV3, vGTY3);
								vU4			= spu_sel(vZero, vU4, vGTX4);
								vV4			= spu_sel(vZero, vV4, vGTY4);
								vU5			= spu_sel(vZero, vU5, vGTX5);
								vV5			= spu_sel(vZero, vV5, vGTY5);
								vU6			= spu_sel(vZero, vU6, vGTX6);
								vV6			= spu_sel(vZero, vV6, vGTY6);
								vU7			= spu_sel(vZero, vU7, vGTX7);
								vV7			= spu_sel(vZero, vV7, vGTY7);

								vU0			= spu_sel(vModifyWidthIn, vU0, vLTX0);
								vV0			= spu_sel(vModifyHeightIn, vV0, vLTY0);
								vU1			= spu_sel(vModifyWidthIn, vU1, vLTX1);
								vV1			= spu_sel(vModifyHeightIn, vV1, vLTY1);
								vU2			= spu_sel(vModifyWidthIn, vU2, vLTX2);
								vV2			= spu_sel(vModifyHeightIn, vV2, vLTY2);
								vU3			= spu_sel(vModifyWidthIn, vU3, vLTX3);
								vV3			= spu_sel(vModifyHeightIn, vV3, vLTY3);
								vU4			= spu_sel(vModifyWidthIn, vU4, vLTX4);
								vV4			= spu_sel(vModifyHeightIn, vV4, vLTY4);
								vU5			= spu_sel(vModifyWidthIn, vU5, vLTX5);
								vV5			= spu_sel(vModifyHeightIn, vV5, vLTY5);
								vU6			= spu_sel(vModifyWidthIn, vU6, vLTX6);
								vV6			= spu_sel(vModifyHeightIn, vV6, vLTY6);
								vU7			= spu_sel(vModifyWidthIn, vU7, vLTX7);
								vV7			= spu_sel(vModifyHeightIn, vV7, vLTY7);

			S32	i0_0 = spu_extract(vU0, 0) + spu_extract(vV0, 0) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_1 = spu_extract(vU0, 1) + spu_extract(vV0, 1) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_2 = spu_extract(vU0, 2) + spu_extract(vV0, 2) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_3 = spu_extract(vU0, 3) + spu_extract(vV0, 3) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_4 = spu_extract(vU1, 0) + spu_extract(vV1, 0) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_5 = spu_extract(vU1, 1) + spu_extract(vV1, 1) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_6 = spu_extract(vU1, 2) + spu_extract(vV1, 2) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_7 = spu_extract(vU1, 3) + spu_extract(vV1, 3) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_8 = spu_extract(vU2, 0) + spu_extract(vV2, 0) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_9 = spu_extract(vU2, 1) + spu_extract(vV2, 1) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_10 = spu_extract(vU2, 2) + spu_extract(vV2, 2) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_11 = spu_extract(vU2, 3) + spu_extract(vV2, 3) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_12 = spu_extract(vU3, 0) + spu_extract(vV3, 0) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_13 = spu_extract(vU3, 1) + spu_extract(vV3, 1) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_14 = spu_extract(vU3, 2) + spu_extract(vV3, 2) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_15 = spu_extract(vU3, 3) + spu_extract(vV3, 3) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_16 = spu_extract(vU4, 0) + spu_extract(vV4, 0) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_17 = spu_extract(vU4, 1) + spu_extract(vV4, 1) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_18 = spu_extract(vU4, 2) + spu_extract(vV4, 2) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_19 = spu_extract(vU4, 3) + spu_extract(vV4, 3) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_20 = spu_extract(vU5, 0) + spu_extract(vV5, 0) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_21 = spu_extract(vU5, 1) + spu_extract(vV5, 1) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_22 = spu_extract(vU5, 2) + spu_extract(vV5, 2) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_23 = spu_extract(vU5, 3) + spu_extract(vV5, 3) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_24 = spu_extract(vU6, 0) + spu_extract(vV6, 0) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_25 = spu_extract(vU6, 1) + spu_extract(vV6, 1) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_26 = spu_extract(vU6, 2) + spu_extract(vV6, 2) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_27 = spu_extract(vU6, 3) + spu_extract(vV6, 3) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_28 = spu_extract(vU7, 0) + spu_extract(vV7, 0) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_29 = spu_extract(vU7, 1) + spu_extract(vV7, 1) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_30 = spu_extract(vU7, 2) + spu_extract(vV7, 2) * pData->m_iModifyPitch / sizeof(F32);
			S32	i0_31 = spu_extract(vU7, 3) + spu_extract(vV7, 3) * pData->m_iModifyPitch / sizeof(F32);

			vector unsigned int	vMaskX0		= spu_and(vLTX0, vGTX0);
			vector unsigned int	vMaskY0		= spu_and(vLTY0, vGTY0);
			vector unsigned int	vMaskX1		= spu_and(vLTX1, vGTX1);
			vector unsigned int	vMaskY1		= spu_and(vLTY1, vGTY1);
			vector unsigned int	vMaskX2		= spu_and(vLTX2, vGTX2);
			vector unsigned int	vMaskY2		= spu_and(vLTY2, vGTY2);
			vector unsigned int	vMaskX3		= spu_and(vLTX3, vGTX3);
			vector unsigned int	vMaskY3		= spu_and(vLTY3, vGTY3);
			vector unsigned int	vMaskX4		= spu_and(vLTX4, vGTX4);
			vector unsigned int	vMaskY4		= spu_and(vLTY4, vGTY4);
			vector unsigned int	vMaskX5		= spu_and(vLTX5, vGTX5);
			vector unsigned int	vMaskY5		= spu_and(vLTY5, vGTY5);
			vector unsigned int	vMaskX6		= spu_and(vLTX6, vGTX6);
			vector unsigned int	vMaskY6		= spu_and(vLTY6, vGTY6);
			vector unsigned int	vMaskX7		= spu_and(vLTX7, vGTX7);
			vector unsigned int	vMaskY7		= spu_and(vLTY7, vGTY7);

			vector unsigned int	vMask0		= spu_and(vMaskX0, vMaskY0);
			vector unsigned int	vMask1		= spu_and(vMaskX1, vMaskY1);
			vector unsigned int	vMask2		= spu_and(vMaskX2, vMaskY2);
			vector unsigned int	vMask3		= spu_and(vMaskX3, vMaskY3);
			vector unsigned int	vMask4		= spu_and(vMaskX4, vMaskY4);
			vector unsigned int	vMask5		= spu_and(vMaskX5, vMaskY5);
			vector unsigned int	vMask6		= spu_and(vMaskX6, vMaskY6);
			vector unsigned int	vMask7		= spu_and(vMaskX7, vMaskY7);

			vector float	vSamples0		= (vector float) { pTexture[i0_0], pTexture[i0_1], pTexture[i0_2], pTexture[i0_3] };
			vector float	vSamples1		= (vector float) { pTexture[i0_4], pTexture[i0_5], pTexture[i0_6], pTexture[i0_7] };
			vector float	vSamples2		= (vector float) { pTexture[i0_8], pTexture[i0_9], pTexture[i0_10], pTexture[i0_11] };
			vector float	vSamples3		= (vector float) { pTexture[i0_12], pTexture[i0_13], pTexture[i0_14], pTexture[i0_15] };
			vector float	vSamples4		= (vector float) { pTexture[i0_16], pTexture[i0_17], pTexture[i0_18], pTexture[i0_19] };
			vector float	vSamples5		= (vector float) { pTexture[i0_20], pTexture[i0_21], pTexture[i0_22], pTexture[i0_23] };
			vector float	vSamples6		= (vector float) { pTexture[i0_24], pTexture[i0_25], pTexture[i0_26], pTexture[i0_27] };
			vector float	vSamples7		= (vector float) { pTexture[i0_28], pTexture[i0_29], pTexture[i0_30], pTexture[i0_31] };
							vSamples0		= spu_sel(vZeroF, vSamples0, vMask0);
							vSamples1		= spu_sel(vZeroF, vSamples1, vMask1);
							vSamples2		= spu_sel(vZeroF, vSamples2, vMask2);
							vSamples3		= spu_sel(vZeroF, vSamples3, vMask3);
							vSamples4		= spu_sel(vZeroF, vSamples4, vMask4);
							vSamples5		= spu_sel(vZeroF, vSamples5, vMask5);
							vSamples6		= spu_sel(vZeroF, vSamples6, vMask6);
							vSamples7		= spu_sel(vZeroF, vSamples7, vMask7);
			
			pOutput[x+0] = spu_madd(vSamples0, vScale, pCurrent[x+0]);
			pOutput[x+1] = spu_madd(vSamples1, vScale, pCurrent[x+1]);
			pOutput[x+2] = spu_madd(vSamples2, vScale, pCurrent[x+2]);
			pOutput[x+3] = spu_madd(vSamples3, vScale, pCurrent[x+3]);
			pOutput[x+4] = spu_madd(vSamples4, vScale, pCurrent[x+4]);
			pOutput[x+5] = spu_madd(vSamples5, vScale, pCurrent[x+5]);
			pOutput[x+6] = spu_madd(vSamples6, vScale, pCurrent[x+6]);
			pOutput[x+7] = spu_madd(vSamples7, vScale, pCurrent[x+7]);
		}

		Spu::Wait(1);
		Spu::StoreAsync(pData->m_pNext + y * pData->m_iSurfacePitch, pOutput, pData->m_iSurfaceWidth * sizeof(F32), 1);
		
		Ut::Swap(pOutput, pOutputSave);
		Ut::Swap(pCurrent, pCurrentLoad);
	}

	Spu::Wait(0);
}

#else

void HeightfieldModify(Scheduler::Task* pTask)
{
	HeightfieldModifyData* pData = (HeightfieldModifyData*)Spu::FetchAlloc(pTask->Output, sizeof(HeightfieldModifyData));

	for (U32 y = 0; y < pData->m_iSurfaceHeight; ++y)
	{
		const F32* pInput		= (F32*)(pData->m_pCurrent + pData->m_iSurfacePitch * y);
		volatile F32* pOutput	= (F32*)(pData->m_pNext + pData->m_iSurfacePitch * y);
		for (U32 x = 0; x < pData->m_iSurfaceWidth; ++x)
		{
			const F32 fX = x / (F32) pData->m_iSurfaceWidth;
			const F32 fY = y / (F32) pData->m_iSurfaceHeight;
			const F32 fU = fX * pData->m_vTransformU.x + fY * pData->m_vTransformU.y + pData->m_vTransformU.z;
			const F32 fV = fX * pData->m_vTransformV.x + fY * pData->m_vTransformV.y + pData->m_vTransformV.z;
			const S32 iU = (S32)(fU * pData->m_iModifyWidth);			
			const S32 iV = (S32)(fV * pData->m_iModifyHeight);
			
			const U32 iSelectU = (iU >= 0) && (iU < (S32)pData->m_iModifyWidth);
			const U32 iSelectV = (iV >= 0) && (iV < (S32)pData->m_iModifyHeight);
			const U32 iSelect = iSelectU * iSelectV;

			const U32 iClampedU = iU * iSelectU;
			const U32 iClampedV = iV * iSelectV;

			const F32 fSample = pData->m_pModify[iClampedV * pData->m_iModifyPitch + iClampedU];

			*pOutput++ = *pInput++ + fSample * pData->m_fScale * iSelect;
		}
	}
}

#endif

}; // namespace Effect

JQ_DECLARE_SPU_MODULE(HeightfieldModifyModule, Effect::HeightfieldModify, HeightfieldModify, "effect.fluids");
