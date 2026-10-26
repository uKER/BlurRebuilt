#ifndef SAMPLES_VIEWPORT_H
#define SAMPLES_VIEWPORT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Samples
{

class C_Viewport : public Display::C_Viewport
{
public:

	C_Viewport();
	~C_Viewport();

public:

	F32 m_X1;
	F32 m_Y1;
	F32 m_X2;
	F32 m_Y2;
};

}; // namespace Samples

#endif // SAMPLES_VIEWPORT_H
