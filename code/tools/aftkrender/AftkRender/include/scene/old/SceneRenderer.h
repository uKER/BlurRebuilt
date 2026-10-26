#ifndef AFTK_RENDER_SCENE_INCLUDE
#define AFTK_RENDER_SCENE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
};

namespace AftkRender
{
	class C_RenderMaterial;
	class I_RenderShaderEffect;
	class C_RenderQueue;

	enum E_SceneRendererEffectType
	{
		E_SceneRendererEffectType_DiffuseAlpha=0,
		E_SceneRendererEffectType_SPTBillboard_Leaf,

		E_SceneRendererEffectType_DiffuseAlphaVertexLit,
		E_SceneRendererEffectType_SPTBillboard_LeafVertexLit,

		E_SceneRendererEffectType_Count
	};

	class I_SceneRendererTaskFilter
	{
	public:
		virtual bool IsOKToRender(const C_RenderTask& renderTask) = 0;
	};

	class I_SceneRendererShaderDelegate
	{
	public:
		virtual void Execute(const C_RenderTask& renderTask) = 0;
	};

	class C_SceneRenderer : public Display::I_Renderer
	{
	public:

		struct C_Flags
		{
			unsigned texturesLoaded:1;
			unsigned frustumCulling:1;
			unsigned animated:1;
			unsigned lightUsingVertexColours:1;
			unsigned overrideshaders:1;
			unsigned filterRenderTasks:1;
		};

		struct C_Constants
		{
			F32 vertexColourScale;
		};

		C_SceneRenderer();
		C_SceneRenderer(Aftk::C_Scene *pAftkScene);
		~C_SceneRenderer();

		bool OnCreate();
		void OnDestroy();

		bool OnCreateVolatile();
		void OnDestroyVolatile();

		Aftk::C_Status BuildFromScene(Aftk::C_Scene *pScene);
		Aftk::C_Status BuildFromSceneNode(Aftk::C_SceneNode *pSceneNode);

		void Update(
			const Maths::C_TimeS& timeStep);

		// Render entire scene using default shaders.
		void Render();

		const Aftk::C_Scene* GetAftkScenePtr();
		const Aftk::C_Scene* GetAftkScenePtr() const;

		void Clear();

		void UpdateNodes();

		const C_RenderMaterial* GetRenderMaterial(U32 aftkMaterialIndex);
		const C_RenderMaterial* GetRenderMaterial(U32 aftkMaterialIndex) const;

		void GenerateRenderQueue();
		C_RenderQueue& GetRenderQueue();
		const C_RenderQueue& GetRenderQueue() const;
		C_RenderQueue& GetRenderQueueAlpha();
		const C_RenderQueue& GetRenderQueueAlpha() const;

		// To replace textures which are not present
		const Display::C_Texture2D& GetDummyTexture() const;

		void RenderAllTasks();
		void SetRenderTaskFilter(I_SceneRendererTaskFilter* pTaskFilter);

		void UseDefaultShaders();
		void OverrideDefaultShaders(I_SceneRendererShaderDelegate* pShaderDelegate);

		const C_Flags& GetRendererFlags() const;
		C_Flags& GetRendererFlags();

		const C_Constants& GetRendererConstants() const;
		C_Constants& GetRendererConstants();

	private:
		C_SceneRenderer(
			const C_SceneRenderer& node);

		const C_SceneRenderer& operator=(
			const C_SceneRenderer& rhs);

		void ClearSceneRenderNodes();

		Aftk::C_Status BuildMaterialList();
		void ClearMaterialList();

		Aftk::C_Status LoadTextures();
		Aftk::C_Status CreateDefaultShaders();
		void DestroyDefaultShaders();

		Aftk::C_Scene* m_pAftkScene;

		Ut::UtVector< Ut::DynamicVector<C_RenderNode*> >		m_RenderNodes;
		Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*>		m_AftkToRenderNodeMap;

		I_RenderShaderEffect*									m_RenderShaderEffects[E_SceneRendererEffectType_Count];
		Ut::UtVector< Ut::DynamicVector<C_RenderMaterial*> >	m_RenderMaterials;
		Ut::UtVector< Ut::DynamicVector<C_RenderMaterial*> >	m_AftkMaterialMapping;

		C_RenderQueue			m_RenderQueue;
		C_RenderQueue			m_RenderQueue_Alpha;
		C_Flags					m_RenderSceneFlags;
		C_Constants				m_RenderSceneConstants;

		Display::C_Texture2D	m_DummyTexture;

		I_SceneRendererTaskFilter*		m_pRenderTaskFilter;
		I_SceneRendererShaderDelegate*	m_pShaderDelegate;
	};
}
#endif