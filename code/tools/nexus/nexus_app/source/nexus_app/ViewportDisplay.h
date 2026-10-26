#ifndef NEXUS_VIEWPORTDISPLAY_H
#define NEXUS_VIEWPORTDISPLAY_H

namespace Nexus
{
	class C_ViewportDisplay
	{
	public:
		C_ViewportDisplay();

		~C_ViewportDisplay();

		bool Create(CWnd* pParentCWnd);

		bool Draw();

		void OnCameraZoom();

		void CentreCamera();

		void FrameAll();

		void FrameSelected();

		void Frame(const Aftk::C_Bounds& bounds);

		NexusRender::Camera* GetCameraPtr();

		bool IsCreated() const;

		void Resize(U32 width, U32 height);

		void PreResetDevice();

		void PostResetDevice();

	private:
		void Destroy();

		bool					m_isCreated;
		NexusRender::Camera*	m_pCamera;
		C_Overlay2D*			m_pOverlay2D;
		CWnd*					m_pParentCWnd;
		IDirect3DSwapChain9*	m_pSwapChain;
		LPDIRECT3DSURFACE9		m_pDepthSurf;
		LPDIRECT3DSURFACE9		m_pRenderSurf;
	};
}

#endif
