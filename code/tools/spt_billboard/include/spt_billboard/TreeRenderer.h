#ifndef SPEEDTREE_BILLBOARD_TREE_RENDERER_INCLUDE
#define SPEEDTREE_BILLBOARD_TREE_RENDERER_INCLUDE


#ifdef _MSC_VER
#pragma once
#endif

namespace Attk
{
	class C_Image;
}

namespace SpeedtreeBillboard
{
	class C_TreeRenderer : public AftkRender::C_SceneRenderer
	{
	public:
		struct Flags
		{
			unsigned VertexColourLighting	:1;
			unsigned CullLeaves				:1;
		};

		C_TreeRenderer();
		~C_TreeRenderer();

		void Render(Flags flags);
		void AddRenderContext(AftkRender::C_RenderContext& rc, const AftkRender::C_SceneGraphNode* pNode);

	private:
	};



	class C_TreeRendererApp : public AftkRender::C_Application
	{
	public:
		C_TreeRendererApp();
		~C_TreeRendererApp();

		bool Create(const C_TreeRendererSettings *pSettings);
		void Destroy();

		void Run(Aftk::C_Scene *pScene);

	private:

		void Render(E_Conditions condition, bool isEvergreen);
		void Update(const Maths::C_TimeS& timeStep);

		bool SetSceneNodePtr(Aftk::C_SceneNode *pSceneNode);

		bool GetUniqueTrees(Aftk::C_Scene *pScene);
		void ClearUniqueTreeList();

		bool IsSpeedtreeEvergreen(const char* filename);
	private:

		void WriteRenderSurfaces(const Util::C_String& diffuseFilename, const Util::C_String& normalFilename);
		void ClearRenderSurfaces();

		Ut::UtVector< Ut::DynamicVector<Ut::Pair<C_TreeBillboard*, Aftk::C_SceneMeshNode*> > > m_UniqueTrees;
		Ut::BitVector m_UniqueTrees_Evergreen;

		const C_TreeRendererSettings	*m_pSettings;

		Attk::C_Image	*m_pDiffuseAtlas;
		//Attk::C_Image	*m_pNormalAtlas;

		C_TreeRenderer					*m_pTreeRenderer;
		AftkRender::C_Scene				*m_pAftkRenderScene;
		AftkRender::C_ViewContext		*m_pAftkRenderViewContext;
	};
}

#endif

