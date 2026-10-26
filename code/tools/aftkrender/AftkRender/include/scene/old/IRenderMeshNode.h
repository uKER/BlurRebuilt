#ifndef AFTK_RENDER_MESH_NODE_INTERFACE_INCLUDE
#define AFTK_RENDER_MESH_NODE_INTERFACE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	class I_Renderer;
	namespace D3D9
	{
		class C_VertexDeclaration;
	}
}
namespace Aftk
{
	class C_SceneNode;
	class C_Status;
}
namespace AftkRender
{
	class C_RenderNode;
	class C_RenderMeshCluster;

	class I_RenderMeshNode : public C_RenderNode
	{
	public:
		enum E_EffectMode
		{
			E_EffectMode_None = -1,
			E_EffectMode_Static = 0,
			E_EffectMode_Skinned,
			E_EffectMode_BonedInstance,

			E_EffectMode_Count,
		};

		I_RenderMeshNode(
			Aftk::C_SceneNode *pAftkSceneNode,
			C_SceneRenderer *pRenderer);
		
		virtual ~I_RenderMeshNode();

		virtual void Clear();

		virtual Aftk::C_Status Rebuild() = 0;

		virtual Aftk::C_Status Render(
			U32 lodLevel=0) = 0;

		virtual Aftk::C_Status Render(
			U32 lodLevel,
			U32 clusterIndex) = 0;

		virtual const F32* GetLightMapUVOffsets() const = 0;

		virtual U32 GetNumClusters(
			U32 lod) const = 0;

		virtual const C_RenderMeshCluster* GetMeshClusterPtr(
			U32 lodLevel,
			U32 clusterIndex) const = 0;

		virtual C_RenderMeshCluster* GetMeshClusterPtr(
			U32 lodLevel,
			U32 clusterIndex) = 0;

		virtual Display::C_VertexDeclaration* GetVertexDeclaration() const = 0;

		virtual E_EffectMode GetEffectMode() const = 0;

		virtual U8 GetNumUVSets() const = 0;

		virtual E_RenderNodeType GetType() const;

		virtual bool HasType(E_RenderNodeType type) const;
	};
}

#endif