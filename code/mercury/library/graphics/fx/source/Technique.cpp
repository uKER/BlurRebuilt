#include "stdafx.h"

#include <system/Serialise.h>
#include <system/DebugLog.h>
#include <system/Debug/Variables.h>

#include "fx/Fx.h"
#include "fx/Effect.h"
#include "fx/Technique.h"

namespace FX
{

TechniqueManager g_TechniqueManager;

/// register a new subtechnique
TechniqueHandle TechniqueManager::RegisterSubtechnique(const char *pszName)
{
	// check for duplicates
	for (int i=0; i<m_Subtechniques.Size(); i++)
	{
		if (!strcmp(m_Subtechniques[i].sName, pszName))
			return TechniqueHandle(1<<i);
	}

	// add new
	m_Subtechniques.PushBack();
	Subtechnique *p = &m_Subtechniques.Back();
	p->psName = new Util::C_String(Util::C_String("Graphics.Techniques.Enable") + pszName);
	p->sName = pszName;
	p->pDebugVariable = new Debug::Bool(p->psName->Chars(), false);

	return TechniqueHandle(1<<(m_Subtechniques.Size()-1));
}

/// destructor
TechniqueManager::~TechniqueManager()
{
	for (int i=0; i<m_Subtechniques.Size(); i++)
	{
		delete m_Subtechniques[i].psName;
		delete m_Subtechniques[i].pDebugVariable;
	}
}

/// calculate the technique handles on each technique of an effect
void TechniqueManager::CalculateTechniqueHandles(Effect *pEffect)
{
	if (m_Subtechniques.Size() == 0)
		return;

	// for each technique, calculate a subtechnique bitfield
	for (int i=0; i<pEffect->Techniques.Size(); i++)
		pEffect->Techniques[i].hTechnique = CalculateSubtechniqueBitfield(&pEffect->Techniques[i]);
}

/// calculate a subtechnique bitfield for a particular technique
TechniqueHandle TechniqueManager::CalculateSubtechniqueBitfield(Technique *pTech)
{
	TechniqueHandle rtn = 0;
	for (int i=0; i<m_Subtechniques.Size(); i++)
	{
		if (strstr(pTech->Name.Chars(), m_Subtechniques[i].sName.Chars()))
			rtn |= (1<<i);
	}

	return rtn;
}

/// return the override technique as specified by the debug variables
int TechniqueManager::GetOverrideTechnique()
{
	int rtn = 0;
	for (int i=0; i<m_Subtechniques.Size(); i++)
	{
		if (m_Subtechniques[i].pDebugVariable->Value())
			rtn |= (1<<i);
	}
	return rtn;
}

/// register a subtechnique
TechniqueHandle RegisterSubtechnique(const char *pszName)
{
	return g_TechniqueManager.RegisterSubtechnique(pszName);
}

} // namespace

