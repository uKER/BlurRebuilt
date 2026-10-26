#ifndef AFTK_MESH_CALCULATE_TANGENTS_H
#define AFTK_MESH_CALCULATE_TANGENTS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_MeshCalculateTangents
	{
	public:
		///\brief Calculates the tangent and binormal vectors for each texture coordinate set.
		///
		///Note that the tangents and binormals are not shared initially.
		///@param normalise Indicates if the tangents space vectors are to be normalised.
		///@return C_Status class, indicating the result of the operation.
		static C_Status CalculateTangents(
			C_SceneMeshNode* pMesh,
			bool normalise,
			bool useVertexNormal);

		///\brief Calculates the tangent and binormal vectors for a texture coordinate set.
		///
		///Note that the tangents and binormals are not shared initially.
		///@param set The UV set to use for the calculation.
		///@param normalise Indicates if the tangents space vectors are to be normalised.
		///@return C_Status class, indicating the result of the operation.
		static C_Status CalculateTangents(
			C_SceneMeshNode* pMesh,
			S32 set, 
			bool normalise,
			bool useVertexNormal);

		struct C_CalculateTangentsVisitor
		{
			C_CalculateTangentsVisitor()
			{
				normalise = false;
				useVertexNormal = false;
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

				C_MeshCalculateTangents::CalculateTangents(
					(Aftk::C_SceneMeshNode*)pNode,
					normalise,
					useVertexNormal);

				return C_Status::E_Status_Success;
			}

			//Params
			bool normalise;
			bool useVertexNormal;
		};
	};
}

#endif
