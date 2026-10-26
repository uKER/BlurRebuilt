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
#include "converter_shared/renderdataconversion/CustomSemanticManager.h"

//#pragma optimize("", off)

namespace RenderDataConversion
{

/// constructor
VtxInputAnnotationReader::VtxInputAnnotationReader(Effect *pEffect, FX::InputSemantic *pSemantic)
{
	m_pEffect = pEffect;
	m_pSemantic = pSemantic;
	m_bBias = false;

	ParseFormatAnnotation();
}

/// destructor
VtxInputAnnotationReader::~VtxInputAnnotationReader()
{
	for (int i=0; i<m_Sources.Size(); i++)
		delete m_Sources[i].pSource;
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
	{ "xyz", "normal", RenderingData::VERTEX_ELEMENT_TYPE_SHHD_3N },		// Normal
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

// typing aid
#define WARNING_PARAMS m_pEffect->GetName().Chars(), GetSemanticName(m_pSemantic).Chars()

/// return a string that can identify this vtx input annotation so we can print out a warning about it
Util::C_String VtxInputAnnotationReader::GetWarningParams()
{
	Util::C_String tmp;
	tmp.Printf("%s: Semantic %s", m_pEffect->GetName().Chars(), GetSemanticName(m_pSemantic).Chars());
	return tmp;
}

/// return all of the annotations in a string (for debugging)
Util::C_String VtxInputAnnotationReader::DebugDesc()
{
	Util::C_String sRtn;
	Btl::Vector<FX::Annotation> &annotations = m_pSemantic->Annotations;
	for (int i=0; i<annotations.Size(); i++)
	{
		Util::C_String sTmp;
		switch (annotations[i].Type)
		{
		case FX::NGLFX_ANNOTATION_STRING:
			sTmp.Printf("%s:%s ", annotations[i].Name.Chars(), annotations[i].String);
			break;
		case FX::NGLFX_ANNOTATION_INT:
			sTmp.Printf("%s:%d ", annotations[i].Name.Chars(), annotations[i].Int);
			break;
		default:
			sTmp.Printf("%s ", annotations[i].Name);
			break;
		}
		sRtn += sTmp;
	}
	return sRtn;
}

// TODO: put something in that filters out duplicate warning messages

/// returns a string containing the semantic name e.g. POSITION0 (for printing warnings etc.)
Util::C_String GetSemanticName(FX::InputSemantic *pSemantic)
{
	int x = pSemantic->Usage;
	if (x>=9)
		x++;
	Util::C_String tmp(Display::GetName((Display::E_VertexElementUsage)x));
	tmp = tmp.Upper();
	if (pSemantic->UsageIndex > 0)
		tmp += Util::C_String('0' + pSemantic->UsageIndex);
	return tmp;	
}

/// parse format annotation and set m_Type if possible. returns false if the annotation was not recognised
bool VtxInputAnnotationReader::ParseFormatAnnotation(const char *_pszFormat)
{
	char pszFormat[1024];
	strcpy(pszFormat, _pszFormat);
	char *pszDot = strchr(pszFormat, '.');
	if (pszDot)
	{
		if (!stricmp(pszDot+1, "bias"))
			m_bBias = true;
		else
		{
			PrintWarning("%s: Semantic '%s': unrecognised vertex element format modifier '%s'.\n",
				WARNING_PARAMS, pszDot+1);
		}
		*pszDot = 0;
	}

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

	VtxSourceSemantic item;

	char *pszSourceDesc = _strdup(_pszSourceDesc);
	item.dest = *pXYZW;

	char *pszSource = pszSourceDesc, *pszName = "", *pszSwizzle="";

	// detect swizzle spec '.'
	char *p = strchr(pszSource, '.');
	if (p)
	{
		*p = 0;
		pszSwizzle = p+1;
	}

	item.pSource = g_CustomSemanticManager.FindVtxInputSemanticHandler(this, pszSource);
	if (!item.pSource)
	{
		PrintWarning("%s: Source type '%s' not recognised.\n",
			GetWarningParams().Chars(), pszSource);
		free(pszSourceDesc);
		return;
	}

	// parse swizzling string
	if (pszSwizzle[0] == 0)
		pszSwizzle = "xyzw";
	if (!ParseXYZWString(pszSwizzle, &item.src))
	{
		PrintWarning("Semantic %s: Error in swizzle string '%s'.\n", _pszSourceDesc, pszSwizzle);
	}
	if (item.src.nScalars < item.dest.nScalars)
	{
		PrintWarning("Semantic %s: Swizzle string '%s' has fewer scalars than destination.\n",
			_pszSourceDesc, pszSwizzle);
	}

	free(pszSourceDesc);

	// add this input source to the list
	m_Sources.PushBack(item);
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
bool VtxInputAnnotationReader::GetVertexDescElement(RenderingData::VertexFormat::VertexElement *pElement, S32 *piStream)
{
	if (m_Type == RenderingData::VERTEX_ELEMENT_TYPE_MAX)
		return false;

	pElement->usage = (RenderingData::VertexElementUsage)m_pSemantic->Usage;
	pElement->usageIndex = m_pSemantic->UsageIndex;
	pElement->type = m_Type;
	pElement->method = RenderingData::VERTEX_ELEMENT_METHOD_DEFAULT; // TODO: set this using annotation?
	*piStream  = m_iStream;
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

/// remap UVs that are going outside of the F16 floating point range back nearer the [0,1] range
static void RemapUVs(float *pf, int *pi, int ni, float maxUV = 32.f)
{
	for (int i=0; i<ni; i++)
	{
		int o = pi[i];
		float min = FLT_MAX;
		float max = -FLT_MAX;

		for (int x=0; x<3; x++)
		{
			float t = pf[x*4+o];
			min = Maths::Min(min, t); 
			max = Maths::Max(max, t); 
		}

		S32 n = 0;

		if (min > 0.f)
			n = S32(min / maxUV);
		else if (max < 0.f)
			n = S32(max / maxUV);

		if (n != 0)
		{
			for (int x=0; x<3; x++)
			{
				float* t = &pf[x*4+o];
				*t = *t - n * maxUV;
			}
		}
	}
}

/// get data from aftk vertex in vertex buffer format
/// data pointer is automatically incremented
bool VtxInputAnnotationReader::GetVertexData(const AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, U8 **ppData)
{
	bool bOk = false;

	// for each data source
	//float combinedData[4];
	float combinedData[4] = {0};
	for (int i=0; i<m_Sources.Size(); i++)
	{
		VtxSourceSemantic *pSource = &m_Sources[i];

		//float data[4];
		float data[4] = {0};

		// re-center UVs nearer the origin for F16 etc.
		if (pSource->pSource->DoRecentering() && (m_Type == RenderingData::VERTEX_ELEMENT_TYPE_F16_2 || m_Type == RenderingData::VERTEX_ELEMENT_TYPE_F16_4))
		{
			//float vTmp[3][4];
			float vTmp[3][4] = {0};
			bOk |= pSource->pSource->GetData(pMeshConv, iPolygon, iVtx, vTmp[0]);
			bOk |= pSource->pSource->GetData(pMeshConv, iPolygon, (iVtx+1)%3, vTmp[1]);
			bOk |= pSource->pSource->GetData(pMeshConv, iPolygon, (iVtx+2)%3, vTmp[2]);
			RemapUVs(vTmp[0], pSource->src.iMapping, pSource->src.nScalars);
			memcpy(data, vTmp, sizeof(F32)*4);
		}
		else
			bOk |= pSource->pSource->GetData(pMeshConv, iPolygon, iVtx, data);

		// merge into combined data
		for (int i=0; i<pSource->dest.nScalars && i<pSource->src.nScalars; i++)
			combinedData[pSource->dest.iMapping[i]] = data[pSource->src.iMapping[i]];
	}

	if (!ppData)
		return bOk;

	if (m_bBias)
	{
		for (int i=0; i<4; i++)
		{
			combinedData[i] = (combinedData[i]*0.5f)+0.5f;
			combinedData[i] = Maths::Clamp01(combinedData[i]);
		}
	}

	// convert combined data into output format and copy to output buffer
	switch (m_Type)
	{
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_2:		CopyData2<RenderingData::DataTypes::S16_2>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_4:		CopyData4<RenderingData::DataTypes::S16_4>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_2N:		CopyData2<RenderingData::DataTypes::S16_2N>(ppData, combinedData); break;
	case RenderingData::VERTEX_ELEMENT_TYPE_S16_4N:		CopyData4<RenderingData::DataTypes::S16_4N>(ppData, combinedData); break;
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

