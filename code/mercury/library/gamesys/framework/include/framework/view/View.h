#ifndef FRAMEWORK_VIEW_H
#define FRAMEWORK_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/View/View.h"

namespace Cam2
{
	class C_DebugFeed;
}

namespace Framework
{

//! C_View

class C_View : public Render::C_View
{
public:
	C_View();
	~C_View();

	const Cam2::C_DebugFeed& DebugFeed() const
	{
		assert(m_pDebugFeed);
		return *m_pDebugFeed;
	}

	Cam2::C_DebugFeed& DebugFeed()
	{
		assert(m_pDebugFeed);
		return *m_pDebugFeed;
	}
	
	void Update(const S32 index, const F32 frameTime);

private:
	Cam2::C_DebugFeed* m_pDebugFeed;

#ifdef PLATFORM_PC
public:
	// [Blur2PC] retail Blur PC's view (Blur.exe 1.2, 0x7e0 bytes) adds three viewports, set when the display
	// windows are added or resized (0x0084ebf0, 0x0084d9f0, 0x0084a240); names ours
	Display::C_Viewport m_WindowViewport;			// +0x790 the whole display window: 0, 0, window width, height
	Display::C_Viewport m_WindowScreenViewport;		// +0x7a8 0, 0, the window's width and height on screen
	Display::C_Viewport m_ScreenViewport;			// +0x7c0 the window's place on screen
#endif

};

}; // namespace Framework

#endif // FRAMEWORK_VIEW_H
