#ifndef __LUX_DATATYPES_COLOURALPHASCALE_H__
#define __LUX_DATATYPES_COLOURALPHASCALE_H__

// datatype definition for an RGB ColourAlphaScale.
// This can be used to specialize an iterator, e.g. to render per-vertex ColourAlphaScales or ColourAlphaScale textures

namespace LUX
{

namespace DataTypes
{

class ColourAlphaScale : public BaseType
{
public:
	float r,g,b;

	/// default constructor
	ColourAlphaScale()
	{
	}

	/// constructor
	ColourAlphaScale(float ir, float ig, float ib) :
		r(ir), g(ig), b(ib)		
	{
	}

	/// constructor
	ColourAlphaScale(float f) :
		r(f), g(f), b(f)		
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
	ColourAlphaScale operator =(const float f)
	{
		r = g = b = f;
		return *this;
	}

	/// scale an item
	ColourAlphaScale operator *(const float fScale) const
	{
		return ColourAlphaScale(r * fScale, g * fScale, b * fScale);
	}

	/// add two items
	ColourAlphaScale operator +(const ColourAlphaScale &i2) const
	{
		return ColourAlphaScale(r + i2.r, g + i2.g, b + i2.b);
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		// 3 components + alpha scaling
		return 4;
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
		float4 v = GetAlphaScaled(float3(r,g,b));

		// output
		p[0] = v.x;
		p[1] = v.y;
		p[2] = v.z;
		p[3] = v.w;
		p += 4;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, ColourAlphaScale &t)
	{
		ar >> t.r >> t.g >> t.b;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, ColourAlphaScale &t)
	{
		ar << t.r << t.g << t.b;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#endif
