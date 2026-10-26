#ifndef AFTK_MESH_REMOVE_DEGENERATES_H
#define AFTK_MESH_REMOVE_DEGENERATES_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_MeshRemoveDegenerates
	{
	public:
		static U32 RemoveDegenerates(Aftk::C_SceneMeshNode* pMesh);

		static U32 RemoveDegenerates(Aftk::C_SceneMeshNode* pMesh, U32 lodLevel);

		struct C_RemoveDegeneratesVisitor
		{
			C_RemoveDegeneratesVisitor()
			{
			}

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

				C_MeshRemoveDegenerates::RemoveDegenerates(
					(Aftk::C_SceneMeshNode*)pNode);

				return C_Status::E_Status_Success;
			}
		};
	};
}

#endif
