#ifndef __LUX_DATATYPES_SCALAR_H__
#define __LUX_DATATYPES_SCALAR_H__

// datatype definition for a single Scalar.
// This can be used to specialize an iterator, e.g. to render per-vertex light intensity or single-channel textures

namespace LUX
{

namespace DataTypes
{

class Scalar : public BaseType
{
public:
	float f;

	/// default constructor
	Scalar()
	{
	}

	/// constructor
	Scalar(float _f) :
		f(_f)
	{
	}

	/// compare all elements to a value (usually 0)
	bool operator ==(float _f)
	{
		return (f == _f);
	}

	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		float3 vColour = sample.vColour;

		f = (vColour.x + vColour.y + vColour.z) * (1.f/3.f);
	}

	/// force all elements to a value (usually 0)
	Scalar operator =(float _f)
	{
		f = _f;
		return *this;
	}

	/// scale an item
	Scalar operator *(float fScale) const
	{
		return Scalar(f * fScale);
	}

	/// add two items
	Scalar operator +(const Scalar &i2) const
	{
		return Scalar(f + i2.f);
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return 1;
	}

	/// apply gamma correction to this item
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
		if (f>0)
			f = powf(f, fGamma);
	}

	/// get the contents of a particular layer
	void GetData(float*&p, ContextDataType &context)
	{
		p[0] = f;
		p++;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, Scalar &t)
	{
		ar >> t.f;
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, Scalar &t)
	{
		ar << t.f;
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#endif
