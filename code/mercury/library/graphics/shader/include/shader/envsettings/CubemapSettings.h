#ifndef __SHADER_CUBEMAPSETTINGS_H__
#define __SHADER_CUBEMAPSETTINGS_H__

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentString.h>
#include <FX/EnvironmentFloat.h>
#include <FX/ENvironmentColour.h>

#include <xt/xtGame.h>

namespace Debug
{
	class String;
};

namespace EnvironmentSettings
{

// structures from Sorcery
// global structure containing the current lighting settings
struct WorldCubemapSettings
{
	WorldCubemapSettings() : m_refCount(0) {}
	Util::C_String			m_sEnvironmentMap;
	Display::C_TextureCube	m_tEnvironmentMap;
	U32 m_refCount;
};

/// implementation of I_EnvironmentSettings for C_TextureSkyDome
class CubemapEnvSettings : public FX::I_EnvironmentSettings
{
public:
	/// constructor
	CubemapEnvSettings(const char *pszFilename);

	/// construct by lerping two other CubemapEnvSettings
	CubemapEnvSettings(const CubemapEnvSettings *pA, const CubemapEnvSettings *pB, float f);
	
	/// constructor
	~CubemapEnvSettings();

	/// cause these environment settings to be applied to the scene
	void Set();

	/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
	I_EnvironmentSettings *Lerp(const I_EnvironmentSettings *p, float f) const;

	/// GUID for this settings block
	Util::C_String Name() const;

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

	static void InitialiseParameters();

private:
	FX::EnvStringParam	m_sEnvironmentMap;

	static WorldCubemapSettings m_CubemapSettings;
};

} // namespace

#endif
