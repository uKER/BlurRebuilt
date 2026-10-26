#ifndef AFTK_RENDER_MESH_BUILDER_INCLUDE
#define AFTK_RENDER_MESH_BUILDER_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	template<class T>
	class C_SharedResource;

	class C_MaterialNode;
	typedef C_SharedResource<C_MaterialNode>	C_MaterialResource;
}

namespace AftkRender
{
	class C_MeshResource;

	class C_MeshBuilder
	{
		static const U32 MESHBUILD_PolygonOutline	= 0x00000001;
		static const U32 MESHBUILD_All				= 0xffffffff;

	public:
		static bool Flatten(
			C_MeshResource* pMesh,
			const Aftk::C_SceneNode *pBaseNode,
			Ut::UtVector< Ut::DynamicVector<const Aftk::C_MaterialResource*> >& materialMapping);

		static bool Merge(
			C_MeshResource* pMesh,
			E_PrimitiveType type,
			U32 mask = E_ElementType_Position|E_ElementType_Normal|E_ElementType_TexCoord0);

		static bool Build(
			C_MeshResource* pMesh, 
			const Aftk::C_SceneMeshNode *pAftkMeshNode,
			U32 flags = MESHBUILD_All);

		static const U32 MAX_MATRIX_PALETTE_SIZE = 30;
		static const U32 MAX_VERTEX_INFLUENCERS  = 4;

	private:
		struct SkinPartition
		{
			Ut::UtVector< Ut::DynamicVector<const Aftk::C_SceneNode*> > influenceNodes;
			Ut::UtVector< Ut::DynamicVector<U32> > polygons;
			
			bool AddPolygon(U32 polyIndex, Ut::UtVector< Ut::DynamicVector<const Aftk::C_SceneNode*> >&influencers);
		};

		struct VertexWeight
		{
			F32							weights[MAX_VERTEX_INFLUENCERS];
			const Aftk::C_SceneNode*	nodes[MAX_VERTEX_INFLUENCERS];

			VertexWeight();
			void Add(F32 weight, const Aftk::C_SceneNode* pNode);
		};

		static bool BuildSkinPartitions(
			const Aftk::C_SceneMeshNode *pAftkMeshNode,
			Ut::UtVector< Ut::DynamicVector<SkinPartition> >& partitions,
			Ut::UtVector< Ut::DynamicVector<VertexWeight> >& weights,
			U32 lod);
	};

#include "MeshBuilder.inl"

}

#endif