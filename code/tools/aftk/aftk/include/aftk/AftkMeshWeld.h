#ifndef AFTK_MESH_WELD_H
#define AFTK_MESH_WELD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_MeshWeld
	{
	public:
		///
		///@param threshold The welding distance.
		///@return The number of texture coordinates welded.
		static U32 WeldTextureCoordinates(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			F32 threshold = AFTK_EPSILON);

		static U32 WeldTextureCoordinates(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			U32 set,
			F32 threshold = AFTK_EPSILON);
	    
		///\brief Shares vertex normals within the given threshold.
		///
		///@param threshold The welding distance.
		///@return The number of normals welded.
		static U32 WeldVertexNormals(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			F32 threshold = AFTK_EPSILON);

		///\brief Shares vertex positions within the given threshold.
		///
		///@param threshold The welding distance.
		///@return The number of positions welded.
		static U32 WeldVertexPositions(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			F32 threshold = AFTK_EPSILON);

		///\brief Shares vertex tangents and binormals within the given threshold.
		///
		///@param set The texture coordinate set.
		///@param threshold The welding distance.
		///@return The number of tangents and binormals welded.
		static U32 WeldVertexTangentsAndBinormals(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			U32 set, 
			F32 threshold = AFTK_EPSILON);

		///\brief Shares vertex tangents and binormals within the given threshold for all uv sets.
		///
		///@param threshold The welding distance.
		///@return The number of tangents and binormals welded.
		static U32 WeldVertexTangentsAndBinormals(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			F32 threshold = AFTK_EPSILON);

		///\brief Shares vertex tangents within the given threshold.
		///
		///@param set The texture coordinate set.
		///@param threshold The welding distance.
		///@return The number of tangents welded.
		static U32 WeldVertexTangents(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			U32 set,
			F32 threshold = AFTK_EPSILON);

		///\brief Shares vertex binormals within the given threshold.
		///
		///@param set The texture coordinate set.
		///@param threshold The welding distance.
		///@return The number of binormals welded.
		static U32 WeldVertexBinormals(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			U32 set,
			F32 threshold = AFTK_EPSILON);

		///\brief Welds all mesh components and polygon vertices.
		///
		///@param threshold The welding distance.
		///@return The number of mesh components and polygon vertices welded.
		static U32 Weld(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			F32 positionThreshold = AFTK_EPSILON,
			F32 normalThreshold = AFTK_EPSILON,
			F32 uvThreshold = AFTK_EPSILON,
			F32 tangentThreshold = AFTK_EPSILON);

		///\brief Weld the polygon vertices of all meshes within the given threshold.
		///
		///This function will share polygon vertices if all of their components are within the given
		///welding distance. Therefore one should first weld the other mesh components before invoking this
		///function.
		///@param threshold The tolerence within which to weld the polygon vertices.
		///@return The number of polygon vertices welded.
		static U32 WeldPolygonVertices(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			F32 threshold = AFTK_EPSILON);

		static U32 WeldPolygons(
			Aftk::C_SceneMeshNode* pMesh);

		static U32 WeldPolygons(
			Aftk::C_SceneMeshNode* pMesh,
			U32 lodLevel);

		///\brief Weld the components of all meshes within the given threshold.
		///
		///This shares positions, normals, tangents, binormals and texture coordinates that are within
		///the given welding distance. C_Polygon vertices can then be welded after calling this function
		///via a call to 'WeldMeshPolygonVertices'.
		///
		///@param threshold The tolerence within which to weld the mesh components.
		///@return The number of mesh components welded.
		static U32 WeldComponents(
			C_MeshWeldMemoryManager& weldMemoryManager,
			Aftk::C_SceneMeshNode* pMesh,
			F32 threshold = AFTK_EPSILON);

		static void RemapVertexWeights(
			Aftk::C_SceneMeshNode* pMesh,
			const Ut::UtVector< Ut::DynamicVector<S32> >& remapping);

		struct C_WeldComponentsVisitor
		{
			C_WeldComponentsVisitor()
			{
				threshold = AFTK_EPSILON;
			}

			static bool IsInteresting(Aftk::C_SceneNode* pSceneNode) 
			{
				if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
					return true;

				return false;
			}

			C_Status Visit(
				Aftk::C_SceneNode* pNode,
				C_MeshWeldMemoryManager& memoryManager)
			{
				if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
				{
					//Ignore this node.
					return C_Status::E_Status_Success;
				}

				C_MeshWeld::WeldComponents(
					memoryManager,
					(Aftk::C_SceneMeshNode*)pNode,
					threshold);

				return C_Status::E_Status_Success;
			}

			//Params.
			F32 threshold;
		};

		struct C_WeldPolygonVerticesVisitor
		{
			C_WeldPolygonVerticesVisitor()
			{
				threshold = AFTK_EPSILON;
			}

			static bool IsInteresting(Aftk::C_SceneNode* pSceneNode) 
			{
				if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
					return true;

				return false;
			}

			C_Status Visit(
				Aftk::C_SceneNode* pNode,
				C_MeshWeldMemoryManager& memoryManager)
			{
				if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
				{
					//Ignore this node.
					return C_Status::E_Status_Success;
				}

				C_MeshWeld::WeldPolygonVertices(
					memoryManager,
					(Aftk::C_SceneMeshNode*)pNode,
					threshold);

				return C_Status::E_Status_Success;
			}

			//Params.
			F32 threshold;
		};

		struct C_WeldVisitor
		{
			C_WeldVisitor()
			{
				positionThreshold	= AFTK_EPSILON;
				normalThreshold		= AFTK_EPSILON;
				uvThreshold			= AFTK_EPSILON;
				tangentThreshold	= AFTK_EPSILON;
			}

			static bool IsInteresting(Aftk::C_SceneNode* pSceneNode) 
			{
				if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
					return true;

				return false;
			}

			C_Status Visit(
				Aftk::C_SceneNode* pNode,
				C_MeshWeldMemoryManager& memoryManager)
			{
				if (pNode->GetType() != Aftk::E_ObjType_SceneMesh)
				{
					//Ignore this node.
					return C_Status::E_Status_Success;
				}

				C_MeshWeld::Weld(
					memoryManager,
					(Aftk::C_SceneMeshNode*)pNode,
					positionThreshold,
					normalThreshold,
					uvThreshold,
					tangentThreshold);
	
				return C_Status::E_Status_Success;
			}

			//Params.
			F32 positionThreshold;
			F32 normalThreshold;
			F32 uvThreshold;
			F32 tangentThreshold;
		};

	private:
		struct C_VertexWeightWeldRecord
		{
			S32 nWeights;
			F32 weight;
		};

		template<class T>
		static E_Axis InternalGetWidestAxisPosition(
			const Ut::UtVector< Ut::DynamicVector<T> >& dataArray);

		template<class T>
		static S32 InternalLinearFindFirstWeldIndex(
			const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
			const Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >& sortedIndexList,
			U32 sortedCompareIndex,
			F32 threshold,
			U32 axis,
			U32 sortedStartIndex);

		template <class T>
		static S32 InternalBinaryFindFirstWeldIndex(
			const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
			const Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >& sortedIndexList,
			U32 sortedCompareIndex,
			F32 threshold,
			U32 axis,
			U32 sortedLowIndex,
			U32 sortedHighIndex);

		
		class I_ComponentFunctor
		{
		public:
			virtual bool operator()(U32 i1, U32 i2) = 0;
		};

		template<class T>
		static U32 InternalWeldComponents(
			C_MeshWeldMemoryManager& weldMemoryManager,
			const Ut::UtVector< Ut::DynamicVector<T> >& searchArray,
			const Matrix44* pTransform,
			F32 threshold,
			I_ComponentFunctor* additionalCheckPtr = 0);

		static S32 InternalLinearFindFirstPolygonVertexWeldIndex(
			const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
			const Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >& sortedIndexList,
			U32 sortedCompareIndex,
			F32 threshold,
			U32 sortedStartIndex);

		static S32 InternalBinaryFindFirstPolygonVertexWeldIndex(
			const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
			const Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >& sortedIndexList,
			U32 sortedCompareIndex,
			F32 threshold,
			U32 sortedLowIndex,
			U32 sortedHighIndex);

		static U32 InternalWeldPolygonVertices(
			C_MeshWeldMemoryManager& weldMemoryManager,
			const Ut::UtVector< Ut::DynamicVector<Position3> >& positionArray,
			const Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >& searchArray,
			F32 threshold);
	};

	#include "AftkMeshWeld.inl"
}

#endif
