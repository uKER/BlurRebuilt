#ifndef __LUX_DATATYPES_SHRGB_H__
#define __LUX_DATATYPES_SHRGB_H__

// datatype definition for an RGB SH vector of arbitrary order
// This can be used to specialize an iterator, e.g. to render per-vertex SH or SH textures

namespace LUX
{

namespace DataTypes
{

#define COEFF (ORDER*ORDER)

// template argument ORDER containing the SH order
// the number of coefficients is ORDER*ORDER
template<int ORDER>
class SHVectorRGB : public BaseType
{
public:
	SHVector<ORDER> r,g,b;

	/// default constructor
	SHVectorRGB()
	{
	}

	/// constructor
	SHVectorRGB(float f) :
		r(f), g(f), b(f)
	{
	}

	/// constructor
	SHVectorRGB(float *pR, float *pG, float *pB) :
		r(pR), g(pG), b(pB)
	{
	}

	/// compare all elements to a value (usually 0)
	bool operator ==(float f)
	{
		return (r == f && g == f && b == f);
	}

	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		float coeff[9];
		assert(sample.vColour.m_c == 3 ||sample.vColour.m_c == 4);
		float3 vColour( sample.vColour.m_v[0], sample.vColour.m_v[1], sample.vColour.m_v[2] );

		SHDirection(&sample.vDir.x, coeff);
		for (int i=0; i<COEFF; i++)
			r.v[i] = coeff[i] * SH_SCALE_VALUE * vColour.x;
		for (int i=0; i<COEFF; i++)
			g.v[i] = coeff[i] * SH_SCALE_VALUE * vColour.y;
		for (int i=0; i<COEFF; i++)
			b.v[i] = coeff[i] * SH_SCALE_VALUE * vColour.z;
	}

	/// force all elements to a value (usually 0)
	SHVectorRGB operator =(float f)
	{
		r = f;
		g = f;
		b = f;
		return *this;
	}

	/// scale an item
	SHVectorRGB operator *(float fScale) const
	{
		SHVectorRGB tmp;
		tmp.r = r * fScale;
		tmp.g = g * fScale;
		tmp.b = b * fScale;
		return tmp;
	}

	/// add two items
	SHVectorRGB operator +(const SHVectorRGB &i2) const
	{
		SHVectorRGB tmp;
		tmp.r = r + i2.r;
		tmp.g = g + i2.g;
		tmp.b = b + i2.b;
		return tmp;
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return SHVector<ORDER>::Scalars() * 3;
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
		r.ApplyGammaCorrection(fGamma, context);
		g.ApplyGammaCorrection(fGamma, context);
		b.ApplyGammaCorrection(fGamma, context);
	}

	/// apply windowing function
	void ApplyWindowing(float f)
	{
		r.ApplyWindowing(f);
		g.ApplyWindowing(f);
		b.ApplyWindowing(f);
	}

	/// get the contents of a particular layer
	void GetData(float*&p, ContextDataType &context)
	{
		r.GetData(p, context);
		g.GetData(p, context);
		b.GetData(p, context);
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, SHVectorRGB &t)
	{
		ar >> t.r >> t.g >> t.b;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, SHVectorRGB &t)
	{
		ar << t.r << t.g << t.b;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#undef COEFF

#endif
