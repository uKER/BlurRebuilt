#ifndef __LUX_DATATYPES_SH01_H__
#define __LUX_DATATYPES_SH01_H__

// same as a normal SH vector, except that all components are scaled to be [0,1] instead of [-1,1]
// this is for use with outputs that can only output positive values (e.g. U8 textures or RGBA32 vertex colours)

namespace LUX
{

namespace DataTypes
{

#define COEFF (ORDER*ORDER)

// template argument ORDER containing the SH order
// the number of coefficients is ORDER*ORDER
template<int ORDER>
class SHVector01 : public SHVector<ORDER>
{
public:
	typedef Void ContextDataType;

	/// constructor
	SHVector01() :
	  SHVector<ORDER>::SHVector()
	{
	}

	/// constructor
	SHVector01(float f) :
		SHVector<ORDER>::SHVector(f)
	{
	}

	/// scale an item
	SHVector01 operator *(float fScale) const
	{
		SHVector01 tmp;
		for (int i=0; i<COEFF; i++)
			tmp.v[i] = SHVector<ORDER>::v[i] * fScale;
		return tmp;
	}

	/// add two items
	SHVector01 operator +(const SHVector01 &i2) const
	{
		SHVector01 tmp;
		for (int i=0; i<COEFF; i++)
			tmp.v[i] = SHVector<ORDER>::v[i] + i2.v[i];
		return tmp;
	}

	/// get the content of this item
	void GetData(float*&p, ContextDataType &context)
	{
		p[0] = SHVector<ORDER>::v[0];

		for (int i=1; i<COEFF; i++)
			p[i] = (SHVector<ORDER>::v[i] * 0.5f) + 0.5f;
		p += COEFF;
	}
};

}; // namespace DataTypes

}; // namespace LUX

#undef COEFF

#endif
