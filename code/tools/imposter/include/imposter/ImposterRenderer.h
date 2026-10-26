#ifndef IMPOSTER_IMPOSTERRENDERER_H
#define IMPOSTER_IMPOSTERRENDERER_H


#ifdef _MSC_VER
#pragma once
#endif


namespace Imposter
{
	class Renderer
	{
	public:
		//We need a valid hwnd to initialise the d3d device.
		//NOTE: we won't actually be presenting anything onto the window.
		static bool DeviceInit(HWND hwnd = 0);
		static void DeviceDestroy();

		static bool IsInitialised();

		static bool BeginRendering();
		static void EndRendering();

		static AftkRender::C_Scene& GetScene();
		static AftkRender::C_ViewContext* GetViewContext();

	private:
		static AftkRender::C_Scene			*m_pScene;
		static AftkRender::C_Application	*m_pRenderApplication;
	};



	class C_ImposterRenderer : public AftkRender::C_SceneRenderer
	{
	public:
		C_ImposterRenderer();
		~C_ImposterRenderer();

		void Render();
		void AddRenderContext(AftkRender::C_RenderContext& rc, const AftkRender::C_SceneGraphNode *pNode);

	private:

	};
}

#endif