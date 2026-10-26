#ifndef AFTK_MESH_CALCULATE_NORMALS_H
#define AFTK_MESH_CALCULATE_NORMALS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_MeshCalculateNormals
	{
	public:
		static C_Status CalculateNormals(C_SceneMeshNode* pMesh);

		struct C_CalculateNormalsVisitor
		{
			static bool IsInteresting(Aftk::C_SceneNode* pSceneNode) 
			{
				if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
					return true;

				return false;
			}

			C_Status Visit(
				Aftk::C_SceneNode* pNode,
				C_NullMemoryManager& memoryManager)
			{
				if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
				{
					//Ignore this node.
					return C_Status::E_Status_Success;
				}

				C_MeshCalculateNormals::CalculateNormals(
					(Aftk::C_SceneMeshNode*)pNode);

				return C_Status::E_Status_Success;
			}
		};
	};
}

#endif
