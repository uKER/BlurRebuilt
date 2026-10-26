#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H

namespace SRD
{
	struct SceneRendererSettings
	{
		bool	bRealtimeLighting;
		bool	bViewLightPaths;
		bool	bViewSamplePoints;
		enum LightMapType
		{
			LMType_FullColour,
			LMType_DXT1,
			LMType_A8L8,
			LMType_Multiplied,
			LMType_DebugUnwrapping,
			LMType_Bumpy,
			LMType_VertexSH,
		}		iLightmapType;
	};

	class SceneRenderer
	{
	public:
		virtual ~SceneRenderer();

		//Create and associate with a window and scene.
		SceneRenderer(D3dWindow* pD3dWindow, const Aftk::C_Scene* pScene);

		virtual bool Create() = 0;
		virtual void Destroy() = 0;

		virtual bool Clear() = 0;

		void SetCameraPtr(const Camera* pCamera);

		virtual bool OnResetDevice() = 0;
		///
		///Render the current scene to the display device
		virtual void OnUpdateDevice() = 0;
		///
		virtual bool RebuildVertexBuffers() = 0;

		inline void SetSettings(SceneRendererSettings &settings)
					{ m_settings = settings; }

	protected:
		virtual void Reset();

		SceneRenderer();
		SceneRenderer(const SceneRenderer& rhs);
		const SceneRenderer& operator=(const SceneRenderer& rhs);

		const Camera*		m_pCamera;

		const Aftk::C_Scene*	m_pScene;
		///
		///Store the window that we are attached to.
		D3dWindow*			m_pD3dWindow;

		///
		bool				m_vertexBuffersBuiltFlag;

		SceneRendererSettings m_settings;

		// layers

	};
}

#endif
