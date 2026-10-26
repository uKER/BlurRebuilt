#ifndef __LUX_DATATYPES_GOTHAMNIGHT_H__
#define __LUX_DATATYPES_GOTHAMNIGHT_H__

// datatype definition for Gotham nighttime 

namespace LUX
{

namespace DataTypes
{

class GothamNight : public BaseType
{
public:
	Colour a1, a2;

	typedef GothamNightContext ContextDataType;

	/// default constructor
	GothamNight()
	{
	}

	/// constructor
	GothamNight(const Colour &i1, const Colour &i2) :
		a1(i1), a2(i2)
	{
	}

	/// constructor
	GothamNight(float f) :
		a1(f), a2(f)
	{
	}

	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		assert(sample.vColour.m_c == 3 || sample.vColour.m_c == 4); // work with direct or bounce lighting
		float3 vColour = float3(sample.vColour.m_v[0], sample.vColour.m_v[1], sample.vColour.m_v[2]);
		float f = vColour.x + vColour.y + vColour.z;
		if (f <= 0)
			return;	// this is to prevent empty samples from settings the animation subchannel

		switch (sample.target)
		{
		default:
			// add to both channels
			a1.r = vColour.x, a1.g = vColour.y, a1.b = vColour.z;
			a2.r = vColour.x, a2.g = vColour.y, a2.b = vColour.z;
			break;
		case RenderChannel::AnimatedOff:
			// add to first channel only
			a1.r = vColour.x, a1.g = vColour.y, a1.b = vColour.z;
			context.m_bDualLayer = true;
			if (f>context.m_fHighestSample)
			{
				context.m_iSubChannel = sample.iSubChannel;
				context.m_fHighestSample = f;
			}
			break;
		case RenderChannel::AnimatedOn:
			// add to second channel only
			a2.r = vColour.x, a2.g = vColour.y, a2.b = vColour.z;
			context.m_bDualLayer = true;
			if (f>context.m_fHighestSample)
			{
				context.m_iSubChannel = sample.iSubChannel;
				context.m_fHighestSample = f;
			}
			break;
		}
	}

	/// force all elements to a value (usually 0)
	GothamNight operator =(const float f)
	{
		a1 = f;
		a2 = f;
		return *this;
	}

	/// scale an item
	GothamNight operator *(const float fScale) const
	{
		return GothamNight(a1 * fScale, a2 * fScale);
	}

	/// add two items
	GothamNight operator +(const GothamNight &i2) const
	{
		return GothamNight(a1 + i2.a1, a2 + i2.a2);
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		// (3 components + alpha scaling) *  2 layers
		return 8;
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
		Colour::ContextDataType dummy;
		a1.ApplyGammaCorrection(fGamma, dummy);
		a2.ApplyGammaCorrection(fGamma, dummy);
	}

	/// get the contents of a particular layer
	void GetData(float*&p, ContextDataType &context)
	{
		float4 v;
		v = GetAlphaScaled(float3(a1.r,a1.g,a1.b));
		p[0] = v.x,	p[1] = v.y,	p[2] = v.z,	p[3] = v.w;
		p += 4;
		v = GetAlphaScaled(float3(a2.r,a2.g,a2.b));
		p[0] = v.x,	p[1] = v.y,	p[2] = v.z,	p[3] = v.w;
		p += 4;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, GothamNight &t)
	{
		ar >> t.a1 >> t.a2;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, GothamNight &t)
	{
		ar << t.a1 << t.a2;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#endif
