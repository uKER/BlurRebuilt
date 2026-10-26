#include "stdafx.h"

using namespace Aftk;

bool CalculateInfluenceCombinationIndices(
	const Aftk::C_SceneMeshNode *pMeshNode,
	Ut::UtVector< Ut::DynamicVector<U32> >& indices)
{
	//Generate a key for each vertex using the combination of nodes that the vertex is weighted to
	for(U32 iInfluence = 0; iInfluence < pMeshNode->GetNumInfluenceClusters(); iInfluence++)
	{
		const C_InfluenceClusterNode* pInfluenceCluster = pMeshNode->GetInfluenceClusterPtr(iInfluence);

		if (pInfluenceCluster->GetUsage() != AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
			continue;

		for (U32 iEnvelope = 0; iEnvelope < pInfluenceCluster->GetNumEnvelopes(); iEnvelope++)
		{
			const C_WeightEnvelope* pEnvelope = &pInfluenceCluster->GetEnvelope(iEnvelope);
			for (U32 iVtx = 0; iVtx < pEnvelope->GetNumWeights(); iVtx++)
			{
				const C_WeightEnvelope* pEnvelope = &pInfluenceCluster->GetEnvelope(iEnvelope);

				bool isInfluenced = false;
				for (U32 iWeight = 0; iWeight<pEnvelope->GetNumWeights(); iWeight++)
				{
					Aftk::C_Weight<F32> weight;
					pEnvelope->GetWeight(iWeight, weight);

					if (weight.GetWeight() > AFTK_EPSILON)
					{
						U32 vertexIndex = weight.GetIndex();
						indices[vertexIndex] += Ut::HashKeyOpByteArray<U32>::GetHash(pEnvelope->GetInfluenceNodePtr()->GetUniqueId());
					}
				}
			}
		}
	}
	return true;
}

U32 C_MeshWeld::WeldTextureCoordinates(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	F32 threshold)
{
	U32 numWelds = 0;
	for (U32 iSet = 0; iSet < pMesh->GetNumTextureCoordinateSets(); iSet++)
	{
		numWelds += WeldTextureCoordinates(
			weldMemoryManager,
			pMesh,
			iSet, 
			threshold);
	}
	return numWelds;
}

U32 C_MeshWeld::WeldTextureCoordinates(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	U32 set, 
	F32 threshold)
{
	C_TextureCoordinateSet& uvSet = pMesh->GetTextureCoordinateSet(set);
	TextureCoordinate4Array& uvs
		= uvSet.GetTextureCoordinateArray();
	if (uvs.Size() == 0)
		return 0;
	
	PolygonVertexArray& polygonVertices 
		= pMesh->GetPolygonVertexArray();
	if (polygonVertices.Size() == 0)
		return 0;

	//This stores information regarding how to get from our old indices to our
	//new indices.
	Ut::UtVector< Ut::DynamicVector<S32> >*						pRemapping;
	weldMemoryManager.GetRemappingArray(&pRemapping);
	//This stores our new vertex normals.
	Ut::UtVector< Ut::DynamicVector<TextureCoordinate4> >*		pNewData;
	weldMemoryManager.GetIntermediateArray(&pNewData);

	U32 numWelds = 0;

	//Create a new array of welded component data, and a table mapping from our existing data
	//to our new data.
	numWelds += InternalWeldComponents(
		weldMemoryManager,
		uvSet.GetTextureCoordinateArray(),
		0,
		threshold);
	if (numWelds == 0)
		return 0;

	//We now have a welded array of data, and the required mapping table. Iterate through the components
	//and perform the remapping operation.
	for (U32 iPolyVert = 0; iPolyVert < (U32)polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex& polyVert = polygonVertices[iPolyVert];

		//Skip invalid position index.
		if (polyVert.GetTextureCoordinateIndex(set) == -1)
			continue;
		
		U32 oldIndex = (U32)polyVert.GetTextureCoordinateIndex(set);
		U32 newIndex = (U32)((*pRemapping)[oldIndex]);

		//Finally, we set the new component index.
		polyVert.SetTextureCoordinateIndex(set, newIndex);
	}

	//Overwrite old component data.
	uvSet.GetTextureCoordinateArray() = *pNewData;
	return numWelds;
}

U32 C_MeshWeld::WeldVertexNormals(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	F32 threshold)
{
	Normal3Array& vertexNormals = pMesh->GetVertexNormalArray();
	if ((U32)vertexNormals.Size() == 0)
		return 0;
	
	PolygonVertexArray& polygonVertices = pMesh->GetPolygonVertexArray();
	if ((U32)polygonVertices.Size() == 0)
		return 0;


	//This stores information regarding how to get from our old indices to our
	//new indices.
	Ut::UtVector< Ut::DynamicVector<S32> >*			pRemapping;
	weldMemoryManager.GetRemappingArray(&pRemapping);
	//This stores our new vertex normals.
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >*	pNewData;
	weldMemoryManager.GetIntermediateArray(&pNewData);

	//Animation helpers are being invalidated here.
	pMesh->DirtyGeometryDependantCaches();

	U32 numWelds = 0;

	//Create a new array of welded component data, and a table mapping from our existing data
	//to our new data.

	numWelds = InternalWeldComponents(
		weldMemoryManager,
		(Ut::UtVector< Ut::DynamicVector<Vector3F32> >&)vertexNormals,
		0,
		threshold);

	if (numWelds == 0)
	{
		return 0;
	}

	//We now have a welded array of data, and the required mapping table. Iterate through the components
	//and perform the remapping operation.
	for (U32 iPolyVert = 0; iPolyVert < (U32)polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolyVert = &polygonVertices[iPolyVert];

		//Skip invalid position index.
		if (pPolyVert->GetNormalIndex() == -1)
		{
			continue;
		}
		
		U32 oldIndex = (U32)pPolyVert->GetNormalIndex();
		S32 newIndex = (U32)((*pRemapping)[oldIndex]);

		//Finally, we set the new component index.
		pPolyVert->SetNormalIndex(newIndex);
	}

	//Overwrite old component data.
	vertexNormals = *((Ut::UtVector<Ut::DynamicVector<Normal3> >*)pNewData);
	return numWelds;
}

U32 C_MeshWeld::WeldVertexPositions(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	F32 threshold)
{
	Position3Array& vertexPositions = pMesh->GetVertexPositionArray();
	if ((U32)vertexPositions.Size() == 0)
		return 0;

	PolygonVertexArray& polygonVertices = pMesh->GetPolygonVertexArray();
	if ((U32)polygonVertices.Size() == 0)
		return 0;

	//Animation helpers are being invalidated here.
	pMesh->DirtyGeometryDependantCaches();

	Ut::UtVector< Ut::DynamicVector<S32> >*			pRemapping;
	weldMemoryManager.GetRemappingArray(&pRemapping);
	//This stores our new position data.
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >*	pNewData;
	weldMemoryManager.GetIntermediateArray(&pNewData);


	class WeldWeightedVertexPositionCheck : public I_ComponentFunctor
	{
	public:
		WeldWeightedVertexPositionCheck(Aftk::C_SceneMeshNode *pMeshNode)
			: m_pMeshNode(pMeshNode)
		{
			m_VertexInfluenceIndices.Resize(pMeshNode->GetNumVertexPositions());
			memset(&m_VertexInfluenceIndices[0], 0, sizeof(U32)*m_VertexInfluenceIndices.Size());
			CalculateInfluenceCombinationIndices(pMeshNode, m_VertexInfluenceIndices);
		}
		bool operator()(U32 posIndex1, U32 posIndex2)
		{
			if (m_VertexInfluenceIndices[posIndex1] == m_VertexInfluenceIndices[posIndex2])
				return true;
			else
				return false;
		}
		Aftk::C_SceneMeshNode *m_pMeshNode;
		Ut::UtVector< Ut::DynamicVector<U32> > m_VertexInfluenceIndices;
	};

	WeldWeightedVertexPositionCheck checkVertexInfluencers(pMesh);

	Matrix44 worldMatrix = pMesh->GetWorldMatrix();
	U32 numWelds = InternalWeldComponents(
		weldMemoryManager,
		(Ut::UtVector< Ut::DynamicVector<Vector3F32> >&)vertexPositions,
		&worldMatrix,
		threshold,
		&checkVertexInfluencers);
	if (numWelds == 0)
		return 0;

	RemapVertexWeights(
		pMesh,
		*pRemapping);

	//By this point we've removed any indices pointing to duplcated vectors.
	for (U32 polyVertIndex = 0; polyVertIndex < (U32)polygonVertices.Size(); polyVertIndex++)
	{
		C_PolygonVertex* pPolyVert = &polygonVertices[polyVertIndex];

		//Skip invalid position index.
		if (pPolyVert->GetPositionIndex() == -1)
		{
			continue;
		}
		
		U32 posIndex = pPolyVert->GetPositionIndex();
		U32 newPosIndex = (U32)((*pRemapping)[posIndex]);

		pPolyVert->SetPositionIndex(newPosIndex);
	}

	vertexPositions = *((Ut::UtVector<Ut::DynamicVector<Position3> >*)pNewData);
	return numWelds;
}

void C_MeshWeld::RemapVertexWeights(
	Aftk::C_SceneMeshNode* pMesh,
	const Ut::UtVector< Ut::DynamicVector<S32> >& remapping)
{
	//Fix animation envelopes. CAVEAT - will screw things up if, for example, two identical meshes use the same
	//influence cluster and you only vertex weld one of the two meshes. The animation envelope will be adjusted
	//for only one of the meshes.
	if (pMesh->GetNumInfluenceClusters() > 0)
	{
		Ut::UtVector< Ut::DynamicVector<C_VertexWeightWeldRecord> >				newWeightList;
		Ut::UtVector< Ut::DynamicVector<C_VertexWeightWeldRecord> >				oldWeightList;
		newWeightList.Resize(pMesh->GetNumVertexPositions());
		for(U32 iInfluence = 0; iInfluence < pMesh->GetNumInfluenceClusters(); iInfluence++)
		{
			C_InfluenceClusterNode* pInfluenceCluster = pMesh->GetInfluenceClusterPtr(iInfluence);

			//Only remap vertex weighting information.
			if (pInfluenceCluster->GetUsage() == AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
			{
				for (U32 iEnvelope = 0; iEnvelope < pInfluenceCluster->GetNumEnvelopes(); iEnvelope++)
				{
					//Clear our working area.
					memset(&newWeightList[0], 0, sizeof(C_VertexWeightWeldRecord) * newWeightList.Size());

					C_WeightEnvelope* pEnvelope = &pInfluenceCluster->GetEnvelope(iEnvelope);
					for (U32 iVtx = 0; iVtx < pEnvelope->GetNumWeights(); iVtx++)
					{
						C_Weight<F32> weight;
						pEnvelope->GetWeight(iVtx, weight);
						//old vertex position index
						U32 posIndex = weight.GetIndex();
						//new vertex position index
						S32 newPosIndex = remapping[posIndex];
						if (newPosIndex == -1)
							continue;

						AFTK_ASSERT(newPosIndex < newWeightList.Size());

						//Set vertex weight
						newWeightList[newPosIndex].weight += weight.GetWeight();
						newWeightList[newPosIndex].nWeights++;
						AFTK_ASSERT(newWeightList[newPosIndex].nWeights != 0);
					}

					Ut::UtVector< Ut::DynamicVector<C_Weight<F32> > > tempWeightList;
					for (U32 iVtx = 0; iVtx < pEnvelope->GetNumWeights(); iVtx++)
					{
						C_Weight<F32> weight;
						pEnvelope->GetWeight(iVtx, weight);
						//old vertex position index
						U32 posIndex = weight.GetIndex();
						//new vertex position index
						S32 newPosIndex = remapping[posIndex];
						if (newPosIndex == -1)
							continue;

						if (newWeightList[newPosIndex].nWeights > 0)
						{
							weight.SetIndex(newPosIndex);
							//Set vertex weight
							newWeightList[newPosIndex].weight /= newWeightList[newPosIndex].nWeights;
							weight.SetWeight(newWeightList[newPosIndex].weight);
							//Flag this new weight so that its not processed again.
						 	newWeightList[newPosIndex].nWeights = -1;
							tempWeightList.PushBack(weight);
						}
					}

					pEnvelope->Clear();
					//Copy in the new weights.
					for (U32 iWeight = 0; iWeight < (U32)tempWeightList.Size(); iWeight++)
					{
						pEnvelope->AddWeight(tempWeightList[iWeight]);
					}
				}
			}
			break;
		}
	}
}

U32 C_MeshWeld::WeldVertexTangents(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	U32 set, 
	F32 threshold)
{
	AFTK_ASSERT(pMesh->GetNumTextureCoordinateSets() > set);

	Vector3F32Array& tangentArray = pMesh->GetTextureCoordinateSet(set).GetTangentArray();
	if (tangentArray.Size() == 0)
		return 0;

	PolygonVertexArray& polygonVertices = pMesh->GetPolygonVertexArray();
	if ((U32)polygonVertices.Size() == 0)
		return 0;

	//Animation helpers are being invalidated here.
	pMesh->ClearAnimationHelpers();

	U32 numWelds = 0;

	//This stores information regarding how to get from our old normal indices to our
	//new normal indices.
	Ut::UtVector< Ut::DynamicVector<S32> >*			pRemapping;
	weldMemoryManager.GetRemappingArray(&pRemapping);
	//This stores our new vertex normals.
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >*	pNewData;
	weldMemoryManager.GetIntermediateArray(&pNewData);

	//Create a new array of welded component data, and a table mapping from our existing data
	//to our new data.

	numWelds = InternalWeldComponents(
		weldMemoryManager,
		tangentArray,
		0,
		threshold);
	if (numWelds == 0)
		return 0;

	//We now have a welded array of data, and the required mapping table. Iterate through the components
	//and perform the remapping operation.
	for (U32 iPolyVert = 0; iPolyVert < (U32)polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolyVert = &polygonVertices[iPolyVert];

		//Skip invalid position index.
		if (pPolyVert->GetTangentIndex(set) == -1)
			continue;
		
		U32 oldIndex = (U32)pPolyVert->GetTangentIndex(set);
		U32 newIndex = (U32)((*pRemapping)[oldIndex]);

		//Finally, we set the new component index.
		pPolyVert->SetTangentIndex(set, newIndex);
	}

	//Overwrite old component data.
	tangentArray = *pNewData;
	return numWelds;
}


U32 C_MeshWeld::WeldVertexBinormals(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	U32 set, 
	F32 threshold)
{
	AFTK_ASSERT(pMesh->GetNumTextureCoordinateSets() > set);

	Vector3F32Array& binormalArray = pMesh->GetTextureCoordinateSet(set).GetBinormalArray();
	if (binormalArray.Size() == 0)
		return 0;

	PolygonVertexArray& polygonVertices = pMesh->GetPolygonVertexArray();
	if ((U32)polygonVertices.Size() == 0)
		return 0;

	//Animation helpers are being invalidated here.
	pMesh->ClearAnimationHelpers();

	U32 numWelds = 0;

	//This stores information regarding how to get from our old normal indices to our
	//new normal indices.
	Ut::UtVector< Ut::DynamicVector<S32> >*			pRemapping;
	weldMemoryManager.GetRemappingArray(&pRemapping);
	//This stores our new vertex normals.
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >*	pNewData;
	weldMemoryManager.GetIntermediateArray(&pNewData);

	//Create a new array of welded component data, and a table mapping from our existing data
	//to our new data.

	numWelds = InternalWeldComponents(
		weldMemoryManager,
		binormalArray,
		0,
		threshold);

	if (numWelds == 0)
		return 0;

	//We now have a welded array of data, and the required mapping table. Iterate through the components
	//and perform the remapping operation.
	for (U32 iPolyVert = 0; iPolyVert < (U32)polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolyVert = &polygonVertices[iPolyVert];

		//Skip invalid position index.
		if (pPolyVert->GetBinormalIndex(set) == -1)
			continue;
			
		U32 oldIndex = (U32)pPolyVert->GetBinormalIndex(set);
		U32 newIndex = (U32)((*pRemapping)[oldIndex]);

		//Finally, we set the new component index.
		pPolyVert->SetBinormalIndex(set, newIndex);
	}

	//Overwrite old component data.
	binormalArray = *pNewData;
	return numWelds;
}

U32 C_MeshWeld::WeldVertexTangentsAndBinormals(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	U32 set, 
	F32 threshold)
{
	//Animation helpers are being invalidated here.
	pMesh->ClearAnimationHelpers();

	U32 numWelds;
	numWelds = WeldVertexTangents(weldMemoryManager, pMesh, set, threshold);

	numWelds += WeldVertexBinormals(weldMemoryManager, pMesh, set, threshold);

	return numWelds;
}

U32 C_MeshWeld::WeldVertexTangentsAndBinormals(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	F32 threshold)
{
	//Animation helpers are being invalidated here.
	pMesh->ClearAnimationHelpers();

	U32 numWelds = 0;
	for (U32 iSet = 0; iSet < pMesh->GetNumTextureCoordinateSets(); iSet++)
	{
		numWelds += WeldVertexTangents(
			weldMemoryManager,
			pMesh,
			iSet,
			threshold);
		numWelds += WeldVertexBinormals(
			weldMemoryManager,
			pMesh,
			iSet,
			threshold);
	}

	return numWelds;
}

U32 C_MeshWeld::Weld(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	F32 positionThreshold,
	F32 normalThreshold,
	F32 uvThreshold,
	F32 tangentThreshold)
{
	U32 count;
	count =	WeldVertexPositions(
		weldMemoryManager,
		pMesh,
		positionThreshold);

	count += WeldVertexNormals(
		weldMemoryManager,
		pMesh,
		normalThreshold);

	count += WeldTextureCoordinates(
		weldMemoryManager,
		pMesh,
		uvThreshold);

	for (U32 iSet = 0; iSet < pMesh->GetNumTextureCoordinateSets(); iSet++)
	{
		count += WeldVertexTangentsAndBinormals(
			weldMemoryManager,
			pMesh,
			iSet,
			tangentThreshold);
	}

	count += WeldPolygonVertices(
		weldMemoryManager,
		pMesh,
		positionThreshold);
#ifdef WELD_POLYGONS
	count += WeldPolygons();
#endif
	return count;
}

U32 C_MeshWeld::WeldPolygonVertices(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	F32 threshold)
{
	//Animation helpers are being invalidated here.
	pMesh->DirtyGeometryDependantCaches();

	U32 welds = 0;
	F32 thresholdSquared = threshold*threshold;

	Position3Array vertexPositions =
		pMesh->GetVertexPositionArray();
	if (vertexPositions.Size() == 0)
		return 0;
	
	PolygonVertexArray& polygonVertices 
		= pMesh->GetPolygonVertexArray();
	if (polygonVertices.Size() == 0)
		return 0;

	//This stores information regarding how to get from our old indices to our
	//new indices.
	Ut::UtVector< Ut::DynamicVector<S32> >*						pRemapping;
	weldMemoryManager.GetRemappingArray(&pRemapping);
	//This stores our new vertex normals.
	Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >*			pNewData;
	weldMemoryManager.GetIntermediateArray(&pNewData);


	welds = InternalWeldPolygonVertices(
		weldMemoryManager,
		vertexPositions,
		polygonVertices,
		threshold);

	for (U32 iLodLevel = 0; iLodLevel < pMesh->GetNumLodLevels(); iLodLevel++)
	{
		for (U32 polyIndex = 0; polyIndex < pMesh->GetNumPolygons(iLodLevel); polyIndex++)
		{
			C_Polygon* pPoly = &pMesh->GetPolygon(iLodLevel, polyIndex);
			for (U32 iPolyVert = 0; iPolyVert < pPoly->GetNumPolygonVertices(); iPolyVert++)
			{
				U32 polyVertIndex = pPoly->GetPolygonVertexIndex(iPolyVert);
				U32 newPolyVertIndex = (*pRemapping)[polyVertIndex];

				pPoly->SetPolygonVertexIndex(iPolyVert, newPolyVertIndex);
			}
		}
	}

	polygonVertices = *pNewData;
	return welds;
}

U32 C_MeshWeld::WeldPolygons(Aftk::C_SceneMeshNode* pMesh)
{
	U32 welds = 0;
	for (U32 i = 0; i < pMesh->GetNumLodLevels(); i++)
	{
		welds += WeldPolygons(pMesh, i);
	}
	return welds;
}

U32 C_MeshWeld::WeldPolygons(Aftk::C_SceneMeshNode* pMesh, U32 lodLevel)
{
	Ut::OpenHashMap<Ut::UtSet<S32>, const Aftk::C_Polygon*, HashKeyOpSet<S32> >
		uniquePolys;
	
	for (U32 i = 0; i < pMesh->GetNumPolygons(lodLevel); i++)
	{
		const Aftk::C_Polygon& poly = pMesh->GetPolygon(lodLevel, i);
		Ut::UtSet<S32> iIdxSet;
		for (U32 j = 0; j < poly.GetNumPolygonVertices(); j++)
			iIdxSet.Insert(poly.GetPolygonVertexIndex(j));

		Ut::OpenHashMap<Ut::UtSet<S32>, const Aftk::C_Polygon*, HashKeyOpSet<S32> >::InsertionReturnType result =
			uniquePolys.Insert(iIdxSet);
		result.first->GetValue() = &poly;
	}

	U32 welds = pMesh->GetNumPolygons(lodLevel) - uniquePolys.Size();
	if (welds == 0)
		return 0;

	PolygonArray newData;
	newData.Reserve(uniquePolys.Size());
	for (Ut::OpenHashMap<Ut::UtSet<S32>, const Aftk::C_Polygon*, HashKeyOpSet<S32> >::Iterator it = uniquePolys.Begin();
		it != uniquePolys.End();
		++it)
	{
		newData.PushBack(*it->GetValue());
	}

	for (U32 i = 0; i < (U32)pMesh->GetNumPolygonClusters(); i++)
	{
		Aftk::C_PolygonCluster* pCluster = pMesh->GetPolygonClusterPtr(i);
		//Remove all indices for this lod level.
		pCluster->DeletePolygonIndicesDoNotModifyPolygons(lodLevel);
	}

	//Overwrite old poly data.
	Ut::MoveRaw(pMesh->GetPolygonArray(lodLevel), newData);

	for (U32 i = 0; i < (U32)pMesh->GetNumPolygons(lodLevel); i++)
	{
		Aftk::C_Polygon& poly = pMesh->GetPolygon(lodLevel, i);
		//Associate with cluster again.
		if (poly.GetClusterPtr())
			poly.GetClusterPtr()->AddPolygonIndexDoNotModifyPolygon(lodLevel, i);
	}


	return welds;
}

U32 C_MeshWeld::WeldComponents(
	C_MeshWeldMemoryManager& weldMemoryManager,
	Aftk::C_SceneMeshNode* pMesh,
	F32 threshold)
{
	U32 numWelds;

	numWelds = WeldVertexNormals(
		weldMemoryManager,
		pMesh,
		threshold);

	for (U32 set = 0; set < pMesh->GetNumTextureCoordinateSets(); set++)
	{
		numWelds += WeldVertexTangents(
			weldMemoryManager,
			pMesh,
			set,
			threshold);

		numWelds += WeldVertexBinormals(
			weldMemoryManager,
			pMesh,
			set,
			threshold);
	
		numWelds += WeldTextureCoordinates(
			weldMemoryManager,
			pMesh,
			set, 
			threshold);
	}

	return numWelds;
}

S32 C_MeshWeld::InternalLinearFindFirstPolygonVertexWeldIndex(
	const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
	const Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >& sortedIndexList,
	U32 sortedCompareIndex,
	F32 threshold,
	U32 sortedStartIndex)
{
	F32 thresholdSquared = threshold*threshold;
	S32 firstIndex = -1;
	//Search backward for the first welding index
	for (S32 i = sortedStartIndex; i >= 0; i--)
	{
		F32 test = sortedIndexList[sortedCompareIndex].axis - sortedIndexList[i].axis;
		if (fabsf(test) <= threshold)
		{
			firstIndex = i;
		}
		else
		{
			return firstIndex;
		}
	}

	return firstIndex;
}


S32 C_MeshWeld::InternalBinaryFindFirstPolygonVertexWeldIndex(
	const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
	const Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >& sortedIndexList,
	U32 sortedCompareIndex,
	F32 threshold,
	U32 sortedLowIndex,
	U32 sortedHighIndex)
{
	F32 thresholdSquared = threshold*threshold;
	S32 firstIndex = -1;

	//Cut-off below which we perform a brute force O(n^2) search.
	static U32 cutOff = 1;
	if (sortedLowIndex + cutOff > sortedHighIndex)
	{
		//We've only a few entries to search. Find the first point within our threshold.
		//Search backward for the first welding index
		for (U32 i = sortedLowIndex; i <= sortedHighIndex; i++)
		{
			if (AFTK_MESH_WELDER_GET_SORTED(sortedCompareIndex).EqualityTest(
				AFTK_MESH_WELDER_GET_SORTED(i), thresholdSquared))
			{
				return i;
			}
		}
	}
	else
	{
		//The middle index is our 'pivot' point.
		U32 middleSorted = (sortedLowIndex + sortedHighIndex) / 2;
		C_PolygonVertex target = AFTK_MESH_WELDER_GET_SORTED(sortedCompareIndex);
		C_PolygonVertex pivot = AFTK_MESH_WELDER_GET_SORTED(middleSorted);

		F32 test = sortedIndexList[sortedCompareIndex].axis - sortedIndexList[middleSorted].axis;
		if (fabsf(test) <= threshold)
		{
			//The pivot is within our weld threshold. Go left from the pivot to find
			//the first point within our threshold.
			S32 firstPointWithin = InternalLinearFindFirstPolygonVertexWeldIndex(
				searchArray,
				sortedIndexList,
				sortedCompareIndex,
				threshold,
				middleSorted);

			return firstPointWithin;
		}
		else
		{
			//Go left if the value we're looking for lies on the left of the pivot point.
			if (sortedIndexList[sortedCompareIndex].axis < sortedIndexList[middleSorted].axis)
			{
				firstIndex = InternalBinaryFindFirstPolygonVertexWeldIndex(
					searchArray,
					sortedIndexList,
					sortedCompareIndex, 
					threshold, 
					sortedLowIndex, 
					middleSorted-1);
			}
			//Else go right if the value we're looking for lies on the right of the pivot point.
			else
			{
				firstIndex = InternalBinaryFindFirstPolygonVertexWeldIndex(
					searchArray, 
					sortedIndexList,
					sortedCompareIndex,
					threshold,
					middleSorted+1,
					sortedHighIndex);
			}
		}
	}

	return firstIndex;
}


U32 C_MeshWeld::InternalWeldPolygonVertices(
	C_MeshWeldMemoryManager& weldMemoryManager,
	const Ut::UtVector< Ut::DynamicVector<Position3> >& positionArray,
	const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
	F32 threshold)
{
	if (searchArray.Size() == 0)
	{
		return 0;
	}

	F32 thresholdSquared = threshold*threshold;

	U32 numWelds = 0;

	Ut::UtVector<Ut::DynamicVector<C_MeshWeldComponentAxis> >* pSortedIndexList;
	weldMemoryManager.GetSortedIndexArray(&pSortedIndexList);
	pSortedIndexList->Resize(searchArray.Size());

	Ut::UtVector<Ut::DynamicVector<bool> >* pComponentsToSkip;
	weldMemoryManager.GetComponentsToSkipArray(&pComponentsToSkip);

	Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >* pIntermediateData;
	weldMemoryManager.GetIntermediateArray(&pIntermediateData);

	Ut::UtVector< Ut::DynamicVector<S32> >* pRemapping;
	weldMemoryManager.GetRemappingArray(&pRemapping);

	//Size arrays
	pRemapping->Resize(searchArray.Size());
	pComponentsToSkip->Reserve(searchArray.Size());
	pIntermediateData->Reserve(searchArray.Size());

	E_Axis axis = InternalGetWidestAxisPosition<Position3>(positionArray);

	//Initialise arrays.
	for (U32 i = 0; i < (U32)searchArray.Size(); i++)
	{
		(*pSortedIndexList)[i].axis = positionArray[searchArray[i].GetPositionIndex()][axis];
		(*pSortedIndexList)[i].index = i;
		pComponentsToSkip->PushBack(false);
	}

	C_MeshWeldComponentAxis* pStart = &(*pSortedIndexList)[0];
	C_MeshWeldComponentAxis* pEnd = &(*pSortedIndexList)[searchArray.Size()-1];
	pEnd++;
	std::sort(pStart, pEnd);

	for (U32 i = 0; i < (U32)searchArray.Size(); i++)
	{
		C_PolygonVertex pv = searchArray[(*pSortedIndexList)[i].index];
	}

	for (U32 i = 0 ; i < (U32)searchArray.Size(); i++)
	{
		if ((*pComponentsToSkip)[i])
		{
			continue;
		}

		U32 posIndexI = (*pSortedIndexList)[i].index;
		pIntermediateData->PushBack(searchArray[posIndexI]);
		U32 newDataIndex = pIntermediateData->Size()-1;
		(*pRemapping)[posIndexI] = newDataIndex;

		S32 firstWeldIndex;

		firstWeldIndex = InternalBinaryFindFirstPolygonVertexWeldIndex(
			searchArray,
			*pSortedIndexList,
			i,
			threshold,
			0, 
			searchArray.Size()-1);

		if (firstWeldIndex == -1)
		{
			continue;
		}


		for (U32 j = (U32)firstWeldIndex; j < (U32)searchArray.Size(); j++)
		{
			U32 posIndexJ = (*pSortedIndexList)[j].index;

			//Make sure we skip the 'current' vertex position index.
			if (i == j)
			{
				continue;
			}

			F32 test = (*pSortedIndexList)[j].axis - (*pSortedIndexList)[i].axis;
			if (fabsf(test) > threshold)
			{
				if ((*pSortedIndexList)[j].axis > (*pSortedIndexList)[i].axis)
				{
					//Break out of inner loop
					break;
				}
			}

			if (searchArray[posIndexI].EqualityTest(
				searchArray[posIndexJ], thresholdSquared))
			{
				numWelds++;

				(*pComponentsToSkip)[j] = true;
				(*pRemapping)[posIndexJ] = newDataIndex;
			}
		}
	}

	return numWelds;
}
