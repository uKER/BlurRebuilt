#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>

#include <renderingdata/renderingdata.h>
#include <renderingdata/render.h>
#include "converter_shared\renderdataconversion\Effect.h"
#include <fx/fxb.h>
#include <fx/Effect.h>
#include <fx/Shared.h>

#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConstantAnnotationReader.h"

#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\MaterialManager.h"
#include "converter_shared\renderdataconversion\ConvertAftkMaterial.h"

namespace RenderDataConversion
{

/// constructor
ConstantAnnotationReader::ConstantAnnotationReader(FX::Parameter *pParam, const Util::C_String &sEffectName) :
	ParamAnnotationReader(pParam, sEffectName)
{
	m_pParam = pParam;
	m_iFormat = 0;
	m_iElementCount = m_pParam->Size;
	memset(m_fDefault, 0, sizeof(m_fDefault));
}

/// destructor
ConstantAnnotationReader::~ConstantAnnotationReader()
{
}

/// identify source semantic
bool ConstantAnnotationReader::IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
{
	assert(0); // this function is not used anymore. TODO: make this a pure virtual function
	return false;
}

// list of supported format specifications
const ConstantAnnotationReader::FormatDef ConstantAnnotationReader::m_pFormatDefs[] =
{
	{ "F32", RenderingData::MATERIAL_ELEMENT_TYPE_F32 },
	{ "U8", RenderingData::MATERIAL_ELEMENT_TYPE_U8 },

	{ "" },
};

/// parse a string representing a series of floating point values
void ReadVectorString(const char *psz, float *pfOut, int cMax)
{
	bool bLast = false;
	int idx = 0;
	do
	{
		char buffer[512];
		const char *p = strchr(psz, ',');
		if (p)
		{
			strncpy(buffer, psz, p-psz);
			buffer[p-psz+1] = 0;
		}
		else
		{
			strcpy(buffer, psz);
			bLast = true;
		}
		psz = p+1;

		pfOut[idx] = (float)atof(buffer);
		idx++;
	}
	while (!bLast && idx < cMax);
}

/// parse the annotation and work out where to source the texture from
bool ConstantAnnotationReader::ParseAnnotation(FX::Annotation *pAnnotation)
{
	if (!stricmp(pAnnotation->Name, "Format"))
	{
		if (pAnnotation->Type != FX::NGLFX_ANNOTATION_STRING)
			return false;

		int iFormat;
		for (iFormat=0; m_pFormatDefs[iFormat].pszName[0] > 0; iFormat++)
		{
			if (!stricmp(m_pFormatDefs[iFormat].pszName, pAnnotation->String))
			{
				m_iFormat = iFormat;
				break;
			}
		}
		if (m_pFormatDefs[iFormat].pszName[0] > 0)
		{
			PrintWarning( "%s: unrecognised format specification '%s' on parameter '%s'\n", m_sEffectName.Chars(), pAnnotation->String, m_pParam->Name.Chars());
		}
		return true;
	}
	else if (!stricmp(pAnnotation->Name, "Default"))
	{
		if (pAnnotation->Type != FX::NGLFX_ANNOTATION_STRING)
			return false;

		ReadVectorString(pAnnotation->String, m_fDefault, 512);

		return true;
	}

	return ParamAnnotationReader::ParseAnnotation(pAnnotation);
}

/// read out data for this parameter, given an Aftk Material
/// automatically advances the pData pointer
bool ConstantAnnotationReader::GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **ppData)
{
	Util::C_String sTextureName;

	// TODO: support for U8's
	pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
	pMtlElement->count = m_iElementCount;
	pMtlElement->name = m_pParam->Name.Chars();

	float combinedData[512];
	memcpy(combinedData, m_fDefault, sizeof(combinedData));

	Util::C_String sAnimatedName = "";
	if (!GetAftkData(pMtl, combinedData, &sAnimatedName))
		return false;

	// handle the case of an animated parameter
	if (sAnimatedName != "")
	{
		SetupAnimatedParameter(pMtl, pMtlElement, ppData, sAnimatedName);
		return true;
	}

	if (!ppData)
		return true;

	// convert to target format
	int n = m_iElementCount;
	for (int i=0; i<n; i++)
	{
		if (m_pFormatDefs[m_iFormat].type == RenderingData::MATERIAL_ELEMENT_TYPE_U8)
		{
			*(((U8*)(*ppData))) = (int)(combinedData[i] * 255.f);
			(*ppData)++;
		}
		else if (m_pFormatDefs[m_iFormat].type == RenderingData::MATERIAL_ELEMENT_TYPE_F32)
		{
			*(((float*)(*ppData))) = combinedData[i];
			(*ppData)+=4;
		}
		else
		{
			assert(0); // shouldn't happen
			return false;
		}
	}

	return true;
}

};
