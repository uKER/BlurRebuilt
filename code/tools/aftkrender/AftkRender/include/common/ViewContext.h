#ifndef AFTK_RENDER_VIEWPORT_H
#define AFTK_RENDER_VIEWPORT_H


#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_Camera;
	
	class C_ViewContext
	{
	public:
		struct View
		{
			C_Camera		camera;
			Maths::C_Rect	windowCoords;
			S32				viewIndex;

			View()
			: viewIndex(-1), windowCoords(Maths::HVec2(0.f,0.f), Maths::HVec2(1.f,1.f))
			{ }
		};
		
		struct Timer
		{
			F32	frameCount, frameTime, frameRate, elapsedTime, cycle;
			bool paused;

			Timer()
			: frameCount(0), frameTime(0), frameRate(0), elapsedTime(0), cycle(0), paused(false)
			{ }
		};

		C_ViewContext();
		~C_ViewContext();

		bool Create();
		bool Create(const Display::C_DeviceParams& deviceParams);
		bool Rebuild();
		bool Resize(U32 width, U32 height);
		void Destroy();

		virtual void Update();
		virtual void BeginRender() const;
		virtual void EndRender(bool present = true) const;

		View& GetView();
		const View& GetView() const;

		const D3DPRESENT_PARAMETERS& PresentParams() const;

	protected:
		void UpdateTimer();

		static View AddView(const C_Camera& camera, const Maths::C_Rect& windowCoords);
		void SetupView(const View& view) const;


		D3DPRESENT_PARAMETERS	m_PresentParams;
		D3DFORMAT				m_DepthStencilFormat;

		IDirect3DSwapChain9		*m_pSwapChain;
		Display::C_Surface		*m_pColourSurface;
		Display::C_Surface		*m_pDepthStencilSurface;

		View					m_View;
		Timer					m_Timer;

		bool					m_ImplicitSwapChain;
	};
}


#endif
