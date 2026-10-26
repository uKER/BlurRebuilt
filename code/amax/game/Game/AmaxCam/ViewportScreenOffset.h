#ifndef VIEWPORT_SCREEN_OFFSET_H
#define VIEWPORT_SCREEN_OFFSET_H

#include <Xt/XtGame.h>

namespace AmaxCam
{

class C_ViewportScreenOffset
{
public:
	
	XtVec2 m_TwoViewsTop;
	XtVec2 m_TwoViewsBottom;

	XtVec2 m_ThreeViewsTop;
	XtVec2 m_ThreeViewsBottomLeft;
	XtVec2 m_ThreeViewsBottomRight;

	XtVec2 m_FourViewsTopLeft;
	XtVec2 m_FourViewsTopRight;
	XtVec2 m_FourViewsBottomLeft;
	XtVec2 m_FourViewsBottomRight;
	
	static void SorceryCallback(XtEventType evt, XtRecord*, C_ViewportScreenOffset* self, XtCallbackContext*);
};

} // End of namespace

#endif // VIEWPORT_SCREEN_OFFSET_H
