#ifndef HERMITECURVE_H
#define HERMITECURVE_H

#include <core/Maths.h>
#include <btl/containers/vector.h>

namespace Curve
{

struct HermiteKeyFrame
{
	HermiteKeyFrame() {}
	HermiteKeyFrame(F32 _position, F32 _value, F32 _tangentIn, F32 _tangentOut) : position(_position), value(_value), tangentIn(_tangentIn), tangentOut(_tangentOut) {}

	F32 position;
	F32 value;
	F32 tangentIn;
	F32 tangentOut;
};

template<class Vector>
class T_HermiteCurve
{
public:
	// construction
	void	SetDefault(F32 value = 1.0f);	// give the curve some default values
	void	Clear();	// clear all current keyframes
	bool	AddKeyFrame(const HermiteKeyFrame& keyFrame);
	bool	AddKeyFrame(F32 position, F32 value, F32 tangentIn, F32 tangentOut);

	// evaluation
	void	EvaluateTable(Ut::Vector<F32>& table, S32 steps) const;
	F32		Evaluate(F32 position) const;
	F32		Length() const;
	bool	Valid() const;
private:
	F32		GetValue(S32 keya, S32 keyb, const Maths::HF32& position) const;
	S32		GetKey(F32 x) const;
	
	Vector	m_KeyFrames;
};

template<class Vector>
INLINE void T_HermiteCurve<Vector>::Clear()
{
	m_KeyFrames.Clear();
}

template<class Vector>
INLINE void T_HermiteCurve<Vector>::SetDefault(F32 value)
{
	m_KeyFrames.Clear();
	m_KeyFrames.PushBack(HermiteKeyFrame(0.0f, value, 0.0f, 0.0f));
	m_KeyFrames.PushBack(HermiteKeyFrame(1.0f, value, 0.0f, 0.0f));
}

template<class Vector>
INLINE F32 T_HermiteCurve<Vector>::Length() const
{
	// Find last keyframe:
	if(m_KeyFrames.Size() <= 0)
		return 0.f;
	
	return m_KeyFrames[m_KeyFrames.Size() - 1].position; // Return x position of last keyframe
}

template<class Vector>
INLINE bool T_HermiteCurve<Vector>::Valid() const
{
	return (m_KeyFrames.Size() > 1);
}

template<class Vector>
bool T_HermiteCurve<Vector>::AddKeyFrame(F32 position, F32 value, F32 tangentIn, F32 tangentOut)
{
	// Convert from degrees to the internal gradient we use to calculate:
	HermiteKeyFrame keyFrame;
	keyFrame.position = position;
	keyFrame.value = value;
	keyFrame.tangentIn = tan(tangentIn * (F32)M_PI / 180.f) / 3.f;
	keyFrame.tangentOut = tan(tangentOut * (F32)M_PI / 180.f) / 3.f;
	return AddKeyFrame(keyFrame);
}

template<class Vector>
bool T_HermiteCurve<Vector>::AddKeyFrame(const HermiteKeyFrame& keyFrame)
{
	// only allow keyframes to be added in order
	if(!m_KeyFrames.Empty())
	{
		if(keyFrame.position > m_KeyFrames.Back().position)
			m_KeyFrames.PushBack(keyFrame);
		else
			return false;
	}
	else
		m_KeyFrames.PushBack(keyFrame);

	return true;
}

// Binary search for the keyframe which is before or at a certain point x:
template<class Vector>
S32 T_HermiteCurve<Vector>::GetKey(F32 position) const
{
	S32 low = 0;
	S32 high = m_KeyFrames.Size() - 1;
	
	while(high > low + 1)
	{
		S32 mid = (low + high) >> 1;
		if(position < m_KeyFrames[mid].position)
			high = mid;
		else
			low = mid;
	}
	
	return low;
}

template<class Vector>
void T_HermiteCurve<Vector>::EvaluateTable(Ut::Vector<F32>& table, S32 steps) const
{
	if(Valid())
	{
		table.Clear();
		table.Resize(steps);
		
		steps -= 1;
		Maths::HF32 step = 1.0f/(F32)steps;
		Maths::HF32 time = 0.0f;
		Maths::HF32 size = (F32)steps;


		S32 ka = 0;
		for(S32 i = 0; i <= steps; ++i, time += step)
		{
			if(ka + 1 < m_KeyFrames.Size() - 1)
			{
				if(time > m_KeyFrames[ka+1].position)
					++ka;
			}

			table[i] = GetValue(ka, ka + 1, time);
		}
	}
}

template<class Vector>
F32 T_HermiteCurve<Vector>::Evaluate(F32 position) const
{
	if(Valid())
	{
		// get keyframe for this position
		S32 ka = GetKey(position);
		S32 max = m_KeyFrames.Size();

		if(ka < 0 || ka >= max)
			return 0.f;

		S32 kb = ka + 1;

		if(kb >= max)
			return m_KeyFrames[ka].value;

		return GetValue(ka, kb, position);
	}
	else
	{
		return 0.f;
	}
}

template<class Vector>
F32 T_HermiteCurve<Vector>::GetValue(S32 keya, S32 keyb, const Maths::HF32& position) const
{
	Maths::HF32 period = m_KeyFrames[keyb].position - m_KeyFrames[keya].position;
	if(period <= 1e-6f)
		return m_KeyFrames[keya].value;

	Maths::HF32 u = (position - m_KeyFrames[keya].position) / period;
	
	u = Max(u, 0.0f);
	u = Min(u, 1.0f);
		
	Maths::HF32 y1 = m_KeyFrames[keya].value;
	Maths::HF32 y4 = m_KeyFrames[keyb].value;

	Maths::HF32 y2 = y1 + m_KeyFrames[keya].tangentOut * period;
	Maths::HF32 y3 = y4 - m_KeyFrames[keyb].tangentIn * period;

	Maths::HF32 u2 = u * u;
	Maths::HF32 u3 = u2 * u;
	Maths::HF32 h1 = -u3 + 3.f * u2 - 3.f * u + 1.f;
	Maths::HF32 h2 = 3.f * u3 - 6.f * u2 + 3.f * u;
	Maths::HF32 h3 = -3.f * u3 + 3.f * u2;
	Maths::HF32 h4 = u3;

	return y1 * h1 + y2 * h2 + y3 * h3 + y4 * h4;
}

class C_HermiteCurve : public T_HermiteCurve<Btl::Vector<HermiteKeyFrame> > {};
class C_HermiteCurveFixed2 : public T_HermiteCurve<Btl::Vector<HermiteKeyFrame, Btl::VectorParameters_Static<2> > >  {};
class C_HermiteCurveFixed3 : public T_HermiteCurve<Btl::Vector<HermiteKeyFrame, Btl::VectorParameters_Static<3> > >  {};
class C_HermiteCurveFixed4 : public T_HermiteCurve<Btl::Vector<HermiteKeyFrame, Btl::VectorParameters_Static<4> > >  {};

}

#endif // HERMITECURVE_H
