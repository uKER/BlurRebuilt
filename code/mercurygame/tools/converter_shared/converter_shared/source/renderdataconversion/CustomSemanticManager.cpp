#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include "converter_shared/renderdataconversion/CustomSemantic.h"
#include "converter_shared/renderdataconversion/CustomSemanticManager.h"
#include "converter_shared/renderdataconversion/Common.h"
#include "converter_shared/renderdataconversion/AnnotationReader.h"
#include "converter_shared/renderdataconversion/ParamAnnotationReader.h"
#include "converter_shared/renderdataconversion/VtxInputAnnotationReader.h"
#include "converter_shared/renderdataconversion/Effect.h"

#include <fx/effect.h>

namespace RenderDataConversion
{

CustomSemanticManager g_CustomSemanticManager;

/// destructor
CustomSemanticManager::~CustomSemanticManager()
{
	for (int i=0; i<m_Semantics.Size(); i++)
		delete m_Semantics[i];
	for (int i=0; i<m_VtxSemantics.Size(); i++)
		delete m_VtxSemantics[i];
}

/// register a custom input semantic with the manager
void CustomSemanticManager::RegisterSemantic(const I_CustomSemanticFactory *p)
{
	m_Semantics.PushBack(p);
}

/// register a custom input semantic with the manager
void CustomSemanticManager::RegisterSemantic(const I_CustomVtxInputSemanticFactory *p)
{
	m_VtxSemantics.PushBack(p);
}

/// find the source annotation and return the name of the source
const char *CustomSemanticManager::GetSource(Btl::Vector<FX::Annotation> &annotations)
{
	// find source annotation
	const char *pszMySource = NULL;
	for (int i=0; i<annotations.Size(); i++)
	{
		FX::Annotation *pAnnotation = &annotations[i];

		if (!stricmp(pAnnotation->Name, "Source") &&
			pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
		{
			pszMySource = pAnnotation->String;
			break;
		}
	}

	return pszMySource;
}

/// returns a pointer to an object that can interpret the given source semantic
ParamAnnotationReader *CustomSemanticManager::FindAnnotationReader(FX::Parameter *pParam, const char *pszFXName)
{
	// find parameter type
	E_CustomSemanticType type;
	switch (pParam->Type)
	{
	case FX::NGLFX_PARAM_SAMPLER:
		type = E_CustomSemanticType_Texture;
		break;
	case FX::NGLFX_PARAM_VECTOR:
	case FX::NGLFX_PARAM_MATRIX:
		type = E_CustomSemanticType_Constant;
		break;
	default:
		return NULL;
	}

	const char *pszMySource = GetSource(pParam->Annotations);

	// source specifier found
	if (pszMySource)
	{
		// find an annotation reader that understands this source semantic
		for (int i=0; i<m_Semantics.Size(); i++)
		{
			ParamAnnotationReader *p = m_Semantics[i]->Instantiate(pParam, pszFXName);
			if (p->IdentifySemantic(pszMySource, type))
			{
				p->ParseAnnotations();
				return p;
			}
			delete p;
		}

		// unrecognised source semantic
		PrintWarning( "%s: Unrecognised source semantic '%s' on parameter '%s'\n",
			pszFXName, pszMySource, pParam->Name.Chars() );
	}

	return NULL;
}

/// returns a pointer to an object that can interpret the given vtx input source semantic
I_CustomVtxInputSemantic *CustomSemanticManager::FindVtxInputSemanticHandler(VtxInputAnnotationReader *pParent, const char *pszMySource)
{
	for (int i=0; i<m_VtxSemantics.Size(); i++)
	{
		I_CustomVtxInputSemantic *p = m_VtxSemantics[i]->Instantiate(pParent);
		if (p->IdentifySemantic(pszMySource))
			return p;
		delete p;
	}

	return NULL;
}

/// functions for registering a custom input semantic
void RegisterSemantic(const I_CustomSemanticFactory *p)
{
	g_CustomSemanticManager.RegisterSemantic(p);
}

/// functions for registering a custom vtx input semantic
void RegisterSemantic(const I_CustomVtxInputSemanticFactory *p)
{
	g_CustomSemanticManager.RegisterSemantic(p);
}

};

