#ifndef AFTK_RENDER_MESH_SCENEGRAPH_NODE_H
#define AFTK_RENDER_MESH_SCENEGRAPH_NODE_H


#ifdef _MSC_VER
#pragma once
#endif

#include "common/ResourcePtr.h"

namespace AftkRender
{
	class C_MeshSceneGraphNode : public C_SceneGraphNode
	{
	public:
		C_MeshSceneGraphNode(C_Scene *pRenderScene, const char* pName, S32 id=-1);
		~C_MeshSceneGraphNode();

		void SetMeshResource(C_ResourcePtr<C_MeshResource> pMesh);
		C_ResourcePtr<C_MeshResource> GetMeshResource();
		const C_ResourcePtr<C_MeshResource> GetMeshResource() const;

		void SetGeometryNode(C_ResourcePtr<C_GeometryNode> pGeometry);
		C_ResourcePtr<C_GeometryNode> GetGeometryNode();
		const C_ResourcePtr<C_GeometryNode> GetGeometryNode() const;

		virtual void UpdateCache() const;

		virtual E_SceneGraphNodeType GetType() const
		{
			return E_SceneGraphNode_NativeMesh;
		}

		virtual bool IsA(E_SceneGraphNodeType type) const
		{
			return (type==E_SceneGraphNode_NativeMesh || C_SceneGraphNode::IsA(type));
		}

		void SetMaterialMapping(U32 index, S16 mapping);
		const Ut::UtVector< Ut::DynamicVector<S16> >& GetMaterialMapping() const;

		void AttachToRenderer(U8 lod=0, E_RendererPass pass=E_RendererPass_Normal, bool recurse=true);

	protected:
		C_ResourcePtr<C_MeshResource>	m_pMeshResource;
		C_ResourcePtr<C_GeometryNode>	m_pGeometryNode;

		Ut::UtVector< Ut::DynamicVector<S16> > m_MaterialIndexMap;
	};



	class C_MeshSkinnedSceneGraphNode : public C_MeshSceneGraphNode
	{
	public:
		C_MeshSkinnedSceneGraphNode(C_Scene *pRenderScene, const char* pName, S32 id=-1);
		~C_MeshSkinnedSceneGraphNode();

		virtual void UpdateCache() const;

		virtual E_SceneGraphNodeType GetType() const
		{
			return E_SceneGraphNode_NativeSkinnedMesh;
		}

		virtual bool IsA(E_SceneGraphNodeType type) const
		{
			return (type==E_SceneGraphNode_NativeSkinnedMesh || C_MeshSceneGraphNode::IsA(type));
		}

		void SetMatrixPaletteEntry(U32 index, const Maths::HMat44& matrix);

		void AttachToRenderer(U8 lod=0,  E_RendererPass pass=E_RendererPass_Normal, bool recurse=true);
	private:
		Maths::HMat44	*m_pMatrixPalette;

		static const U32 m_MaxMatrixPaletteSize;
	};
}

#endif

