#include "stdafx.h"
#include "PreProcScene.h"

bool C_PreProcScene::PreProcScene(Aftk::C_Scene& aftkScene)
{
	if (!aftkScene.GetRootSceneNodePtr())
		return false;

	Aftk::Matrix44 id;
	UtMaths::SetIdentity(&id);

	//aftk triangulate 
	if (!Aftk::C_SceneNodeVisit::Traverse(
		&aftkScene, 
		Aftk::C_MeshTriangulate::C_TriangulateVisitor(),
		0))
		return false;

	{
		const bool normalise = true;					               
		for (Aftk::C_SceneNodeIterator it(&aftkScene); it; ++it)
		{
			Aftk::C_SceneNode* pCurrAftkNode = it;
			if(pCurrAftkNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				Aftk::C_SceneMeshNode* pMesh = (Aftk::C_SceneMeshNode*)pCurrAftkNode;
				U32 tSpaceCount = 0;
				for (U32 iSet = 0; iSet < pMesh->GetNumTextureCoordinateSets(); iSet++)
				{
					tSpaceCount += pMesh->GetNumVertexTangents(iSet);
					tSpaceCount += pMesh->GetNumVertexBinormals(iSet);
				}

				//Calculate tangent space if not present.
				const bool normalize = true;					               
				if (tSpaceCount == 0)
				{
					Aftk::C_MeshCalculateTangents::C_CalculateTangentsVisitor calcTBVisitor;
					calcTBVisitor.normalise = normalize;
					calcTBVisitor.useVertexNormal = false;

					Aftk::C_Status status = Aftk::C_SceneNodeVisit::Traverse(
						&aftkScene, 
						calcTBVisitor,
						0);	
				}

				//Move our object to the origin.
				Aftk::Matrix44 worldMatrix = pMesh->GetWorldMatrix(false);
				
				//Set position to origin.
				worldMatrix[3][0] = 0;
				worldMatrix[3][1] = 0;
				worldMatrix[3][2] = 0;
				
				pMesh->SetWorldMatrix(id);
				pMesh->ApplyMatrix(worldMatrix);
			}
		}
	}
	aftkScene.GetRootSceneNodePtr()->SetLocalMatrix(id);

	return true;
}