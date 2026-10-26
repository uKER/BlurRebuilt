#ifndef SAMPLES_APPLICATION_H
#define SAMPLES_APPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Render/DebugRender.h"
#include "Samples/Section/Section.h"
#include "Samples/System/Input.h"
#include "Samples/System/Timer.h"
#include "Samples/System/View.h"
#include "Samples/System/Application.h"

namespace Samples
{

//! E_ViewSetup

enum E_ViewSetup
{
	E_ViewSetup_1,
	E_ViewSetup_2h,
	E_ViewSetup_2v,
	E_ViewSetup_3h,
	E_ViewSetup_3v,
	E_ViewSetup_4,
};

//! C_Application

class C_Application
{
public:
	
	C_Application();
	virtual ~C_Application();

	bool Create();
	void Destroy();

	bool ReadConfig(const Util::C_String& fileName);
	
	virtual void Run() = 0;

	const C_View* GetView(S32 view) const
	{
		if(view < m_GameViewCount)
			return &m_GameViews[view];
		else
			return NULL;
	}

	void SetupResolutions(const U32 width, const U32 height, const Display::E_Format format, const Display::E_Multisample multisample, const U32 flags);

protected:

	void AddSection(C_SectionLauncherBase* pSection);

	virtual void Update();
	virtual void Render();
	virtual void RenderGameViewWithAA();
	virtual void RenderGameViewWithoutAA();
	virtual void RenderHudView();
	virtual void BeginRenderSplitscreenWithoutAA();
	virtual void EndRenderSplitscreenWithoutAA();
	virtual void RenderStage1();
	virtual void RenderStage2();
	virtual void RenderUI();

	virtual bool Resize();

	virtual bool MyCreate();
	virtual void MyDestroy();
	virtual void MyUpdate();

	void ReadConfigLine(char* pLine);
	void ReadConfigItem(char* pName, char* pItem);

	void UpdateMemoryManager();

protected:

	void AddWindow(const Maths::C_Rect& windowCoords);
	void AddWindows();
	void ResizeWindows();

	void AddViews();
	void SetupViews();
	void SetupView(C_View* pView);

public:

	bool						m_EnterSection;

	Display::C_DeviceParams		m_DeviceParams;
	
protected:

	C_Timer						m_Timer;
	F32							m_Cycle;
	C_Input						m_Input;
	C_SectionManager*			m_pSection;
	Display::C_Manager*			m_pDisplay;
	Display::C_Device*			m_pDevice;
	Display::C_DebugRender*		m_pDebugRender;
	Display::C_Texture2D		m_LogoTexture;
	S32							m_Resolution;
	Util::C_String				m_Name;
	bool						m_DeviceLost;

	C_View						m_HudView;
	S32							m_GameViewCount;
	static const S32			m_MaxGameViewCount = 4;
	C_View						m_GameViews[m_MaxGameViewCount];
};

C_Application* GetApp();
void SetApp(C_Application* pApp);

}; //namespace Samples

#endif // SAMPLES_APPLICATION_H
