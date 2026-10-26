#ifndef __FX_ENVSETTINGSSTRING_H__
#define __FX_ENVSETTINGSSTRING_H__

namespace Debug
{
class String;
}

namespace FX
{

struct StringPersistentData;

/// string parameter utility class for writing implementations of I_EnvironmentSettings
class EnvStringParam
{
public:
	/// constructor
	EnvStringParam(const char *pszString);

	/// construct by lerping between two other EnvTextureParams
	EnvStringParam(const EnvStringParam &a, const EnvStringParam &b, float f);

	/// get the platform-specific GPU texture pointer
	operator Util::C_String () const;

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

private:
	Util::C_String m_sString;

	StringPersistentData *m_pPersistentData;
};

} // namespace

#endif
