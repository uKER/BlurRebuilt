#ifndef __LUX_DATATYPES_SAMPLEPOINT_H__
#define __LUX_DATATYPES_SAMPLEPOINT_H__

// datatype definition for a SH sample point from PGR4 or The Club
// This can be used to specialize an iterator

namespace LUX
{

namespace DataTypes
{

class SamplePoint : public BaseType
{
public:
	SHVectorRGB<2> sh;
	Scalar fSun, fSky;

	/// default constructor
	SamplePoint()
	{
	}

	/// constructor
	SamplePoint(float f) :
		sh(f),
		fSun(f),
		fSky(f)
	{
	}

	/// compare all elements to a value (usually 0)
	bool operator ==(float f)
	{
		return (sh == f && fSun == f && fSky == f);
	}

	/// set an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		if (sample.vColour.m_c == 4)
		{
			// add bounce lighting into RGB + sky component, same as lightmaps
			sh.SetSample(sample, mtxTangent, context);
			fSky.f = sample.vColour.m_v[3];
			return;
		}

		// add direct lighting
		switch (sample.target)
		{
		case LUX::RenderChannel::RGB:
			sh.SetSample(sample, mtxTangent, context); break;
		case LUX::RenderChannel::Red:
			sh.r.SetSample(sample, mtxTangent, context); break;
		case LUX::RenderChannel::Green:
			sh.g.SetSample(sample, mtxTangent, context); break;
		case LUX::RenderChannel::Blue:
			sh.b.SetSample(sample, mtxTangent, context); break;
		case LUX::RenderChannel::Sky:
			fSky.SetSample(sample, mtxTangent, context); break;
		case LUX::RenderChannel::Sun:
			fSun.SetSample(sample, mtxTangent, context); break;
		}
	}

	/// force all elements to a value (usually 0)
	SamplePoint operator =(float f)
	{
		sh = f;
		fSky = f;
		fSun = f;
		return *this;
	}

	/// scale an item
	SamplePoint operator *(float fScale) const
	{
		SamplePoint tmp;
		tmp.sh = sh * fScale;
		tmp.fSky = fSky * fScale;
		tmp.fSun = fSun * fScale;
		return tmp;
	}

	/// add two items
	SamplePoint operator +(const SamplePoint &i2) const
	{
		SamplePoint tmp;
		tmp.sh = sh + i2.sh;
		tmp.fSky = fSky + i2.fSky;
		tmp.fSun = fSun + i2.fSun;
		return tmp;
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return SHVectorRGB<2>::Scalars() + Scalar::Scalars() + Scalar::Scalars();
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
		//sh.ApplyGammaCorrection(fGamma, context);
		//fSky.ApplyGammaCorrection(fGamma, context);
		//fSun.ApplyGammaCorrection(fGamma, context);
	}

	/// get the content of this item
	void GetData(float*&p, ContextDataType &context)
	{
		sh.r.ApplyWindowing(2.f);
		sh.g.ApplyWindowing(2.f);
		sh.b.ApplyWindowing(2.f);

		*(float4*)&p[0] = PreTransformSHVector(sh.r.v); p+=4;
		*(float4*)&p[0] = PreTransformSHVector(sh.g.v); p+=4;
		*(float4*)&p[0] = PreTransformSHVector(sh.b.v); p+=4;
		fSky.f = fSky.f / 1.36f;
		fSun.GetData(p, context);
		fSky.GetData(p, context);
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, SamplePoint &t)
	{
		ar >> t.sh >> t.fSky >> t.fSun;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, SamplePoint &t)
	{
		ar << t.sh << t.fSky << t.fSun;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX



#endif
