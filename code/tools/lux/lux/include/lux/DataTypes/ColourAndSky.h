#ifndef __LUX_DATATYPES_ColourAndSky_AND_SKY_H__
#define __LUX_DATATYPES_ColourAndSky_AND_SKY_H__

// datatype definition for an RGB ColourAndSky.
// This can be used to specialize an iterator, e.g. to render per-vertex ColourAndSkys or ColourAndSky textures

namespace LUX
{

namespace DataTypes
{

class ColourAndSky : public BaseType
{
public:
	float r,g,b,sky;

	/// default constructor
	ColourAndSky()
	{
	}

	/// constructor
	ColourAndSky(float ir, float ig, float ib, float isky) :
		r(ir), g(ig), b(ib), sky(isky)
	{
	}

	/// constructor
	ColourAndSky(float f) :
		r(f), g(f), b(f), sky(f)		
	{
	}
		
	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		// second bounce sample
		if (sample.vColour.m_c == 4)
		{
			r = sample.vColour.m_v[0];
			g = sample.vColour.m_v[1];
			b = sample.vColour.m_v[2];
			sky = sample.vColour.m_v[3];
			return;
		}

		float3 vColour = sample.vColour;

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
		case RenderChannel::Sky:
		case RenderChannel::Sun:
			sky = (vColour.x + vColour.y + vColour.z) / 3.f;
			break;
		}

	}

	/// force all elements to a value (usually 0)
	ColourAndSky operator =(const float f)
	{
		r = g = b = sky = f;
		return *this;
	}

	/// scale an item
	ColourAndSky operator *(const float fScale) const
	{
		return ColourAndSky(r * fScale, g * fScale, b * fScale, sky * fScale);
	}

	/// add two items
	ColourAndSky operator +(const ColourAndSky &i2) const
	{
		return ColourAndSky(r + i2.r, g + i2.g, b + i2.b, sky + i2.sky);
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return 4;
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
	}

	/// get the contents of a particular layer
	void GetData(float*&p, ContextDataType &context)
	{
		p[0] = r;
		p[1] = g;
		p[2] = b;
		p[3] = sky;
		p += 4;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, ColourAndSky &t)
	{
		ar >> t.r >> t.g >> t.b;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, ColourAndSky &t)
	{
		ar << t.r << t.g << t.b;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#endif
