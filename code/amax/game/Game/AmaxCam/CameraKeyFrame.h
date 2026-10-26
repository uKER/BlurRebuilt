#ifndef AMAXCAM_CAMERAKEYFRAME_H
#define AMAXCAM_CAMERAKEYFRAME_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/XtGame.h>

namespace AmaxCam
{

class C_CameraKeyFrame
{
public:
	F32 u, value, in, out;
	
	static F32 Evaluate(const XtArray<C_CameraKeyFrame>& keyFrames, F32 u);
};

} // End of namespace

#endif // AMAXCAM_CAMERAKEYFRAME_H
