#include "stdafx.h"

#include <btl/containers/vector.h>
#include <system/debug/Variables.h>
#include "fx/EnvironmentSettings.h"
#include "fx/EnvironmentFloat.h"

namespace FX
{

struct FloatPersistentData
{
	FloatPersistentData() { bInitialised = false; }

	Util::C_String sDBgVarName;
	bool bInitialised;
	Debug::Float *pVar;
};

/// constructor
EnvFloatParam::EnvFloatParam(float fValue)
{
	m_fValue = fValue;
	m_pPersistentData = NULL;
}

/// cause these environment settings to be applied to the scene
EnvFloatParam::operator float() const
{
	if (m_pPersistentData)
		return m_pPersistentData->pVar->Value();
	else
		return m_fValue;
}

/// constructs a new environment settings block with is a lerped version of this and the other environment settings passed in
EnvFloatParam::EnvFloatParam(const EnvFloatParam &a, const EnvFloatParam &b, float f)
{
	m_fValue = a + (b - a)*f;
	m_pPersistentData = NULL;
}

/// associate this block of environment settings with the given debug variable or debug variable page
void EnvFloatParam::AssociateDebugVariables(const Util::C_String &sName)
{
	m_pPersistentData = GetPersistentData<FloatPersistentData>(sName);
	if (!m_pPersistentData->bInitialised)
	{
		m_pPersistentData->bInitialised = true;
		m_pPersistentData->sDBgVarName = sName;
		m_pPersistentData->pVar = new Debug::Float(m_pPersistentData->sDBgVarName, m_fValue);
	}
	else
	{
		m_pPersistentData->pVar->SetValue(m_fValue);
	}
}

} // namespace
