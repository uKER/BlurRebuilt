#include "stdafx.h"

#include <Btl/Containers/Vector.h>
#include <Display.h>
#include <System/Debug/Variables.h>

#include <Fx/Fx.h>
#include <Fx/EnvironmentSettings.h>
#include <Render/PostProcess/PostProcessEnvSettings.h>
#include <Render/PostProcess/HDR/HDR.h>
#include <Render/PostProcess/Manager.h>
#include <Render/State/RenderState.h>

XT_TYPE(PostProcess::Settings,"PostProcess")
	XT_FIELD(middleGrey, "middleGrey")
	XT_FIELD(exposureMin, "exposureMin")
	XT_FIELD(exposureMax, "exposureMax")
	XT_FIELD(bloomScale, "bloomScale")
	XT_FIELD(bloomScale0, "bloomScale0")
	XT_FIELD(bloomScale1, "bloomScale1")
	XT_FIELD(bloomScale2, "bloomScale2")
	XT_FIELD(bloomScale3, "bloomScale3")
	XT_FIELD(bloomThreshold, "bloomThreshold")
XT_END_TYPE

namespace PostProcess
{

/// constructor
PostProcessEnvSettings::PostProcessEnvSettings(Settings *pData)
	: m_MiddleGrey(pData->middleGrey)
	, m_ExposureMin(pData->exposureMin)
	, m_ExposureMax(pData->exposureMax)
	, m_BloomScale(pData->bloomScale)
	, m_BloomScale0(pData->bloomScale0)
	, m_BloomScale1(pData->bloomScale1)
	, m_BloomScale2(pData->bloomScale2)
	, m_BloomScale3(pData->bloomScale3)
	, m_BloomThreshold(pData->bloomThreshold)
{
}

/// construct by lerping two other PostProcessEnvSettings
PostProcessEnvSettings::PostProcessEnvSettings(const PostProcessEnvSettings *pA, const PostProcessEnvSettings *pB, float f)
	: m_MiddleGrey(pA->m_MiddleGrey, pB->m_MiddleGrey, f)
	, m_ExposureMin(pA->m_ExposureMin, pB->m_ExposureMin, f)
	, m_ExposureMax(pA->m_ExposureMax, pB->m_ExposureMax, f)
	, m_BloomScale(pA->m_BloomScale, pB->m_BloomScale, f)
	, m_BloomScale0(pA->m_BloomScale0, pB->m_BloomScale0, f)
	, m_BloomScale1(pA->m_BloomScale1, pB->m_BloomScale1, f)
	, m_BloomScale2(pA->m_BloomScale2, pB->m_BloomScale2, f)
	, m_BloomScale3(pA->m_BloomScale3, pB->m_BloomScale3, f)
	, m_BloomThreshold(pA->m_BloomThreshold, pB->m_BloomThreshold, f)
{
}

/// cause these environment settings to be applied to the scene
void PostProcessEnvSettings::Set()
{
	if (Render::C_RenderState::Instance().HDR())
	{
		C_HighDynamicRange* pHDR = (C_HighDynamicRange*)Render::C_RenderState::Instance().HDR();
		for (S32 iViewIndex = 0; iViewIndex < (S32)pHDR->Manager().GetViewCount(); ++iViewIndex)
		{
			if(HighDynamicRangeParameters* pParameters = pHDR->GetParameters(iViewIndex))
			{
				pParameters->fMiddleGrey = m_MiddleGrey;
				pParameters->fExposureMin = m_ExposureMin;
				pParameters->fExposureMax = m_ExposureMax;
				pParameters->fBloomThreshold = m_BloomThreshold;
				pParameters->fDownsampleScale[0] = 1.5f;
				pParameters->fBloomScale[0] = m_BloomScale * m_BloomScale0;
				pParameters->fBloomScale[1] = m_BloomScale * m_BloomScale1;
				pParameters->fBloomScale[2] = m_BloomScale * m_BloomScale2;
				pParameters->fBloomScale[3] = m_BloomScale * m_BloomScale3;
				pParameters->Update();
			}
		}
	}
}

/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
FX::I_EnvironmentSettings *PostProcessEnvSettings::Lerp(const FX::I_EnvironmentSettings *p, float f) const
{
	assert(Name() == p->Name());
	return new PostProcessEnvSettings( this, (PostProcessEnvSettings*)p, f);
}

/// GUID for this settings block
Util::C_String PostProcessEnvSettings::Name() const
{
	return "PostProcessEnvSettings";
}

/// associate this block of environment settings with the given debug variable or debug variable page
void PostProcessEnvSettings::AssociateDebugVariables(const Util::C_String &sName)
{
	m_MiddleGrey.AssociateDebugVariables(sName + ".PostProcess.MiddleGrey");
	m_ExposureMin.AssociateDebugVariables(sName + ".PostProcess.ExposureMin");
	m_ExposureMax.AssociateDebugVariables(sName + ".PostProcess.ExposureMax");
	m_BloomScale.AssociateDebugVariables(sName + ".PostProcess.BloomScale");
	m_BloomThreshold.AssociateDebugVariables(sName + ".PostProcess.BloomThreshold");
}

} // namespace PostProcess
