#include "stdafx.h"

#include <Gravity/NarrowPhase/Primitive.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>

#include <EditorLib/Collision/Ray.h>
#include <EditorLib/Collision/CollisionStructs.h>
#include <EditorLib/Collision/CollisionResult.h>
#include <EditorLib/Collision/CollisionManager.h>
#include <EditorLib/Collision/CollisionScene.h>

#include <EditorLib/Aftk/AftkCollisionBuilder.h>
#include <EditorLib/Aftk/AftkRefHelpers.h>

#include <EditorLib/Core.h>

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUtQuery.h>
#include <Aftkut/AftkUtUtility.h>

namespace EditorLib
{

//Note that bendy barriers aren't currently supported, this may need to be added. We should probably at
//least think about it.

C_Status C_AftkCollisionBuilder::Build(
	C_CollisionScene& collisionScene, 
	const Btl::String& originalFilePath,
	const AftkDgMap& aftkDgMap)
{
	AftkCollisionSourceMap collisionSourceMap;
	C_Status status;
	status = BuildSources(
		collisionScene, 
		collisionSourceMap, 
		originalFilePath,
		aftkDgMap);
	if (!status) return status;

	status = BuildCollisionInstances(
		collisionScene, 
		collisionSourceMap,
		aftkDgMap);
	if (!status) return status;

	return E_Status_Success;
}

C_Status C_AftkCollisionBuilder::BuildSources(
	C_CollisionScene& collisionScene, 
	AftkCollisionSourceMap& collisionSourceMap,
	const Btl::String& originalFilePath,
	const AftkDgMap& aftkDgMap)
{
	C_Status status;
	for (AftkDgMap::ConstIterator it(aftkDgMap.Begin()); it != aftkDgMap.End(); ++it)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = it->GetKey();
		//Skip non exportable.
		if (AftkRefHelpers::IsToBeHiddenPermanently(pAftkSceneNode) && pAftkSceneNode->GetNumInstancesOfThisNode() == 0)
			continue;

		if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			status = AddMeshSource(
				collisionScene,
				collisionSourceMap,
				originalFilePath,
				(const Aftk::C_SceneMeshNode*)pAftkSceneNode);
			if (!status) return status;
		}
		else if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneSpline)
		{
			status = AddSplineSource(
				collisionScene,
				collisionSourceMap,
				originalFilePath,
				(const Aftk::C_SceneSplineNode*)pAftkSceneNode);
			if (!status) return status;
		}
		else if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneNode)
		{
			status = AddPointSource(
				collisionScene,
				collisionSourceMap,
				pAftkSceneNode);
			if (!status) return status;
		}
	}

	return E_Status_Success;
}

C_Status C_AftkCollisionBuilder::AddMeshSource(
	C_CollisionScene& collisionScene, 
	AftkCollisionSourceMap& collisionSourceMap,
	const Btl::String& originalFilePath,
	const Aftk::C_SceneMeshNode* pAftkMesh)
{
	//Ignore meshes with no polygons.
	if (pAftkMesh->GetNumPolygons(0) == 0 || 
		pAftkMesh->GetNumVertexPositions() < 3 ||
		pAftkMesh->GetNumPolygonVertices() < 3)
		return E_Status_Success;

	C_Status status;

	C_CollisionMesh* pCollisionMesh = new C_CollisionMesh;
	Maths::HVec3 sphereCentre(		
		pAftkMesh->GetLocalBounds().GetCentre().x,
		pAftkMesh->GetLocalBounds().GetCentre().y,
		pAftkMesh->GetLocalBounds().GetCentre().z);

	F32 radius = Maths::Sqrt(pAftkMesh->GetLocalBounds().GetObjectRadiusSquared());

	//OBB calculation.
	{
		pCollisionMesh->obb.transform.SetIdentity();
		pCollisionMesh->obb.transform.AW(sphereCentre);
		Aftk::Vector3F32 extents = 
			pAftkMesh->GetLocalBounds().GetMax() - pAftkMesh->GetLocalBounds().GetMin();
		extents = extents * 0.5f;
		if (extents[0] * extents[1] * extents[2] < 0.01f)
		{
			if (extents[0] < 0.01f)
				extents[0] = 0.01f;
			if (extents[1] < 0.01f)
				extents[1] = 0.01f;
			if (extents[2] < 0.01f)
				extents[2] = 0.01f;
		}
		pCollisionMesh->obb.extents = Maths::HVec3(extents[0], extents[1], extents[2]);
	}

	pCollisionMesh->nTris = pAftkMesh->GetNumPolygons(0);
	U32 nIndices = pCollisionMesh->nTris * 3;
	pCollisionMesh->nVertices = pAftkMesh->GetNumVertexPositions();
	pCollisionMesh->pVertices = new Vector3[pCollisionMesh->nVertices];
	pCollisionMesh->pTriIndices = new U32[nIndices];

	assert(sizeof(Vector3) == sizeof(Aftk::Position3));
	memcpy(pCollisionMesh->pVertices, &pAftkMesh->GetVertexPositionArray()[0], sizeof(Vector3)*pCollisionMesh->nVertices);

	U32* pTriangle = pCollisionMesh->pTriIndices;
	for (U32 i = 0; i < pCollisionMesh->nTris; i++, pTriangle+=3)
	{
		const Aftk::C_Polygon& poly = pAftkMesh->GetPolygon(i);
		S32 indices[3];
		indices[0] = poly.GetPolygonVertex(0).GetPositionIndex();
		indices[1] = poly.GetPolygonVertex(1).GetPositionIndex();
		indices[2] = poly.GetPolygonVertex(2).GetPositionIndex();

		if (poly.GetNumPolygonVertices() != 3 || 
			indices[0] == -1 || 
			indices[1] == -1 || 
			indices[2] == -1)
		{
			//Make degenerate.
			pTriangle[0] = 0;
			pTriangle[1] = 0;
			pTriangle[2] = 0;
			continue;
		}

		//Copy indices.
		memcpy(pTriangle, indices, sizeof(U32)*3);
	}

	status = collisionScene.AddCollisionSource(
		pCollisionMesh,
		originalFilePath.Chars());
	if (!status) return status;

	//Store Aftk->Collision source mapping.
	collisionSourceMap[pAftkMesh] = pCollisionMesh;

	return E_Status_Success;
}

C_Status C_AftkCollisionBuilder::AddSplineSource(
	C_CollisionScene& collisionScene, 
	AftkCollisionSourceMap& collisionSourceMap,
	const Btl::String& originalFilePath,
	const Aftk::C_SceneSplineNode* pAftkSpline)
{
	return E_Status_Success;
}

C_Status C_AftkCollisionBuilder::AddPointSource(
	C_CollisionScene& collisionScene, 
	AftkCollisionSourceMap& collisionSourceMap,
	const Aftk::C_SceneNode* pAftkNode)
{
	return E_Status_Success;
}

C_Status C_AftkCollisionBuilder::BuildCollisionInstances(
	C_CollisionScene& collisionScene, 
	const AftkCollisionSourceMap& collisionSourceMap,
	const AftkDgMap& aftkDgMap)
{
	C_Status status;
	for (AftkDgMap::ConstIterator it(aftkDgMap.Begin()); it != aftkDgMap.End(); ++it)
	{
		const Aftk::C_SceneNode* pPlacementNode = it->GetKey();
		if (AftkRefHelpers::IsToBeHiddenPermanently(pPlacementNode))
			continue;

		I_DgNode* pDgNode = it->GetValue();

		const Aftk::C_SceneNode* pSourceNode = AftkUt::C_Utility::GetInstanceSourceNodePtr(pPlacementNode);

		if (pSourceNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			AftkCollisionSourceMap::Iterator colMapIt = collisionSourceMap.Find(pSourceNode);
			if (colMapIt == collisionSourceMap.End()) continue;

			I_CollisionSource* pSource = colMapIt->GetValue();

			C_InternalCollisionInstance* pInstance = new C_InternalCollisionInstance;
			pInstance->pSource = pSource;
			pInstance->toWorldSpace = pPlacementNode->GetWorldMatrix();
			UtMaths::Invert4x4(
				&pInstance->toObjectSpace, 
				pPlacementNode->GetWorldMatrix());

			collisionScene.AddCollisionInstance(pDgNode, pInstance);
		}
	}

	return E_Status_Success;
}

}
