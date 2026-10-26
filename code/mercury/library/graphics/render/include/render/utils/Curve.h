#ifndef RENDER_UTILS_CURVE_H
#define RENDER_UTILS_CURVE_H

#include <Misc_Shared/Curve/HermiteCurve.h>
#include <Xt/XtGame.h>

namespace Render
{

struct HermiteKeyFrame
{
	XtF32 time;
	XtF32 value;
	XtF32 inTangent;
	XtF32 outTangent;
};

struct Curve
{
	XtArray<HermiteKeyFrame> keyFrames;
};

class C_HermiteCurve
{
public:

	INLINE C_HermiteCurve()
	{
		m_Curve.Clear();
	}

	INLINE C_HermiteCurve(const Curve& curve)
	{
		m_Curve.Clear();
		for (S32 i = 0; i < curve.keyFrames.count; i++)
		{
			const HermiteKeyFrame& keyFrame = curve.keyFrames[i];

			m_Curve.AddKeyFrame(keyFrame.time,
				keyFrame.value,
				keyFrame.inTangent,
				keyFrame.outTangent);
		}
	}

	INLINE F32 Evaluate(F32 fTime, F32 fDefault = 0.0f) const
	{
		return m_Curve.Valid() ? m_Curve.Evaluate(fTime) : fDefault;
	}

	INLINE void Clear()
	{
		m_Curve.Clear();
	}

private:

	::Curve::C_HermiteCurveFixed4		m_Curve;

};

}; // namespace Render

#endif // RENDER_UTILS_CURVE_H
