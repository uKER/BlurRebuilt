#include "stdafx.h"

#include <btl/containers/vector.h>
#include <system/debug/Variables.h>
#include "fx/EnvironmentSettings.h"
#include "fx/EnvironmentString.h"

namespace FX
{

struct StringPersistentData
{
	StringPersistentData() { bInitialised = false; }

	Util::C_String sDBgVarName;
	bool bInitialised;
	Debug::String *pVar;
};

/// constructor
EnvStringParam::EnvStringParam(const char *pszString)
{
	m_sString = pszString;
	m_pPersistentData = NULL;
}

/// cause these environment settings to be applied to the scene
EnvStringParam::operator Util::C_String() const
{
	if (m_pPersistentData)
		return m_pPersistentData->pVar->Value();
	else
		return m_sString;
}

/// constructs a new environment settings block with is a lerped version of this and the other environment settings passed in
EnvStringParam::EnvStringParam(const EnvStringParam &a, const EnvStringParam &b, float f)
{
	if (f>0)
		m_sString = b;
	else
		m_sString = a;
	m_pPersistentData = NULL;
}

/// associate this block of environment settings with the given debug variable or debug variable page
void EnvStringParam::AssociateDebugVariables(const Util::C_String &sName)
{
	m_pPersistentData = GetPersistentData<StringPersistentData>(sName);
	if (!m_pPersistentData->bInitialised)
	{
		m_pPersistentData->bInitialised = true;
		m_pPersistentData->sDBgVarName = sName;
		m_pPersistentData->pVar = new Debug::String(m_pPersistentData->sDBgVarName, "");
		m_pPersistentData->pVar->SetValue(m_sString);
	}
	else
	{
		m_pPersistentData->pVar->SetValue(m_sString);
	}
}

} // namespace
