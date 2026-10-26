#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>
#include <renderingdata/renderingdata.h>
#include <system/misc/StringUtil.h>

#include "converter_shared/renderdataconversion/Common.h"
#include "converter_shared/renderdataconversion/ConvertAftkMesh.h"
#include "converter_shared/renderdataconversion/Effect.h"

#include "converter_shared/Helpers/aftkMaterialHelpers.h"

namespace RenderDataConversion
{

/// constructor
AftkMeshConverter::AftkMeshConverter(const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_SceneNode *pInstance)
{
	m_pMesh = pMesh;
	m_pInstance = pInstance;
	m_bCalculatedTangentSpace = false;
}

/// destructor
AftkMeshConverter::~AftkMeshConverter()
{
}

// initialise matrices
void AftkMeshConverter::SetMatrices()
{
	// calculate default matrices depending on whether this is instanced mesh
	if(m_pMesh->GetNumInstancesOfThisNode() > 0)
	{
		// set identity matrices for instances
		UtMaths::SetIdentity(&m_PositionMatrix);
		UtMaths::SetIdentity(&m_NormalMatrix);
	}
	else
	{
		// set indentity local matrix
		Maths::HMat44 identity;
		SetMatrices(identity);
	}
}

// initialise matrices with external local matrix
void AftkMeshConverter::SetMatrices(const Maths::HMat44& localMatrix)
{
	Maths::HMat44 world;
	// get world matrix 
	Aftk::TemplateToNightmareMatrix44(world, m_pMesh->GetWorldMatrix());
	SetMatrices(world, localMatrix);
}

// private internal set matrix
void AftkMeshConverter::SetMatrices(const Maths::HMat44& worldMatrix, const Maths::HMat44& localMatrix)
{
	Maths::HMat44 localMatrixInverse(localMatrix);
	localMatrixInverse = localMatrixInverse.Inverse();

	Maths::HMat44 positionMatrix = worldMatrix * localMatrixInverse;

	Maths::HMat44 normalMatrix = positionMatrix;
	normalMatrix = normalMatrix.Inverse();
	normalMatrix.Transpose();
	
	Aftk::NightmareToTemplateMatrix44(m_PositionMatrix, positionMatrix);
	Aftk::NightmareToTemplateMatrix44(m_NormalMatrix, normalMatrix);
}

/// get pointer to current meshnode
const Aftk::C_SceneMeshNode *AftkMeshConverter::GetMeshNode() const
{
	return m_pMesh;
}

/// get vertex position
void AftkMeshConverter::GetPosition(const Aftk::C_Polygon *pPoly, int iVtx, float *pData, int nComponents) const
{
	const Aftk::C_PolygonVertex *pVtx = &pPoly->GetPolygonVertex(iVtx);

	Aftk::Position3 pos = m_pMesh->GetVertexPosition(pVtx->GetPositionIndex());
	pos = Aftk::ApplyMatrix(m_PositionMatrix, pos);

	for (int i = 0; i < nComponents; i++)
	{
		if (i < 3)
			pData[i] = pos[i];
		else
			pData[3] = 1.f;
	}
}

/// get vertex normal
void AftkMeshConverter::GetNormal(const Aftk::C_Polygon *pPoly, int iVtx, float *pData) const
{
	const Aftk::C_PolygonVertex *pVtx = &pPoly->GetPolygonVertex(iVtx);

	Aftk::Normal3 aftkNormal = m_pMesh->GetVertexNormal(pVtx->GetNormalIndex());
	aftkNormal = Aftk::ApplyMatrix(m_NormalMatrix, aftkNormal);
	Aftk::Normalise(aftkNormal);

	pData[0] = aftkNormal.x;
	pData[1] = aftkNormal.y;
	pData[2] = aftkNormal.z;
	pData[3] = 0.f;
}

/// get UV
bool AftkMeshConverter::GetUV(const Aftk::C_Polygon *pPoly, int iVtx, int iUVSet, float *pData, int nComponents) const
{
	const Aftk::C_PolygonVertex *pVtx = &pPoly->GetPolygonVertex(iVtx);

	if (iUVSet<0 || iUVSet>=(int)pVtx->GetNumTextureCoordinateIndices())
	{
		// UV set not found - set to 0
		for (int i = 0; i < nComponents; i++)
			pData[i] = 0.f;

		return false;
	}

	S32 texCoordIndex = pVtx->GetTextureCoordinateIndex(iUVSet);
	const Aftk::TextureCoordinate4 aftkTexCoord = m_pMesh->GetTextureCoordinate(iUVSet, texCoordIndex);

	for (int i = 0; i < nComponents; i++)
		pData[i] = aftkTexCoord[i];

	return true;
}

/// get tangent
bool AftkMeshConverter::GetTangent(const Aftk::C_Polygon *pPoly, int iVtx, int iUVSet, float *pData) const
{
	const Aftk::C_PolygonVertex *pVtx = &pPoly->GetPolygonVertex(iVtx);

	if (iUVSet<0 || iUVSet>=(int)pVtx->GetNumTextureCoordinateIndices())
	{
		// UV set not found - set to 0
		pData[0] = pData[1] = pData[2] = pData[3] = 0.f;
		return false;
	}
	S32 tangentIndex = pVtx->GetTangentIndex(iUVSet);
	if (tangentIndex == -1)
	{
		Aftk::C_MeshCalculateTangents::CalculateTangents(const_cast<Aftk::C_SceneMeshNode*>(m_pMesh), iUVSet, true, false);
		pVtx = &pPoly->GetPolygonVertex(iVtx);
		tangentIndex = pVtx->GetTangentIndex(iUVSet);
	}
	Aftk::Normal3 aftkTangent = (Aftk::Normal3&)m_pMesh->GetVertexTangent(iUVSet, tangentIndex);
	
	aftkTangent = Aftk::ApplyMatrix(m_NormalMatrix, aftkTangent);
	Aftk::Normalise(aftkTangent);
	
	pData[0] = aftkTangent.x;
	pData[1] = aftkTangent.y;
	pData[2] = aftkTangent.z;
	return true;
}

/// get binormal
bool AftkMeshConverter::GetBinormal(const Aftk::C_Polygon *pPoly, int iVtx, int iUVSet, float *pData) const
{
	const Aftk::C_PolygonVertex *pVtx = &pPoly->GetPolygonVertex(iVtx);

	if (iUVSet<0 || iUVSet>=(int)pVtx->GetNumTextureCoordinateIndices())
	{
		// UV set not found - set to 0
		pData[0] = pData[1] = pData[2] = pData[3] = 0.f;
		return false;
	}
	S32 binormalIndex = pVtx->GetBinormalIndex(iUVSet);
	if (binormalIndex == -1)
	{
		Aftk::C_MeshCalculateTangents::CalculateTangents(const_cast<Aftk::C_SceneMeshNode*>(m_pMesh), iUVSet, true, false);
		pVtx = &pPoly->GetPolygonVertex(iVtx);
		binormalIndex = pVtx->GetBinormalIndex(iUVSet);
	}
	Aftk::Normal3 aftkBinormal = (Aftk::Normal3&)m_pMesh->GetVertexBinormal(iUVSet, binormalIndex);
	//if(!m_bLocalSpace)
	aftkBinormal = Aftk::ApplyMatrix(m_NormalMatrix, aftkBinormal);
	Aftk::Normalise(aftkBinormal);
	pData[0] = aftkBinormal.x;
	pData[1] = aftkBinormal.y;
	pData[2] = aftkBinormal.z;
	return true;
}

/// get vertex data channel
bool AftkMeshConverter::GetVertexData(const Aftk::C_Polygon *pPoly, int iVtx, int iChannel, float *pData) const
{
	const Aftk::C_PolygonVertex *pVtx = &pPoly->GetPolygonVertex(iVtx);

	Aftk::Vector4F32 colour;
	if (iChannel<0 || iChannel >= (int)pVtx->GetNumData() ||
		(pPoly->GetMeshNodePtr()->GetVertexDataChannelInfo(iChannel).GetDataType() != Aftk::E_GenericAttributeType_Vector4F32 &&
		pPoly->GetMeshNodePtr()->GetVertexDataChannelInfo(iChannel).GetDataType() != Aftk::E_GenericAttributeType_U32))
	{
		// vertex data channel not found - set to white
		pData[0] = pData[1] = pData[2] = pData[3] = 1.f;
		return false;
	}
	if (pPoly->GetMeshNodePtr()->GetVertexDataChannelInfo(iChannel).GetDataType() == Aftk::E_GenericAttributeType_U32)
	{
		U32 uColour = 0xFFFFFFFF;
		pVtx->GetData(uColour, iChannel);
		colour = Aftk::ConvertU32ToVector4F32(uColour);
	}
	else
	{
		pVtx->GetData(colour, iChannel);
	}

	pData[0] = colour.x;
	pData[1] = colour.y;
	pData[2] = colour.z;
	pData[3] = colour.w;
	return true;
}

/// get vertex colour data - this has had to be hard-coded a bit so it can understand how to pull vertex colour off instances
bool AftkMeshConverter::GetVertexColours(const Aftk::C_Polygon *pPoly, int iVtx, float *pData) const
{
	// by default get the vertex colours from mesh
	int iVtxColChannel = FindNamedVtxDataChannel("vertexColour");

	// get the vertex colours from an instance -- EWW nasty special-case hacky code
	if (m_pInstance != NULL)
	{
		const Aftk::C_GenericAttribute *pVtxColAttr = AftkHelpers::FindAttribute(*m_pInstance, "VertexColourChannel");
		if (pVtxColAttr)
		{
			// find the vertex data channel on the source object that contains the instance's vertex colours
			std::string vtxChanName;
			pVtxColAttr->GetValue(vtxChanName);
			iVtxColChannel = FindNamedVtxDataChannel(vtxChanName.c_str());
		}
	}

	return GetVertexData(pPoly, iVtx, iVtxColChannel, pData);
}

/// return the index of the UV set with the given name
int AftkMeshConverter::FindNamedUVSet(const char *pszName, int iPolygon) const
{
	for (U32 i = 0; i < m_pMesh->GetNumTextureCoordinateSets(); i++)
	{
		const Aftk::C_TextureCoordinateSet& set = m_pMesh->GetTextureCoordinateSet(i);
		
		if (!strcmp(set.GetName().c_str(), pszName))
		{
			return i;
		}
	}

	//XSI
	if (iPolygon > -1)
	{
		const Aftk::C_Polygon& aftkPolygon = m_pMesh->GetPolygon(0, iPolygon);
		if (const Aftk::C_MaterialResource *pMtlRes = AftkHelpers::GetMaterialResourcePtr(&aftkPolygon))
		{		
			for (U32 iShader = 0; iShader < pMtlRes->GetContent().GetNumShaderNodes(); iShader++)
			{
				const Aftk::C_ShaderNode* pShader = pMtlRes->GetContent().GetShaderNodePtr(iShader);

				if (pShader->GetName() == pszName)
				{
					if (const Aftk::C_DynamicPlug* pPlug = pShader->GetDynamicPlugPtr("tspace_id"))
					{
						std::string projectionName;
						pPlug->GetValue(projectionName);
						return FindNamedUVSet(projectionName.c_str(), -1);
					}	
				}
			}										
		}
	}

	return -1; // not found
}

/// return the index of the vertex data channel with the given name
int AftkMeshConverter::FindNamedVtxDataChannel(const char *pszName) const
{
	for (U32 i = 0; i < m_pMesh->GetNumVertexDataChannels(); i++)
	{
		const Aftk::C_VertexDataChannelInfo& info = m_pMesh->GetVertexDataChannelInfo(i);
		
		if (!strcmp(info.GetName().c_str(), pszName))
		{
			return i;
		}
	}
	return -1; // not found
}

/// convert the given polygon into a renderingdata primitive info which can be added into a renderingdata model
void AftkMeshConverter::GetVertexData(Effect *pEffect, int iPolygon, RenderingData::PrimitiveInfo *pPrim, E_NodeType type) const
{
	// set a single stream
	pPrim->streams.Destroy();
	pPrim->streams.Resize(1);
	RenderingData::PrimitiveInfo::StreamInfo& stream = pPrim->streams.Back();
	stream.streamIndex = 0;

	// get vertex format
	const RenderingData::VertexFormat *pVertexFormat = pEffect->GetVertexFormat(type);
	stream.vertices.Create(*pVertexFormat);
	stream.vertices.Resize(3);

	// get data for each vertex
	int cVtxData = pVertexFormat->GetDataSize();
	U8 *pBuffer = (U8*)alloca(cVtxData);
	for (int iVertex=0; iVertex<3; iVertex++)
	{
		U8 *pData = pBuffer;
		pEffect->GetVertexData(this, iPolygon, iVertex, type, &pData);
		assert(pData-pBuffer == cVtxData);

		stream.vertices[iVertex].SetData(pBuffer, cVtxData);
	}

	pPrim->CalculateBoundsFromVertices();
}

// Calculate non-smoothed and unnormalised tangent and binormal for a polygon. 
void AftkMeshConverter::CalculateTangentandBiNormal(const Aftk::C_Polygon& aftkPolygon, int vertex, int set, Aftk::Vector3F32* tangent, Aftk::Vector3F32* binormal)
{
	UtMaths::MMatrix43 m;
	Aftk::Vector3F32 t;
	Aftk::Vector3F32 b;

	// get next and previous vertices
	int i0 = vertex;
	int i1 = vertex + 1; 
	int i2 = vertex - 1;
	if (i1 >= (int) aftkPolygon.GetNumPolygonVertices()) i1 -= aftkPolygon.GetNumPolygonVertices();
	if (i2 < 0) i2 += aftkPolygon.GetNumPolygonVertices();

	Aftk::Position3 p0, p1, p2;

	GetPosition(&aftkPolygon, i0, (float*) &p0, 3);
	GetPosition(&aftkPolygon, i1, (float*) &p1, 3);
	GetPosition(&aftkPolygon, i2, (float*) &p2, 3);

	Aftk::Vector3F32 normal = (p0 - p1) ^ (p0 - p2);

	Aftk::Vector2F32 t0, t1, t2;

	GetUV(&aftkPolygon, i0, set, (float*) &t0, 2);
	GetUV(&aftkPolygon, i1, set, (float*) &t1, 2);
	GetUV(&aftkPolygon, i2, set, (float*) &t2, 2);

	Aftk::Vector3F32 r0 = p1 - p0;
	Aftk::Vector3F32 r1 = p2 - p0;

	m.AX(UtMaths::MMatrix43::DataType(r0[0], r0[1], r0[2], t0.x - t1.x));
	m.AY(UtMaths::MMatrix43::DataType(r1[0], r1[1], r1[2], t0.x - t2.x));
	m.AZ(UtMaths::MMatrix43::DataType(normal.X(), normal.Y(), normal.Z(), 0.f));

	UtMaths::GESolve(m, &t);
	*tangent = t;

	m.AX(UtMaths::MMatrix43::DataType(r0[0], r0[1], r0[2], t0.y - t1.y));
	m.AY(UtMaths::MMatrix43::DataType(r1[0], r1[1], r1[2], t0.y - t2.y));
	m.AZ(UtMaths::MMatrix43::DataType(normal.X(), normal.Y(), normal.Z(), 0.f));

	UtMaths::GESolve(m, &b);
	*binormal = b;
}

// Returns pair of texture scale, area.
// texture scale = size in world space of one unit in texture space.
// area = size of triangle in uv space.
void AftkMeshConverter::CalculateTextureScale(const Aftk::C_Polygon& aftkPolygon, int set, TextureScale& output)
{
	Aftk::Vector3F32 tangent, binormal;

	// Don't want to use smoothed tangents for this so calculate our own.
	CalculateTangentandBiNormal(aftkPolygon, 0, set, &tangent, &binormal);

	// Sometimes get zero for tangents - on degenerate triangles?
	// Don't want this to skew the results, so return something sensible.
	if (Magnitude(tangent) < Maths::Constants<float>::Epsilon() || Magnitude(binormal) < Maths::Constants<float>::Epsilon())
	{
		output.uScale = 1.f;
		output.vScale = 1.f;
		output.weight = 0.f;
		return;
	}

	Aftk::Vector3F32 t0, t1, t2;

	GetUV(&aftkPolygon, 0, set, (float*) &t0, 2);
	GetUV(&aftkPolygon, 1, set, (float*) &t1, 2);
	GetUV(&aftkPolygon, 2, set, (float*) &t2, 2);

	t0[2] = t1[2] = t2[2] = 0.f;

	float textureArea = Magnitude((t0 - t1) ^ (t0 - t2)) * 0.5f;
	float mt = Magnitude(tangent);
	float mb = Magnitude(binormal);

	float uScale = 1.f / mt;
	float vScale = 1.f / mb;

	// bias against small areas
	float weight = textureArea * (1.f / mt + 1.f / mb);

	output.uScale = uScale;
	output.vScale = vScale;
	output.weight = weight;

	return;
}


};
