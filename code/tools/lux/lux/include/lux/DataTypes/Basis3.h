#ifndef __LUX_DATATYPES_BASIS3_H__
#define __LUX_DATATYPES_BASIS3_H__

// datatype definition for a single monochromatic Basis3 vector (like in Half-Life)
// This can be used to specialize an iterator, e.g. to render per-vertex Basis3 or Basis3 textures

namespace LUX
{

namespace DataTypes
{

class Basis3 : public BaseType
{
public:
	float v[3];

	/// default constructor
	Basis3()
	{
	}

	/// construct with a float
	Basis3(float f)
	{
		for (int i=0; i<3; i++)
			v[i] = f;
	}

	// construct from a polymorphic datatype
	Basis3(const PolymorphicType &data)
	{
		assert(data.m_c == 3);
		for (int i=0; i<3; i++)
			v[i] = data.m_v[i];
	}

	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		// turn a colour + direction into a basis3 vector
		float3 vColour = sample.vColour;

		float fAvg = (vColour.x + vColour.y + vColour.z) * (1.f/3.f);

		*this = Basis3(sample.vDir) * fAvg;
	}

	// construct a basis3 vector from a direction
	Basis3(float3 vDir)
	{
		v[0] = DotProduct(vDir, m_vAxis[0]);
		v[1] = DotProduct(vDir, m_vAxis[1]);
		v[2] = DotProduct(vDir, m_vAxis[2]);
	}

	/// force all elements to a value (usually 0)
	Basis3 operator =(float f)
	{
		for (int i=0; i<3; i++)
			v[i] = f;
		return *this;
	}

	/// scale an item
	Basis3 operator *(float fScale) const
	{
		Basis3 tmp;
		for (int i=0; i<3; i++)
			tmp.v[i] = v[i] * fScale;
		return tmp;
	}

	/// add two items
	Basis3 operator +(const Basis3 &i2) const
	{
		Basis3 tmp;
		for (int i=0; i<3; i++)
			tmp.v[i] = v[i] + i2.v[i];
		return tmp;
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return (3);
	}

	/// apply gamma correction to this item - NA
	void ApplyGammaCorrection(float fGamma, ContextDataType &context)
	{
	}

	/// get the content of this item
	void GetData(float*&p, ContextDataType &context)
	{
		for (int i=0; i<3; i++)
			p[i] = v[i];
		p += 3;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, Basis3 &t)
	{
		for (int i=0; i<3; i++)
			ar >> t.v[i];
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, Basis3 &t)
	{
		for (int i=0; i<3; i++)
			ar << t.v[i];
		return ar;
	}

protected:
	const static float3 m_vAxis[3];
};

}; // namespace DataTypes

}; // namespace LUX

#undef COEFF

#endif
