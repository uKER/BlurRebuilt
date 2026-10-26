#ifndef AFTK_MESH_EXPAND_POLYGON_VERTICES_H
#define AFTK_MESH_EXPAND_POLYGON_VERTICES_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_MeshExpandPolygonVertices
	{
	public:
		///\brief Removes all sharing of polygon vertices.
		///
		///This function makes each polygon vertex in the mesh unique.
		///@return The number of polygon vertices expanded.
		static U32 ExpandPolygonVertices(C_SceneMeshNode* pMesh);

		///\brief Removes all sharing of polygon vertices.
		///
		///This function makes each polygon vertex in the mesh unique.
		///@param lodLevel The polygon set to modify.
		///@return The number of polygon vertices expanded.
		static U32 ExpandPolygonVertices(C_SceneMeshNode* pMesh, U32 lodLevel);

		struct C_ExpandPolygonVerticesVisitor
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

				C_MeshExpandPolygonVertices::ExpandPolygonVertices(
					(Aftk::C_SceneMeshNode*)pNode);

				return C_Status::E_Status_Success;
			}
		};
	};
}
#endif

