#ifndef EDITOR_RENDER_PIPELINE_H
#define EDITOR_RENDER_PIPELINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_RenderViewport;
	class C_SimpleRenderer;
	class C_AftkSceneRenderer;
	class C_ComplexRenderer;
	class C_VertexRenderer;
	class C_SpriteRenderer;
	class C_CollisionRenderer;
	class C_DgNodeSelection;

	class C_RenderPipeline
	{
	public:
		struct GlobalRenderState
		{
			F32 gridSize;
			F32 gridSpacing;

			bool locatorVisible	: 1;
			bool spriteVisible	: 1;
			bool meshVisible	: 1;

			bool hudOrientation		: 1;
			bool hudSelectionCount	: 1;

			GlobalRenderState()
			{
				gridSize = 20.f; gridSpacing = 1.f;
				locatorVisible = spriteVisible = meshVisible = true;
				hudOrientation = true;
				hudSelectionCount = false;
			}
		};

		struct ViewportRenderState
		{
			bool labelAftkRef		: 1;
			bool labelDgSceneNodes	: 1;
			bool labelDgPropNodes	: 1;
			bool labelDgSelection	: 1;

			U32 fillmode	: 3;
			U32 cullmode	: 3;
			bool overlay	:1;	
			bool xray		:1;
			bool textured	:1;
			bool lit		:1;
			bool lineAA		:1;
			bool grid		:1;

			ViewportRenderState()
			{
				labelAftkRef = labelDgSceneNodes = labelDgPropNodes = labelDgSelection = false;
				fillmode = Display::E_FillMode_Wireframe;
				cullmode = Display::E_CullMode_Ccw;
				overlay = xray = textured = lineAA = false;
				lit = grid = true;
			}
		};

		C_RenderPipeline();
		~C_RenderPipeline();

		bool CreateRenderers();
		void DestroyRenderers();
		void UpdateActorRenderers(AftkRender::E_RendererPass pass);

		bool Render(const C_RenderViewport* pViewport);

		C_AftkSceneRenderer&		GetAftkSceneRenderer();
		C_SimpleRenderer&			GetSimpleRenderer();
		C_SpriteRenderer&			GetSpriteRenderer();
		C_ComplexRenderer&			GetComplexRenderer();
		C_VertexRenderer&			GetVertexRenderer();
		AftkRender::C_FontRenderer&	GetFontRenderer();

		void LoadTextures();

		GlobalRenderState& GetGlobalRenderState();
		const GlobalRenderState& GetGlobalRenderState() const;

		ViewportRenderState& GetViewportRenderState(const C_RenderViewport *pViewport);
		const ViewportRenderState& GetViewportRenderState(const C_RenderViewport *pViewport) const;
		
		bool Select(const RECT& region, C_DgNodeSelection& selectionCandidates);

	private:

		typedef AftkRender::C_ResourcePtr<Display::C_RefTexture2D> TextureHandle;
		typedef Ut::UtMap<const C_RenderViewport*, ViewportRenderState>	ViewportRenderStateMap;

		C_AftkSceneRenderer*				m_pSceneRenderer;
		C_SimpleRenderer*					m_pSimpleRenderer;
		C_ComplexRenderer*					m_pComplexRenderer;
		C_VertexRenderer*					m_pVertexRenderer;
		C_SpriteRenderer*					m_pSpriteRenderer;
		Display::C_BasicRender*				m_pBasicRenderer;
		AftkRender::C_FontRenderer*			m_pFontRenderer;
		C_CollisionRenderer*				m_pCollisionRenderer;
		Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;


		//stippling texture for occluded objects (sprites, manipulators, ...)
		TextureHandle						m_pStippleTexture;
		GlobalRenderState					m_globalRenderState;
		ViewportRenderStateMap				m_viewportRenderStateMap;
	};
}

#endif