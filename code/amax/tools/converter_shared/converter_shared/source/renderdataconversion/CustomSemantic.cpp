#include "stdafx.h"

#include "converter_shared/renderdataconversion/CustomSemantic.h"
#include "converter_shared/renderdataconversion/CustomSemanticManager.h"

#include <fx/Effect.h>

namespace RenderDataConversion
{

/// functions for registering a custom input semantic
void RegisterSemantic(const I_CustomSemanticFactory *p)
{
	g_CustomSemanticManager.RegisterSemantic(p);
}

void I_CustomSemantic::SetAnnotation(const char *psz, const char *pszValue)
{
	FX::Annotation tmp;
	tmp.Name = psz;
	tmp.String = pszValue;
	tmp.Type = FX::NGLFX_ANNOTATION_STRING;

	m_pParent->ParseAnnotation(&tmp);
	tmp.String = NULL;
}

void I_CustomSemantic::SetAnnotation(const char *psz, bool bValue)
{
	FX::Annotation tmp;
	tmp.Name = psz;
	tmp.Bool = bValue;
	tmp.Type = FX::NGLFX_ANNOTATION_BOOL;

	m_pParent->ParseAnnotation(&tmp);
	tmp.String = NULL;
}

}
