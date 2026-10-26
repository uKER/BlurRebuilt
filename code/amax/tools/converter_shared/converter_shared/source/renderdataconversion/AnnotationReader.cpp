#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <fx/effect.h>
#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\Common.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\EffectPermutationSelector.h"


namespace RenderDataConversion
{

/// constructor
AnnotationReader::AnnotationReader()
{
	m_InstanceMode = E_InstanceMode_Base;
	m_bAllowOmit = false;
}

/// parse an xyzw string and return a usage bitfield
/// returns false if the string is not an xyzw string
bool AnnotationReader::ParseXYZWString(const char *psz, XYZW *pOut)
{
	int i = 0;
	pOut->iUsageMask = 0;
	while (*psz)
	{
		if (i>3)
			return false;	// too many characters
		int x = *psz - 'x';
		if ((x<0 || x>2) && x != -1)
			return false;	// not an xyzw character
		if (x == -1)
			x = 3;

		pOut->iMapping[i] = x;
		pOut->iUsageMask |= 1<<x;
		i++;
		psz++;
	}
	pOut->nScalars = i;
	return true;
}

/// checks whether this parameter needs exporting in the current pass
bool AnnotationReader::NeedsExport(E_NodeType type)
{
	switch (type)
	{
	case E_NodeType_Mesh:
		return m_InstanceMode != E_InstanceMode_InstanceOnly;
	case E_NodeType_Base:
		return m_InstanceMode == E_InstanceMode_Base;
	case E_NodeType_Instance:
		return m_InstanceMode != E_InstanceMode_Base;
	default:
		assert(0);
		return false;
	}
}

/// parse annotation
bool AnnotationReader::ParseAnnotation(FX::Annotation *pAnnotation)
{
	if (!stricmp(pAnnotation->Name, "Define") && pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
	{
		m_sDefine = pAnnotation->String;
		m_bAllowOmit = true;
		return true;
	}
	else if (!stricmp(pAnnotation->Name, "InstanceMode") && pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
	{
		if (!stricmp(pAnnotation->String, "Base"))
			m_InstanceMode = E_InstanceMode_Base;
		else if (!stricmp(pAnnotation->String, "Instance"))
		{
			m_InstanceMode = E_InstanceMode_Instance;
		}
		else if (!stricmp(pAnnotation->String, "InstanceOnly"))
		{
			m_InstanceMode = E_InstanceMode_InstanceOnly;
		}
		else
		{
			PrintWarning( "Unrecognised instance mode %s.\n", pAnnotation->String);
		}
		return true;
	}

	return false;
}

/// if this parameter has a permutation, add it to the EPS
void AnnotationReader::SetPermutation(EffectPermutationSelector *pEPS)
{
	if (m_bAllowOmit && m_sDefine != "")
		pEPS->AddPermutation(m_sDefine);
}

};


