#ifndef __LUX_DATATYPES_GOTHAMSAMPLEPOINT_H__
#define __LUX_DATATYPES_GOTHAMSAMPLEPOINT_H__

// datatype definition for a SH sample point from PGR4 or The Club
// This can be used to specialize an iterator

namespace LUX
{

namespace DataTypes
{

class GothamSamplePoint : public BaseType
{
public:
	SHVectorRGB<2> sh1,sh2;
	bool m_bDualLayer;
	int m_iSubChannel;
	float m_fHighestSample;

	/// default constructor
	GothamSamplePoint()
	{
	}

	/// constructor
	GothamSamplePoint(float f) :
		sh1(f),
		sh2(f),
		m_fHighestSample(0),
		m_iSubChannel(0),
		m_bDualLayer(false)
	{
	}

	/// set an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		SHVectorRGB<2> sh = 0;
		float f = sample.vColour.m_v[0] + sample.vColour.m_v[1] + sample.vColour.m_v[2];
		if (f <= 0)
			return;	// this is to prevent empty samples from settings the animation subchannel

		switch (sample.target)
		{
		case LUX::RenderChannel::RGB:
			sh.SetSample(sample, mtxTangent, context);
			break;
		case LUX::RenderChannel::Red:
			sh.r.SetSampleFromColourAndDirection(sample.vColour.ConvertToFloat3(), sample.vDir);
			break;
		case LUX::RenderChannel::Green:
			sh.g.SetSampleFromColourAndDirection(sample.vColour.ConvertToFloat3(), sample.vDir);
			break;
		case LUX::RenderChannel::Blue:
			sh.b.SetSampleFromColourAndDirection(sample.vColour.ConvertToFloat3(), sample.vDir);
			break;

		case LUX::RenderChannel::AnimatedOff:
			m_bDualLayer = true;
			if (f > m_fHighestSample)
			{
				m_iSubChannel = sample.iSubChannel;
				m_fHighestSample = f;
			}
			sh1.SetSample(sample, mtxTangent, context);
			return;
		case LUX::RenderChannel::AnimatedOn:
			m_bDualLayer = true;
			if (f > m_fHighestSample)
			{
				m_iSubChannel = sample.iSubChannel;
				m_fHighestSample = f;
			}
			sh2.SetSample(sample, mtxTangent, context);
			return;
		}

		sh1 = sh;
		sh2 = sh;
	}

	/// force all elements to a value (usually 0)
	GothamSamplePoint operator =(float f)
	{
		sh1 = f;
		sh2 = f;
		m_fHighestSample = 0;
		m_iSubChannel = 0;
		m_bDualLayer = false;
		return *this;
	}

	/// compare all elements to a value (usually 0)
	bool operator ==(float f)
	{
		return (sh1 == f && sh2 == f);
	}

	/// scale an item
	GothamSamplePoint operator *(float fScale) const
	{
		GothamSamplePoint tmp;
		tmp.sh1 = sh1 * fScale;
		tmp.sh2 = sh2 * fScale;
		tmp.m_iSubChannel = m_iSubChannel;
		tmp.m_bDualLayer = m_bDualLayer;
		tmp.m_fHighestSample = m_fHighestSample;
		return tmp;
	}

	/// add two items
	GothamSamplePoint operator +(const GothamSamplePoint &i2) const
	{
		GothamSamplePoint tmp;
		tmp.sh1 = sh1 + i2.sh1;
		tmp.sh2 = sh2 + i2.sh2;
		if (m_fHighestSample > i2.m_fHighestSample)
		{
			tmp.m_iSubChannel = m_iSubChannel;
			tmp.m_bDualLayer = m_bDualLayer;
			tmp.m_fHighestSample = m_fHighestSample;
		}
		else
		{
			tmp.m_iSubChannel = i2.m_iSubChannel;
			tmp.m_bDualLayer = i2.m_bDualLayer;
			tmp.m_fHighestSample = i2.m_fHighestSample;
		}
		return tmp;
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return SHVectorRGB<2>::Scalars();
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
//		sh1.ApplyGammaCorrection(fGamma, context);
//		sh2.ApplyGammaCorrection(fGamma, context);
	}

	/// get the content of this item
	void GetData(float*&p, ContextDataType &context)
	{
		sh1.r.ApplyWindowing(2.f);
		sh1.g.ApplyWindowing(2.f);
		sh1.b.ApplyWindowing(2.f);
		*(float4*)&p[0] = PreTransformSHVector(sh1.r.v); p+=4;
		*(float4*)&p[0] = PreTransformSHVector(sh1.g.v); p+=4;
		*(float4*)&p[0] = PreTransformSHVector(sh1.b.v); p+=4;

/*
		sh1.GetData(p, context);
		sh2.GetData(p, context);
		if (m_bDualLayer)
		{
			*p = (int)m_iSubChannel;
		}
		else
		{
			*p = -1.f;
		}
		p++;
*/
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, GothamSamplePoint &t)
	{
		ar >> t.sh1 >> t.sh2;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, GothamSamplePoint &t)
	{
		ar << t.sh1 << t.sh2;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX



#endif
