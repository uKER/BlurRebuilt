#include "stdafx.h"

#include <btl/containers/vector.h>
#include "fx/Fx.h"
#include "fx/Effect.h"
#include "fx/EnvironmentSettings.h"
#include "fx/Structs.h"

#include <system/debug/Variables.h>

namespace FX
{

/// return a pointer to the environmentsettings class with the given name
/// returns NULL if not found
I_EnvironmentSettings *I_EnvironmentSettings::GetSettingsBlockByName(const char *pszName)
{
	if (!stricmp(pszName, Name().Chars()))
		return this;
	else
		return NULL;
}

/// add a settings block
void EnvironmentSettings::AddBlock(I_EnvironmentSettings *pBlock)
{
	m_Blocks.PushBack(pBlock);
}

/// GUID for this settings block
Util::C_String EnvironmentSettings::Name() const
{
	Util::C_String sRtn;
	for (int i=0; i<m_Blocks.Size(); i++)
		sRtn += m_Blocks[i]->Name() + "/";

	return sRtn;
}

/// recurse into child settings to find a settings block with the name we're looking for
I_EnvironmentSettings *EnvironmentSettings::GetSettingsBlockByName(const char *pszName)
{
	I_EnvironmentSettings *pRtn = NULL;
	for (int i=0; i<m_Blocks.Size(); i++)
	{
		pRtn = m_Blocks[i]->GetSettingsBlockByName(pszName);
		if (pRtn)
			return pRtn;
	}

	return NULL;
}

/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
I_EnvironmentSettings *EnvironmentSettings::Lerp(const I_EnvironmentSettings *_p, float f) const
{
	assert(Name() == _p->Name());
	EnvironmentSettings *p = (EnvironmentSettings*)_p;

	EnvironmentSettings *pNew = new EnvironmentSettings;

	for (int i=0; i<m_Blocks.Size(); i++)
	{
		assert(p->m_Blocks[i]->Name() == m_Blocks[i]->Name());

		pNew->m_Blocks.PushBack(m_Blocks[i]->Lerp(p->m_Blocks[i], f));
	}

	return pNew;
}

/// associate this block of environment settings with the given debug variable or debug variable page
void EnvironmentSettings::AssociateDebugVariables(const Util::C_String &sName)
{
	for (int i=0; i<m_Blocks.Size(); i++)
	{
		m_Blocks[i]->AssociateDebugVariables(sName);
	}
}

/// cause these environment settings to be applied to the scene
void EnvironmentSettings::Set()
{
	for (int i=0; i<m_Blocks.Size(); i++)
		m_Blocks[i]->Set();
}

/// destructor
EnvironmentSettings::~EnvironmentSettings()
{
	for (int i=0; i<m_Blocks.Size(); i++)
		delete m_Blocks[i];
}

// -- ShaderSettings

struct VarItem
{
	Util::C_String sName;
	int iOffset;
	int iSize;
	Debug::VarBase *p;
	float *pVarData;
};

struct PersistentItem
{
	PersistentItem() { m_cSize = -1; }

	Util::C_String m_sName;
	Struct *m_pStruct;
	Btl::Vector<VarItem> m_Variables;
	int m_cSize;
};

Btl::Vector<void*> g_PersistentData;

/// return a pointer to the persistent debug variable data
void **GetPersistentDataInternal(const Util::C_String &sName)
{
	for (int i=0; i<g_PersistentData.Size(); i++)
	{
		if (*((Util::C_String*)g_PersistentData[i]) == sName)
			return &g_PersistentData[i];
	}

	// make new
	g_PersistentData.PushBack((void*)NULL);
	return &g_PersistentData.Back();
}

/// find and extra the min-max range from a struct member's annotations
bool AnnotationsGetRange(const Btl::Vector<Annotation> &annotations, float *pfMin, float *pfMax)
{
	for (int i=0; i<annotations.Size(); i++)
	{
		const Annotation *p = &annotations[i];
		if (!stricmp(p->Name.Chars(), "range"))
		{
			const char *psz = p->String;
			char *pszBuf = (char*)alloca(strlen(psz)+1);
			strcpy(pszBuf,psz);
			char *pComma = strchr(pszBuf, ',');
			if (!pComma)
				return false;
			*pComma = 0;
			*pfMin = atof(pszBuf);
			*pfMax = atof(pComma+1);
			return true;
		}
	}
	return false;
}

} // namespace
