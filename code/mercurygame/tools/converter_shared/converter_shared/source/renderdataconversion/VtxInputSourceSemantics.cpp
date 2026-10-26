#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\Effect.h"
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include <display/device/DeviceTypes.h>

#include <fx/effect.h>
#include <Aftk/Aftk.h>

#include <renderingdata/vertexbuffer.h>
#include "converter_shared\renderdataconversion\VtxInputAnnotationReader.h"
#include "converter_shared\renderdataconversion\ConvertAftkMesh.h"
#include "converter_shared\renderdataconversion\VtxInputSourceSemantics.h"

namespace RenderDataConversion
{

void RegisterDefaultVtxInputSemantics()
{
	RenderDataConversion::RegisterVtxInputAnnotationReader<DefaultVtxInputSourceSemantics>();
}

/// constructor
DefaultVtxInputSourceSemantics::DefaultVtxInputSourceSemantics(VtxInputAnnotationReader *pParent)
{
	m_pParent = pParent;
}

/// source type names. this array corresponds to the E_VtxSource enum
const char *DefaultVtxInputSourceSemantics::m_pszSourceTypeNames[] =
{
	"Position",	// E_VtxSource_Position,
	"Normal",	// E_VtxSource_Normal,
	"UV",		// E_VtxSource_UV,
	"Tangent",	// E_VtxSource_Tangent,
	"Binormal",	// E_VtxSource_Binormal,
	"Sense",	// E_VtxSource_Sense,
	"VtxData",	// E_VtxSource_VtxData,
	"Colour",	// E_VtxSource_Colour
	"UVNoRemap",// E_VtxSource_UVNoRemap,
	"",
};

/// returns true if the given input source semantic name can be handled by this class
bool DefaultVtxInputSourceSemantics::IdentifySourceSemantic(const char *pszSource)
{
	// interpret the source type
	for (int i=0; true; i++)
	{
		if (m_pszSourceTypeNames[i][0]==0)
			return false;
		if (!stricmp(m_pszSourceTypeNames[i], pszSource))
		{
			// found it
			m_iSource = (E_VtxSource)i;
			return true;
		}
	}
	return false;
}

/// returns true if the given input source semantic name can be handled by this class
bool DefaultVtxInputSourceSemantics::IdentifySemantic(const char *_pszSource)
{
	char *pszSource = strdup(_pszSource);
	char *pszName = "";

	// detect named source ':'
	char *p = strchr(pszSource, ':');
	if (p)
	{
		*p = 0;
		pszName = p+1;
	}

	if (!IdentifySourceSemantic(pszSource))
	{
		free(pszSource);
		return false;
	}

	// interpret the name/index
	for (int i=0; true; i++)
	{
		// got the the end and encountered only numbers... it's an index
		if (pszName[i] == 0)
		{
			if (i > 0)
				m_iIndex = atoi(pszName);
			else
				m_iIndex = 0;	// nothing specified - use index=0
			break;
		}
		// contains characters other than 0-9... it's a string
		if (pszName[i] < '0' || pszName[i] > '9')
		{
			m_iIndex = -1;
			m_sName = pszName;
			break;
		}
	}

	free(pszSource);
	return true;
}

/// returns true if UV re-centering needs to be performed on the output data
bool DefaultVtxInputSourceSemantics::DoRecentering()
{
	return m_iSource == E_VtxSource_UV;
}

/// get the data for the given vertex (this function is called by MergeData)
bool DefaultVtxInputSourceSemantics::GetData(const AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, float *data)
{
	const Aftk::C_SceneMeshNode *pMesh = pMeshConv->GetMeshNode();
	const Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(0, iPolygon);

	int iSet;

	// retrieve data from aftk
	switch (m_iSource)
	{
	case E_VtxSource_Position:
		// get position
		pMeshConv->GetPosition(pPoly, iVtx, data);

		// update bounding box
		pMeshConv->GetBounds().UpdateExtents(Maths::HVec3(data[0], data[1], data[2]));

		return true;
	case E_VtxSource_Normal:
		// get normal
		pMeshConv->GetNormal(pPoly, iVtx, data);
		return true;
	case E_VtxSource_UV:
	case E_VtxSource_Tangent:
	case E_VtxSource_Binormal:
		// if it's a named UV set, find the name
		if (m_iIndex != -1)
			iSet = m_iIndex;
		else
			iSet = pMeshConv->FindNamedUVSet(m_sName, iPolygon);
		if (iSet == -1)
		{
			// lightmap UVs are hard-coded to suppress warnings
			if (stricmp(m_sName.Chars(), "luxlightmap"))
				PrintWarning("%s couldn't find named UV set '%s' on mesh %s.\n",
					m_pParent->GetWarningParams().Chars(), m_sName.Chars(), pMesh->GetName().c_str());
		}
		// get data
		if (m_iSource == E_VtxSource_UV || m_iSource == E_VtxSource_UVNoRemap)
			return pMeshConv->GetUV(pPoly, iVtx, iSet, data);
		else if (m_iSource == E_VtxSource_Tangent)
			return pMeshConv->GetTangent(pPoly, iVtx, iSet, data);
		else if (m_iSource == E_VtxSource_Binormal)
			return pMeshConv->GetBinormal(pPoly, iVtx, iSet, data);
		break;
	case E_VtxSource_Sense:
		{
			// if it's a named UV set, find the name
			if (m_iIndex != -1)
				iSet = m_iIndex;
			else
				iSet = pMeshConv->FindNamedUVSet(m_sName, iPolygon);

			float fNormal[4];
			float fTangent[4];
			float fBinormal[4];
			pMeshConv->GetNormal(pPoly, iVtx, fNormal);
			if (!(pMeshConv->GetTangent(pPoly, iVtx, iSet, fTangent) &&
			      pMeshConv->GetBinormal(pPoly, iVtx, iSet, fBinormal)))
				  return false;
			float fNCrossT[3];
			fNCrossT[0] = fNormal[1] * fTangent[2] - fNormal[2] * fTangent[1];
			fNCrossT[1] = fNormal[2] * fTangent[0] - fNormal[0] * fTangent[2];
			fNCrossT[2] = fNormal[0] * fTangent[1] - fNormal[1] * fTangent[0];
			float fSense = fBinormal[0] * fNCrossT[0] + fBinormal[1] * fNCrossT[1] + fBinormal[2] * fNCrossT[2];
			data[0] = (fSense > 0.0f) ? 1.0f : -1.0f;
			data[1] = 0.0f;
			data[2] = 0.0f;
			data[3] = 0.0f;
		}
		break;
	case E_VtxSource_VtxData:
		// if it's a named vertex data channel, find the name
		if (m_iIndex != -1)
			iSet = m_iIndex;
		else
			iSet = pMeshConv->FindNamedVtxDataChannel(m_sName);
		if (iSet == -1)
		{
			if (!m_pParent->m_bAllowOmit)
			{
				PrintWarning("%s couldn't find named vertex channel '%s' on mesh %s.\n",
					m_pParent->GetWarningParams().Chars(), m_sName.Chars(), pMesh->GetName().c_str());
			}
			data[0] = data[1] = data[2] = data[3] = 1.f;
		}
		else
		{
			// get data
			return pMeshConv->GetVertexData(pPoly, iVtx, iSet, data);
		}
		break;
	case E_VtxSource_Colour:
		return pMeshConv->GetVertexColours(pPoly, iVtx, data, m_sName);
		break;
	default:
		return false;
	}

	return false;
}

};

