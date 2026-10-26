#include "stdafx.h"

#include "converter_shared/renderdataconversion/CustomSemantic.h"
#include "converter_shared/renderdataconversion/CustomSemanticManager.h"

namespace RenderDataConversion
{

CustomSemanticManager g_CustomSemanticManager;

/// destructor
CustomSemanticManager::~CustomSemanticManager()
{
	for (int i=0; i<m_Semantics.Size(); i++)
		delete m_Semantics[i];
}

/// register a custom input semantic with the manager
void CustomSemanticManager::RegisterSemantic(const I_CustomSemanticFactory *p)
{
	m_Semantics.PushBack(p);
}

/// returns a pointer to an object that can interpret the given texture source semantic
I_CustomSemantic *CustomSemanticManager::ParseSourceSemantic(const char *pszSource, E_CustomSemanticType type, FX::Parameter *pParam, I_ParseAnnotation *pParent)
{
	for (int i=0; i<m_Semantics.Size(); i++)
	{
		I_CustomSemantic *p = m_Semantics[i]->Instantiate();
		if (p->IdentifySemantic(pszSource, type, pParam))
		{
			p->m_pParent = pParent;
			return p;
		}
		delete p;
	}
	return NULL;
}

};
