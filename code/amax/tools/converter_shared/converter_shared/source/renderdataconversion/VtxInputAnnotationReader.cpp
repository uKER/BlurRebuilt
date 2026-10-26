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

namespace RenderDataConversion
{

/// constructor
VtxInputAnnotationReader::VtxInputAnnotationReader(Effect *pEffect, FX::InputSemantic *pSemantic)
{
	m_pEffect = pEffect;
	m_pSemantic = pSemantic;

	ParseFormatAnnotation();
}

/// this list matches the RenderingData::VertexElementType enum
/// TODO: unify this list and the list of strings that is used for XML serialisation inside the RenderingData library itself
const char *VtxInputAnnotationReader::m_pszFormatStrings[] =
{
	"S16_2",
	"S16_4",
	"S16_2N",
	"S16_4N",
	"F32_1",
	"F32_2",
	"F32_3",
	"F32_4",
	"F16_2",
	"F16_4",
	"U8_4",
	"U8_4N",
	"SHHD_3N",
	"",
};

/// store the number of scalars in a particular format
/// this list matches the RenderingData::VertexElementType enum
const int VtxInputAnnotationReader::m_nFormatScalars[] =
{
	2,
	4,
	2,
	4,
	1,
	2,
	3,
	4,
	2,
	4,
	4,
	4,
	3,
};

/// default sources for each type of semantic. used if no annotation has been specified
const DefaultVtxElement VtxInputAnnotationReader::m_DefaultElementValues[] =
{
	{ "xyz", "position", RenderingData::VERTEX_ELEMENT_TYPE_F32_3 },		// Position
	{ "" },																	// BlendWeight
	{ "" },																	// BlendIndices
	{ "xyz", "normal", RenderingData::VERTEX_ELEMENT_TYPE_F32_3 },			// Normal
	{ "" },																	// PSize
	{ "xy", "uv:%d", RenderingData::VERTEX_ELEMENT_TYPE_MAX },				// TexCoord
	{ "xyz", "tangent:%d", RenderingData::VERTEX_ELEMENT_TYPE_MAX },		// Tangent
	{ "xyz" ,"binormal:%d", RenderingData::VERTEX_ELEMENT_TYPE_MAX },		// Binormal
	{ "" },																	// TessFactor
	{ "xyzw", "colour:%d", RenderingData::VERTEX_ELEMENT_TYPE_U8_4N },		// Color
	{ "" },																	// Fog
	{ "" },																	// Depth
	{ "" },																	// Sample
};

/// source type names. this array corresponds to the E_VtxSource enum
const char *VtxInputAnnotationReader::m_pszSourceTypeNames[] =
{
	"Position",	// E_VtxSource_Position,
	"Normal",	// E_VtxSource_Normal,
	"UV",		// E_VtxSource_UV,
	"Tangent",	// E_VtxSource_Tangent,
	"Binormal",	// E_VtxSource_Binormal,
	"Sense",	// E_VtxSource_Sense,
	"VtxData",	// E_VtxSource_VtxData,
	"Colour",	// E_VtxSource_Colour
	"",
};

// typing aid
#define WARNING_PARAMS m_pEffect->GetName().Chars(), SemanticName().Chars()

// TODO: put something in that filters out duplicate warning messages

/// returns a string containing the semantic name e.g. POSITION0 (for printing warnings etc.)
Util::C_String VtxInputAnnotationReader::SemanticName()
{
	int x = m_pSemantic->Usage;
	if (x>=9)
		x++;
	Util::C_String tmp(Display::GetName((Display::E_VertexElementUsage)x));
	tmp = tmp.Upper();
	if (m_pSemantic->UsageIndex > 0)
		tmp += Util::C_String('0' + m_pSemantic->UsageIndex);
	return tmp;	
}

/// parse format annotation and set m_Type if possible. returns false if the annotation was not recognised
bool VtxInputAnnotationReader::ParseFormatAnnotation(const char *pszFormat)
{
	int x=0;
	while (m_pszFormatStrings[x][0]>0)
	{
		if (!stricmp(m_pszFormatStrings[x], pszFormat))
		{
			m_Type = (RenderingData::VertexElementType)x;
			return true;
		}
		x++;
	}

	PrintWarning("%s: Semantic '%s': unrecognised vertex element format specification '%s'.\n",
		WARNING_PARAMS, pszFormat);

	return false;
}

/// add a vertex input data source
void VtxInputAnnotationReader::AddSourceElement(XYZW *pXYZW, const char *_pszSourceDesc)
{
	if (_pszSourceDesc == NULL)
		return;

	char *pszSourceDesc = _strdup(_pszSourceDesc);
	VtxElementSource item;
	item.dest = *pXYZW;

	char *pszSource = pszSourceDesc, *pszName = "", *pszSwizzle="";

	// detect swizzle spec '.'
	char *p = strchr(pszSource, '.');
	if (p)
	{
		*p = 0;
		pszSwizzle = p+1;
	}
	// detect named source ':'
	p = strchr(pszSource, ':');
	if (p)
	{
		*p = 0;
		pszName = p+1;
	}

	// interpret the source type
	for (int i=0; true; i++)
	{
		if (m_pszSourceTypeNames[i][0]==0)
		{
			PrintWarning("%s: Semantic %s: Source type '%s' not recognised.\n",
				WARNING_PARAMS, pszSource);
			item.iSource = RenderDataConversion::E_VtxSource_None;
			break;
		}
		if (!stricmp(m_pszSourceTypeNames[i], pszSource))
		{
			// found it
			item.iSource = (E_VtxSource)i;
			break;
		}
	}

	// interpret the name/index
	for (int i=0; true; i++)
	{
		// got the the end and encountered only numbers... it's an index
		if (pszName[i] == 0)
		{
			if (i > 0)
				item.iIndex = atoi(pszName);
			else
				item.iIndex = 0;	// nothing specified - use index=0
			break;
		}
		// contains characters other than 0-9... it's a string
		if (pszName[i] < '0' || pszName[i] > '9')
		{
			item.iIndex = -1;
			item.sName = pszName;
			break;
		}
	}

	// parse swizzling string
	if (pszSwizzle[0] == 0)
		pszSwizzle = "xyzw";
	if (!ParseXYZWString(pszSwizzle, &item.src))
	{
		PrintWarning("%s: Semantic %s: Error in swizzle string '%s'.\n", WARNING_PARAMS, pszSwizzle);
	}
	if (item.src.nScalars < item.dest.nScalars)
	{
		PrintWarning("%s: Semantic %s: Swizzle string '%s' has fewer scalars than destination.\n",
			WARNING_PARAMS, pszSwizzle);
	}

	// add this input source to the list
	m_Sources.PushBack(item);

	free(pszSourceDesc);
}

/// decide on the output format based on the annotations
void VtxInputAnnotationReader::ParseFormatAnnotation()
{
	int iUsageMask = 0;
	bool bFormatSet = false;
	m_Sources.Clear();
	XYZW xyzw;

	for (int i=0; i<m_pSemantic->Annotations.Size(); i++)
	{
		FX::Annotation *pA = &m_pSemantic->Annotations[i];

		if (AnnotationReader::ParseAnnotation(pA))
			continue;
		// parse 'format' annotation
		else if (!stricmp(pA->Name, "format") && pA->Type == FX::NGLFX_ANNOTATION_STRING)
		{
			if (bFormatSet)
			{
				PrintWarning("%s: Format specified twice on input semantic %s.\n", WARNING_PARAMS );
			}

			bFormatSet = ParseFormatAnnotation(pA->String);
		}
		// parse xyzw etc. annotation
		else if (pA->Type == FX::NGLFX_ANNOTATION_STRING && ParseXYZWString(pA->Name, &xyzw))
		{
			AddSourceElement(&xyzw, pA->String);
			iUsageMask |= xyzw.iUsageMask;
		}
		else
		{
			PrintWarning("%s: Semantic %s: Unrecognised annotation '%s' specified.\n", WARNING_PARAMS, pA->Name.Chars());
		}
	}

	// if no source was set from the annotations, try using the default for this semantic
	if (!iUsageMask)
	{
		XYZW xyzw;
		if (ParseXYZWString(m_DefaultElementValues[(int)m_pSemantic->Usage].pszName, &xyzw))
		{
			Util::C_String buffer;
			buffer.Printf((char*)m_DefaultElementValues[(int)m_pSemantic->Usage].pszSource, m_pSemantic->UsageIndex);
			AddSourceElement(&xyzw, buffer.Chars());
			iUsageMask |= xyzw.iUsageMask;
		}
	}

	// if no format was set from the annotations, try using the default values for this semantic
	if (!bFormatSet)
	{
		if (m_DefaultElementValues[(int)m_pSemantic->Usage].pszName[0] > 0)
		{
			m_Type = m_DefaultElementValues[(int)m_pSemantic->Usage].type;
			if (m_Type != RenderingData::VERTEX_ELEMENT_TYPE_MAX)
				bFormatSet = true;
		}
	}

	if (bFormatSet)
	{
		// check that the number of scalars in the format matches the number of scalars in the xyzw usage
		int iFormatMask = ((1<<m_nFormatScalars[(int)m_Type])-1);
		int iCheckMask = iFormatMask & iUsageMask;
		if (iCheckMask < iFormatMask )
		{
			// usage mask has bits missing
			PrintWarning("%s: Unitialised elements on input semantic %s.\n", WARNING_PARAMS );
		}
		else if (iCheckMask < iUsageMask)
		{
			// fornat mask has bits missing
			PrintWarning("%s: Output truncated by format specification on input semantic %s.\n", WARNING_PARAMS );
		}
	}
	else
	{
		// no format was specified, so decide on an output format depending on the usage mask
		if (iUsageMask>=8)
			m_Type = RenderingData::VERTEX_ELEMENT_TYPE_F32_4;
		else if (iUsageMask>=4)
			m_Type = RenderingData::VERTEX_ELEMENT_TYPE_F32_3;
		else if (iUsageMask>=2)
			m_Type = RenderingData::VERTEX_ELEMENT_TYPE_F32_2;
		else if (iUsageMask>=1)
			m_Type = RenderingData::VERTEX_ELEMENT_TYPE_F32_1;
		else
		{
			PrintWarning("%s: No source specified on input semantic %s.\n", WARNING_PARAMS );
			m_Type = RenderingData::VERTEX_ELEMENT_TYPE_MAX;
		}
	}
}

/// get the data format for this parameter
bool VtxInputAnnotationReader::GetVertexDescElement(RenderingData::VertexFormat::VertexElement *pElement)
{
	if (m_Type == RenderingData::VERTEX_ELEMENT_TYPE_MAX)
		return false;

	pElement->usage = (RenderingData::VertexElementUsage)m_pSemantic->Usage;
	pElement->usageIndex = m_pSemantic->UsageIndex;
	pElement->type = m_Type;
	pElement->method = RenderingData::VERTEX_ELEMENT_METHOD_DEFAULT; // TODO: set this using annotation?
	return true;
}

#ifdef PLATFORM_PC
#pragma warning(disable:4244)
#endif

// template functions for converting 4 floats to a vertex input datatype
template<typename T> void CopyData1(U8 **ppData, float *pIn)
{
	((T*)(*ppData))->X(pIn[0]);
	(*ppData) += sizeof(T);
}

template<typename T> void CopyData2(U8 **ppData, float *pIn)
{
	((T*)(*ppData))->X(pIn[0]);
	((T*)(*ppData))->Y(pIn[1]);
	(*ppData) += sizeof(T);
}

template<typename T> void CopyData3(U8 **ppData, float *pIn)
{
	((T*)(*ppData))->X(pIn[0]);
	((T*)(*ppData))->Y(pIn[1]);
	((T*)(*ppData))->Z(pIn[2]);
	(*ppData) += sizeof(T);
}

template<typename T> void CopyData4(U8 **ppData, float *pIn)
{
	((T*)(*ppData))->X(pIn[0]);
	((T*)(*ppData))->Y(pIn[1]);
	((T*)(*ppData))->Z(pIn[2]);
	((T*)(*ppData))->W(pIn[3]);
	(*ppData) += sizeof(T);
}

/// get data from aftk vertex in vertex buffer format
/// data pointer is automatically incremented
bool VtxInputAnnotationReader::GetVertexData(const AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, U8 **ppData)
{
	const Aftk::C_SceneMeshNode *pMesh = pMeshConv->GetMeshNode();

	// for each vertex
	const Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(0, iPolygon);

	bool bOk = false;

	// for each data source
	float combinedData[4] = {0};
	for (int i=0; i<m_Sources.Size(); i++)
	{
		float data[4] = {0};
		VtxElementSource *pSrc = &m_Sources[i];
		int iSet;

		// retrieve data from aftk
		switch (pSrc->iSource)
		{
		case E_VtxSource_Position:
			// get position
			pMeshConv->GetPosition(pPoly, iVtx, data);
			bOk = true;
			break;
		case E_VtxSource_Normal:
			// get normal
			pMeshConv->GetNormal(pPoly, iVtx, data);
			bOk = true;
			break;
		case E_VtxSource_UV:
		case E_VtxSource_Tangent:
		case E_VtxSource_Binormal:
			// if it's a named UV set, find the name
			if (pSrc->iIndex != -1)
				iSet = pSrc->iIndex;
			else
				iSet = pMeshConv->FindNamedUVSet(pSrc->sName, iPolygon);
			if (iSet == -1)
			{
				// lightmap UVs are hard-coded to suppress warnings
				if (stricmp(pSrc->sName.Chars(), "luxlightmap"))
					PrintWarning("%s: Semantic %s couldn't find named UV set '%s' on mesh %s.\n",
						WARNING_PARAMS, pSrc->sName.Chars(), pMesh->GetName().c_str());
			}
			// get data
			if (pSrc->iSource == E_VtxSource_UV)
				bOk |= pMeshConv->GetUV(pPoly, iVtx, iSet, data);
			else if (pSrc->iSource == E_VtxSource_Tangent)
				bOk |= pMeshConv->GetTangent(pPoly, iVtx, iSet, data);
			else if (pSrc->iSource == E_VtxSource_Binormal)
				bOk |= pMeshConv->GetBinormal(pPoly, iVtx, iSet, data);
			break;
		case E_VtxSource_Sense:
			{
				float fNormal[4];
				float fTangent[4];
				float fBinormal[4];
				pMeshConv->GetNormal(pPoly, iVtx, fNormal);
				bOk |= pMeshConv->GetTangent(pPoly, iVtx, iSet, fTangent);
				bOk |= pMeshConv->GetBinormal(pPoly, iVtx, iSet, fBinormal);
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
			if (pSrc->iIndex != -1)
				iSet = pSrc->iIndex;
			else
				iSet = pMeshConv->FindNamedVtxDataChannel(pSrc->sName);
			if (iSet == -1)
			{
				if (!m_bAllowOmit)
				{
					PrintWarning("%s: Semantic %s couldn't find named vertex channel '%s' on mesh %s.\n",
						WARNING_PARAMS, pSrc->sName.Chars(), pMesh->GetName().c_str());
				}
				data[0] = data[1] = data[2] = data[3] = 1.f;
			}
			else
			{
				// get data
				bOk |= pMeshConv->GetVertexData(pPoly, iVtx, iSet, data);
			}
			break;
		case E_VtxSource_Colour:
			bOk |= pMeshConv->GetVertexColours(pPoly, iVtx, data);
			break;
		}

		// merge into combined data
		for (int i=0; i<pSrc->dest.nScalars && i<pSrc->src.nScalars; i++)
			combinedData[pSrc->dest.iMapping[i]] = data[pSrc->src.iMapping[i]];
	}

	if (!ppData)
		return bOk;

	// TODO: think about remapping UVs nearer the origin for F16 etc.

	// convert combined data into output format and copy to output buffer
	switch (m_Type)
	{
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_2:		CopyData2<RenderingData::DataTypes::S16_2>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_4:		CopyData4<RenderingData::DataTypes::S16_4>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_2N:		CopyData2<RenderingData::DataTypes::S16_2N>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_4N:		CopyData2<RenderingData::DataTypes::S16_4N>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F32_1:		CopyData1<RenderingData::DataTypes::F32_1>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F32_2:		CopyData2<RenderingData::DataTypes::F32_2>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F32_3:		CopyData3<RenderingData::DataTypes::F32_3>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F32_4:		CopyData4<RenderingData::DataTypes::F32_4>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F16_2:		CopyData2<RenderingData::DataTypes::F16_2>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_F16_4:		CopyData4<RenderingData::DataTypes::F16_4>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_U8_4:		CopyData4<RenderingData::DataTypes::U8_4>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_U8_4N:		CopyData4<RenderingData::DataTypes::U8_4N>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_SHHD_3N:	CopyData3<RenderingData::DataTypes::SHHD_3N>(ppData, combinedData); break;
	}

	return bOk;
}

};

