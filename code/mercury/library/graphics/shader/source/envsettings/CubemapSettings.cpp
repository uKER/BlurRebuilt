#include "stdafx.h"
#include "shader/envsettings/CubemapSettings.h"

#include <Btl/Containers/Vector.h>
#include <Display.h>
#include <System/Debug/Variables.h>

#include <Fx/Fx.h>
#include <Fx/EnvironmentSettings.h>


namespace EnvironmentSettings
{

static FX::ParameterHandle g_tEnvironment;

inline Maths::HVec4 Load(const XtRgba& v)
{
	return Maths::Load<Maths::HVec4>((const MathsStorage::Float4*)&v);
}

WorldCubemapSettings CubemapEnvSettings::m_CubemapSettings;

/// constructor
CubemapEnvSettings::CubemapEnvSettings(const char *pszFilename)
	: m_sEnvironmentMap( pszFilename )
{
	m_CubemapSettings.m_refCount++;
}

/// construct by lerping two other CubemapEnvSettings
CubemapEnvSettings::CubemapEnvSettings(const CubemapEnvSettings *pA, const CubemapEnvSettings *pB, float f)
	: m_sEnvironmentMap(pA->m_sEnvironmentMap)
{
	m_CubemapSettings.m_refCount++;
	if (f > 0)
		m_sEnvironmentMap = pB->m_sEnvironmentMap;
}

///release refcount on static shared resource
CubemapEnvSettings::~CubemapEnvSettings()
{
	m_CubemapSettings.m_refCount--;
	if (m_CubemapSettings.m_refCount == 0)
	{
		Display::Device()->Destroy(&m_CubemapSettings.m_tEnvironmentMap);
		m_CubemapSettings.m_sEnvironmentMap = "";
	}
}

void CubemapEnvSettings::InitialiseParameters()
{
	g_tEnvironment = FX::GetSharedParameter("g_tEnvironment");
}

/// cause these environment settings to be applied to the scene
void CubemapEnvSettings::Set()
{
	// reload environment map texture if necessary
	if (m_CubemapSettings.m_sEnvironmentMap != m_sEnvironmentMap)
	{
		RESOURCE_MEMORY_ID("Cubemap");
		m_CubemapSettings.m_sEnvironmentMap = m_sEnvironmentMap;
#if defined(PLATFORM_PS3)
		Display::Device()->BlockUntilIdle();
#endif
		Display::Device()->Destroy(&m_CubemapSettings.m_tEnvironmentMap);
		Display::CreateTextureCubeFromFile(m_sEnvironmentMap, &m_CubemapSettings.m_tEnvironmentMap);
	}

	const void *pTex = m_CubemapSettings.m_tEnvironmentMap.GetGPUTexture();
	if (pTex != NULL)
		FX::SetParameter(g_tEnvironment, &pTex );
}

/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
FX::I_EnvironmentSettings *CubemapEnvSettings::Lerp(const FX::I_EnvironmentSettings *p, float f) const
{
	assert(Name() == p->Name());
	return new CubemapEnvSettings(this, (CubemapEnvSettings*)p, f);
}

/// GUID for this settings block
Util::C_String CubemapEnvSettings::Name() const
{
	return "CubemapEnvSettings";
}

/// associate this block of environment settings with the given debug variable or debug variable page
void CubemapEnvSettings::AssociateDebugVariables(const Util::C_String &sName)
{
	m_sEnvironmentMap.AssociateDebugVariables(sName + ".EnvironmentMap");
}

} // namespace


