#pragma once

namespace Nexus
{

	class NexusRender
	{
	public:
		NexusRender();
		~NexusRender();

		Tu::Status Create(
			CView* pView,
			NexusSystem* pNexusSystem);
		void Destroy();

		bool IsCreated() const;

		void SetViewportRectangle(const RECT& rect);

		Tu::Status OnLoadTextures();

		void SetGlobalShadingMode(NexusShadingTypeEnum state);

		NexusShadingTypeEnum GlobalShadingMode() const;

		//Events...

		Tu::Status OnUpdateScene();

		Tu::Status Draw();

		void OnCameraZoom();

		void CentreCamera();

		void FrameAll();

		void FrameSelected();

		void Frame(const Tu::Bounds& bounds);

		//Queries
        
		SRD::Camera* CameraPtr();

		//Only allow d3d to be queried, not driven from outside this class.
		const SRD::D3DWindow* D3DWindowPtr() const;

		const SRD::ISceneRenderer* ISceneRendererPtr() const;

		SRD::ISceneRenderer* ISceneRendererPtr();

		void Resize(U32 width, U32 height);

		void Pick();

		void SetTime(F32 time);

		void UpdateTime(F32 time);

		F32 Time() const;
		
		bool AnimationPaused() const;
		void ToggleAnimation();
		void ResetAnimation();
		void FrameSkip(S32 skip);

		void SetSettings(const SRD::SceneRendererSettings& settings);

		SRD::SceneRendererSettings& Settings();

		const SRD::SceneRendererSettings& Settings() const;

		bool TexturesLoaded() const;

		void Clear();

	private:
		NexusRender(const NexusRender& rhs);
		const NexusRender& operator=(const NexusRender& rhs);

		bool Draw2D();
		bool Draw3D();

		Tu::Status OnResetDevice();

		void CleanUpAfterFailure();

		bool				m_isCreated;
		CView*				m_pView;
		RECT				m_viewportRect;

		SRD::ISceneRenderer*	m_pISceneRenderer;
		SRD::D3DWindow		m_d3dWindow;
		SRD::Camera*		m_pCamera;

		NexusSystem*		m_pNexusSystem;

		NexusShadingTypeEnum m_globalShadingMode;

		F32					m_time;

		bool				m_animationPaused;

		//Frequency of the PC's performance counter.
		LARGE_INTEGER		m_timerFrequency;

		LARGE_INTEGER		m_startTime;

		SRD::SceneRendererSettings m_settings;
	};
}

