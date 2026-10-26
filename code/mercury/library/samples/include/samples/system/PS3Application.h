#ifndef SAMPLES_PS3APPLICATION_H
#define SAMPLES_PS3APPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/System/Application.h"
#include "Samples/System/PS3Pad.h"

namespace Samples
{

class C_PS3Application : public C_Application
{
public:
	
	C_PS3Application();
	virtual ~C_PS3Application();

	bool Create();

	void Run();

protected:

	void Update();

private:

	bool		m_Quit;
	C_PS3Pad	m_Pad;
};

}; // namespace Samples

#endif // SAMPLES_PS3APPLICATION_H
