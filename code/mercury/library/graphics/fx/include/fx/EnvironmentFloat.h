#ifndef __FX_ENVSETTINGSFLOAT_H__
#define __FX_ENVSETTINGSFLOAT_H__

namespace Debug
{
class String;
}

namespace FX
{

struct FloatPersistentData;

/// float value parameter utility class for writing implementations of I_EnvironmentSettings
class EnvFloatParam
{
public:
	/// constructor
	EnvFloatParam(float fFloat = 0.f);

	/// construct by lerping between two other EnvTextureParams
	EnvFloatParam(const EnvFloatParam &a, const EnvFloatParam &b, float f);

	/// get the current float value
	operator float () const;

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

private:
	float m_fValue;

	FloatPersistentData *m_pPersistentData;
};

} // namespace

#endif
