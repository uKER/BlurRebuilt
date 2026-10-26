#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <fx/effect.h>
#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\Common.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\EffectPermutationSelector.h"


namespace RenderDataConversion
{

bool g_bExportingModel = true;

/// constructor
AnnotationReader::AnnotationReader()
{
	m_InstanceMode = E_InstanceMode_Base;
	m_bAllowOmit = false;
	m_bExportOnModel = true;
	m_iStream = 0;
}

/// returns true if we're exporting a model or platform, false if we're exporting static streamed level mesh
bool AnnotationReader::IsExportingModel()
{
	return g_bExportingModel;
}

/// parse an xyzw string and return a usage bitfield
/// returns false if the string is not an xyzw string
bool ParseXYZWString(const char *psz, XYZW *pOut)
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
	if (!m_bExportOnModel && g_bExportingModel)
		return false;

	switch (type)
	{
	case E_NodeType_Mesh:
		return m_InstanceMode != E_InstanceMode_InstanceOnly;
	case E_NodeType_Base:
		return m_InstanceMode == E_InstanceMode_Base;
	case E_NodeType_Instance:
		return m_InstanceMode != E_InstanceMode_Base;
	case E_NodeType_InstanceReset:
		return m_InstanceMode == E_InstanceMode_InstanceOnly;
	default:
		assert(0);
		return false;
	}
}

/// parse annotation
bool AnnotationReader::ParseAnnotation(FX::Annotation *pAnnotation)
{
	if (!stricmp(pAnnotation->Name, "Source"))
		return true; // source annotations are parsed separately
	else if (!stricmp(pAnnotation->Name, "Define") && pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
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
			m_bExportOnModel = false;
		}
		else if (!stricmp(pAnnotation->String, "ResetMtl"))
		{
			m_InstanceMode = E_InstanceMode_InstanceOnly;
		}
		else
		{
			PrintWarning( "Unrecognised instance mode %s.\n", pAnnotation->String);
		}
		return true;
	}
	else if (!stricmp(pAnnotation->Name, "Type") || !stricmp(pAnnotation->Name, "Range") || !stricmp(pAnnotation->Name, "Uninitialised"))
	{
		// this is a hint for DebugView and can be ignored by the scene converter
		return true;
	}
	else if (!stricmp(pAnnotation->Name, "Stream"))
	{
		if (pAnnotation->Type != FX::NGLFX_ANNOTATION_INT)
			return false;

		if (pAnnotation->Int < 0 || pAnnotation->Int >= MAX_MATERIAL_STREAMS)
			return false;

		m_iStream = pAnnotation->Int;

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

/// returns true if this annotation reader needs exporting for the given stream index
bool AnnotationReader::NeedsExportingForStream(int iStream)
{
	return iStream == -1 || iStream == m_iStream;
}

};


