#ifndef VIEWPORTFOVSCALE_H
#define VIEWPORTFOVSCALE_H

#include <Xt/XtGame.h>

namespace AmaxCam
{

class C_ViewportFovScale
{
public:
	XtF32 m_TwoPlayer;
	XtF32 m_ThreePlayer;
	XtF32 m_FourPlayer;
	C_ViewportFovScale() { m_TwoPlayer = m_ThreePlayer = m_FourPlayer = 1.f; }
	
	static void SorceryCallback(XtEventType evt, XtRecord*, C_ViewportFovScale* self, XtCallbackContext*);
};

} // End of namespace

#endif
