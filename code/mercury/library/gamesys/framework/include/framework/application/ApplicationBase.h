#ifndef FRAMEWORK_APPLICATION_BASE_H
#define FRAMEWORK_APPLICATION_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Threads/Misc.h>
#include "Framework/Application/I_Application.h"
#include "Framework/View/View.h"
#include "Framework/Timer/Timer.h"
#include "Framework/Layer/LayerManager.h"
#include <Input/Input.h>

#ifndef CFG_GOLD
#define GAMEAPP_USE_DISPLAY_DEBUGRENDER
#define GAMEAPP_USE_DEBUGRENDER
#define GAMEAPP_USE_FONTSYSTEM
#endif

namespace Cam2 { class C_DebugFeed; };
namespace Render { namespace Debug { class C_DebugRenderSys; }; };
namespace Render { namespace Font { class C_FontSystem; }; };

namespace Framework
{
	extern Debug::Int g_debugPresentInterval;

//! E_UserInputIndex

enum E_UserInputIndex
{
	E_UserInputIndex_0 = 0,
	// [Blur2PC] Retail PC's split-screen input loop addresses all four slots.
	E_UserInputIndex_1,
	E_UserInputIndex_2,
	E_UserInputIndex_3,
	E_UserInputIndex_Count,
};

enum E_LayerPause
{
	E_LayerPause_DebugMenu	= 0,
	E_LayerPause_Freeze,
	E_LayerPause_Game,

	E_LayerPause_Count,
};

enum E_ViewSetup
{
	E_ViewSetup_Unknown,
	E_ViewSetup_1,
	E_ViewSetup_2h,
	E_ViewSetup_2v,
	E_ViewSetup_3,
	E_ViewSetup_3h,
	E_ViewSetup_3v,
	E_ViewSetup_4,
};

enum E_RunState
{
	E_RunState_Run,
	E_RunState_Quit,
	E_RunState_QuitForced
};

class C_Layer;

//! C_ApplicationBase. Cross-platform application.

class C_ApplicationBase : public I_Application
{
public:

	//! Constructor.
	C_ApplicationBase();

	//! Destructor.
	virtual ~C_ApplicationBase();

	//! Gets the application instance.
	static C_ApplicationBase* GetInstance()
	{
		return s_pInstance;
	}
	
	//! Exit from the main loop
	void RequestShutdown(bool force = false)
	{
		if (force)
			m_RunState = E_RunState_QuitForced;
		else
			m_RunState = E_RunState_Quit;
	}

	//! Is the application paused due to loss of focus?
	bool IsFocusPaused() const
	{
		return m_FocusPaused;
	}

	void SetFocusPaused(const bool paused)
	{
		m_FocusPaused = paused;
	}

	//! Has the application been initialised?
	bool IsInitialised() const
	{
		return m_Initialised;
	}

	//! Gets the timer.
	C_Timer& GetTimer()
	{
		return m_Timer;
	}

	//! Gets the timer.
	const C_Timer& GetTimer() const
	{
		return m_Timer;
	}

	const C_Timer& DebugTimer() const
	{
		return m_DebugTimer;
	}

	const S32 GetCurrentSplitscreenView() const
	{
		return m_iSplitscreenView;
	}

	void SetCurrentSplitscreenView(const S32 iView)
	{
		m_iSplitscreenView = iView;
	}

	//! Gets the fullscreen view.
	C_View& GetFullscreenView()
	{
		return m_FullscreenView;
	}

	//! Sets the view setup for splitscreen views
	void SetViewSetup(const E_ViewSetup viewSetup);

	//! Gets the splitscreen view count.
	const S32 GetSplitscreenViewCount() const
	{
		return m_SplitscreenViews.Size();
	}

	//! Gets the splitscreen view.
	const C_View& GetSplitscreenView() const
	{
		assert(m_iSplitscreenView >= 0 && m_iSplitscreenView < m_SplitscreenViews.Size());
		return m_SplitscreenViews[m_iSplitscreenView];
	}

	//! Gets the splitscreen view.
	const C_View& GetSplitscreenView(const S32 viewIndex) const
	{
		assert(viewIndex >= 0 && viewIndex < m_SplitscreenViews.Size());
		return m_SplitscreenViews[viewIndex];
	}

	Cam2::C_DebugFeed& GetDebugFeed(const S32 viewIndex)
	{
		assert(viewIndex >= 0 && viewIndex < m_SplitscreenViews.Size());
		return m_SplitscreenViews[viewIndex].DebugFeed();
	}

	//! Sets the splitscreen view count.
	void SetSplitscreenViewCount(const S32 viewCount);

	//! Gets the input user count.
	const S32 GetInputUserCount() const
	{
		return m_InputUserCount;
	}

	//! Gets the input button count.
	const S32 GetInputButtonCount() const
	{
		return m_InputButtonCount;
	}

	//! Gets the input axis count.
	const S32 GetInputAxisCount() const
	{
		return m_InputAxisCount;
	}

	//! Sets the input user count.
	void SetInputUserCount(const S32 count);

	//! Sets the input button count.
	void SetInputButtonCount(const S32 count);

	//! Sets the input axis count.
	void SetInputAxisCount(const S32 count);

	//! Gets the keyboard.
	Input::C_Device* GetKeyboard(const U32 index) const
	{
		assert(index >= 0 && index < E_UserInputIndex_Count);
		return m_pKeyboardList[index];
	}

	//! Gets the mouse.
	Input::C_Device* GetMouse(const U32 index) const
	{
		assert(index >= 0 && index < E_UserInputIndex_Count);
		return m_pMouseList[index];
	}

	//! Gets the pad.
	Input::C_Device* GetPad(const U32 index) const
	{
		assert(index >= 0 && index < E_UserInputIndex_Count);
		return m_pPadList[index];
	}

	//! Sets up the user controls.
	virtual void SetupUserControls();

	//! Initialises the application.
	virtual HRESULT Initialise(const C_ApplicationParams& params);

	virtual void PollPlatformCallbacks() {}

	//! Shutown the application.
	virtual void Shutdown(bool force = false);

	//! Do all the common update gubbins.
	virtual void CommonUpdate();

	//! Do all the common render gubbins.
	virtual void CommonRender();

	//! Begins the scene.
	virtual bool BeginRender();

	//! Ends the scene.
	virtual void EndRender();

	//! Inserts a layer into the application.
	void InsertLayer(C_Layer* pLayer);

	//! Removes a layer from the application.
	void DestroyLayer(C_Layer* pLayer);

	//! Inserts a layer into the application. The layer initialisation will happen on a different thread.
	void InsertLayerAsync(C_Layer* pLayer);

	//! Removes a layer from the application. The layer shutdown will happen on a different thread.
	void DestroyLayerAsync(C_Layer* pLayer);

	void EnableLoadingScreen();

	void DisableLoadingScreen();

	bool IsLoadingScreenEnabled() const;

	void Pause( const S32 pauseId );

	void Resume( const S32 pauseId );

	bool IsPaused() const;

	bool IsPaused( const S32 pauseId ) const;

	#ifdef PLATFORM_PC
	virtual bool CanRender() = 0;

	// [Blur2PC] retail Blur PC: applies graphics settings changed since the last frame (C_Application
	// override 0x0084bad0), called from BeginRender.
	virtual void UpdateGraphicsSettings() {}

	// [Blur2PC] retail Blur PC: pushes the graphical detail level (1-4) to the game; the framework's is
	// empty (retail 0x00407830), the game's applies the quality preset (C_GameApp, retail 0x0042e1c0).
	virtual void ApplyGraphicalDetail(const U32 graphicalDetail) {}
	#endif

	void SetupViews();

	MT::ThreadId GetMainThreadId() const
	{
		return m_MainThreadId;
	}

	virtual void ExitChecks();

	bool GetUpdatePresentRender() const;
	void SetUpdatePresentRender(const bool enable);

	virtual void PrePresent();

	virtual void ProcessLoadingScreen();

protected:
	
	//! Platform specific application initialise.
	virtual HRESULT PlatformInitialise(const C_ApplicationParams& params);

	//! Platform specific application shutdown.
	virtual void PlatformShutdown();
	
	//! User callback application initialise.
	virtual HRESULT OnInitialise();

	//! User callback application shutdown.
	virtual void OnShutdown();
	
	//! User callback application update.
	virtual void OnUpdate();

	//! User callback application render.
	virtual void OnRender();

	//! User callback application loading screen update.
	virtual void OnLoadingScreenUpdate();

	//! User callback application loading screen render.
	virtual void OnLoadingScreenRender();

	//! Creates display.
	virtual HRESULT CreateDisplay();

	//! Destroys display.
	virtual void DestroyDisplay();

	//! Creates the device.
	virtual HRESULT CreateDevice();

	//! Destroys the device.
	virtual void DestroyDevice();

	//! Sets up the device memory params.
	virtual void SetupDeviceMemoryParams();

	//! Sets up the device params.
	virtual void SetupDeviceParams();

	//! Creates the input manager.
	virtual HRESULT CreateInputManager(const C_ApplicationParams& params);

	//! Destroys the input manager.
	virtual void DestroyInputManager();

	//! Updates the input manager.
	virtual void UpdateInputManager();

	//! Updates the application layers.
	virtual void UpdateLayers();

	virtual void BeginFrame();

	virtual void BeginSplitscreen();

	virtual void ScheduleOnce();

	virtual void ScheduleSplitscreen();

	virtual void RenderOnce();

	virtual void PreRenderSplitscreen();

	virtual void RenderSplitscreenWithAA();

	virtual void BeginRenderSplitscreenWithoutAA();

	virtual void RenderSplitscreenWithoutAA();

	virtual void EndRenderSplitscreenWithoutAA();

	virtual void ResetDebugRender();

	virtual void RenderFullscreen();

	virtual void EndFrame();

	virtual HRESULT SendMessageToLayers(const U32 message);

#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x0084e6a0, 0x0084ebf0) sizes the windows from the device parameters it is given
	virtual void AddDisplayWindows(const Display::C_DeviceParams& deviceParams);

	virtual void AddDisplayWindow(const Display::C_DeviceParams& deviceParams, const Maths::C_Rect& windowCoords);
#else
	virtual void AddDisplayWindows();

	virtual void AddDisplayWindow(const Maths::C_Rect& windowCoords);
#endif

	virtual void ResizeDisplayWindows();

	virtual void SetupDisplayResolutions(const U32 width, const U32 height, const Display::E_Format format, const Display::E_Multisample multisample, const U32 flags);

	void Present();

	void RenderSplitscreenWithAndWithoutAA();

	void PreRenderSplitscreenView(const S32 viewIndex);

	void RenderSplitscreenViewWithAA(const S32 viewIndex);

	void RenderSplitscreenViewWithoutAA(const S32 viewIndex);

protected:

	E_RunState								m_RunState;

	bool									m_FocusPaused;
	bool									m_Created;
	bool									m_Initialised;
	bool									m_UpdatePresentRender;
	bool									m_LoadingScreenEnabled;

	C_Timer									m_Timer;
	C_Timer									m_DebugTimer;
	
	Display::C_DeviceParams					m_DisplayDeviceParams;
	Display::C_Device*						m_pDisplayDevice;
	Display::C_Manager*						m_pDisplayManager;

	S32										m_InputUserCount;
	S32										m_InputButtonCount;
	S32										m_InputAxisCount;

	Input::C_DeviceManager*					m_pInputDeviceManager;
	Input::C_GameInputManager*				m_pGameInputManager;
	Input::C_Device*						m_pKeyboardList[E_UserInputIndex_Count];
	Input::C_Device*						m_pMouseList[E_UserInputIndex_Count];
	Input::C_Device*						m_pPadList[E_UserInputIndex_Count];

	C_LayerManager							m_LayerManager;

	C_View									m_FullscreenView;
	Ut::Vector<C_View>						m_SplitscreenViews;
	S32										m_iSplitscreenView;
	E_ViewSetup								m_ViewSetup;

	#ifdef GAMEAPP_USE_DISPLAY_DEBUGRENDER
	Display::C_DebugRender*					m_pDebugRender;
	#else
	Display::C_BasicRender*					m_pBasicRender;
	#endif // else

	#ifdef GAMEAPP_USE_DEBUGRENDER
	Render::Debug::C_DebugRenderSys*		m_pDebugRenderSys;
	#endif
	
	#ifdef GAMEAPP_USE_FONTSYSTEM
	Render::Font::C_FontSystem*				m_pFontSystem;
	#endif

	S32										m_Resolution;
	MT::ThreadId							m_MainThreadId;

private:
	
	static C_ApplicationBase*				s_pInstance;
};

#ifdef PLATFORM_PC
// [Blur2PC] retail Blur PC: set a view's three window viewports (ApplicationBase.cpp)
void SetWindowViewports(C_View& view, const Display::C_Window* pWindow, const U32 screenWidth, const U32 screenHeight);
#endif

// Specific CaptureService with callback for changing FPS in MercuryGame / Amax
class C_CaptureService;

extern C_CaptureService g_CaptureService;

}; // namespace Framework

#endif // FRAMEWORK_APPLICATION_BASE_H
