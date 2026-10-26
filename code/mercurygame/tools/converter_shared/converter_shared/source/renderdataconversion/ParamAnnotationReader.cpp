#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\Effect.h"
#include <fx/effect.h>

#include "converter_shared\renderdataconversion\Common.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"

#include "converter_shared\renderdataconversion\ConvertAftkMaterial.h"

namespace RenderDataConversion
{

// this list of source names matches the enum in ParamAnnotationReader.h
const char *ParamAnnotationReader::m_pszSourceNames[] = {
"<None>",
"<Diffuse>",
"<Lightmap>",
"<WorldMatrix>",
"<LuxCustom>",
"<InvalidTexture>",
""
};

/// constructor
ParamAnnotationReader::ParamAnnotationReader(FX::Parameter *p, const Util::C_String &sEffectName)
{
	m_pParam = p;
	m_Source = E_Source_None;
	m_sEffectName = sEffectName;
	m_iOffset = 0;
}

/// interpret a 'Source' annotation and set variables accordingly
bool ParamAnnotationReader::IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
{
	// work out what type of source it is
	const char **pszSources = m_pszSourceNames;
	const char *pszMySource = pszSource;
	// if it wrapped in '<>', assume it's a source semantic
	if (pszMySource[0] == '<' && pszMySource[strlen(pszMySource)-1] == '>')
	{
		if (!strnicmp(pszMySource, "<LuxCustom+", 11))
		{
			m_Source = E_Source_LuxCustom;
			char buffer[512];
			strcpy(buffer, pszMySource+11);
			buffer[strlen(buffer)-1] = 0;
			m_iOffset = atoi(buffer);
			return true;
		}

		int iSource = 0;
		while (pszSources[iSource][0] != 0)
		{
			if (!stricmp(pszSources[iSource], pszMySource))
			{
				m_Source = (E_Source)iSource;
				return true;
			}
			iSource++;
		}
		return false;
	}
	// handle named parameters
	m_Source = E_Source_NamedParam;
	m_sNamedParamName = pszMySource;
	return true;
}

/// interpret an annotation
bool ParamAnnotationReader::ParseAnnotation(FX::Annotation *pAnnotation)
{
	bool bResult = AnnotationReader::ParseAnnotation(pAnnotation);

	if (!m_pParam->bShared && m_InstanceMode != E_InstanceMode_Base)
		PrintWarning("Parameter %s is InstanceMode!=Base without being declared as 'shared'.\n", m_pParam->Name.Chars());

	return bResult;
}

/// set the owner effect's name (for printing warning messages)
void ParamAnnotationReader::SetFXName(const char *pszFXName)
{
	m_sEffectName = pszFXName;
}

/// parse the annotation and work out where to source the texture from
void ParamAnnotationReader::ParseAnnotations()
{
	// does the parameter have 'source'? if so it must come from Maya
	for (int i=0; i<m_pParam->Annotations.Size(); i++)
	{
		FX::Annotation *pAnnotation = &m_pParam->Annotations[i];

		if (!ParseAnnotation(pAnnotation))
		{
			PrintWarning( "%s: unrecognised annotation '%s' on parameter '%s'\n", m_sEffectName.Chars(), pAnnotation->Name.Chars(), m_pParam->Name.Chars());
		}
	}
}

/// utility function to output an animated material parameter
/// this function looks at the material element datatype and generates a dynamic material with that datatype on
void ParamAnnotationReader::SetupAnimatedParameter(AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **ppData, const char *pszName)
{
	RenderingData::Build::MaterialList *pMaterialList = &pMtl->GetSceneExport()->GetMaterialList();

	Util::C_String sAnimParamName = pszName;

	// the name of the dynamic material is the name of the animated parameter 
	RenderingData::MaterialElement dynElement;
	dynElement.count = 2;	// the element stores the required type and count of the output material
	dynElement.name = sAnimParamName.Chars();
	dynElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_U32;

	// create the dynamic material type
	RenderingData::MaterialType materialType;
	materialType.SetName("AnimatedParam");
	materialType.SetDynamicListIndex(IsExportingModel()?0:2);
	materialType.AddElement(dynElement);
	int iDynamicMtlType = pMaterialList->InsertMaterialType(materialType);

	// create the dynamic material and store it's index in the static material data
	if (ppData)
	{
		RenderingData::Material material;
		pMaterialList->CreateMaterial(material, iDynamicMtlType);
		material.SetData(0, pMtlElement->type);
		material.SetData(4, pMtlElement->count);
		*((int*)*ppData) =  pMaterialList->InsertMaterial(material);;
		(*ppData) += 4;
	}

	// set the static material element
	pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_DATA_INDEX;
	pMtlElement->name = m_pParam->Name.Chars();
	pMtlElement->count = 1;
}

/// return the parameter that is associated with this annotation reader
FX::Parameter *ParamAnnotationReader::GetFXParameter()
{
	return m_pParam;
}

/// set the bUsed member variable
void ParamAnnotationReader::SetUsed(bool bUsed)
{
	m_bUsed = bUsed;
}

/// get the bUsed member variable
bool ParamAnnotationReader::IsUsed()
{
	return m_bUsed;
}

}

