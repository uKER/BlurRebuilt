#ifndef EFFECT_SKY_ATMOSPHERICSKYDOME_H
#define EFFECT_SKY_ATMOSPHERICSKYDOME_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Sky/AtmosphericParameters.h"
#include "Effect/Sky/AtmosphericSolver.h"
#include "Effect/Sky/SkyDomeBase.h"
#include "System/Scheduler/jobqueue.h"

namespace Effect
{
namespace Sky
{

static const U32 kAtmosphericTextureCount = 3;

class C_AtmosphericSkyDome : public C_SkyDomeBase
{
public:

	C_AtmosphericSkyDome();
	virtual ~C_AtmosphericSkyDome();
	
	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroy();
	void OnDestroyVolatile();

	void Update();
	virtual void Bind(Display::C_Context *pContext) const;
	virtual void Unbind(Display::C_Context *pContext) const;

public:

	void SetPolicy(const Scheduler::PolicyHandle& hPolicy)			{ m_hPolicy = hPolicy; }
	void SetSunDirection(const Display::C_Float4& vSunDirection)	{ m_vSunDirection = vSunDirection; }

protected:

	void BeginSky();
	void EndSky();
	void IterateSky();

	void GetTask();
	void GetParameters(C_AtmosphericParameters& params);

private:

	Display::C_Texture2D				m_MieTextures[kAtmosphericTextureCount];
	Display::C_Texture2D				m_RayleighTextures[kAtmosphericTextureCount];
	U32									m_TextureNo;
	U32									m_TextureLineNo;

	Display::C_Float4					m_MieColour[kAtmosphericTextureCount];
	Display::C_Float4					m_RayleighColour[kAtmosphericTextureCount];

	Display::C_PixelShaderParameter		m_LuminanceParam;
	Display::C_PixelShaderParameter		m_SunDirParam;
	Display::C_PixelShaderParameter		m_RayleighSunIntensityParam;
	Display::C_PixelShaderParameter		m_MieSunIntensityParam;
	Display::C_PixelShaderParameter		m_MieGParam;

	C_AtmosphericTask					m_Task;
	C_AtmosphericParameters				m_Parameters;
	Display::C_Float4					m_vSunDirection;

	Scheduler::GroupHandle				m_hJob;
	Scheduler::PolicyHandle				m_hPolicy;
};

} // namespace Sky
} // namespace Effect

#endif // EFFECT_SKY_ATMOSPHERICSKYDOME_H
