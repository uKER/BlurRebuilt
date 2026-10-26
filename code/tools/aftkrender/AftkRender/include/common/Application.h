#ifndef AFTK_RENDER_APPLICATION_H
#define AFTK_RENDER_APPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
	namespace Debug
	{
		class C_DebugRenderSys;
	}
}

namespace AftkRender
{
	class C_ViewContext;

	class C_Application
	{
	public:
		
		C_Application();
		virtual ~C_Application();

		virtual bool Create(U32 width, U32 height);
		virtual bool Resize(U32 width, U32 height, U32 context = 0);
		virtual bool Resize(U32 width, U32 height, const C_ViewContext *pContext);

		virtual void Destroy();
		virtual void Update();

		C_ViewContext* AddViewContext(const Display::C_DeviceParams& params);

		C_ViewContext* GetViewContext(U32 index);
		const C_ViewContext* GetViewContext(U32 index) const;

		C_ViewContext* GetViewContextFromHWND(HWND hwnd);
		const C_ViewContext* GetViewContextFromHWND(HWND hwnd) const;

		void DestroyViewContext(C_ViewContext* pContext);

		bool BeginRendering(const C_ViewContext *pContext);
		void EndRendering(const C_ViewContext *pContext, bool present=true);
		
		bool CheckDeviceState();

	public:
		Display::C_DeviceParams	m_DeviceParams;

	protected:

		Ut::UtVector< Ut::DynamicVector<C_ViewContext*> > m_ViewContextArray;

		Display::C_Manager*		m_pDisplayManager;
		Display::C_Device*		m_pDisplayDevice;
	};

}
#endif