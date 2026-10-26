#ifndef NEXUS_DISPLAYMANAGER_H
#define NEXUS_DISPLAYMANAGER_H

namespace Nexus
{
	class C_DisplayManager
	{
	public:
		C_DisplayManager();
		~C_DisplayManager();

		Tu::Status Create(CWnd* pWnd);

		bool IsCreated() const;

		const NexusRender::C_D3DWindow* GetD3DWindowPtr() const;

		NexusRender::C_D3DWindow* GetD3DWindowPtr();

		const NexusRender::SceneRenderer* GetRendererPtr() const;

		NexusRender::SceneRenderer* GetRendererPtr();

		void Resize(U32 width, U32 height);

		void SetSettings(const NexusRender::SceneRendererSettings& settings);

		NexusRender::SceneRendererSettings& GetSettings();

		const NexusRender::SceneRendererSettings& GetSettings() const;

		Tu::Status ResetDevice();

		Tu::Status PostResetDevice();

		Tu::Status OnLoadTextures();

		Tu::Status OnLoadCropSheetTextures();

		void FrameAll();

		C_ViewportDisplay* GetViewportDisplayPtr(U32 index);

		void Clear();

	private:
		void Destroy();

		bool Draw3D();

		////////////////////////////////////////////////////////////////////////
		/// Static tasks
		////////////////////////////////////////////////////////////////////////
		static Tu::Status ReloadDeviceTask(void* pProgress, const Nexus::StringArray& arguments);
		static Tu::Status LoadTexturesTask(void* pProgress, const Nexus::StringArray& arguments);
		static Tu::Status LoadCropSheetTexturesTask(void* pProgress, const Nexus::StringArray& arguments);
		static Tu::Status UpdateTexturesTask(void* pProgress, const Nexus::StringArray& arguments);
		static Tu::Status UpdateSceneTask(void* pProgress, const Nexus::StringArray& arguments);

		////////////////////////////////////////////////////////////////////////
		/// Static callback hooks 
		////////////////////////////////////////////////////////////////////////
		static Tu::Status OnUpdateGeometry(Nexus::Progress* pProgress);
		static Tu::Status OnUpdateTextures(Nexus::Progress* pProgress);
		static Tu::Status OnUpdateScene(Nexus::Progress* pProgress);
		static Tu::Status OnUpdateCropSheetDatabase(Nexus::Progress* pProgress);
		////////////////////////////////////////////////////////////////////////

		bool				m_isCreated;
		CWnd*				m_pCWnd;

		NexusRender::SceneRenderer* m_pSceneRenderer;

		C_ApplicationState			m_applicationState;

		NexusSystem*				m_pNexusSystem;

		NexusRender::C_D3DWindow	m_d3dWindow;
		NexusRender::SceneRendererSettings m_settings;
	};
}

#endif
