#ifndef EDITOR_RENDERER_CORE_H
#define EDITOR_RENDERER_CORE_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Editor
{
	class C_RenderViewport;
	class C_CameraConnection;
	class I_CameraController;
	class C_RenderScene;
	class C_RenderPipeline;
	class C_ManipulatorManager;
	class C_GameMessaging;

	typedef Ut::UtVector< Ut::DynamicVector<C_RenderViewport*> >	RenderViewportArray;
	typedef Ut::UtVector< Ut::DynamicVector<I_CameraController*> >	CameraControllerArray;

	class C_RenderCore : public Ut::Singleton<C_RenderCore, Ut::CreateUsingNew>
	{
	public:

		C_RenderCore();
		~C_RenderCore();

		bool								Initialise(HWND hwnd);	//initialises d3d device. must be called before viewports can be registered
		bool								Shutdown();				//destroys the d3d device. implicity destroys view context of any registered viewports

		AftkRender::C_ViewContext*			RegisterViewport(C_RenderViewport *pViewport);
		void								UnregisterViewport(C_RenderViewport *pViewport);

		bool								ResizeViewport(U32 w, U32 h, C_RenderViewport* pViewport);
		bool								BeginRendering(const C_RenderViewport* pViewport);
		void								EndRendering(const C_RenderViewport* pViewport);
		const RenderViewportArray&			GetRenderViewports() const;
		void								RefreshAllViewports();
		void								DirtyAllViewports();

		C_CameraConnection*					GetCameraConnection();
		I_CameraController*					GetCameraController(const C_RenderViewport* pViewport);
		const I_CameraController*			GetCameraController(const C_RenderViewport* pViewport) const;
		void								SetCameraController(const C_RenderViewport* pViewport, I_CameraController* pController);
		void								DestroyCameraController(I_CameraController *pController);
		void								DestroyAllCameraControllers();
		C_GameMessaging*					GetGameMessaging();

		C_RenderScene&						GetRenderScene();
		const C_RenderScene&				GetRenderScene() const;
		C_RenderPipeline&					GetRenderPipeline();
		const C_RenderPipeline&				GetRenderPipeline() const;

		C_ManipulatorManager&				GetManipulatorManager();
		const C_ManipulatorManager&			GetManipulatorManager() const;

		void								FrameSelection(C_RenderViewport *pViewport) const;
		void								FrameScene(C_RenderViewport *pViewport) const;

	private:

		RenderViewportArray					m_pViewports;
		CameraControllerArray				m_pCameraControllers;
		AftkRender::C_Application			*m_pRenderApplication;
		C_CameraConnection					*m_pCameraConnection;
		C_GameMessaging						*m_pGameMessaging;

		C_RenderScene*						m_pRenderScene;
		C_RenderPipeline*					m_pRenderPipeline;
		C_ManipulatorManager*				m_pManipulatorManager;
	};
}

#endif