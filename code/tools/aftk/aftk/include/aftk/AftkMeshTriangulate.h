#ifndef AFTK_MESH_TRIANGULATE_H
#define AFTK_MESH_TRIANGULATE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_MeshTriangulate
	{
	public:
		///\brief Triangulate this mesh.
		///
		///@return C_Status class, indicating the result of the operation.
		static C_Status Triangulate(C_SceneMeshNode* pMesh);

		///\brief Triangulate this mesh.
		///
		///@param lodLevel The polygon set to query.
		///@return C_Status class, indicating the result of the operation.
		static C_Status Triangulate(C_SceneMeshNode* pMesh, U32 lodLevel);

		struct C_TriangulateVisitor
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

				C_MeshTriangulate::Triangulate(
					(Aftk::C_SceneMeshNode*)pNode);

				return C_Status::E_Status_Success;
			}
		};
	};
}
#endif
