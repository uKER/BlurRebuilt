#ifndef AMAXCHASEPARAM_H
#define AMAXCHASEPARAM_H

#include <Xt/XtGame.h>

namespace AmaxCam
{

class C_AmaxChaseParam
{
public:
	C_AmaxChaseParam();

	XtF32 m_Distance, m_Height, m_Fov, m_Tilt, m_BehindSpring, m_AngularLimit, m_AngularFade;
	XtF32 m_PivotZ, m_SafeHeight;
};

} // End of namespace

#endif
