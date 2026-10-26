#include "stdafx.h"

#define FX_ALLOW_INCLUDE	// only for EvaluateSSH.cpp

#include <core/Maths.h>
#include <fx/fxContext.h>

namespace Shaders
{

Maths::HVec3 EvaluateRGBSH(Display::C_DeviceContext *pContext, float *pData, const Maths::HVec3 vNormal)
{
	Maths::HVec4 _vNormal(vNormal);
	_vNormal.W(1.f);

	Maths::HVec3 vColour(
		_vNormal|Maths::HVec4(&pData[0]),
		_vNormal|Maths::HVec4(&pData[4]),
		_vNormal|Maths::HVec4(&pData[8]));

	return vColour;
}


Maths::HVec3 EvaluateBasisSH(Display::C_DeviceContext *pContext, float *pData, const Maths::HVec3 vNormal)
{
	FX::Context *pFX = FX::GetContext(FX::GetContext(pContext));

	static FX::ParameterHandle hSunColour = FX::GetSharedParameter("g_vSunColour");
	Maths::HVec3 vSunColour((float*)pFX->GetSharedParameterValue(hSunColour));

	static FX::ParameterHandle hSkyColour = FX::GetSharedParameter("g_vSkyColour");
	Maths::HVec3 vSkyColour((float*)pFX->GetSharedParameterValue(hSunColour));

	Maths::HVec3 vColour = EvaluateRGBSH(pContext, pData, vNormal);

	float fSunOcclusion = pData[12];
	float fSkyOcclusion = pData[13];

	vColour += vSunColour * fSunOcclusion;
	vColour += vSkyColour * fSkyOcclusion;

	return vColour;
}

}
