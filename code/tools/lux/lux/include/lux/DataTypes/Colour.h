#ifndef __LUX_DATATYPES_RGB_H__
#define __LUX_DATATYPES_RGB_H__

// datatype definition for an RGB colour.
// This can be used to specialize an iterator, e.g. to render per-vertex colours or colour textures

namespace LUX
{

namespace DataTypes
{

class Colour : public BaseType
{
public:
	float r,g,b;

	/// default constructor
	Colour()
	{
	}

	/// constructor
	Colour(float ir, float ig, float ib) :
		r(ir), g(ig), b(ib)		
	{
	}

	/// constructor
	Colour(float f) :
		r(f), g(f), b(f)		
	{
	}

	/// constructor
	Colour(float3 v) :
		r(v[0]), g(v[1]), b(v[2])
	{
	}
		
	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		assert(sample.vColour.m_c == 3 || sample.vColour.m_c == 4); // work with direct or bounce lighting
		float3 vColour = float3(sample.vColour.m_v[0], sample.vColour.m_v[1], sample.vColour.m_v[2]);

		switch (sample.target)
		{
		default:
			r = vColour.x;
			g = vColour.y;
			b = vColour.z;
			break;
		case RenderChannel::Red:
			r = (vColour.x + vColour.y + vColour.z) / 3.f;
			break;
		case RenderChannel::Green:
			g = (vColour.x + vColour.y + vColour.z) / 3.f;
			break;
		case RenderChannel::Blue:
			b = (vColour.x + vColour.y + vColour.z) / 3.f;
			break;
		}
	}

	/// force all elements to a value (usually 0)
	Colour operator =(const float f)
	{
		r = g = b = f;
		return *this;
	}

	/// scale an item
	Colour operator *(const float fScale) const
	{
		return Colour(r * fScale, g * fScale, b * fScale);
	}

	/// add two items
	Colour operator +(const Colour &i2) const
	{
		return Colour(r + i2.r, g + i2.g, b + i2.b);
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return 3;
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
		if (r>0)
			r = powf(r, fGamma);
		if (g>0)
			g = powf(g, fGamma);
		if (b>0)
			b = powf(b, fGamma);
	}

	/// get the contents of a particular layer
	void GetData(float*&p, ContextDataType &context)
	{
		p[0] = r;
		p[1] = g;
		p[2] = b;
		p += 3;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, Colour &t)
	{
		ar >> t.r >> t.g >> t.b;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, Colour &t)
	{
		ar << t.r << t.g << t.b;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#endif
