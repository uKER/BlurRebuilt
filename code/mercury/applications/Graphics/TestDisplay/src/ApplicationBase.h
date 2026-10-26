#ifndef APPLICATION_BASE_H
#define APPLICATION_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Input.h"
#include "Timer.h"
#include "Camera.h"

class C_Sample;

struct C_ApplicationBaseParams
{
	C_ApplicationBaseParams();

	S32							m_BackBufferWidth;
	S32							m_BackBufferHeight;
};


class C_ApplicationBase
{
public:

	C_ApplicationBase();
	~C_ApplicationBase();

	void SetSample(C_Sample* pSample);

	bool Create();
	void Destroy();

	virtual void Update();
	virtual void Render();

public:

	C_Timer						m_Timer;
	C_Camera					m_Camera;

	Display::C_ViewSetup		m_ViewSetup;
	Display::C_View				m_View;

	Display::C_Manager			m_DeviceManager;
	Display::C_DeviceParams		m_DeviceParams;
	Display::C_Device			m_Device;
	Display::C_DebugRender		m_DebugRender;

	C_InputManager				m_InputManager;

private:

	C_Sample*					m_pSample;
};

#endif // APPLICATION_BASE_H
