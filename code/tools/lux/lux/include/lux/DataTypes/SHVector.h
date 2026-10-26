#ifndef __LUX_DATATYPES_SH_H__
#define __LUX_DATATYPES_SH_H__

// datatype definition for a monochromatic SH vector of arbitrary order
// This can be used to specialize an iterator, e.g. to render per-vertex SH or SH textures

namespace LUX
{

namespace DataTypes
{

#define COEFF (ORDER*ORDER)

// template argument ORDER containing the SH order
// the number of coefficients is ORDER*ORDER
template<int ORDER>
class SHVector : public BaseType
{
public:
	float v[COEFF];

	/// default constructor
	SHVector()
	{
	}

	/// construct with a float
	SHVector(float f)
	{
		for (int i=0; i<COEFF; i++)
			v[i] = f;
	}

	/// compare all elements to a value (usually 0)
	bool operator ==(float f)
	{
		for (int i=0; i<COEFF; i++)
		{
			if (v[i] != f)
				return false; 
		}
		return true;
	}

	// construct from a polymorphic datatype
	SHVector(const PolymorphicType &data)
	{
		assert(data.m_c == COEFF);
		for (int i=0; i<COEFF; i++)
			v[i] = data.m_v[i];
	}

	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		if (sample.vColour.m_c == 3)
		{
			// turn a colour + direction into a spherical harmonic
			SetSampleFromColourAndDirection(sample.vColour, sample.vDir);
		}
		else
		{
			// take an existing spherical harmonic and simply copy the contents
			assert(sample.vColour.m_c == COEFF);

			for (int i=0; i<sample.vColour.m_c; i++)
				v[i] = sample.vColour.m_v[i];
		}
	}

	/// create an item from a colour + direction
	void SetSampleFromColourAndDirection(const float3 &vColour, const float3 &vDir)
	{
		float fAvg = (vColour.x + vColour.y + vColour.z) * (1.f/3.f);

		*this = SHVector(vDir) * fAvg;
	}

	// construct an SH vector from a direction
	SHVector(float3 vDir)
	{
		float coeff[9];
		SHDirection(&vDir.x, coeff);
		for (int i=0; i<COEFF; i++)
			v[i] = coeff[i] * SH_SCALE_VALUE;
	}

	/// force all elements to a value (usually 0)
	SHVector operator =(float f)
	{
		for (int i=0; i<COEFF; i++)
			v[i] = f;
		return *this;
	}

	/// scale an item
	SHVector operator *(float fScale) const
	{
		SHVector tmp;
		for (int i=0; i<COEFF; i++)
			tmp.v[i] = v[i] * fScale;
		return tmp;
	}

	/// add two items
	SHVector operator +(const SHVector &i2) const
	{
		SHVector tmp;
		for (int i=0; i<COEFF; i++)
			tmp.v[i] = v[i] + i2.v[i];
		return tmp;
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return (COEFF);
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context);

	/// apply a windowing function to reduce ringing artefacts
	void ApplyWindowing(float f);

	/// get the content of this item
	void GetData(float*&p, ContextDataType &context)
	{
		for (int i=0; i<COEFF; i++)
			p[i] = v[i];
		p += COEFF;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, SHVector &t)
	{
		for (int i=0; i<COEFF; i++)
			ar >> t.v[i];
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, SHVector &t)
	{
		for (int i=0; i<COEFF; i++)
			ar << t.v[i];
		return ar;
	}
};

/// returns the dot product of two SH vectors
template <int ORDER>
float DotProduct(const SHVector<ORDER> &v1, const SHVector<ORDER> &v2)
{
	float f = 0;
	for (int i=0; i<COEFF; i++)
		f += v1.v[i] * v2.v[i];
	return f;
}

// TODO: implement gamma correction for any order of SH
template <int ORDER>
void SHVector<ORDER>::ApplyGammaCorrection(float fGamma, ContextDataType &context)
{
}

/// apply a windowing function to reduce ringing artefacts
template <int ORDER>
void SHVector<ORDER>::ApplyWindowing(float f)
{
	float fScale = (float)ORDER * f;

	int x = 0;
	for (int iBand=0; iBand<ORDER; iBand++)
	{
		// calculate scaling for this band
		float fScale = cosf(M_PI/2.f*((float)iBand/f));

		// apply scaling
		int n = (iBand+1)*(iBand+1);
		for (; x<n; x++)
			v[x] *= fScale;
	}

};

}; // namespace DataTypes

}; // namespace LUX

#undef COEFF

#endif
