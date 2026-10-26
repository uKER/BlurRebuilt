#ifndef __LUX_DATATYPES_RGBDIRECTION_H__
#define __LUX_DATATYPES_RGBDIRECTION_H__

// datatype definition for RGB direction 

namespace LUX
{

namespace DataTypes
{

class RGBDirection : public BaseType
{
public:
	Colour m_rgb;
	Direction m_dir;
	Scalar m_sun;
	Scalar m_sky;

	typedef TheClubContextData ContextDataType;

	/// default constructor
	RGBDirection()
	{
	}

	/// constructor
	RGBDirection(const Colour rgb, Direction dir, Scalar sun, Scalar sky) :
		m_rgb(rgb), m_dir(dir), m_sun(sun), m_sky(sky)
	{
	}

	/// constructor
	RGBDirection(float f) :
		m_rgb(f), m_dir(f), m_sun(f), m_sky(f)
	{
	}

	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		if (sample.vColour.m_c == 3)
		{
			// direct lighting
			float3 vColour = sample.vColour;

			if (sample.target == RenderChannel::Sun)
			{
				m_sun.SetSample(sample, mtxTangent, context);
				context.m_fSun += sample.vColour.m_v[0];
				return;
			}
			if (sample.target == RenderChannel::Sky)
			{
				m_sky.SetSample(sample, mtxTangent, context);
				context.m_fSky += sample.vColour.m_v[0];
				return;
			}
			HVec3 _vNormal(sample.vDir.x, sample.vDir.y, sample.vDir.z);
			m_dir = Direction(_vNormal) * (vColour.x + vColour.y + vColour.z);
			m_rgb = vColour;
		}
		else
		{
			// bounce lighting
			assert(sample.vColour.m_c == 4);

			float4 vColour = sample.vColour;
			m_rgb = float3(vColour);

			HVec3 _vNormal(sample.vDir.x, sample.vDir.y, sample.vDir.z);
			m_dir = Direction(_vNormal) * (vColour.x + vColour.y + vColour.z);

			context.m_fDirectional += vColour.x + vColour.y + vColour.z; // paulm - I added this line.

			m_sky = vColour.w; // paulm - I added this line too.
			context.m_fSky += vColour.w;
		}
	}

	/// force all elements to a value (usually 0)
	RGBDirection operator =(const float f)
	{
		m_rgb = f;
		m_dir = f;
		m_sun = f;
		m_sky = f;
		return *this;
	}

	/// scale an item
	RGBDirection operator *(const float fScale) const
	{
		//return RGBDirection(m_rgb * fScale, m_dir, m_sun * fScale, m_sky * fScale);
		return RGBDirection(m_rgb * fScale, m_dir * fScale, m_sun * fScale, m_sky * fScale); // paulm is experimenting
	}

	/// add two items
	RGBDirection operator +(const RGBDirection &i2) const
	{
		return RGBDirection(m_rgb + i2.m_rgb, m_dir + i2.m_dir, m_sun + i2.m_sun, m_sky + i2.m_sky);
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		// 3 textures: 3x4
		return 12;
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
		Colour::ContextDataType dummy;
		m_rgb.ApplyGammaCorrection(fGamma, dummy);
		m_sun.ApplyGammaCorrection(fGamma, dummy);
		m_sky.ApplyGammaCorrection(fGamma, dummy);
	}

	/// get the contents of a particular layer
	void GetData(float*&p, ContextDataType &context)
	{
		float4 v;
		v = GetAlphaScaled(float3(m_rgb.r,m_rgb.g,m_rgb.b));
		p[0] = v.x,	p[1] = v.y,	p[2] = v.z,	p[3] = v.w;
		p += 4;
		p[0] = 0; p[1] = m_sky.f; p[2] = m_sun.f; p[3] = 0;
		p += 4;
		m_dir.GetData(p, context);
		*p = 0;
		p++;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, RGBDirection &t)
	{
		ar >> t.m_rgb >> t.m_dir >> t.m_sun >> t.m_sky;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, RGBDirection &t)
	{
		ar << t.m_rgb << t.m_dir << t.m_sun << t.m_sky;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#endif
