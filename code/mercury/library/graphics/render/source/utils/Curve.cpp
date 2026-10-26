#include "stdafx.h"
#include "Render/Utils/Curve.h"

XT_TYPE(Render::HermiteKeyFrame, "Render_HermiteKeyFrame")
	XT_FIELD(time, "time")
	XT_FIELD(value, "value")
	XT_FIELD(inTangent, "inTangent")
	XT_FIELD(outTangent, "outTangent")
XT_END_TYPE

XT_TYPE(Render::Curve, "Render_Curve")
	XT_FIELD(keyFrames, "keyFrames")
XT_END_TYPE
