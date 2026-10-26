#include "stdafx.h"

#include <btl/containers/vector.h>
#include <system/debug/Variables.h>
#include "fx/EnvironmentSettings.h"
#include "fx/EnvironmentColour.h"

namespace FX
{

struct ColourPersistentData
{
	ColourPersistentData() { bInitialised = false; }

	Util::C_String sDBgVarName;
	bool bInitialised;
	Debug::Colour *pVar;
};

/// constructor
//EnvColourParam::EnvColourParam(Maths::HVec4 vColour) // JASOND: Win32 SSE compiler error: "C2719: 'vColour': formal parameter with __declspec(align('16')) won't be aligned"
EnvColourParam::EnvColourParam(const Maths::HVec4 PCREF vColour)
{
	m_vColour = vColour;
	m_pPersistentData = NULL;
}

/// cause these environment settings to be applied to the scene
EnvColourParam::operator Maths::C_RGBA_F32() const
{
	if (m_pPersistentData)
	{
		Debug::ColourStruct s = m_pPersistentData->pVar->Value();
		return Maths::HVec4(s.r, s.g, s.b, s.a);
	}
	else
		return m_vColour;
}

/// constructs a new environment settings block with is a lerped version of this and the other environment settings passed in
EnvColourParam::EnvColourParam(const EnvColourParam &a, const EnvColourParam &b, float f)
{
	const Maths::C_RGBA_F32 va = a, vb = b;
	const Maths::C_RGBA_F32 v = va + (vb - va)*f;
	m_vColour = v.m_V;
	m_pPersistentData = NULL;
}

/// associate this block of environment settings with the given debug variable or debug variable page
void EnvColourParam::AssociateDebugVariables(const Util::C_String &sName)
{
	m_pPersistentData = GetPersistentData<ColourPersistentData>(sName);
	if (!m_pPersistentData->bInitialised)
	{
		m_pPersistentData->bInitialised = true;
		m_pPersistentData->sDBgVarName = sName;
		m_pPersistentData->pVar = new Debug::Colour(m_pPersistentData->sDBgVarName, m_vColour.X(), m_vColour.Y(), m_vColour.Z(), m_vColour.W());
	}
	else
	{
		m_pPersistentData->pVar->SetValue(Debug::ColourStruct(m_vColour.X(), m_vColour.Y(), m_vColour.Z(), m_vColour.W()));
	}
}

} // namespace
