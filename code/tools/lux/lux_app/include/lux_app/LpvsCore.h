#ifndef __LPVSCORE_H__
#define __LPVSCORE_H__

class CPolygonPicker;

namespace LPVS
{
	#define LPVSCORE LpvsCore::GetSingleton()

	enum LpvsCoreStateEnum
	{
		APP_MAIN_STATE_IDLE,
		APP_MAIN_STATE_RENDERING,
		APP_MAIN_STATE_READING_PROJECT,

		APP_MAIN_STATE_COUNT
	};

	class LpvsCore : public LUX::Core
	{
	public:
		LpvsCore();
		~LpvsCore();

		static LpvsCore&		GetSingleton();
		static bool				m_bShuttingDown;

		bool					Create(HWND renderWindowHandle);
		void					Destroy();

		bool					ReadProject(
			const std::string& fileName,
				MultiProgressNotification* pProgress = 0);
		bool					WriteProject(
			const std::string& fileName,
				MultiProgressNotification* pProgress = 0);
		void					CloseProject();

		void					SetLightingModelPropertiesPtr(LUX::LightingModelProperties* pProperties);
		
		const LUX::LightingModelProperties* GetLightingModelPropertiesPtr() const;
		LUX::LightingModelProperties*		GetLightingModelPropertiesPtr();

		Aftk::C_Scene				&GetScene() { return m_aftk; }

		///
		///Renders the current scene.
		bool					RenderScene();

		bool					ReadScene(MultiProgressNotification* pProgress = 0);
		void					ComputeSceneBounds();

		void					CentreCamera();

		SRD::SceneRenderer		*GetSceneRenderer() { return m_pSceneRenderer; }

		//
		///Frames the entire scene within the viewport
		void					FrameAll();
		///
		///Resizes the current scene.
		void					Resize(U32 width, U32 height);

		void					SetCameraType(SRD::SrCameraTypeEnum cameraType);
		
		///Returns true if the application is currently running.
		bool					GetIsRunningFlag() const;

		SRD::Camera*				GetCameraPtr();
		const SRD::Camera*		GetCameraPtr() const;
		const SRD::D3dWindow*	GetD3dWindowPtr() const;
		SD::SpatialHierarchy*	GetSpatialHierarchy() { return m_pSpatialHierarchy; }

		const Bounds			&GetSceneBounds() const;

		void					SetViewportRectangle(const RECT& rect);

		void					SetProgressCallBackProc(void *pThis, LUX::ProgressCallbackProc *pCallback);
		
		void					SetIsRunningFlag(bool flag);
		void					UpdateVertexBuffers();

		LpvsCoreStateEnum		GetState() const;
		/****** Gets and Sets ******/
		void					Clear();

		void					SetSelection(SD::RayTraceResult *pResult);

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void Reset();

		//Rendering code for main window
		bool Render2D();
		bool Render3D();
		//End rendering code for main window

		void AddLightsFromScene();

		void					CloseScene();

		static void Init();

		static LpvsCore*		m_pSingleton;

		HWND					m_renderWindowHandle;

		SRD::D3dWindow			m_d3dWindow;

		bool					m_runningFlag;
		// Timing
		F32						m_fps;

		SRD::HierachyRenderer*	m_pHierarchyRenderer;
		SRD::SceneRenderer*		m_pSceneRenderer;

		DebugRender				m_debugRender;

		RECT					m_rect;

		SRD::Camera*				m_pCamera;

		LpvsCoreStateEnum		m_state;

		Bounds					m_sceneBounds;
		std::map<Aftk::C_SceneNode*, Bounds>	
								m_aftkSceneNodeBounds;

		// For drawing the selection
		CPolygonPicker			*m_pPicker;
	#endif
	};
}

#endif