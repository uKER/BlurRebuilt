#ifndef AFTK_RENDER_SCENE_NODE_H
#define AFTK_RENDER_SCENE_NODE_H


#ifdef _MSC_VER
#pragma once
#endif

#include "common/ResourcePtr.h"

namespace AftkRender
{
	class C_AftkSceneGraphNode : public C_SceneGraphNode
	{
	public:
		C_AftkSceneGraphNode(C_Scene *pRenderScene, Aftk::C_SceneNode* pNode);
		~C_AftkSceneGraphNode();

		Aftk::C_SceneNode* GetAftkSceneNodePtr();
		const Aftk::C_SceneNode* GetAftkSceneNodePtr() const;

		virtual E_SceneGraphNodeType GetType() const
		{
			return E_SceneGraphNode_Aftk;
		}

		virtual bool IsA(E_SceneGraphNodeType type) const
		{
			return (type==E_SceneGraphNode_Aftk || C_SceneGraphNode::IsA(type));
		}

		void AttachToRenderer(U8 lod=0, E_RendererPass pass=E_RendererPass_Normal, bool recurse=true);

	protected:
		Aftk::C_SceneNode				*m_pAftkSceneNode;
	};


	class C_AftkMeshSceneGraphNode : public C_AftkSceneGraphNode
	{
	public:
		C_AftkMeshSceneGraphNode(C_Scene *pRenderScene, Aftk::C_SceneNode* pNode);
		~C_AftkMeshSceneGraphNode();

		void SetGeometryNode(C_ResourcePtr<C_GeometryNode> pGeometry);
		C_ResourcePtr<C_GeometryNode> GetGeometryNode();
		const C_ResourcePtr<C_GeometryNode> GetGeometryNode() const;

		virtual E_SceneGraphNodeType GetType() const
		{
			return E_SceneGraphNode_AftkMesh;
		}

		virtual bool IsA(E_SceneGraphNodeType type) const
		{
			return (type==E_SceneGraphNode_AftkMesh || C_AftkSceneGraphNode::IsA(type));
		}

		void AttachToRenderer(U8 lod=0, E_RendererPass pass=E_RendererPass_Normal, bool recurse=true);

	protected:
		C_ResourcePtr<C_GeometryNode>	m_pGeometryNode;
	};


	class C_AftkMeshSkinnedSceneGraphNode : public C_AftkMeshSceneGraphNode
	{
	public:
		C_AftkMeshSkinnedSceneGraphNode(C_Scene *pRenderScene, Aftk::C_SceneNode* pNode);
		~C_AftkMeshSkinnedSceneGraphNode();

		virtual E_SceneGraphNodeType GetType() const
		{
			return E_SceneGraphNode_AftkSkinnedMesh;
		}

		virtual bool IsA(E_SceneGraphNodeType type) const
		{
			return (type==E_SceneGraphNode_AftkSkinnedMesh || C_AftkSceneGraphNode::IsA(type));
		}

		void AttachToRenderer(U8 lod=0, E_RendererPass pass=E_RendererPass_Normal, bool recurse=true);
	private:
		Maths::HMat44	*m_pMatrixPalette;

		static const U32 m_MaxMatrixPaletteSize;
	};
}

#endif

