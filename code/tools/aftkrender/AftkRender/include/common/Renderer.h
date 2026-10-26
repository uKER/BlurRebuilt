#ifndef AFTK_RENDER_RENDERER_H
#define AFTK_RENDER_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_SceneGraphNode;
	class C_TextureManager;
	class C_ShaderManager;

	enum E_RendererPass
	{
		E_RendererPass_Normal,
		E_RendererPass_Alpha,
		E_RendererPass_VisibilityTest,
		E_RendererPass_2D // Enables font rendering and disables depth buffer.
	};

	class C_Renderer : public Display::I_Renderer
	{
	public:

		C_Renderer(const char* pName = "Aftk Scene Renderer");
		~C_Renderer();

		void Render();

		void BeginVisibilityTest(const RECT &region);
		bool EndVisibilityTest();

		static C_TextureManager& GetTextureManager();
		static C_ShaderManager& GetShaderManager();

	protected:

		//Display::I_Renderer overrides
		virtual bool OnCreate();
		virtual bool OnCreateVolatile();
		virtual void OnDestroyVolatile();
		virtual void OnDestroy();

		virtual bool OnInitialise();
		virtual void OnShutdown();
		virtual void OnReset();

		static C_ShaderManager		m_ShaderManager;
		static C_TextureManager		m_TextureManager;

		IDirect3DQuery9	*m_pOcclusionQuery;
		bool			m_InVisibilityTest;
	};

	inline C_TextureManager& C_Renderer::GetTextureManager()
	{
		return m_TextureManager;
	}

	inline C_ShaderManager& C_Renderer::GetShaderManager()
	{
		return m_ShaderManager;
	}
}

#endif