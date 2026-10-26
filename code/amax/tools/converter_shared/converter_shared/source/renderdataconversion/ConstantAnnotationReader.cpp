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
	ParseAnnotation();
}

/// destructor
ConstantAnnotationReader::~ConstantAnnotationReader()
{
}

/// returns true if this parameter is supposed to go on the renderingdata material
bool ConstantAnnotationReader::IsMaterialParameter()
{
	return m_Source != E_Source_None;
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
void ConstantAnnotationReader::ParseAnnotation()
{
	// does the parameter have 'source'? if so it must come from Maya
	for (int i=0; i<m_pParam->Annotations.Size(); i++)
	{
		FX::Annotation *pAnnotation = &m_pParam->Annotations[i];

		if (!stricmp(pAnnotation->Name, "Format"))
		{
			if (pAnnotation->Type != FX::NGLFX_ANNOTATION_STRING)
				continue;

			for (int iFormat=0; m_pFormatDefs[iFormat].pszName[0] > 0; iFormat++)
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
		}
		else if (!stricmp(pAnnotation->Name, "Default"))
		{
			if (pAnnotation->Type != FX::NGLFX_ANNOTATION_STRING)
				continue;

			ReadVectorString(pAnnotation->String, m_fDefault, 16);
		}
		else if (!ParamAnnotationReader::ParseAnnotation(pAnnotation))
		{
			PrintWarning( "%s: unrecognised annotation '%s' on parameter '%s'\n", m_sEffectName.Chars(), pAnnotation->Name.Chars(), m_pParam->Name.Chars());
		}
	}
}

/// return a RenderingData::MaterialElement that describes this parameter
/// returns false if this parameter is not supposed to be stored in the renderingdata material
bool ConstantAnnotationReader::GetMaterialElement(RenderingData::MaterialElement *pMtlElement)
{
	if (!IsMaterialParameter())
		return false;

	// TODO: support for U8's
	pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
	pMtlElement->count = m_iElementCount;
	pMtlElement->name = m_pParam->Name.Chars();
	return true;
}

/// read out data for this parameter, given an Aftk Material
/// automatically advances the pData pointer
bool ConstantAnnotationReader::GetMaterialData(AftkMaterialConverter *pMtl, U8 **ppData)
{
	Util::C_String sTextureName;

	float combinedData[16];
	memcpy(combinedData, m_fDefault, sizeof(combinedData));

	if (!IsMaterialParameter())
		return false;

	// TODO: in the future we might want support for packing multiple sources into one shader constant register
	switch (m_Source)
	{
	default:
		assert(0); // shouldn't get here
		return false;
	case E_Source_WorldMatrix:
		pMtl->GetWorldMatrix(combinedData);
		break;
	case E_Source_LuxCustom:
		pMtl->GetLuxCustomData(combinedData, m_iOffset);
		break;
	case E_Source_NamedParam:
		pMtl->GetFloat4FromPlug(m_sNamedParamName, combinedData);
		break;
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
