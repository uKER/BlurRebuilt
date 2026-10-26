#include "stdafx.h"
#include "Effect/Sky/AtmosphericSolver.h"
#include "Effect/Sky/AtmosphericParameters.h"

#include "Core/Threads/Atomic.h"
#include "Misc_Shared/Types/Float16.h"
#include <System/Scheduler.h>

namespace Effect
{
namespace Sky
{

Maths::HVec3 SolveOpticalDepth(const Maths::HVec3& vBegin, const Maths::HVec3& vEnd, const U32 nSamples, const C_AtmosphericParameters& params)
{
	const Maths::HVec4	vRayleighCoeffs	= Maths::Load<Maths::HVec4>(&params.m_vRayleighCoeffs);
	const Maths::HVec4	vMieCoeffs		= Maths::Load<Maths::HVec4>(&params.m_vMieCoeffs);
	const Maths::HVec3	vRadiiScale		= Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&params.m_fInnerRadius);

	const Maths::HVec3	vRay			= vEnd - vBegin;
	const Maths::HVec3	vSampleRay		= vRay * 1.0f / (F32) nSamples;
	const Maths::HF32	vSampleLength	= Maths::Length(vSampleRay);
	const Maths::HF32	vScaledLength	= vSampleLength * vRadiiScale.Z();
	Maths::HVec3		vSamplePoint	= vBegin + vSampleRay * Maths::Constants<Maths::HF32>::Half();

	Maths::HF32	vRayleigh	= Maths::Constants<Maths::HF32>::Zero();
	Maths::HF32	vMie		= Maths::Constants<Maths::HF32>::Zero();
	for (U32 iSample = 0; iSample < nSamples; ++iSample)
	{
		const Maths::HF32	vHeight			= Maths::Length(vSamplePoint);
		const Maths::HF32	vScaledHeight	= vRadiiScale.Z() * (vRadiiScale.X() - vHeight);

		vRayleigh		= vRayleigh + expf(vRayleighCoeffs.W() * vScaledHeight);
		vMie			= vMie + expf(vMieCoeffs.W() * vScaledHeight);
		vSamplePoint	= vSamplePoint + vSampleRay;
	}
	vRayleigh = vRayleigh * vScaledLength;
	vMie = vMie * vScaledLength;

	return Maths::HVec3(vRayleighCoeffs) * vRayleigh + Maths::HVec3(vMieCoeffs) * vMie;
}

void SolveInScattering(const Maths::HVec3& vEye, const Maths::HVec3& vPosition, const U32 nSamples, const C_AtmosphericParameters& params, Maths::HVec3& vRayleigh, Maths::HVec3& vMie)
{
	const Maths::HVec4	vSunDirection	= Maths::Load<Maths::HVec4>(&params.m_vSunDirection);
	const Maths::HVec4	vRayleighCoeffs	= Maths::Load<Maths::HVec4>(&params.m_vRayleighCoeffs);
	const Maths::HVec4	vMieCoeffs		= Maths::Load<Maths::HVec4>(&params.m_vMieCoeffs);
	const Maths::HVec3	vRadiiScale		= Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&params.m_fInnerRadius);

	const Maths::HVec3	vRay			= vPosition - vEye;
	const Maths::HVec3	vSampleRay		= vRay * 1.0f / (F32) nSamples;
	const Maths::HF32	vSampleLength	= Maths::Length(vSampleRay);
	const Maths::HF32	vScaledLength	= vSampleLength * vRadiiScale.Z();
	Maths::HVec3		vSamplePoint	= vEye + vSampleRay * Maths::Constants<Maths::HF32>::Half();

	vRayleigh	= Maths::HVec3(Maths::Constants<Maths::HF32>::Zero());
	vMie		= Maths::HVec3(Maths::Constants<Maths::HF32>::Zero());
	for (U32 iSample = 0; iSample < nSamples; ++iSample)
	{
		const Maths::HF32	vPDotD				= vSamplePoint | Maths::HVec3(vSunDirection);
		const Maths::HF32	vPDotP				= vSamplePoint | vSamplePoint;
		const Maths::HF32	vLambda				= -vPDotD + Maths::Sqrt(vPDotD * vPDotD - vPDotP + vRadiiScale.Y() * vRadiiScale.Y());
		const Maths::HVec3	vSunPosition		= vSamplePoint + vLambda * Maths::HVec3(vSunDirection);
		const Maths::HVec3	vSunOpticalDepth	= SolveOpticalDepth(vSamplePoint, vSunPosition, nSamples, params);
		const Maths::HVec3	vCameraOpticalDepth	= SolveOpticalDepth(vSamplePoint, vEye, nSamples, params);
		const Maths::HVec3	vAttenuation		= Maths::HVec3(expf(-vSunOpticalDepth.X() - vCameraOpticalDepth.X()), expf(-vSunOpticalDepth.Y() - vCameraOpticalDepth.Y()), expf(-vSunOpticalDepth.Z() - vCameraOpticalDepth.Z()));
		const Maths::HF32	vScaledHeight		= vRadiiScale.Z() * (vRadiiScale.X() - Maths::Length(vSamplePoint));
		vRayleigh		= vRayleigh + expf(vRayleighCoeffs.W() * vScaledHeight) * vAttenuation;
		vMie			= vMie + expf(vMieCoeffs.W() * vScaledHeight) * vAttenuation;
		vSamplePoint	= vSamplePoint + vSampleRay;
	}

	vRayleigh	= vRayleigh * vScaledLength;
	vMie		= vMie * vScaledLength;
}

void SolveAtmosphere(const C_AtmosphericTask* const pTask)
{
	PERF_EVENT("AtmosphericSolver");

	const U32 nLineBegin	= MT::AtomicAdd(pTask->m_pLinePosition, pTask->m_LinesToUpdate);
	const U32 nLineEnd		= nLineBegin + pTask->m_LinesToUpdate;

	U16* pRayleighBuffer	= (U16*)(pTask->m_pRayleighBuffer + nLineBegin * pTask->m_iRayleighPitch);
	U16* pMieBuffer			= (U16*)(pTask->m_pMieBuffer + nLineBegin * pTask->m_iMiePitch);

	for (U32 y = nLineBegin; y < nLineEnd; ++y)
	{
		if (y >= pTask->m_iBufferHeight)
			break;

		Maths::HVec3 vRayleighSum(Maths::Constants<Maths::HF32>::Zero());
		Maths::HVec3 vMieSum(Maths::Constants<Maths::HF32>::Zero());

		U16* pRayleigh	= pRayleighBuffer;
		U16* pMie		= pMieBuffer;
		for (U32 x = 0; x < pTask->m_iBufferWidth; ++x)
		{
			const F32 fU = Maths::Pi * 2.000f * x / (F32) pTask->m_iBufferWidth;
			const F32 fV = pTask->m_Parameters.m_fFieldOfView * y / (F32) pTask->m_iBufferHeight;
			const Maths::HVec3	vPosition(sinf(fV) * cosf(fU) * pTask->m_Parameters.m_fOuterRadius, cosf(fV) * pTask->m_Parameters.m_fOuterRadius, sinf(fV) * sinf(fU) * pTask->m_Parameters.m_fOuterRadius);
			const Maths::HVec3	vEye(0.0f, pTask->m_Parameters.m_fInnerRadius, 0.0f);
			
			Maths::HVec3 vRayleigh, vMie;
			SolveInScattering(vEye, vPosition, pTask->m_iSamples, pTask->m_Parameters, vRayleigh, vMie);

			vRayleighSum += vRayleigh;
			vMieSum += vMie;

			*pRayleigh++ = Half::F16(vRayleigh.X()).m_U16;
			*pRayleigh++ = Half::F16(vRayleigh.Y()).m_U16;
			*pRayleigh++ = Half::F16(vRayleigh.Z()).m_U16;
			*pRayleigh++ = Half::F16(1.0f).m_U16;
			*pMie++ = Half::F16(vMie.X()).m_U16;
			*pMie++ = Half::F16(vMie.Y()).m_U16;
			*pMie++ = Half::F16(vMie.Z()).m_U16;
			*pMie++ = Half::F16(1.0f).m_U16;
		}
		pRayleighBuffer	+= pTask->m_iRayleighPitch / sizeof(U16);
		pMieBuffer		+= pTask->m_iMiePitch / sizeof(U16);

		vRayleighSum *= 1.0f / pTask->m_iBufferWidth;
		vMieSum *= 1.0f / pTask->m_iBufferWidth;

		Maths::Store(Maths::HVec4(vRayleighSum), pTask->m_pRayleighColour);
		Maths::Store(Maths::HVec4(vMieSum), pTask->m_pMieColour);
	}
}

void SolveAtmosphereTask(Scheduler::Task* pTask)
{
	SolveAtmosphere((C_AtmosphericTask*) pTask->Output);
}

}; // namespace Sky
}; // namespace Effect
