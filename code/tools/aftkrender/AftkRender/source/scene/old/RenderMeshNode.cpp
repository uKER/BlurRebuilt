#include "stdafx.h"
#include "AftkRender.h"
#include "Shaders/vsConstants.h"
#include "Utility.h"

using namespace AftkRender;

static const U32 MAX_VERTEX_BUFFER_SIZE = 65535;

static void GetVertexColourChannelAndType(
	const Aftk::C_SceneNode *pAftkSceneNode,
	S32& channel,
	Aftk::E_GenericAttributeType& type)
{
	Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*) pAftkSceneNode;
	//Look for the first vertex data channel.
	channel = -1;
	type = Aftk::E_GenericAttributeType_Unknown;
	for (U32 iChannel = 0; iChannel < pAftkMesh->GetNumVertexDataChannels(); iChannel++)
	{
		const Aftk::C_VertexDataChannelInfo* pChannelInfo = 
			&pAftkMesh->GetVertexDataChannelInfo(iChannel);
		if (pChannelInfo->GetName() == "vertexColour")
		{
			channel = iChannel;
			type = pChannelInfo->GetDataType();
			break;
		}
	}
}

static U32 GetLightMapUVSet(
	const Aftk::C_SceneMeshNode *pAftkSceneMeshNode)
{
	U32 lightMapUVSet = -1;
	for (U32 i=0; i<pAftkSceneMeshNode->GetNumTextureCoordinateSets(); ++i)
	{
		if (pAftkSceneMeshNode->GetTextureCoordinateSetName(i) == "luxlightmap")
		{
			lightMapUVSet = i;
			break;
		}
	}
	return lightMapUVSet;
}


C_RenderMeshNode::C_RenderMeshNode(
	Aftk::C_SceneNode *pAftkNode,
	Aftk::C_SceneMeshRemappableNode *pAftkMaterialMappingSouce,
	C_SceneRenderer *pRenderer)
:
	C_RenderMeshRemappableNode(pAftkNode, pAftkMaterialMappingSouce, pRenderer)
{
	Aftk::C_SceneMeshNode *pAftkMeshNode = (Aftk::C_SceneMeshNode*) m_pAftkSceneNode;

	m_nUVSets = pAftkMeshNode->GetNumTextureCoordinateSets();

	m_RenderNodeFlags.isSkinned = pAftkMeshNode->IsSkinned();
	m_RenderNodeFlags.isSkinAnimated = pAftkMeshNode->IsSkinAnimated();

	// Define vertex format...
	bool hasColourChannel=false, has2UVs=false;

	// Stream 0 - Vertex geometry ...
	m_VertexFormat.AddElement(m_VBStreamUsage.Size(), Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	m_VertexFormat.AddElement(m_VBStreamUsage.Size(), Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Normal, 0);

	// ... and vertex colours (if present)
	S32 vertexColourChannel;
	Aftk::E_GenericAttributeType vertexColourType;
	GetVertexColourChannelAndType(pAftkMeshNode, vertexColourChannel, vertexColourType);

	if (vertexColourChannel != -1)
	{
		m_VertexFormat.AddElement(m_VBStreamUsage.Size(), Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
		m_VBStreamUsage.PushBack(VBStream::E_VertexStreamUsage_Position_Normals_Colour);
	}
	else
		m_VBStreamUsage.PushBack(VBStream::E_VertexStreamUsage_Position_Normals);


	//Texcoords ...
	{
		const U32 MAX_TEXCOORDINATE_SETS = 8;
		for (U32 i=0; i<m_nUVSets && i<MAX_TEXCOORDINATE_SETS; i++)
		{
			m_VertexFormat.AddElement(m_VBStreamUsage.Size(), Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, i);
		}
		m_VBStreamUsage.PushBack(VBStream::E_VertexStreamUsage_UVs);
	}

	//Vertex weights & blend indices
	if (m_RenderNodeFlags.isSkinned)
	{
		m_VertexFormat.AddElement(m_VBStreamUsage.Size(), Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_BlendWeight, 0);
		m_VertexFormat.AddElement(m_VBStreamUsage.Size(), Display::E_VertexElementType_U8_4, Display::E_VertexElementUsage_BlendIndices, 0);
		m_VBStreamUsage.PushBack(VBStream::E_VertexStreamUsage_Weights_PaletteIndices);
	}

	//Vertex tangents & binormals
	{
		m_VertexFormat.AddElement(m_VBStreamUsage.Size(), Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Tangent, 0);
		m_VertexFormat.AddElement(m_VBStreamUsage.Size(), Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Binormal, 0);
		m_VBStreamUsage.PushBack(VBStream::E_VertexStreamUsage_Tangents_Binormals);
	}

	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(m_VertexFormat);
}


C_RenderMeshNode::~C_RenderMeshNode()
{
	Clear();
}


void C_RenderMeshNode::Clear()
{
	DestroyVertexBuffers();
	DestroyMeshClusters();
	m_LodPolyToVBMap.Clear();
	m_VBToPolyVertexMap.Clear();
	m_LodMeshClusters.Clear();
}


Aftk::C_Status C_RenderMeshNode::Rebuild()
{
	Aftk::C_Status status;

	//Clear the proporties of this class that are affected by the nature of the
	//currently loaded scene. Note mesh clusters aren't cleared
	Clear();

	//Clear out the contents of any existing mesh clusters
	status = C_RenderMeshRemappableNode::Rebuild();
	if (!status)
		return status;

	for (U32 iLod=0; iLod<(U32)m_LodMeshClusters.Size(); ++iLod)
	{
		for (U32 jCluster=0; jCluster<(U32)m_LodMeshClusters[iLod].Size(); ++jCluster)
		{
			MeshClusterRecord& r = m_LodMeshClusters[iLod][jCluster];
			if (r.pCluster)
				r.pCluster->Clear();
		}
	}

	Aftk::C_SceneMeshNode* pAftkMeshNode = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
	//Skip empty mesh
	{
		U32 polyCount = 0;
		for (U32 iLod=0; iLod<pAftkMeshNode->GetNumLodLevels(); ++iLod)
		{
			polyCount += pAftkMeshNode->GetNumPolygons(iLod);
		}
		if (polyCount == 0)
		{
			//m_nodeFlags.isBuilt = true;
			return Aftk::C_Status::E_Status_Success;
		}
	}
	
	status = Aftk::C_MeshCalculateTangents::CalculateTangents(pAftkMeshNode,true,true);
	if (!status) return status;

	status = pAftkMeshNode->BuildAnimationHelpers();
	if (!status) return status;

	if (pAftkMeshNode->IsSkinned())
	{
		status = pAftkMeshNode->Evaluate();
		if (!status) return status;
	}

	status = CreateVertexBuffers();
	if (!status) return status;

	status = FillVertexBuffers();
	if (!status) return status;

	status = BuildMeshClusters();
	if (!status) return status;

	//m_nodeFlags.isBuilt = true;
	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_RenderMeshNode::Render(U32 lodLevel)
{
	if (m_LodMeshClusters.Size() == 0)
		return Aftk::C_Status::E_Status_Success;

	Aftk::C_Status status;
	for (U32 iCluster=0; iCluster<(U32)m_LodMeshClusters[lodLevel].Size(); ++iCluster)
	{
		if (m_LodMeshClusters[lodLevel][iCluster].pCluster)
		{
			status = m_LodMeshClusters[lodLevel][iCluster].pCluster->Render();
			if (!status)
				return status;
		}
	}
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_RenderMeshNode::Render(U32 lodLevel, U32 clusterIndex)
{
	ASSERT(lodLevel<(U32)m_LodMeshClusters.Size());
	ASSERT(clusterIndex < (U32)m_LodMeshClusters[lodLevel].Size());
	return m_LodMeshClusters[lodLevel][clusterIndex].pCluster->Render();
}

U32 C_RenderMeshNode::GetNumClusters(U32 lod) const
{
	if (lod >= (U32)m_LodMeshClusters.Size())
		return 0;
	return (U32)m_LodMeshClusters[lod].Size();
}

const C_RenderMeshCluster* C_RenderMeshNode::GetMeshClusterPtr(
	U32 lodLevel,
	U32 clusterIndex) const
{
	ASSERT(lodLevel < (U32)m_LodMeshClusters.Size());
	ASSERT(clusterIndex < (U32)m_LodMeshClusters[lodLevel].Size());
	return m_LodMeshClusters[lodLevel][clusterIndex].pCluster;
}

C_RenderMeshCluster* C_RenderMeshNode::GetMeshClusterPtr(
	U32 lodLevel,
	U32 clusterIndex)
{
	ASSERT(lodLevel < (U32)m_LodMeshClusters.Size());
	ASSERT(clusterIndex < (U32)m_LodMeshClusters[lodLevel].Size());
	return m_LodMeshClusters[lodLevel][clusterIndex].pCluster;
}

Display::C_VertexDeclaration* C_RenderMeshNode::GetVertexDeclaration() const
{
	return m_pVertexDeclaration;
}

C_RenderMeshNode::E_EffectMode C_RenderMeshNode::GetEffectMode() const
{
	return m_EffectMode;
}

U32 C_RenderMeshNode::GetNumVertexBuffers()
{
	return m_VBStreamPackages.Size();
}

U8 C_RenderMeshNode::GetNumUVSets() const
{
	return m_nUVSets;
}

E_RenderNodeType C_RenderMeshNode::GetType() const
{
	return E_RenderNodeType_Mesh;
}

bool C_RenderMeshNode::HasType(E_RenderNodeType type) const
{
	return (type == C_RenderMeshNode::GetType()
			|| C_RenderMeshRemappableNode::HasType(type));
}


Aftk::C_Status C_RenderMeshNode::CreateVertexBuffers()
{
	const Aftk::C_SceneMeshNode *pAftkMeshNode = (Aftk::C_SceneMeshNode*) m_pAftkSceneNode;

	//Allocate storage for lod x poly -> VB map
	m_LodPolyToVBMap.Resize(pAftkMeshNode->GetNumLodLevels());
	for (U32 iLod=0; iLod<pAftkMeshNode->GetNumLodLevels(); ++iLod)
	{
		m_LodPolyToVBMap[iLod].Resize(pAftkMeshNode->GetNumPolygons(iLod));
	}

	Ut::UtVector< Ut::DynamicVector<U32> > vertexBufferLengths;
	vertexBufferLengths.PushBack(0);
	m_VBToPolyVertexMap.PushBack();

	{
		for (U32 iLod=0; iLod<pAftkMeshNode->GetNumLodLevels(); ++iLod)
		{
			U32 currentVB = 0;

			// Array used ensure we don't push duplicate vertices into the VB.
			Ut::UtVector< Ut::DynamicVector<S32> > aftkPVToVBPV;

			aftkPVToVBPV.Resize(pAftkMeshNode->GetNumPolygonVertices());
			for (U32 jPv=0; jPv<pAftkMeshNode->GetNumPolygonVertices(); ++jPv)
				aftkPVToVBPV[jPv] = -1;

			for (U32 jPolygon=0; jPolygon<pAftkMeshNode->GetNumPolygons(iLod); ++jPolygon)
			{
				const Aftk::C_Polygon& aftkPolygon = pAftkMeshNode->GetPolygon(iLod, jPolygon);

				Aftk::C_TriangleIterator triIt(aftkPolygon, &pAftkMeshNode->GetWorldMatrix());

				U32 vertexCount = vertexBufferLengths[currentVB];
				U32 numPolyVertices = 3*triIt.GetNumTriangles();

				if ((vertexCount+numPolyVertices)>MAX_VERTEX_BUFFER_SIZE)
				{
					//Need a new vertex buffer
					vertexBufferLengths.PushBack(0);
					m_VBToPolyVertexMap.PushBack();
					m_VBToPolyVertexMap[currentVB+1].Reserve(pAftkMeshNode->GetNumPolygonVertices());

					currentVB++;
					//Clear our array as we're now dealing with a new VB
					for (U32 jPv=0; jPv<pAftkMeshNode->GetNumPolygonVertices(); ++jPv)
						aftkPVToVBPV[jPv] = -1;
				}

				m_LodPolyToVBMap[iLod][jPolygon].m_VBIndex = currentVB;
				m_LodPolyToVBMap[iLod][jPolygon].m_TriVtxIndices.Reserve(numPolyVertices);

				for (U32 kTri=0; kTri<triIt.GetNumTriangles(); ++kTri)
				{
					const U32* pvIndices = triIt;

					for (U32 lPv=0; lPv<3; ++lPv)
					{
						U32 polyVertexIndex = pvIndices[lPv];

						if (aftkPVToVBPV[polyVertexIndex] == -1)
						{
							aftkPVToVBPV[polyVertexIndex] = vertexBufferLengths[currentVB];
							m_VBToPolyVertexMap[currentVB].PushBack(polyVertexIndex);
							vertexBufferLengths[currentVB]++;
						}

						m_LodPolyToVBMap[iLod][jPolygon].m_TriVtxIndices.PushBack(aftkPVToVBPV[polyVertexIndex]);
					}
					triIt++;
				}
			}
		}
	}

	//Early exit if there are no vertices
	{
		U32 totalVBLength = 0;
		for (U32 iVB=0; iVB<(U32)vertexBufferLengths.Size(); ++iVB)
			totalVBLength += vertexBufferLengths[iVB];

		if (totalVBLength==0)
		{
			m_nUVSets = 0;

			m_LodPolyToVBMap.Clear();
			m_VBToPolyVertexMap.Clear();

			return Aftk::C_Status::E_Status_Success;
		}
	}

	U32 numVBStreamPackages = (U32)vertexBufferLengths.Size();
	m_VBStreamPackages.Resize(numVBStreamPackages);

	if (m_VBStreamPackages.Size() != vertexBufferLengths.Size())
	{
		LOGFATAL(g_Log, "Failed to allocate memory for vertex buffer storage");
		return Aftk::C_Status::E_Status_Failure;
	}

	for (U32 iVB=0; iVB<numVBStreamPackages; ++iVB)
	{
		Ut::UtVector< Ut::DynamicVector<VBStream> >& streamPackage = m_VBStreamPackages[iVB];
		for (U32 jStream=0; jStream<(U32)m_VBStreamUsage.Size(); ++jStream)
		{
			streamPackage.PushBack();
			if (!Display::Device()->CreateVertexBuffer(
				vertexBufferLengths[iVB] * m_VertexFormat.GetStride(0), 0, &streamPackage[jStream].m_VertexBuffer))
				return Aftk::C_Status::E_Status_Failure;

			streamPackage[jStream].m_StreamIndex = (U8)jStream;
		}
	}
	return Aftk::C_Status::E_Status_Success;
}


void C_RenderMeshNode::DestroyVertexBuffers()
{
	for (U32 iVBPackage=0; iVBPackage<(U32)m_VBStreamPackages.Size(); ++iVBPackage)
	{
		Ut::UtVector< Ut::DynamicVector<VBStream> >& streamPackage = m_VBStreamPackages[iVBPackage];
		for (U32 jStream=0; jStream<(U32)streamPackage.Size(); ++jStream)
		{
			Display::Device()->Destroy(&streamPackage[jStream].m_VertexBuffer);
		}
		streamPackage.Clear();
	}

	m_VBStreamPackages.Clear();
}


Aftk::C_Status C_RenderMeshNode::FillVertexBuffers()
{
	if (!m_VBStreamPackages.Size())
		return Aftk::C_Status::E_Status_Success;

	Aftk::C_SceneMeshNode *pAftkMeshNode = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;

	S32 vertexColourChannel = -1;
	Aftk::E_GenericAttributeType vertexColourType = Aftk::E_GenericAttributeType_Unknown;
	S32 lightMapUVSet = -1;
	
	GetVertexColourChannelAndType(pAftkMeshNode, vertexColourChannel, vertexColourType);
	lightMapUVSet = GetLightMapUVSet(pAftkMeshNode);

	for (U32 iVB=0; iVB<(U32)m_VBStreamPackages.Size(); ++iVB)
	{
		for (U32 jStream=0; jStream<(U32)m_VBStreamUsage.Size(); ++jStream)
		{
			switch (m_VBStreamUsage[jStream])
			{
				case VBStream::E_VertexStreamUsage_Position_Normals:
				{
					Display::Vertex_XYZ2 *pVertexData = (Display::Vertex_XYZ2*)m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Lock();
					ASSERT(pVertexData);
					for (U32 jPv=0; jPv<(U32)m_VBToPolyVertexMap[iVB].Size(); ++jPv)
					{
						U32 aftkPVIndex = m_VBToPolyVertexMap[iVB][jPv];

						const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
						SetVertexPosition(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_RenderNodeFlags.isSkinned, aftkPVIndex);
						SetVertexNormal(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_RenderNodeFlags.isSkinned, aftkPVIndex);
					}
					m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Unlock();
					break;
				}
				case VBStream::E_VertexStreamUsage_Position_Normals_Colour:
				{
					Display::Vertex_XYZ2_C *pVertexData = (Display::Vertex_XYZ2_C*)m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Lock();
					ASSERT(pVertexData);
					for (U32 jPv=0; jPv<(U32)m_VBToPolyVertexMap[iVB].Size(); ++jPv)
					{
						U32 aftkPVIndex = m_VBToPolyVertexMap[iVB][jPv];

						const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
						SetVertexPosition(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_RenderNodeFlags.isSkinned, aftkPVIndex);
						SetVertexNormal(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_RenderNodeFlags.isSkinned,aftkPVIndex);
						SetVertexColour(&pVertexData[jPv], vertexColourChannel, vertexColourType, pAftkPolygonVertex);
					}
					m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Unlock();
					break;
				}
				case VBStream::E_VertexStreamUsage_UVs:
				{
					Display::Vertex_UV *pVertexData = (Display::Vertex_UV*)m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Lock();
					ASSERT(pVertexData);
					for (U32 jPv=0; jPv<(U32)m_VBToPolyVertexMap[iVB].Size(); ++jPv)
					{
						for (U32 kUV=0; kUV<m_nUVSets; ++kUV)
						{
							U32 aftkPVIndex = m_VBToPolyVertexMap[iVB][jPv];
							const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
							SetVertexTextureCoordinates(&pVertexData[jPv*m_nUVSets + kUV], kUV, pAftkMeshNode, pAftkPolygonVertex);
						}
					}
					m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Unlock();
					break;
				}
				case VBStream::E_VertexStreamUsage_Weights_PaletteIndices:
				{
					Display::Vertex_W4_Idx4 *pVertexData = (Display::Vertex_W4_Idx4*) m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Lock();
					ASSERT(pVertexData);
					for (U32 jPv=0; jPv<(U32)m_VBToPolyVertexMap[iVB].Size(); ++jPv)
					{
						U32 aftkPVIndex = m_VBToPolyVertexMap[iVB][jPv];
						const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
						SetVertexWeightsAndIndices(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex);
					}
					m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Unlock();
				}
				case VBStream::E_VertexStreamUsage_Tangents_Binormals:
				{
					Display::Vertex_XYZ2 *pVertexData = (Display::Vertex_XYZ2*)m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Lock();
					ASSERT(pVertexData);
					for (U32 jPv=0; jPv<(U32)m_VBToPolyVertexMap[iVB].Size(); ++jPv)
					{
						U32 aftkPVIndex = m_VBToPolyVertexMap[iVB][jPv];

						const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
						SetVertexTangentAndBinormal(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_RenderNodeFlags.isSkinned, aftkPVIndex);
					}
					m_VBStreamPackages[iVB][jStream].m_VertexBuffer.Unlock();
					break;
				}
				default:
					ASSERT(false && "Invalid vertex data stream");
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;

}


Aftk::C_Status C_RenderMeshNode::BuildMeshClusters()
{
	if (m_VBStreamPackages.Size() == 0)
		return Aftk::C_Status::E_Status_Success;


	Aftk::C_Status status;

	const Aftk::C_SceneMeshNode *pAftkMeshNode = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
	U8 numLods = (U8)pAftkMeshNode->GetNumLodLevels();

	U32 numClusters = E_ReservedClusterType_Count;
	m_LodMeshClusters.Resize(numLods);

	for (U8 iLod=0; iLod<numLods; ++iLod)
	{
		Ut::UtVector< Ut::DynamicVector<U32> > materialToClusterIndicesMap;
		materialToClusterIndicesMap.Resize(m_pAftkMaterialMappingSource->GetNumPolygonMaterialMappings());

		if (m_LodMeshClusters[iLod].Size() == 0)
		{
			for (U16 iMaterial=0; iMaterial<m_pAftkMaterialMappingSource->GetNumPolygonMaterialMappings(); iMaterial++)
			{
				if (m_pAftkMaterialMappingSource->GetPolygonMaterialMappingReferenceCount(iMaterial) > 0)
				{
					materialToClusterIndicesMap[iMaterial] = numClusters;
					++numClusters;
				}
			}
		}
		else
		{
			for (U16 iMaterial=0; iMaterial<m_pAftkMaterialMappingSource->GetNumPolygonMaterialMappings(); iMaterial++)
			{
				materialToClusterIndicesMap[iMaterial] = numClusters;
				++numClusters;
			}
		}

		Ut::UtVector< Ut::DynamicVector<const Aftk::C_MaterialResource*> > clusterMaterials;
		clusterMaterials.Resize(numClusters);
		memset(&(clusterMaterials[0]), 0, sizeof(void*)*numClusters);

		//Reserve space for clusters
		if (m_LodMeshClusters[iLod].Size() != numClusters)
		{
			ASSERT(m_LodMeshClusters[iLod].Size() == 0);
			m_LodMeshClusters[iLod].Resize(numClusters);
			memset(&(m_LodMeshClusters[iLod][0]), 0, sizeof(MeshClusterRecord)*numClusters);
		}
		else
		{
			//Clear existing poly index lists.
			for (S32 j = 0; j < m_LodMeshClusters[iLod].Size(); j++)
			{
				MeshClusterRecord& r = m_LodMeshClusters[iLod][j];
				r.m_polyIndices.Clear();
			}
		}

		//Create Index lists
		for (U32 jPolygon=0; jPolygon<pAftkMeshNode->GetNumPolygons(iLod); ++jPolygon)
		{
			const Aftk::C_Polygon *pPoly = &pAftkMeshNode->GetPolygon(iLod, jPolygon);

			S16 polyMaterialIndex = pPoly->GetMaterialMappingIndex();
			if (polyMaterialIndex == -1)
			{
				MeshClusterRecord& r = m_LodMeshClusters[iLod][E_ReservedClusterType_No_Material];
				r.m_polyIndices.PushBack(jPolygon);
			}
			else
			{
				U32 clusterIndex = materialToClusterIndicesMap[polyMaterialIndex];
				clusterMaterials[clusterIndex] = 
					m_pAftkMaterialMappingSource->GetPolygonMaterialMappingPtr(polyMaterialIndex);

				MeshClusterRecord& r = m_LodMeshClusters[iLod][clusterIndex];
				r.m_polyIndices.PushBack(jPolygon);
			}
		}


		//Create mesh clusters
		for (U32 jCluster=0; jCluster<(U32)m_LodMeshClusters[iLod].Size(); ++jCluster)
		{
			const Aftk::C_MaterialResource *pAftkMaterial = clusterMaterials[jCluster];

			S32 aftkMaterialIndex;
			if (pAftkMaterial)
				aftkMaterialIndex = pAftkMaterial->GetIndex();
			else
				aftkMaterialIndex = -1;

			MeshClusterRecord& r = m_LodMeshClusters[iLod][jCluster];
			if (!r.pCluster)
			{
				//Only create a new cluster if we've got some polygons to insert into it.
				if (r.m_polyIndices.Size() > 0)
				{
					r.pCluster = new C_RenderMeshCluster(this, aftkMaterialIndex, iLod);
					if (!r.pCluster)
						return Aftk::C_Status::E_Status_OutOfMemory;
				}
			}
			if (r.pCluster)
			{
				status = r.pCluster->Rebuild(&r.m_polyIndices);
				if (!status)
					return status;

				if (pAftkMaterial->GetContent().GetMaterialTypeId() == CGFX_SPEEDTREE_LEAF)
					r.pCluster->GetClusterFlags().speedtree_leaf = true;
			}
		} //Cluster
	}// iLod

	return Aftk::C_Status::E_Status_Success;
}

void C_RenderMeshNode::DestroyMeshClusters()
{
	//Delete existing cluster information.
	for (U32 iLod = 0; iLod < (U32)m_LodMeshClusters.Size(); iLod++)
	{
		for (U32 jCluster = 0; jCluster < (U32)m_LodMeshClusters[iLod].Size(); jCluster++)
		{
			MeshClusterRecord& r = m_LodMeshClusters[iLod][jCluster];
			delete r.pCluster;
		}
	}
	m_LodMeshClusters.Clear();
}


U32 C_RenderMeshNode::GetVBIndexFromAftkPolyIndex(U32 lod, U32 polyIndex)
{
	ASSERT(lod<(U32)m_LodPolyToVBMap.Size());
	ASSERT(polyIndex<(U32)m_LodPolyToVBMap[lod].Size());
	return m_LodPolyToVBMap[lod][polyIndex].m_VBIndex;
}

const Ut::UtVector< Ut::DynamicVector<U32> >& C_RenderMeshNode::GetVBPVIndicesFromAftkPolyIndex(U32 lod, U32 polyIndex)
{
	ASSERT(lod<(U32)m_LodPolyToVBMap.Size());
	ASSERT(polyIndex<(U32)m_LodPolyToVBMap[lod].Size());
	return m_LodPolyToVBMap[lod][polyIndex].m_TriVtxIndices;
}

U32 C_RenderMeshNode::GetAftkPVIndexFromVBPVIndex(U32 vbIndex, U32 vbPvIndex)
{
	ASSERT(vbIndex<(U32)m_VBToPolyVertexMap.Size());
	ASSERT(vbPvIndex<(U32)m_VBToPolyVertexMap[vbIndex].Size());
	return m_VBToPolyVertexMap[vbIndex][vbPvIndex];
}

const Ut::UtVector< Ut::DynamicVector<C_RenderMeshNode::VBStream> >& C_RenderMeshNode::GetVertexBufferStreams(U32 vbIndex) const
{
	ASSERT(vbIndex<(U32)m_VBStreamPackages.Size());
	return m_VBStreamPackages[vbIndex];
}

C_RenderMeshNode::VBStream::E_VertexStreamUsage C_RenderMeshNode::GetVertexStreamUsage(U32 index) const
{
	ASSERT(index<(U32)m_VBStreamUsage.Size());
	return m_VBStreamUsage[index];
}