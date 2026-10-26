#ifndef SAMPLES_VIEW_H
#define SAMPLES_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/System/Camera.h"

namespace Samples
{

class C_View
{
public:

	C_View();
	~C_View();

	void Initialise(const Display::C_Rect& windowCoords);

	void Update(const F32 frameTime);

public:

	//Kier's hack so he can modify this from a constant view
	mutable C_Camera			m_Camera;
	Display::C_Rect				m_WindowCoords;
	S32							m_ViewIndex;
	Display::C_ViewSetup		m_ViewSetup;
};

}; // namespace Samples

#endif // SAMPLES_VIEW_H
