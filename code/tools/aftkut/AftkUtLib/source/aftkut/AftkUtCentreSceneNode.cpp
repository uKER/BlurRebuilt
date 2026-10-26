#include "stdafx.h"
#include "aftkut/aftkUt.h"

using namespace AftkUt;

Aftk::C_Status AftkUt::C_CentreSceneNode::CentreSceneNode(
	Aftk::C_SceneNode* pAftkSceneNode )
{
	// Use bounding box centre as new pivot point
	Aftk::Position3 centreOfNode = pAftkSceneNode->GetEvaluatedWorldBounds().GetCentre();
	Aftk::Position3 centreOfNodeLocal = pAftkSceneNode->GetEvaluatedLocalBounds().GetCentre();

	Aftk::Matrix44 nodeWorldMatrix = pAftkSceneNode->GetWorldMatrix( false );
	Aftk::Matrix44 nodeLocalMatrix = pAftkSceneNode->GetLocalMatrix( false );
	Aftk::Vector4F32 newPivotWorldPosition(
		centreOfNode.x,
		centreOfNode.y,
		centreOfNode.z,
		1);

	Aftk::Vector4F32 nodePivotLocalPosition(
		nodeLocalMatrix[3][0],
		nodeLocalMatrix[3][1],
		nodeLocalMatrix[3][2],
		1);

	// Get bounding box centre in the parent nodes coordinate space
	Aftk::Vector4F32 newPivotParentSpace = newPivotWorldPosition * pAftkSceneNode->GetParentPtr()->GetWorldMatrixInverse( false );

	Aftk::Vector4F32 pivotLocalTranslation(
		newPivotParentSpace[0] - nodePivotLocalPosition[0],
		newPivotParentSpace[1] - nodePivotLocalPosition[1],
		newPivotParentSpace[2] - nodePivotLocalPosition[2],
		1);

	// Create translation matrix & use it to generate new local matrix
	Aftk::Matrix44 localTranslationMatrix;
	UtMaths::SetIdentity( &localTranslationMatrix );
	localTranslationMatrix[3][0] = pivotLocalTranslation[0];
	localTranslationMatrix[3][1] = pivotLocalTranslation[1];
	localTranslationMatrix[3][2] = pivotLocalTranslation[2];

	nodeLocalMatrix = nodeLocalMatrix * localTranslationMatrix;

	// Move vertices to account for change in pivot position
	if( pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh )
	{
		Aftk::C_SceneMeshNode *pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*) pAftkSceneNode;
		U32 nVertices = pAftkSceneMeshNode->GetNumVertexPositions();
		Aftk::Position3Array& aftkVertices = pAftkSceneMeshNode->GetVertexPositionArray();

		for( U32 i=0; i<nVertices; ++i )
		{
			aftkVertices[i].x = aftkVertices[i].x - centreOfNodeLocal.x;
			aftkVertices[i].y = aftkVertices[i].y - centreOfNodeLocal.y;
			aftkVertices[i].z = aftkVertices[i].z - centreOfNodeLocal.z;
			int k=0;
		}
	}

	pAftkSceneNode->SetLocalMatrix( nodeLocalMatrix, false );

	// Adjust vertices in child nodes accordingly
	{
		U32 nChildren = pAftkSceneNode->GetNumChildren();
		for( U32 i=0; i<nChildren; ++i )
		{
			Aftk::C_Status status = CentreSceneNodeChildren( pAftkSceneNode->GetChildPtr(i), centreOfNodeLocal );
			if (!status) { return status; }
		}
	}
	return Aftk::C_Status::E_Status_Success;

}

Aftk::C_Status AftkUt::C_CentreSceneNode::CentreSceneNodeChildren(
	Aftk::C_SceneNode* pAftkSceneNode,
	const Aftk::Position3& pivotOffset)
{

	Aftk::Vector4F32 pivot( pivotOffset.x, pivotOffset.y, pivotOffset.z, 1 );
	pivot = pivot * pAftkSceneNode->GetLocalMatrix( false );

	if( pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh )
	{
		Aftk::C_SceneMeshNode *pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*) pAftkSceneNode;
		U32 nVertices = pAftkSceneMeshNode->GetNumVertexPositions();
		Aftk::Position3Array& aftkVertices = pAftkSceneMeshNode->GetVertexPositionArray();

		for( U32 i=0; i<nVertices; ++i )
		{
			aftkVertices[i].x = aftkVertices[i].x - pivot.x;
			aftkVertices[i].y = aftkVertices[i].y - pivot.y;
			aftkVertices[i].z = aftkVertices[i].z - pivot.z;
		}
	}

	{
		U32 nChildren = pAftkSceneNode->GetNumChildren();
		for( U32 i=0; i<nChildren; ++i )
		{
			Aftk::C_Status status = CentreSceneNodeChildren( pAftkSceneNode->GetChildPtr(i), pivotOffset );
			if (!status) { return status; }
		}
	}
	return Aftk::C_Status::E_Status_Success;

}