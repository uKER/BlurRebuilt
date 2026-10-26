#include "stdafx.h"

//TODO: Once global -1 scale has been made to respect base pose information we can clean up the
//horrid guerilla z-flipping here. The horror is almost too much to bear, but this can't be refactored
//now as everyone's worked around the bug!

using namespace AftkUt;

const char* C_GenerateAdPlacement::m_namingConvention = "MAD_";

const Aftk::C_SceneMeshRemappableNode* GetMeshRemappableNode(const Aftk::C_SceneNode* pSceneNode)
{
	if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
	{
		return (const Aftk::C_SceneMeshRemappableNode*)pSceneNode;
	}
	else if (pSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
	{
		const Aftk::C_SceneInstanceNode* pInst = 
			(const Aftk::C_SceneInstanceNode*)pSceneNode;

		if (pInst->GetSourceNodePtr())
			return GetMeshRemappableNode(pInst->GetSourceNodePtr());
	}

	return 0;
}

Aftk::C_Status C_GenerateAdPlacement::Generate(
	Ut::UtVector<Ut::DynamicVector<Quad> >& quadArray,
	const Aftk::C_Scene* pScene,
	const Aftk::C_LayerNode* pLayer,
	ProcessThisNodeFunc pProcessThisNodeFunc)
{
	Aftk::C_Status status;

	//List of ad materials for a mesh.
	MaterialArray materialList;
	for (Aftk::C_SceneNodeConstIterator it(pScene); it; ++it)
	{
		const Aftk::C_SceneNode* pSceneNode = it;

		//Skip layers that we're not interested in, if applicable.
		if (pLayer)
		{
			bool found = false;
			for (U32 i = 0; i < pSceneNode->GetNumLayers(); i++)
			{
				if (pLayer == pSceneNode->GetLayerPtr(i))
				{
					found = true;
					break;
				}
			}
			if (!found)
				continue;
		}
		else
		{
			//This node is on a layer, skip it.
			if (pSceneNode->GetNumLayers() != 0)
				continue;
		}

		//Use our user-specified callback to determine if this node should be processed.
		if (pProcessThisNodeFunc && !(*pProcessThisNodeFunc)(*pSceneNode))
		{
			//We're supposed to skip this node.
			continue;
		}

		const Aftk::C_SceneMeshRemappableNode* pMaterialSource = 
			GetMeshRemappableNode(pSceneNode);
		if (pMaterialSource)
		{
			status = BuildAdvertisingMaterialList(
				materialList,
				*pMaterialSource);
			if (!status) return status;
		}
		else
			continue;

		//LOD group handling.
		if (pSceneNode->GetParentPtr() && pSceneNode->GetParentPtr()->HasType(
			Aftk::E_ObjType_SceneLodGroup))
		{
			//Grab our parent LOD group.
			const Aftk::C_SceneLodGroupNode& lodGroup = 
				(const Aftk::C_SceneLodGroupNode&)*pSceneNode->GetParentPtr();
			if (lodGroup.GetLodLevelIndex(pSceneNode) > 0)
			{
				//We're only interested in the highest level of detail.
				continue;
			}
		}

		//Ignore meshes that don't actually have any advertising materials applied to their
		//polys.
		if (!HasAdvertising(
			materialList,
			(Aftk::C_SceneMeshRemappableNode&)*pMaterialSource))
			continue;

		if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshBonedInstance))
		{
			status = GenerateFromDeformedMesh(
				quadArray,
				materialList,
				(Aftk::C_SceneMeshBonedInstanceNode&)*pSceneNode);
			if (!status) return status;
		}
		else if (pMaterialSource->HasType(Aftk::E_ObjType_ISceneMesh))
		{
			status = GenerateFromStaticMesh(
				quadArray,
				materialList,
				pSceneNode->GetWorldMatrix(),
				*pMaterialSource);
			if (!status) return status;
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

bool C_GenerateAdPlacement::HasAdvertising(
	const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode)
{
	MaterialArray materialList;
	if (!BuildAdvertisingMaterialList(
		materialList, aftkSceneMeshNode))
		return false;

	return HasAdvertising(materialList,	aftkSceneMeshNode);
}

bool C_GenerateAdPlacement::HasAdvertising(
	const MaterialArray& materialList,
	const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode)
{
	for (U32 iPoly = 0; iPoly < (U32)aftkSceneMeshNode.GetNumPolygons(0); iPoly++)
	{
		S16 mtl = aftkSceneMeshNode.GetPolygon(iPoly).GetMaterialMappingIndex();
		if (mtl != -1 && materialList[mtl].pMaterial)
			//We've found a polygon with an advertising material.
			return true;
	}

	return false;
}

S32 C_GenerateAdPlacement::HasAdvertising(
	const MaterialArray& materialList,
	const Aftk::C_SceneMeshRemappableNode& materialSource,
	const Aftk::C_Polygon& aftkPolygon)
{
	S16 idx = aftkPolygon.GetMaterialMappingIndex();
	if (idx == -1)
		return -1;

	if (!materialList[idx].pMaterial)
		return -1;
	assert(materialList[idx].pMaterial == materialSource.GetPolygonMaterialMappingPtr(idx));
	return materialList[idx].placementIndex;
}

S32 C_GenerateAdPlacement::GetPlacementIndex(const Aftk::C_MaterialNode& material)
{
	for (U32 i = 0; i < material.GetNumShaderImageNodes(); i++)
	{
		const Aftk::C_ShaderImageNode* pImg = material.GetShaderImageNodePtr(i);
		std::string tmp;
		pImg->GetFilePlug().GetValue(tmp);
		if (IsAdvertisingMaterial(tmp.c_str()))
			return GetPlacementIndex(tmp.c_str());
	}

	return -1;
}

S32 C_GenerateAdPlacement::GetPlacementIndex(const Util::C_String& fileName)
{
	S32 offset = (S32)strlen(m_namingConvention);
	S32 tokenIndex = fileName.ISearch(m_namingConvention);
	if (tokenIndex == -1)
		//No index found.
		return -1;

	Util::C_String digitsString = fileName(tokenIndex+offset, fileName.Length());
	return atoi(digitsString.Chars());
}

Aftk::C_Status C_GenerateAdPlacement::BuildAdvertisingMaterialList(
	MaterialArray& materialList,
	const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode)
{
	if (aftkSceneMeshNode.GetNumPolygonMaterialMappings() == 0)
		return Aftk::C_Status::E_Status_Success;

	//Allocate storage for our ad material table.
	materialList.Resize(aftkSceneMeshNode.GetNumPolygonMaterialMappings());
	memset(&materialList[0], 0, sizeof(Material)*materialList.Size());
	//Find advertising materials.
	for (S16 iMtl = 0; iMtl < aftkSceneMeshNode.GetNumPolygonMaterialMappings(); iMtl++)
	{
		const Aftk::C_MaterialResource* pRes = aftkSceneMeshNode.GetPolygonMaterialMappingPtr(iMtl);
		if (!pRes)
			continue;

		Util::C_String textureFilePath;
		const Aftk::C_MaterialNode& material = pRes->GetContent();
		for (U32 jImage = 0; jImage < material.GetNumShaderImageNodes(); jImage++)
		{
			const Aftk::C_ShaderImageNode& img = *material.GetShaderImageNodePtr(jImage);
			std::string tmp;
			img.GetFilePlug().GetValue(tmp);
			textureFilePath = tmp.c_str();
			
			if (IsAdvertisingMaterial(textureFilePath))
			{
				S32 placementIndex = GetPlacementIndex(textureFilePath);
				if (placementIndex != -1)
				{
					materialList[iMtl].pMaterial = pRes;
					materialList[iMtl].placementIndex = placementIndex;
				}
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

bool C_GenerateAdPlacement::IsAdvertisingMaterial(
	const Aftk::C_MaterialNode& material)
{
	for (U32 i = 0; i < material.GetNumShaderImageNodes(); i++)
	{
		const Aftk::C_ShaderImageNode& img = *material.GetShaderImageNodePtr(i);
		std::string tmp;
		img.GetFilePlug().GetValue(tmp);
		if (IsAdvertisingMaterial(tmp.c_str()))
			return true;
	}
	return false;
}

bool C_GenerateAdPlacement::IsAdvertisingMaterial(
	const Util::C_String& textureFilePath)
{
	if (textureFilePath.ISearch(m_namingConvention) != -1)
		return true;

	return false;
}

Aftk::C_Status C_GenerateAdPlacement::GenerateIsland(
	Ut::UtVector<Ut::DynamicVector<U32> >& island,
	const MaterialArray& materialList,
	const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode,
	U32 firstPolyIndex)
{
	Ut::BitVector polygonsProcessed;
	//Initialise table indicating which polys have been processed so far.
	polygonsProcessed.Resize(aftkSceneMeshNode.GetNumPolygons(0));
	polygonsProcessed.ClearBits();

	polygonsProcessed.SetBit(firstPolyIndex);

	Ut::UtVector<Ut::DynamicVector<U32> > polyList;
	polyList.Reserve(aftkSceneMeshNode.GetNumPolygons(0)/2);
	polyList.PushBack(firstPolyIndex);

	//The size of this list will grow during processing.
	for (S32 i = 0; i < polyList.Size(); i++)
	{
		U32 polyIndex = polyList[i];
		const Aftk::C_Polygon& poly = aftkSceneMeshNode.GetPolygon(polyIndex);
		
		//Add to our island.
		island.PushBack(polyIndex);

		//For each remaining polygon, try to create an island of connected polygons.

		U32 nEdges = poly.GetNumPolygonVertices();
		for (U32 jEdge = 0; jEdge < nEdges; jEdge++)
		{
			S32 adjacentPolyIndex = poly.GetAdjacentPolygonIndex(jEdge);
			if (adjacentPolyIndex == -1)
				continue;

			const Aftk::C_Polygon& adjPoly = aftkSceneMeshNode.GetPolygon(adjacentPolyIndex);
			S16 adjMaterialIndex = adjPoly.GetMaterialMappingIndex();
			///Ignore polygons without an advertising material, also ignore if previously processed.
			if (adjMaterialIndex == -1 || 
				!materialList[adjMaterialIndex].pMaterial ||
				polygonsProcessed.GetBitValue(adjacentPolyIndex))
				continue;

			polygonsProcessed.SetBit(adjacentPolyIndex);
			polyList.PushBack(adjacentPolyIndex);
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_GenerateAdPlacement::GenerateFromStaticMesh(
	Ut::UtVector<Ut::DynamicVector<Quad> >& quadArray,
	const MaterialArray& materialList,
	const Aftk::Matrix44& worldMatrix,
	const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode)
{
	Aftk::C_Status status;

	Ut::BitVector polygonsProcessed;
	Ut::UtVector<Ut::DynamicVector<U32> >	toProcess;

	//Initialise table indicating which polys have been processed so far.
	polygonsProcessed.Resize(aftkSceneMeshNode.GetNumPolygons(0));
	polygonsProcessed.ClearBits();

	//Reserve memory.
	toProcess.Reserve(aftkSceneMeshNode.GetNumPolygons(0) /2);

	Ut::UtVector<Ut::DynamicVector<
		Ut::UtVector<Ut::DynamicVector<U32> > > > islands;

	//Generate islands.
	for (U32 i = 0; i < aftkSceneMeshNode.GetNumPolygons(0); i++)
	{
		const Aftk::C_Polygon& poly = aftkSceneMeshNode.GetPolygon(i);

		S16 materialIndex = poly.GetMaterialMappingIndex();
		///Ignore polygons without an advertising material.
		if (materialIndex == -1 || 
			!materialList[materialIndex].pMaterial || 
			polygonsProcessed.GetBitValue(i))
			continue;

		Ut::UtVector<Ut::DynamicVector<U32> > island;
		status = GenerateIsland(
			island,
			materialList,
			aftkSceneMeshNode,
			i);
		if (!status) return status;

		//Add island's polygons to our list of processed polys.
		for (S32 jIslandPoly = 0; jIslandPoly < island.Size(); jIslandPoly++)
			polygonsProcessed.SetBit(island[jIslandPoly]);

		if (island.Size() > 0)
			islands.PushBack(island);
	}

	Aftk::Vector3F32 upVector(0,1.f,0);

	//Generate quads from islands.
	Aftk::Vector3F32 averageNormal;
	memset(&averageNormal[0], 0, sizeof(averageNormal));

	Aftk::Matrix44 rootInv;
	UtMaths::SetIdentity(&rootInv);
	rootInv[2][2] = -1.f;

	Aftk::Matrix44 matrixMinusRoot = 
		worldMatrix * 
		rootInv;

	for (S32 i = 0; i < islands.Size(); i++)
	{
		const Ut::UtVector<Ut::DynamicVector<U32> >& island = islands[i];
		Aftk::C_Bounds worldBounds;
		//Find the vertical extents of the parts of the mesh which actually have advertising on them.

		S32 islandPlacementIndex = -1;
		Ut::Vector<Aftk::Vector3F32> pointCloud;
		for (U32 i = 0; i < (U32)island.Size(); i++)
		{
			U32 polyIndex = island[i];
			const Aftk::C_Polygon& poly = aftkSceneMeshNode.GetPolygon(polyIndex);
			S32 idx = HasAdvertising(
				materialList,
				aftkSceneMeshNode,
				poly);
			if (idx == -1)
				continue;
			else
				islandPlacementIndex = idx;

			averageNormal += (Aftk::Vector3F32)poly.GetNormal(&matrixMinusRoot);

			for (U32 jPV = 0; jPV < poly.GetNumPolygonVertices(); jPV++)
			{
				if (poly.GetPolygonVertexIndex(jPV) == -1)
					continue;

				Aftk::Position3 vPos = aftkSceneMeshNode.GetEvaluatedPolygonVertexPosition(
					poly.GetPolygonVertexIndex(jPV));

				vPos = Aftk::ApplyMatrix(matrixMinusRoot, vPos);

				worldBounds.AddVertex(vPos);
				pointCloud.PushBack((Aftk::Vector3F32&)vPos);
			}
		}

		if (Aftk::MagnitudeSquared(averageNormal) < 1e-3)
		{
			LOGWARN(g_Log, "Failed to generate advertisment placement on mesh %s. This was because the average poly-normal for a group of connected advertising polys was zero. One way this can happen is if advertisment polygons are flagged as double-sided. This requires an art fix, email 'tools' for more info if needed.",
				aftkSceneMeshNode.GetName().c_str());
			continue;
		}

		assert(islandPlacementIndex != -1);
		//Normal vector.
		UtMaths::Normalise(&averageNormal);

		//Get right vector.
		Aftk::Vector3F32 rightVector = averageNormal ^ upVector;
		UtMaths::Normalise(&rightVector);

		Aftk::Vector3F32 normalVector = upVector ^ rightVector;
		UtMaths::Normalise(&normalVector);

		//The plane we're projecting our 3d point-set onto is defined by our
		//upvector and our rightvector. The origin of the plane is defined by 
		//the average of our pointset.

		//Plane origin in world space.
		Aftk::Vector3F32 planeWorldOrigin = (Aftk::Vector3F32&)worldBounds.GetCentre();

		//Build our inv projection matrix.
		Aftk::Matrix44 invProj;
		invProj[0][0] = rightVector[0]; invProj[0][1] = rightVector[1]; invProj[0][2] = rightVector[2]; invProj[0][3] = 0.f;
		invProj[1][0] = upVector[0]; invProj[1][1] = upVector[1]; invProj[1][2] = upVector[2]; invProj[1][3] = 0.f;

		invProj[2][0] = normalVector[0]; invProj[2][1] = normalVector[1]; invProj[2][2] = normalVector[2]; invProj[2][3] = 0.f;
		invProj[3][0] = planeWorldOrigin[0]; invProj[3][1] = planeWorldOrigin[1]; invProj[3][2] = planeWorldOrigin[2]; invProj[3][3] = 1.f;

		//Get projection matrix.
		Aftk::Matrix44 proj;
		UtMaths::Invert4x4(&proj, invProj);

		//Transform point cloud into plane space and calculate extents
		Aftk::C_Bounds cloudBounds;
		for (U32 i = 0; i < (U32)pointCloud.Size(); i++)
		{
			Aftk::Vector3F32 transformedPoint = Aftk::ApplyMatrix(proj, pointCloud[i]);
			cloudBounds.AddVertex((Aftk::Position3&)transformedPoint);
		}

		//Project extents into 3D world space.
		Aftk::Position3 minBounds = cloudBounds.GetMin();
		minBounds = Aftk::ApplyMatrix(invProj, minBounds);

		Aftk::Position3 maxBounds = cloudBounds.GetMax();
		maxBounds = Aftk::ApplyMatrix(invProj, maxBounds);
		
		//Let's add a new quad then.
		quadArray.PushBack();

		Quad& quad = quadArray[quadArray.Size()-1];
		//Bottom left.
		quad.v[3] = minBounds;
		//Top left.
		quad.v[2][0] = minBounds[0];
		quad.v[2][1] = maxBounds[1];
		quad.v[2][2] = minBounds[2];
		//Top right.
		quad.v[1] = maxBounds;
		//Bottom right.
		quad.v[0][0] = maxBounds[0];
		quad.v[0][1] = minBounds[1];
		quad.v[0][2] = maxBounds[2];

		assert(islandPlacementIndex != -1);
		quad.placementIndex = islandPlacementIndex;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_GenerateAdPlacement::GenerateFromDeformedMesh(
	Ut::UtVector<Ut::DynamicVector<Quad> >& quadArray,
	const MaterialArray& materialList,
	const Aftk::C_SceneMeshBonedInstanceNode& aftkSceneMeshBonedInstance)
{
	Aftk::C_Status status;

	//Get bone positions.
	const Aftk::C_SceneNode** pBones = aftkSceneMeshBonedInstance.GetBoneArray();
	if (!pBones[0] || !pBones[1])
	{
		//Warning here.

		return Aftk::C_Status::E_Status_Success;
	}

	//Get world-space positions of bones.
	Aftk::Vector3F32 boneWorldPositions[2];
	memcpy(&boneWorldPositions[0], &pBones[0]->GetWorldMatrix()[3][0], sizeof(Aftk::Vector3F32));
	memcpy(&boneWorldPositions[1], &pBones[1]->GetWorldMatrix()[3][0], sizeof(Aftk::Vector3F32));

	Aftk::Matrix44 rootInv;
	UtMaths::SetIdentity(&rootInv);
	rootInv[2][2] = -1.f;

	boneWorldPositions[0] = Aftk::ApplyMatrix(rootInv, boneWorldPositions[0]);
	boneWorldPositions[1] = Aftk::ApplyMatrix(rootInv, boneWorldPositions[1]);

	//Get xz extents.
	Aftk::Position3 min;
	min[0] = boneWorldPositions[0][0];
	(U32&)min[1] = 0xffffffff;
	min[2] = boneWorldPositions[0][2];
	
	Aftk::Position3 max;
	max[0] = boneWorldPositions[1][0];
	(U32&)max[1] = 0xffffffff;
	max[2] = boneWorldPositions[1][2];

	//Find the vertical extents of the parts of the mesh which actually have advertising on them.
	Aftk::Matrix44 matrixMinusRoot = 
		aftkSceneMeshBonedInstance.GetWorldMatrix() * 
		rootInv;

	S32 placementIndex = -1;
	for (U32 iPoly = 0; iPoly < aftkSceneMeshBonedInstance.GetNumPolygons(0); iPoly++)
	{
		const Aftk::C_Polygon& poly = aftkSceneMeshBonedInstance.GetPolygon(iPoly);
		S32 idx = HasAdvertising(
			materialList,
			aftkSceneMeshBonedInstance,
			poly);
		if (idx == -1)
			continue;
		else
			placementIndex = idx;

		for (U32 jPV = 0; jPV < poly.GetNumPolygonVertices(); jPV++)
		{
			if (poly.GetPolygonVertexIndex(jPV) == -1)
				continue;

			Aftk::Position3 vPos = aftkSceneMeshBonedInstance.GetEvaluatedPolygonVertexPosition(
				poly.GetPolygonVertexIndex(jPV));

			vPos = Aftk::ApplyMatrix(matrixMinusRoot, vPos);

			if ((U32&)min[1] == 0xffffffff)
				min[1] = vPos[1];
			if ((U32&)max[1] == 0xffffffff)
				max[1] = vPos[1];

			if (vPos[1] < min[1])
				min[1] = vPos[1];
			if (vPos[1] > max[1])
				max[1] = vPos[1];
		}
	}

	//Let's add a new quad then.
	quadArray.PushBack();

	Quad& quad = quadArray[quadArray.Size()-1];
	//Bottom left.
	quad.v[0] = min;
	//Top left.
	quad.v[1][0] = min[0];
	quad.v[1][1] = max[1];
	quad.v[1][2] = min[2];
	//Top right.
	quad.v[2] = max;
	//Bottom right.
	quad.v[3][0] = max[0];
	quad.v[3][1] = min[1];
	quad.v[3][2] = max[2];

	//Use the last placement index found.
	quad.placementIndex = placementIndex;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_GenerateAdPlacement::GenerateAftkMeshFromQuadArray(
	Aftk::C_Scene* pScene,
	const Ut::UtVector<Ut::DynamicVector<Quad> >& quadArray)
{
	if (!pScene->GetRootSceneNodePtr())
		return Aftk::C_Status::E_Status_Failure;

	Aftk::C_Status status;

	for (S32 i = 0; i < quadArray.Size(); i++)
	{
		Aftk::C_SceneMeshNode* pMesh = new Aftk::C_SceneMeshNode(pScene);
		if (!pMesh)
			continue;

		status = GenerateAftkMeshFromQuad(*pMesh, quadArray[i]);
		if (!status)
			return status;

		pMesh->SetName("AdQuad");
		pScene->GetRootSceneNodePtr()->AddChildPtr(pMesh);
	}
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_GenerateAdPlacement::GenerateAftkMeshFromQuad(
	Aftk::C_SceneMeshNode& aftkSceneMeshNode,
	const Quad& quad)
{
	aftkSceneMeshNode.AddPolygon();
	Aftk::C_Polygon& poly = aftkSceneMeshNode.GetPolygon(0);
	U32 i;
	for (i = 0; i < 4; i++)
	{
		aftkSceneMeshNode.AddVertexPosition(quad.v[i]);
		aftkSceneMeshNode.AddPolygonVertex();
		Aftk::C_PolygonVertex& pv = aftkSceneMeshNode.GetPolygonVertex(i);
		pv.SetPositionIndex(i);

		poly.AddPolygonVertexIndex(i);
	}

	Aftk::C_Status status = Aftk::C_MeshCalculateNormals::CalculateNormals(&aftkSceneMeshNode);
	if (!status) return status;

	return Aftk::C_Status::E_Status_Success;
}
