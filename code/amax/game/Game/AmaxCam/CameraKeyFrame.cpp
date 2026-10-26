
#include "stdafx.h"
#include "CameraKeyFrame.h"
#include <xt/XtGame.h>

namespace AmaxCam
{

static S32 SearchForKey(const XtArray<C_CameraKeyFrame>& keyFrames, F32 u)
{
	S32 low = 0, high = keyFrames.count - 1;
	
	while (high > low + 1)
	{
		S32 mid = (low + high) >> 1;
		if (u < keyFrames[mid].u)
			high = mid;
		else
			low = mid;
	}
	
	return low;
}

F32 C_CameraKeyFrame::Evaluate(const XtArray<C_CameraKeyFrame>& keyFrames, F32 u)
{
	// Get the key index before the 'u' value:
	S32 kn = SearchForKey(keyFrames, u);
	
	S32 max = keyFrames.count;
	if (kn < 0 || kn >= max)
		return 0.f;
	
	const C_CameraKeyFrame& keya = keyFrames[kn];
	if (kn + 1 >= max)
		return keya.value;

	const C_CameraKeyFrame& keyb = keyFrames[kn + 1];
	
	F32 period = keyb.u - keya.u;
	if (period < 1e-6f)
		return keya.value;
	
	F32 x = (u - keya.u) / period;
	if (x < 0.f)
		x = 0.f;		
	if (x > 1.f)
		x = 1.f;		
	
	F32 x2 = x * x;
	F32 x3 = x * x * x;
	F32 h1 =       -x3 + 3.f * x2 - 3.f * x + 1.f;
	F32 h2 =  3.f * x3 - 6.f * x2 + 3.f * x;
	F32 h3 = -3.f * x3 + 3.f * x2;
	F32 h4 =        x3;

	F32 out = (F32)tan(keya.out * M_PI / 180.f) / 3.f;
	F32 in  = (F32)tan(keyb.in  * M_PI / 180.f) / 3.f;
	
	F32 y1 = keya.value;
	F32 y4 = keyb.value;
	F32 y2 = y1 + out * period;
	F32 y3 = y4 - in  * period;

	F32 value = y1 * h1 + y2 * h2 + y3 * h3 + y4 * h4;
	return value;
}

}

XT_TYPE(AmaxCam::C_CameraKeyFrame, "CameraKeyFrame")
	XT_FIELD(u, "u")
	XT_FIELD(value, "value")
	XT_FIELD(in, "in")
	XT_FIELD(out, "out")
XT_END_TYPE
