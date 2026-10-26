#ifndef __FX_ENVSETTINGSCOLOUR_H__
#define __FX_ENVSETTINGSCOLOUR_H__

namespace Debug
{
class String;
}

namespace FX
{

struct ColourPersistentData;

/// colour parameter utility class for writing implementations of I_EnvironmentSettings
class EnvColourParam
{
public:
	/// constructor
	EnvColourParam(const Maths::HVec4 PCREF vColour);

	/// construct by lerping between two other EnvTextureParams
	EnvColourParam(const EnvColourParam &a, const EnvColourParam &b, float f);

	/// get the current colour value
	operator Maths::C_RGBA_F32 () const;

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

private:
	Maths::HVec4 m_vColour;

	ColourPersistentData *m_pPersistentData;
};

} // namespace

#endif
