#ifndef __LUX_DATATYPES_DIRECTION_H__
#define __LUX_DATATYPES_DIRECTION_H__

// datatype definition for a monochromatic SH vector of arbitrary order
// This can be used to specialize an iterator, e.g. to render per-vertex SH or SH textures

namespace LUX
{

namespace DataTypes
{

#define COEFF (ORDER*ORDER)

// collect the average direction of incoming light
class Direction : public BaseType
{
public:
	float3 v;

	/// default constructor
	Direction()
	{
	}

	/// construct with a float
	Direction(float f)
	{
		for (int i=0; i<3; i++)
			v[i] = f;
	}

	/// compare all elements to a value (usually 0)
	bool operator ==(float f)
	{
		for (int i=0; i<3; i++)
		{
			if (v[i] != f)
				return false; 
		}
		return true;
	}

	// construct from a polymorphic datatype
	Direction(const PolymorphicType &data)
	{
		assert(data.m_c == 3);
		for (int i=0; i<3; i++)
			v[i] = data.m_v[i];
	}

	/// create an item from a sample
	void SetSample(const LightSample &sample, const HMat33 &mtxTangent, ContextDataType &context)
	{
		v = sample.vDir;
	}

	/// create an item from a colour + direction
	void SetSampleFromColourAndDirection(const float3 &vColour, const float3 &vDir)
	{
		v = vDir;
	}

	// construct an SH vector from a direction
	Direction(float3 vDir)
	{
		v = vDir;
	}

	/// force all elements to a value (usually 0)
	Direction operator =(float f)
	{
		for (int i=0; i<3; i++)
			v[i] = f;
		return *this;
	}

	/// scale an item
	Direction operator *(float fScale) const
	{
		Direction tmp;
		tmp.v = v * fScale;
		return tmp;
	}

	/// add two items
	Direction operator +(const Direction &i2) const
	{
		Direction tmp;
		tmp.v = v + i2.v;
		return tmp;
	}

	/// return the number of scalars for this datatype
	static int Scalars()
	{
		return (3);
	}

	/// get the content of this item
	void GetData(float*&p, ContextDataType &context)
	{
		// normalize
		float3 vDir = Normalize(v);
		// scale
		p[0] = 0.5f+(vDir.x*0.5f);
		p[1] = 0.5f+(vDir.y*0.5f);
		p[2] = 0.5f+(vDir.z*0.5f);
		p += 3;
	}

	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, Direction &t)
	{
		for (int i=0; i<3; i++)
			ar >> t.v[i];
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, Direction &t)
	{
		for (int i=0; i<3; i++)
			ar << t.v[i];
		return ar;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#undef COEFF

#endif
