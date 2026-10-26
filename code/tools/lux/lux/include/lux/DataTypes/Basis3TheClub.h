#ifndef __LUX_DATATYPES_BASIS3THECLUB_H__
#define __LUX_DATATYPES_BASIS3THECLUB_H__

// datatype definition for the lightmap format for The Club

namespace LUX
{

namespace DataTypes
{

class Basis3TheClub : public BaseType
{
public:
	Basis3 m_r, m_g, m_b;
	Scalar m_sun;
	Basis3 m_sky;

	typedef TheClubContextData ContextDataType;

	/// default constructor
	Basis3TheClub()
	{
	}

	/// construct with a float
	Basis3TheClub(float f)
	{
		m_r = f;
		m_g = f;
		m_b = f;
		m_sun = f;
		m_sky = f;
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
			HVec3 _vNormal(sample.vDir.x, sample.vDir.y, sample.vDir.z);
			_vNormal = _vNormal * mtxTangent;
			Basis3 basis = Basis3(_vNormal);

			switch (sample.target)
			{
			default:
				m_r = basis * vColour.x;
				m_g = basis * vColour.y;
				m_b = basis * vColour.z;
				context.m_fDirectional += vColour.x + vColour.y + vColour.z;
				break;
			case RenderChannel::Sky:
				m_sky = basis * vColour.y;
				context.m_fSky += vColour.y;
				break;
			}
		}
		else
		{
			// bounce lighting
			assert(sample.vColour.m_c == 4);

			float4 vColour = sample.vColour;

			HVec3 _vNormal(sample.vDir.x, sample.vDir.y, sample.vDir.z);
			_vNormal = _vNormal * mtxTangent;
			Basis3 basis = Basis3(_vNormal);

			m_r = basis * vColour.x;
			m_g = basis * vColour.y;
			m_b = basis * vColour.z;
			m_sky = basis * vColour.w;

			context.m_fDirectional += vColour.x + vColour.y + vColour.z;
			context.m_fSky += vColour.w;
		}
	}

	/// force all elements to a value (usually 0)
	Basis3TheClub operator =(float f)
	{
		*this = Basis3TheClub(f);
		return *this;
	}

	/// scale an item
	Basis3TheClub operator *(float fScale) const
	{
		Basis3TheClub tmp;
		tmp.m_r = m_r * fScale;
		tmp.m_g = m_g * fScale;
		tmp.m_b = m_b * fScale;
		tmp.m_sky = m_sky * fScale;
		tmp.m_sun = m_sun * fScale;
		return tmp;
	}

	/// add two items
	Basis3TheClub operator +(const Basis3TheClub &i2) const
	{
		Basis3TheClub tmp;
		tmp.m_r = m_r + i2.m_r;
		tmp.m_g = m_g + i2.m_g;
		tmp.m_b = m_b + i2.m_b;
		tmp.m_sun = m_sun + i2.m_sun;
		tmp.m_sky = m_sky + i2.m_sky;
		return tmp;
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return 16;
	}

	/// apply gamma correction to this item - NA
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
		context.m_fGamma = fGamma;
	}


	/// get the content of this item
	void GetData(float*&p, ContextDataType &context)
	{
		float fGamma = context.m_fGamma;

		float3 sky =
			Clamp(float3(m_sky.v[0], m_sky.v[1], m_sky.v[2]));

#ifndef SWIZZLED_BASIS
		// apply alpha scaling, collect data into quadrants
		float4 v0 = GetAlphaScaled( LinearClamp( float3( m_r.v[0], m_g.v[0], m_b.v[0] ) ) );
		float4 v1 = GetAlphaScaled( LinearClamp( float3( m_r.v[1], m_g.v[1], m_b.v[1] ) ) );
		float4 v2 = GetAlphaScaled( LinearClamp( float3( m_r.v[2], m_g.v[2], m_b.v[2] ) ) );
		float4 vx(sky.x, sky.y, sky.z, m_sun.f);
#else
		// apply alpha scaling, collect data into quadrants -- swizzled
		float4 v0 = GetAlphaScaled( ( float3( m_r.v[0], m_r.v[1], m_r.v[2] ) ) );
		float4 v1 = GetAlphaScaled( ( float3( m_g.v[0], m_g.v[1], m_g.v[2] ) ) );
		float4 v2 = GetAlphaScaled( ( float3( m_b.v[0], m_b.v[1], m_b.v[2] ) ) );
		float4 vx(sky.x, sky.y, sky.z, m_sun.f);
#endif

		// apply gamma to all alpha components of r,g,b
		if (v0.w > 0) v0.w = powf(v0.w, fGamma);
		if (v1.w > 0) v1.w = powf(v1.w, fGamma);
		if (v2.w > 0) v2.w = powf(v2.w, fGamma);
		// apply gamma curve to vx
		if (vx.x > 0) vx.x = powf(vx.x, fGamma);
		if (vx.y > 0) vx.y = powf(vx.y, fGamma);
		if (vx.z > 0) vx.z = powf(vx.z, fGamma);
		if (vx.w > 0) vx.w = powf(vx.w, fGamma);

		// save data
		p[0] = v0.x, p[1] = v0.y, p[2] = v0.z, p[3] = v0.w;	p+=4;
		p[0] = v1.x, p[1] = v1.y, p[2] = v1.z, p[3] = v1.w;	p+=4;
		p[0] = v2.x, p[1] = v2.y, p[2] = v2.z, p[3] = v2.w;	p+=4;
		p[0] = vx.x, p[1] = vx.y, p[2] = vx.z, p[3] = vx.w;	p+=4;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, Basis3TheClub &t)
	{
		ar >> t.m_r >> t.m_g >> t.m_b >> t.m_sun >> t.m_sky;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, Basis3TheClub &t)
	{
		ar << t.m_r << t.m_g << t.m_b << t.m_sun << t.m_sky;
		return ar;
	}

protected:
	const static float3 m_vAxis[3];
};

// specialize this template to return 'true' for any datatypes that need tangent space calculating
template<>
inline bool NeedTangentSpace<Basis3TheClub>()
{
	return true;
}

}; // namespace DataTypes

}; // namespace LUX

#undef COEFF

#endif
