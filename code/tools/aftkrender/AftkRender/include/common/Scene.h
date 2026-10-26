#ifndef AFTK_RENDER_SCENE_H
#define AFTK_RENDER_SCENE_H

#ifdef _MSC_VEr
#pragma once
#endif

namespace AftkRender
{
	class C_SceneRenderer;
	class C_SceneGraphNode;
	class C_TextureManager;
	class C_Material;
	class C_GeometryNode;
	class C_Ray;

	class C_Scene
	{
	public:
		C_Scene();
		virtual ~C_Scene();

		bool Load(Aftk::C_Scene *pScene, bool useInternalMeshData = true);
		bool Load(Aftk::C_SceneNode *pSceneNode, bool useInternalMeshData = true);

		void Clear();

		bool BuildMeshClusters();
		bool LoadTextures(S32 clampSize = 256, C_TextureManager *pManager = 0);

		C_SceneGraphNode* GetRootSceneGraphNodePtr();
		const C_SceneGraphNode* GetRootSceneGraphNodePtr() const;

		C_SceneGraphNode* GetSceneGraphNodeById(S32 id);
		const C_SceneGraphNode* GetSceneGraphNodeById(S32 id) const;

		S16 GetMaterialIndexMapping(S16 index) const;
		C_Material*	GetMaterial(S16 remappedIndex);
		const C_Material* GetMaterial(S16 remappedIndex) const;

		virtual S32 OnCreateSceneGraphNode(C_SceneGraphNode *pSgNode, S32 preferredId);
		virtual void OnDestroySceneGraphNode(C_SceneGraphNode *pSgNode);

		void DeleteSceneGraphFromNode(C_SceneGraphNode *pSgNode);
		void DetachSceneGraphFromNode(C_SceneGraphNode *pSgNode);

		void InvalidateCache() const;
		const Maths::C_AxisBox& GetAABBox() const;

		void RayIntersectionTest(const C_Ray& ray, Ut::UtSet<C_SceneGraphNode*>& result) const;
		void VisibilityTest(const RECT& region, C_SceneRenderer *pRenderer, Ut::UtSet<C_SceneGraphNode*>& result) const;
		void Hide(C_SceneGraphNode* pNode, U8 hideState) const;

	protected:
		Ut::UtVector< Ut::DynamicVector<C_GeometryNode*> >		m_GeometryNodes;
		Ut::UtMap<S32, C_SceneGraphNode*>						m_SceneGraphNodeIdMap;

		Ut::UtVector< Ut::DynamicVector<C_Material> >			m_Materials;
		Ut::UtVector< Ut::DynamicVector<S32> >					m_MaterialMap;

		mutable struct Flags
		{
			U32	sceneBoundsDirty : 1;
		}														m_Flags;

		mutable	Maths::C_AxisBox								m_SceneBounds;

		C_SceneGraphNode	*m_pRootSceneGraphNode;
	};
}

#endif