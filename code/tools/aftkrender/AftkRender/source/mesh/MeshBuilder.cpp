#include "AftkRenderPch.h"

#include "mesh/MeshElements.h"
#include "mesh/MeshBuilder.h"
#include "mesh/Mesh.h"

#include "Utility.h"

using namespace AftkRender;

bool C_MeshBuilder::Flatten(
	C_MeshResource *pMesh,
	const Aftk::C_SceneNode *pBaseNode,
	Ut::UtVector< Ut::DynamicVector<const Aftk::C_MaterialResource*> >& materialMapping)
{
	Ut::UtMap<const Aftk::C_SceneMeshNode*, C_MeshResource*> mapping;
	Ut::Vector<S32> materialResourceMapping;
	materialResourceMapping.Resize(pBaseNode->GetScenePtr()->GetNumMaterialNodes());
	if (materialResourceMapping.Size())
		memset(&materialResourceMapping[0], -1, sizeof(S32)*materialResourceMapping.Size());

	bool result = true;
	Aftk::C_SceneNodeConstIterator aftkIt(pBaseNode);
	while(aftkIt)
	{
		if (aftkIt->HasType(Aftk::E_ObjType_SceneMesh))
		{
			const Aftk::C_SceneMeshNode* pMesh = (const Aftk::C_SceneMeshNode*) (const Aftk::C_SceneNode*) aftkIt;
			C_MeshResource *pResource = new C_MeshResource();

			result = result && Build(pResource, pMesh, 0);
			mapping.Insert(Ut::UtMap<const Aftk::C_SceneMeshNode*, C_MeshResource*>::DataType(pMesh, pResource));
		}
		aftkIt++;
	}

	Ut::UtMap<const Aftk::C_SceneMeshNode*, C_MeshResource*>::Iterator it = mapping.Begin();
	while (it != mapping.End())
	{
		C_MeshResource *pResource = it->GetValue();
		Aftk::Matrix44 baseNodeRelative = it->GetKey()->GetWorldMatrix();

		for (U32 i=0; i<pResource->GetNumClusters(); ++i)
		{
			C_MeshClusterResource *pCluster = pResource->DetachCluster(i);

			//bake any transforms applied to the vertex positions
			F32* pPositionData = (F32*)pCluster->GetVertexDataElement(E_ElementType_Position);
			assert(pPositionData);

			for (U32 k = 0; k < pCluster->GetNumVertices(); k++)
			{
				Aftk::Position3 v(pPositionData[0], pPositionData[1], pPositionData[2]);
				v = Aftk::ApplyMatrix(baseNodeRelative, v);

				pPositionData[0] = v.x;
				pPositionData[1] = v.y;
				pPositionData[2] = v.z;

				pPositionData += 3;
			}

			F32* pNormalData = (F32*)pCluster->GetVertexDataElement(E_ElementType_Normal);
			if (pNormalData)
			{
				Aftk::Matrix44 invTransBaseNodeRelative;
				UtMaths::Transpose(&invTransBaseNodeRelative);
				UtMaths::Invert4x4(&invTransBaseNodeRelative, baseNodeRelative);					

				for (U32 k = 0; k< pCluster->GetNumVertices(); k++)
				{
					Aftk::Normal3 n;
					n.x = pNormalData[0];
					n.y = pNormalData[1];
					n.z = pNormalData[2];

					n = Aftk::ApplyMatrixToNormal(invTransBaseNodeRelative, n);

					pNormalData[0] = n.x;
					pNormalData[1] = n.y;
					pNormalData[2] = n.z;

					pNormalData += 3;
				}
			}

			for (U32 k=0; k<pCluster->GetNumIndexDataElements(); ++k)
			{
				C_MeshClusterResource::IndexDataElement *pIndexElement = const_cast<C_MeshClusterResource::IndexDataElement*>(pCluster->GetIndexDataElement(k));
				if (pIndexElement->materialId != -1)
				{
					S32 remappedId = -1;
					S32 aftkId = it->GetKey()->GetPolygonMaterialMappingPtr(pIndexElement->materialId)
									? it->GetKey()->GetPolygonMaterialMappingPtr(pIndexElement->materialId)->GetIndex()
									: -1;

					if (aftkId != -1)
					{
						remappedId = materialResourceMapping[aftkId];
						if (remappedId == -1)
						{
							remappedId = materialMapping.Size();
							materialResourceMapping[aftkId] = remappedId;
							materialMapping.PushBack(it->GetKey()->GetPolygonMaterialMappingPtr(pIndexElement->materialId));
						}
					}
					pIndexElement->materialId = remappedId;
				}
			}
			pMesh->AddCluster(pCluster);
		}
		it++;
	}

	//clean up
	it = mapping.Begin();
	while (it != mapping.End())
	{
		delete it->GetValue();
		it++;
	}
	return true;
}


bool C_MeshBuilder::Merge(
	C_MeshResource* pMesh,
	E_PrimitiveType type,
	U32 mask)
{
	assert (mask && "No Vertex Elements to merge");
	if (type == E_PrimitiveType_Unknown)
		return false;

	typedef Ut::Vector<C_MeshClusterResource*> MergeGroup;

	{
		MergeGroup	mergeGroup;
		U32			vertexCount = 0;

		for (U32 j=0; j<pMesh->GetNumClusters(); ++j)
		{
			C_MeshClusterResource *pCluster = pMesh->GetCluster(j);
			bool primitiveTypeCompatible = pCluster->GetNonIndexedPrimitiveType() == type;

			if (!primitiveTypeCompatible)
			{
				//now look through index batches
				for (U32 i=0; i<pCluster->GetNumIndexDataElements() && !primitiveTypeCompatible; ++i)
				{
					const C_MeshClusterResource::IndexDataElement *pIndexElementData = pCluster->GetIndexDataElement(i);
					primitiveTypeCompatible = primitiveTypeCompatible || pIndexElementData->type == type;
				}
			}

			if (primitiveTypeCompatible)
			{
				mergeGroup.PushBack(pCluster);
				vertexCount += pCluster->GetNumVertices();
			}
		}

		if (!vertexCount || mergeGroup.Size()==1)
			return true;
			
		C_MeshClusterResource *pNewCluster = new C_MeshClusterResource();
		pNewCluster->SetNumVertices(vertexCount);

		if (mask & E_ElementType_Position)
		{
			F32 *pPositions = new F32[vertexCount*3];
			assert(pPositions);

			F32 *pCurrent = pPositions;
			for (S32 j=0; j<mergeGroup.Size(); ++j)
			{
				const F32* pData = (F32*)mergeGroup[j]->GetVertexDataElement(E_ElementType_Position);
				if (pData)
					memcpy(pCurrent, pData, mergeGroup[j]->GetNumVertices()*sizeof(F32)*3);
				else
					memset(pCurrent, 0, mergeGroup[j]->GetNumVertices()*sizeof(F32)*3);

				pCurrent += mergeGroup[j]->GetNumVertices()*3;
			}
			assert(pCurrent-pPositions == vertexCount*3);
			pNewCluster->AttachVertexDataElement(pPositions, E_ElementType_Position);
		}

		if (mask & E_ElementType_Normal)
		{
			F32 *pNormals = new F32[vertexCount*3];
			assert(pNormals);

			F32 *pCurrent = pNormals;
			for (S32 j=0; j<mergeGroup.Size(); ++j)
			{
				const F32* pData = (F32*)mergeGroup[j]->GetVertexDataElement(E_ElementType_Normal);
				if (pData)
					memcpy(pCurrent, pData, mergeGroup[j]->GetNumVertices()*sizeof(F32)*3);
				else
					memset(pCurrent, 0, mergeGroup[j]->GetNumVertices()*sizeof(F32)*3);

				pCurrent += mergeGroup[j]->GetNumVertices()*3;
			}
			assert(pCurrent-pNormals == vertexCount*3);
			pNewCluster->AttachVertexDataElement(pNormals, E_ElementType_Normal);
		}

		if (mask & E_ElementType_TexCoord0)
		{
			F32 *pUVs = new F32[vertexCount*3];
			assert(pUVs);

			F32 *pCurrent = pUVs;
			for (S32 j=0; j<mergeGroup.Size(); ++j)
			{
				const F32* pData = (F32*)mergeGroup[j]->GetVertexDataElement(E_ElementType_TexCoord0);
				if (pData)
					memcpy(pCurrent, pData, mergeGroup[j]->GetNumVertices()*sizeof(F32)*2);
				else
					memset(pCurrent, 0, mergeGroup[j]->GetNumVertices()*sizeof(F32)*2);

				pCurrent += mergeGroup[j]->GetNumVertices()*2;
			}
			assert(pCurrent-pUVs == vertexCount*2);
			pNewCluster->AttachVertexDataElement(pUVs, E_ElementType_TexCoord0);
		}

		//index data...
		{
			U32 indexOffset = 0;
			for (S32 j=0; j<mergeGroup.Size(); ++j)
			{
				if (mergeGroup[j]->GetNonIndexedPrimitiveType() == type)
				{
					//create a dummy index buffer
					U32 *pNewIndexData = new U32[mergeGroup[j]->GetNumVertices()];
					U32 *pCurrent = pNewIndexData;

					for (U32 k=0; k<mergeGroup[j]->GetNumVertices(); ++k)
						*pCurrent++ = indexOffset + k;

					pNewCluster->AttachIndexData(pNewIndexData, mergeGroup[j]->GetNumVertices(), type, -1);
				}
				else
				{
					for (U32 k=0; k<mergeGroup[j]->GetNumIndexDataElements(); ++k)
					{
						const C_MeshClusterResource::IndexDataElement *pIndexElement = mergeGroup[j]->GetIndexDataElement(k);
						if (pIndexElement->type == type)
						{
							U32 *pNewIndexData = new U32[pIndexElement->count];
							U32 *pCurrent = pNewIndexData;
							assert(pNewIndexData);

							U32 *pData = pIndexElement->pData;
							for (U32 l=0; l<pIndexElement->count; ++l)
								(*pCurrent++) = indexOffset + (*pData++);

							pNewCluster->AttachIndexData(pNewIndexData, pIndexElement->count, pIndexElement->type, pIndexElement->materialId);
						}
					}
				}
				indexOffset += mergeGroup[j]->GetNumVertices();
			}
		}


		//Remove old clusters
		for (S32 j=0; j<mergeGroup.Size(); ++j)
			for (U32 k=0; k<pMesh->GetNumClusters(); ++k)
				if (mergeGroup[j] == pMesh->GetCluster(k))
					delete pMesh->DetachCluster(k);

		//attach new cluster
		pMesh->AddCluster(pNewCluster);
	}

	return true;
}


bool C_MeshBuilder::Build(
	C_MeshResource *pMesh, 
	const Aftk::C_SceneMeshNode *pAftkMeshNode,
	U32 flags)
{
	if (!pMesh)
		return false;

	//Skip empty mesh
	{
		U32 polyCount = 0;
		for (U32 iLod=0; iLod<pAftkMeshNode->GetNumLodLevels(); ++iLod)
			polyCount += pAftkMeshNode->GetNumPolygons(iLod);

		if (polyCount == 0)
			return false;
	}

	//keep a list of bones in the original aftk order  - for boned instances (ie. barriers)
	Ut::UtVector< Ut::DynamicVector<const Aftk::C_SceneNode*> > originalBoneOrder;

	for (U32 iInf=0; iInf<pAftkMeshNode->GetNumInfluenceClusters(); ++iInf)
	{
		const Aftk::C_InfluenceClusterNode *pInfluence = pAftkMeshNode->GetInfluenceClusterPtr(iInf);
		if (pInfluence->GetUsage() == AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
		{
			originalBoneOrder.Resize(pInfluence->GetNumEnvelopes());
			for (U32 jEnv=0; jEnv<pInfluence->GetNumEnvelopes(); ++jEnv)
				originalBoneOrder[jEnv] = pInfluence->GetEnvelope(jEnv).GetInfluenceNodePtr();
		}
	}


	for (U32 iLod=0; iLod<pAftkMeshNode->GetNumLodLevels(); ++iLod)
	{
		Ut::UtVector< Ut::DynamicVector<SkinPartition> > partitions;
		Ut::UtVector< Ut::DynamicVector<VertexWeight> > weights;

		BuildSkinPartitions(pAftkMeshNode, partitions, weights, iLod);

		//now build our internal batches
		//how? possible options:
		//- batch per aftk cluster, but break into further batches if they span across multiple partitions **non optimal**
		//- ignore aftk clusters, have one per partition/material **easier, lets do it...**

		C_MeshClusterResource *pMeshCluster = new C_MeshClusterResource();
		if (!pMeshCluster)
			return false;

		pMesh->AddCluster(pMeshCluster);

		Ut::UtVector< Ut::DynamicVector<U32> > aftkPVIndexMap;
		aftkPVIndexMap.Resize(pAftkMeshNode->GetNumPolygonVertices()*partitions.Size());
		memset(&aftkPVIndexMap[0], -1, sizeof(U32)*pAftkMeshNode->GetNumPolygonVertices()*partitions.Size());

		Ut::UtVector< Ut::DynamicVector<U32> > indexPVMap;
		indexPVMap.Reserve(pAftkMeshNode->GetNumPolygonVertices()*partitions.Size());
		Ut::UtVector< Ut::DynamicVector<U32> > materialTriCounts;
		materialTriCounts.Resize((pAftkMeshNode->GetNumPolygonMaterialMappings()+1)*partitions.Size());
		memset(&materialTriCounts[0], 0, sizeof(U32)*(pAftkMeshNode->GetNumPolygonMaterialMappings()+1)*partitions.Size());

		bool triangulated = false;

		//map aftk poly verts into per-lod vertex array indices
		for (S32 jPartition=0; jPartition<partitions.Size(); ++jPartition)
		{
			for (S32 kPoly=0; kPoly<partitions[jPartition].polygons.Size(); ++kPoly)
			{
				U32 polyIndex = partitions[jPartition].polygons[kPoly];
				const Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(iLod, polyIndex);

				triangulated = triangulated || polygon.GetBasePolygonIndex() != -1;

				//-1 => no material
				S16 materialMapping = polygon.GetMaterialMappingIndex();
				materialTriCounts[jPartition*(pAftkMeshNode->GetNumPolygonMaterialMappings()+1)+materialMapping+1] += polygon.GetNumTriangles();

				for (U32 lVert=0; lVert<polygon.GetNumPolygonVertices(); ++lVert)
				{
					U32 pvIndex = polygon.GetPolygonVertexIndex(lVert);
					U32 lookupIndex = jPartition*pAftkMeshNode->GetNumPolygonVertices() + pvIndex;
					if (aftkPVIndexMap[lookupIndex] == -1)
					{
						aftkPVIndexMap[lookupIndex] = indexPVMap.Size();
						indexPVMap.PushBack(lookupIndex);
					}
				}
			}
		}

		pMeshCluster->SetNumVertices(indexPVMap.Size());

		if (flags & MESHBUILD_PolygonOutline)
		{
			Ut::BitVector isBillboard;
			if (pAftkMeshNode->GetNumPolygonMaterialMappings())
			{
				isBillboard.Resize(pAftkMeshNode->GetNumPolygonMaterialMappings());
				for (S16 kMaterial=0; kMaterial<pAftkMeshNode->GetNumPolygonMaterialMappings(); ++kMaterial)
				{
					const Aftk::C_MaterialResource *pResource = pAftkMeshNode->GetPolygonMaterialMappingPtr(kMaterial);
					if (pResource)
					{
						const Aftk::C_MaterialNode *pMaterial = &pResource->GetContent();
						if (pMaterial->GetMaterialTypeId() == CGFX_SPEEDTREE_LEAF)
							isBillboard[kMaterial] = true;
						else
						{
							bool speedtreeleaf = false;
							for (U32 i=0; i<pMaterial->GetNumShaderNodes(); ++i)
							{
								const Aftk::C_ShaderNode *pShader = pMaterial->GetShaderNodePtr(i);
								std::string shader;
								const Aftk::I_Plug *pPlug = pShader->GetPlugPtr("shader");
								if (pPlug && pPlug->GetValue(shader) && shader == "SpeedTreeLeaf.fx")
									speedtreeleaf = true;
							}
							isBillboard[kMaterial] = speedtreeleaf;
						}
					}
					else
						isBillboard[kMaterial] = false;
				}
			}
			
			for (S32 jPartition=0; jPartition<partitions.Size(); ++jPartition)
			{
				//ignore materials 
				//TODO:fix if we need textured untriangulated wireframe (unlikely)

				struct PolygonEdge
				{
					PolygonEdge()
						: m_p0(-1), m_p1(-1)
					{ }

					PolygonEdge(U32 p0, U32 p1)
						: m_p0(p0>p1 ? p1 : p0), m_p1(p0>p1 ? p0 : p1)
					{
					}

					static U32 GetHash(const PolygonEdge& e)
					{
						return Ut::Hash((U64)e.m_p0<<32 | (U64)e.m_p1);
					}

					static bool Equal(const PolygonEdge& e1, const PolygonEdge& e2)
					{
						return (e1.m_p0==e2.m_p0 && e1.m_p1==e2.m_p1);
					}

					U32 m_p0, m_p1;
				};

				//build set of unique edges within the currrent partition. Divide if billboard polygon
				Ut::OpenHashSet<PolygonEdge, PolygonEdge> edgeSet[2];

				//is the mesh triangulated?
				if (!triangulated)
				{
					//ASSUMPTION: vertices are listed in edge order
					for (S32 kPoly=0; kPoly<partitions[jPartition].polygons.Size(); ++kPoly)
					{
						U32 polyIndex = partitions[jPartition].polygons[kPoly];
						const Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(iLod, polyIndex);
						
						U32 edgeSetIndex = 0;
						if (polygon.GetMaterialMappingIndex() != -1 && isBillboard[polygon.GetMaterialMappingIndex()])
							edgeSetIndex = 1;

						for (U32 lPv=0; lPv<polygon.GetNumPolygonVertices(); ++lPv)
						{
							U32 p0 = polygon.GetPolygonVertexIndex(lPv % polygon.GetNumPolygonVertices());
							U32 p1 = polygon.GetPolygonVertexIndex((lPv+1) % polygon.GetNumPolygonVertices());

							PolygonEdge edge(p0, p1);
							edgeSet[edgeSetIndex].Insert(edge);
						}
					};
				}
				else
				{
					//rebuild untriangulated polygons
					Ut::UtMap<U32, Ut::UtVector< Ut::DynamicVector<U32> > > polygonMap;
					for (S32 kPoly=0; kPoly<partitions[jPartition].polygons.Size(); ++kPoly)
					{
						U32 polyIndex = partitions[jPartition].polygons[kPoly];
						const Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(iLod, polyIndex);

						U32 basePolygon = polygon.GetBasePolygonIndex();
						if (basePolygon != -1)
						{
							Ut::UtMap<U32, Ut::UtVector< Ut::DynamicVector<U32> > >::InsertionReturnType r =
								polygonMap.Insert(Ut::UtMap<U32, Ut::UtVector< Ut::DynamicVector<U32> > >::DataType(basePolygon));
							r.first->GetValue().PushBack(polyIndex);
						}
						else
							Ut::UtMap<U32, Ut::UtVector< Ut::DynamicVector<U32> > >::InsertionReturnType r =
								polygonMap.Insert(Ut::UtMap<U32, Ut::UtVector< Ut::DynamicVector<U32> > >::DataType(polyIndex));
					}

					//now build the edge list by looking for any non-shared edges
					for (Ut::UtMap<U32, Ut::UtVector< Ut::DynamicVector<U32> > >::Iterator it=polygonMap.Begin(); it!=polygonMap.End(); ++it)
					{
						Ut::OpenHashMap<PolygonEdge, U32, PolygonEdge> polyEdgeCounts;

						U32 basePolyIndex = it->GetKey();
						const Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(iLod, basePolyIndex);

						U32 edgeSetIndex = 0;
						if (polygon.GetMaterialMappingIndex() != -1 && isBillboard[polygon.GetMaterialMappingIndex()])
							edgeSetIndex = 1;

						for (U32 kPv=0; kPv<polygon.GetNumPolygonVertices(); ++kPv)
						{
							U32 p0 = polygon.GetPolygonVertexIndex(kPv % polygon.GetNumPolygonVertices());
							U32 p1 = polygon.GetPolygonVertexIndex((kPv+1) % polygon.GetNumPolygonVertices());

							PolygonEdge edge(p0, p1);
							Ut::OpenHashMap<PolygonEdge, U32, PolygonEdge>::InsertionReturnType r = 
								polyEdgeCounts.Insert(Ut::OpenHashMap<PolygonEdge, U32, PolygonEdge>::DataType(edge, 0));
							(r.first->GetValue())++;
						}

						const Ut::UtVector< Ut::DynamicVector<U32> >& polygons = it->GetValue();
						for (S32 kPoly=0; kPoly<polygons.Size(); ++kPoly)
						{
							U32 polyIndex = polygons[kPoly];
							const Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(iLod, polyIndex);
							for (U32 lPv=0; lPv<polygon.GetNumPolygonVertices(); ++lPv)
							{
								U32 p0 = polygon.GetPolygonVertexIndex(lPv%polygon.GetNumPolygonVertices());
								U32 p1 = polygon.GetPolygonVertexIndex((lPv+1)%polygon.GetNumPolygonVertices());

								PolygonEdge edge(p0, p1);
								Ut::OpenHashMap<PolygonEdge, U32, PolygonEdge>::InsertionReturnType r = 
									polyEdgeCounts.Insert(Ut::OpenHashMap<PolygonEdge, U32, PolygonEdge>::DataType(edge, 0));
								(r.first->GetValue())++;
							}
						}

						for (Ut::OpenHashMap<PolygonEdge, U32, PolygonEdge>::Iterator eIt=polyEdgeCounts.Begin(); eIt!=polyEdgeCounts.End(); ++eIt)
						{
							if (eIt->GetValue() == 1)
								edgeSet[edgeSetIndex].Insert(eIt->GetKey());
						}
					}
				}

				//create index buffers
				for (S32 jSet=0; jSet<2; ++jSet)
				{
					if (edgeSet[jSet].Size() > 0)
					{
						U32 *pIndexData = new U32[edgeSet[jSet].Size()*2];
						U32 *pCurrent = pIndexData;

						//No material for wireframe. Nasty magic number -2 to indicate billboard
						pMeshCluster->AttachIndexData(pIndexData, edgeSet[jSet].Size()*2, E_PrimitiveType_LineList, -1-jSet);

						U32 vertexLookupOffset = jPartition*pAftkMeshNode->GetNumPolygonVertices();
						for (Ut::OpenHashSet<PolygonEdge, PolygonEdge>::Iterator it=edgeSet[jSet].Begin(); it!=edgeSet[jSet].End(); ++it)
						{
							const PolygonEdge& edge = it->GetKey();
							U32 i0 = aftkPVIndexMap[vertexLookupOffset + edge.m_p0];
							U32 i1 = aftkPVIndexMap[vertexLookupOffset + edge.m_p1];

							if (i0 == -1 || i1 == -1)
							{
								//render degenerate line - the base polygon and child polygons are in separate partitions
								i0 = 0;
								i1 = 0;
							}

							*pCurrent++ = i0;
							*pCurrent++ = i1;
						}
					}
				}
			}
		}

		//Index data
		for (S32 jPartition=0; jPartition<partitions.Size(); ++jPartition)
		{
			struct IndexData
			{
				U32 *pStart;
				U32 *pCurrent;

				IndexData()
					: pStart(0), pCurrent(0)
				{ }
			};

			//initialise index arrays
			Ut::UtVector< Ut::DynamicVector<IndexData> > indexData;
			indexData.Resize(pAftkMeshNode->GetNumPolygonMaterialMappings()+1);

			for (S32 kMaterial=0; kMaterial<pAftkMeshNode->GetNumPolygonMaterialMappings()+1; ++kMaterial)
			{
				S32 lookupIndex = jPartition*(pAftkMeshNode->GetNumPolygonMaterialMappings()+1) + kMaterial;
				U32 nTriangles = materialTriCounts[lookupIndex];
				if (nTriangles)
				{
					U32 *pIndexData = new U32[3*nTriangles];
					if (!pIndexData)
						return false;

					indexData[kMaterial].pStart = pIndexData;
					indexData[kMaterial].pCurrent = pIndexData;

					pMeshCluster->AttachIndexData(pIndexData, 3*nTriangles, E_PrimitiveType_TriList, kMaterial-1);
				}
				else
				{
					indexData[kMaterial].pStart = 0;
					indexData[kMaterial].pCurrent = 0;
				}
			}

			//now loop thru all polygons & populate index arrays
			for (S32 kPoly=0; kPoly<partitions[jPartition].polygons.Size(); ++kPoly)
			{
				U32 polyIndex = partitions[jPartition].polygons[kPoly];
				const Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(iLod, polyIndex);
				
				U32 lookup = polygon.GetMaterialMappingIndex()+1;
				U32*& pCurrentData = indexData[lookup].pCurrent;
				assert(pCurrentData != 0);

				U32 vertexLookupOffset = jPartition*pAftkMeshNode->GetNumPolygonVertices();
				for (U32 lTri=0; lTri<polygon.GetNumTriangles(); ++lTri)
				{
					U32 triIndices[3];
					polygon.GetTrianglePolygonVertexIndices(lTri, triIndices);

					*pCurrentData++ = aftkPVIndexMap[vertexLookupOffset + triIndices[0]];
					*pCurrentData++ = aftkPVIndexMap[vertexLookupOffset + triIndices[1]];
					*pCurrentData++ = aftkPVIndexMap[vertexLookupOffset + triIndices[2]];
				}
			}
		}


		//Position data
		{
			F32 *pPositionData = new F32[3*indexPVMap.Size()];
			if (!pPositionData)
				return false;

			const Aftk::Position3Array& aftkPositions = pAftkMeshNode->GetVertexPositionArray();
			const U32 nPVs = pAftkMeshNode->GetNumPolygonVertices();
			F32 *pCurrent = pPositionData;
			for (S32 i=0; i<indexPVMap.Size(); ++i)
			{
				const Aftk::C_PolygonVertex *pAftkPV = &pAftkMeshNode->GetPolygonVertex(indexPVMap[i]%nPVs);
				S32 idx = pAftkPV->GetPositionIndex();
				memcpy(&pPositionData[3*i], &aftkPositions[idx], 3*sizeof(F32));
			}

			if (!pMeshCluster->AttachVertexDataElement(pPositionData, E_ElementType_Position))
			{
				delete[] pPositionData;
				return false;
			}
		}

		//Normal data
		{
			F32 *pNormalData = new F32[3*indexPVMap.Size()];
			if (!pNormalData)
				return false;

			const Aftk::Normal3Array& aftkNormals = pAftkMeshNode->GetVertexNormalArray();
			const U32 nPVs = pAftkMeshNode->GetNumPolygonVertices();
			for (S32 i=0; i<indexPVMap.Size(); ++i)
			{
				const Aftk::C_PolygonVertex *pAftkPV = &pAftkMeshNode->GetPolygonVertex(indexPVMap[i]%nPVs);
				S32 idx = pAftkPV->GetNormalIndex();
				memcpy(&pNormalData[3*i], &aftkNormals[idx], 3*sizeof(F32));
			}

			if (!pMeshCluster->AttachVertexDataElement(pNormalData, E_ElementType_Normal))
			{
				delete[] pNormalData;
				return false;
			}
		}

		//Vertex Colours
		{
			S32 channel;
			Aftk::E_GenericAttributeType type;

			Utility::GetVertexColourChannelAndType(pAftkMeshNode, channel, type);
			if (channel != -1)
			{
				U8* pColourData = new U8[4*indexPVMap.Size()];
				if (!pColourData)
					return false;

				const U32 nPVs = pAftkMeshNode->GetNumPolygonVertices();
				for (S32 i=0; i<indexPVMap.Size(); ++i)
				{
					const Aftk::C_PolygonVertex *pAftkPV = &pAftkMeshNode->GetPolygonVertex(indexPVMap[i]%nPVs);

					switch (type)
					{
					case Aftk::E_GenericAttributeType_Vector4F32:
					{
						Aftk::Vector4F32 vertexColour;
						pAftkPV->GetData(vertexColour, channel);

						pColourData[4*i+0] = (U8)(vertexColour.X()*255.0f);
						pColourData[4*i+1] = (U8)(vertexColour.Y()*255.0f);
						pColourData[4*i+2] = (U8)(vertexColour.Z()*255.0f);
						pColourData[4*i+3] = (U8)(vertexColour.W()*255.0f);
					}
					break;
					case Aftk::E_GenericAttributeType_U32:
					{
						U32 colour;
						pAftkPV->GetData(colour, channel);
						*((U32*)(&pColourData[4*i])) = colour;
					}
					break;
					default:
					{
						*((U32*)(&pColourData[4*i])) = 0xffffffff;
					}
					break;
					}
				}

				if (!pMeshCluster->AttachVertexDataElement(pColourData, E_ElementType_Colour))
				{
					delete[] pColourData;
					return false;
				}
			}
		}

		//TextureCoords
		{
			const U32 nPVs = pAftkMeshNode->GetNumPolygonVertices();
			for (U32 i=0; i<pAftkMeshNode->GetNumTextureCoordinateSets(); ++i)
			{
				F32 *pUVData = new F32[2*indexPVMap.Size()];
				if (!pUVData)
					return false;

				for (S32 j=0; j<indexPVMap.Size(); ++j)
				{
					const Aftk::C_PolygonVertex *pAftkPV = &pAftkMeshNode->GetPolygonVertex(indexPVMap[j]%nPVs);

					S32 idx = pAftkPV->GetTextureCoordinateIndex(i);
					if (idx == -1)
						continue;
					const Aftk::TextureCoordinate4& uv = pAftkMeshNode->GetTextureCoordinate(i, idx);

					memcpy(&pUVData[2*j], &uv, 2*sizeof(F32));
				}

				if (!pMeshCluster->AttachVertexDataElement(pUVData, (E_ElementType)((U32)::E_ElementType_TexCoord0 << i)))
				{
					delete[] pUVData;
					return false;
				}
			}
		}
				
		//Tangents
		if (pAftkMeshNode->GetNumTextureCoordinateSets()>0 &&
			pAftkMeshNode->GetTextureCoordinateSet(0).GetNumTangents()>0)
		{
			const U32 nPVs = pAftkMeshNode->GetNumPolygonVertices();
			F32 *pTangentData = new F32[3*indexPVMap.Size()];
			if (!pTangentData)
				return false;

			const Ut::UtVector< Ut::DynamicVector<Aftk::Vector3F32> >& aftkTangents =
				pAftkMeshNode->GetTextureCoordinateSet(0).GetTangentArray();

			for (S32 i=0; i<indexPVMap.Size(); ++i)
			{
				const Aftk::C_PolygonVertex *pAftkPV = &pAftkMeshNode->GetPolygonVertex(indexPVMap[i]%nPVs);

				S32 idx = pAftkPV->GetTangentIndex(0);
				memcpy(&pTangentData[3*i], &aftkTangents[idx], 3*sizeof(F32));
			}

			if (!pMeshCluster->AttachVertexDataElement(pTangentData, E_ElementType_Tangent))
			{
				delete[] pTangentData;
				return false;
			}
		}

		//Binormals
		if (pAftkMeshNode->GetNumTextureCoordinateSets()>0 &&
			pAftkMeshNode->GetTextureCoordinateSet(0).GetNumBinormals()>0)
		{
			const U32 nPVs = pAftkMeshNode->GetNumPolygonVertices();
			F32 *pBinormalData = new F32[3*indexPVMap.Size()];
			if (!pBinormalData)
				return false;

			const Ut::UtVector< Ut::DynamicVector<Aftk::Vector3F32> >& aftkBinormals =
				pAftkMeshNode->GetTextureCoordinateSet(0).GetBinormalArray();

			for (S32 i=0; i<indexPVMap.Size(); ++i)
			{
				const Aftk::C_PolygonVertex *pAftkPV = &pAftkMeshNode->GetPolygonVertex(indexPVMap[i]%nPVs);

				S32 idx = pAftkPV->GetBinormalIndex(0);
				memcpy(&pBinormalData[3*i], &aftkBinormals[idx], 3*sizeof(F32));
			}

			if (!pMeshCluster->AttachVertexDataElement(pBinormalData, E_ElementType_Binormal))
			{
				delete[] pBinormalData;
				return false;
			}
		}

		//Weights & blend indices
		if (pAftkMeshNode->IsSkinned() || pAftkMeshNode->IsSkinAnimated())
		{
			//TODO: need to store influencer node ptrs somewhere for matrix palette skinning in a generic way
			
			//as we've partitioned the mesh according to the matrix palette size, 
			//we now need to remap the weight indices to those within the partition.


			Ut::UtMap<const Aftk::C_SceneNode*, U32> boneIndexOffset;
			for (S32 jBone=0; jBone<originalBoneOrder.Size(); ++jBone)
				boneIndexOffset[originalBoneOrder[jBone]] = jBone;

			Ut::UtVector< Ut::DynamicVector<U32> > paletteIndexMap;
			paletteIndexMap.Resize(originalBoneOrder.Size() * partitions.Size());
			if (paletteIndexMap.Size() > 0)
			{
				memset(&paletteIndexMap[0], -1, sizeof(U32)*originalBoneOrder.Size()*partitions.Size());

				for (S32 jPartition=0; jPartition<partitions.Size(); ++jPartition)
				{
					if (partitions[jPartition].influenceNodes.Size() > 0)
					{
						Ut::UtMap<const Aftk::C_SceneNode*, U32> influenceIndexMap;

						//Temporary hack: keep barrier the bone order as we don't yet store the bone array anywhere
						if (pAftkMeshNode->GetNumInstancesOfThisNode()>0 &&
							pAftkMeshNode->GetInstanceOfThisNode(0)->GetType()==Aftk::E_ObjType_SceneMeshBonedInstance)
						{
							for (S32 kBone=0; kBone<originalBoneOrder.Size(); ++kBone)
								paletteIndexMap[jPartition*originalBoneOrder.Size()+kBone] = kBone;
						}
						else
						{
							for (S32 kInf=0; kInf<partitions[jPartition].influenceNodes.Size(); ++kInf)
							{
								U32 offset = boneIndexOffset[partitions[jPartition].influenceNodes[kInf]];
								paletteIndexMap[originalBoneOrder.Size()*jPartition + offset] = kInf;
							}
						}
					}
				}

				F32 *pWeightData = new F32[4*indexPVMap.Size()];
				if (!pWeightData)
					return false;

				U8 *pIndexData = new U8[4*indexPVMap.Size()];
				if (!pIndexData)
					return false;

				const U32 nPVs = pAftkMeshNode->GetNumPolygonVertices();
				for (S32 i=0; i<indexPVMap.Size(); ++i)
				{
					U32 partition = indexPVMap[i]/nPVs;
					const Aftk::C_PolygonVertex *pAftkPV = &pAftkMeshNode->GetPolygonVertex(indexPVMap[i]%nPVs);

					S32 idx = pAftkPV->GetPositionIndex();
					memcpy(&pWeightData[4*i], weights[idx].weights, 4*sizeof(F32));

					for (U32 inf=0; inf<MAX_VERTEX_INFLUENCERS; ++inf)
					{
						if (weights[idx].nodes[inf])
							pIndexData[4*i+inf] = paletteIndexMap[ partition*originalBoneOrder.Size()+boneIndexOffset[weights[idx].nodes[inf]] ];
						else
							pIndexData[4*i+inf] = 0;
					}
				}

				if (!pMeshCluster->AttachVertexDataElement(pWeightData, E_ElementType_Weights))
				{
					delete[] pWeightData;
					delete[] pIndexData;
					return false;
				}

				if (!pMeshCluster->AttachVertexDataElement(pIndexData, E_ElementType_BlendIndices))
				{
					delete[] pIndexData;
					return false;
				}
			}
		}
	}

	return true;
}



bool C_MeshBuilder::BuildSkinPartitions(
	const Aftk::C_SceneMeshNode *pAftkMeshNode,
	Ut::UtVector< Ut::DynamicVector< C_MeshBuilder::SkinPartition > >& partitions,
	Ut::UtVector< Ut::DynamicVector< VertexWeight > >& weights,
	U32 lod)
{
	//build list of influencer nodes per vertex position...
	weights.Resize(pAftkMeshNode->GetNumVertexPositions());

	for (U32 iInf=0; iInf<pAftkMeshNode->GetNumInfluenceClusters(); ++iInf)
	{
		const Aftk::C_InfluenceClusterNode *pInfluence = pAftkMeshNode->GetInfluenceClusterPtr(iInf);
		if (pInfluence->GetUsage() == AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
		{
			for (U32 jEnv=0; jEnv<pInfluence->GetNumEnvelopes(); ++jEnv)
			{
				const Aftk::C_WeightEnvelope& envelope = pInfluence->GetEnvelope(jEnv);
				for (U32 kWeight=0; kWeight<envelope.GetNumWeights(); ++kWeight)
				{
					Aftk::C_Weight<F32> weight;
					envelope.GetWeight(kWeight, weight);

					weights[weight.GetIndex()].Add(weight.GetWeight(), envelope.GetInfluenceNodePtr());
				}
			}
		}
	}

	//Generate mesh partitions. Not ideal but should be sufficent
	//Polygon order dependent, but these are generally grouped per aftk cluster/material right?

	const Aftk::PolygonArray& polygons = pAftkMeshNode->GetPolygonArray(lod);

	for (S32 iPolygon=0; iPolygon<polygons.Size(); ++iPolygon)
	{
		const Aftk::C_Polygon& poly = polygons[iPolygon];
		Ut::UtVector< Ut::DynamicVector<const Aftk::C_SceneNode*> > polygonInfluencers;

		//build influence nodes list for the polygon
		for (U32 jVertex=0; jVertex<poly.GetNumPolygonVertices(); ++jVertex)
		{
			U32 vIndex = poly.GetPolygonVertex(jVertex).GetPositionIndex();

			for (S32 kVInf=0; kVInf<MAX_VERTEX_INFLUENCERS; ++kVInf) 
			{
				if (weights[vIndex].nodes[kVInf] != NULL)
				{
					bool added = false;
					for (S32 lPInf=0; lPInf<polygonInfluencers.Size(); ++lPInf)
					{
						if (polygonInfluencers[lPInf] == weights[vIndex].nodes[kVInf])
						{
							added = true;
							break;
						}
					}

					if (!added)
						polygonInfluencers.PushBack(weights[vIndex].nodes[kVInf]);
				}
			}
		}

		//Try to add to existing partition
		bool added = false;
		for (S32 jPartition=0; jPartition<partitions.Size(); ++jPartition)
		{
			if (partitions[jPartition].AddPolygon(iPolygon, polygonInfluencers))
			{
				added = true;
				break;
			}
		}

		//Add to new partition
		if (!added)
		{
			partitions.PushBack();
			partitions.Back().AddPolygon(iPolygon, polygonInfluencers);
		}
	}

	return true;
}
