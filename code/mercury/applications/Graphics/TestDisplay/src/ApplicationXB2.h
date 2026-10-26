#ifndef APPLICATION_PS3_H
#define APPLICATION_PS3_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ApplicationBase.h"

struct C_ApplicationParams : public C_ApplicationBaseParams
{
	C_ApplicationParams();
};

class C_Application : public C_ApplicationBase
{
public:

	C_Application();
	~C_Application();

	bool Create(C_ApplicationParams& params);
	void Destroy();
	void Update();

	bool Run();

private:

	C_Joypad*			m_pJoypad;

	bool				m_Quit;
};

#endif // APPLICATION_PS3_H
