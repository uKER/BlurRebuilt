#ifndef __LUX_POLYMORPHIC_TYPE_H__
#define __LUX_POLYMORPHIC_TYPE_H__

namespace LUX
{

namespace DataTypes
{

// polymorphic
class PolymorphicType : public BaseType
{
public:
	float *m_v;
	int	m_c;
	float m_buffer[4];

	/// default constructor
	PolymorphicType()
	{
		m_v = NULL;
		m_c = 0;
	}

	/// destructor
	~PolymorphicType()
	{
		if (m_c>4)
			delete m_v;
	}

	/// constructor
	PolymorphicType(float *v, int c)
	{
		Allocate(c);
		if (v)
		{
			for (int i=0; i<m_c; i++)
				m_v[i] = v[i];
		}
	}

	/// force all elements to a value (usually 0)
	PolymorphicType operator =(float f)
	{
		for (int i=0; i<m_c; i++)
			m_v[i] = f;
		return *this;
	}

	/// compare all elements to a value (usually 0)
	bool operator ==(float f)
	{
		for (int i=0; i<m_c; i++)
		{
			if (m_v[i] != f)
				return false;
		}
		return true;
	}

	/// scale an item
	PolymorphicType operator *(float fScale)
	{
		PolymorphicType tmp(NULL, m_c);
		for (int i=0; i<m_c; i++)
			tmp.m_v[i] = m_v[i] * fScale;
		return tmp;
	}

	/// add two items
	PolymorphicType operator +(const PolymorphicType &i2)
	{
		PolymorphicType tmp(NULL, m_c);
		for (int i=0; i<m_c; i++)
			tmp.m_v[i] = m_v[i] + i2.m_v[i];
		return tmp;
	}

	/// return the number of scalars for this datatype
	int Scalars()
	{
		return m_c;
	}
/*
	/// get the content of this item
	void GetData(float*&p)
	{
		for (int i=0; i<m_c; i++)
			p[i] = m_v[i];
		p += m_c;
	}
*/
	/// deserialisation
	friend ost::Engine& operator >> (ost::Engine& ar, PolymorphicType &t)
	{
		if (t.m_c>4)
			delete t.m_v;
		ar >> t.m_c;
		t.Allocate(t.m_c);
		for (int i=0; i<t.m_c; i++)
			ar >> t.m_v[i];
		return ar;
	}

	/// serialisation
	friend ost::Engine& operator << (ost::Engine& ar, PolymorphicType &t)
	{
		ar << t.m_c;
		for (int i=0; i<t.m_c; i++)
			ar << t.m_v[i];
		return ar;
	}

	/// copy constructor
	PolymorphicType(const PolymorphicType &t)
	{
		Allocate(t.m_c);
		memcpy(m_v, t.m_v, sizeof(float)*m_c);
	}

	/// assignment operator
	PolymorphicType &operator=( const PolymorphicType &t )
	{
		if (m_c>4)
			delete m_v;
		Allocate(t.m_c);
		for (int i=0; i<m_c; i++)
			m_v[i] = t.m_v[i];
		return *this;
	}

	/// allocate buffer to store given datasize
	void Allocate(int c)
	{
		m_c = c;
		if (c>4)
			m_v = new float[m_c];
		else
			m_v = m_buffer;
	}

	/// automatically construct from a type
	template<typename T>
	PolymorphicType(const T &t)
	{
		Allocate(sizeof(T) / sizeof(float));
		memcpy(m_v, (void*)&t, sizeof(float)*m_c);
	}

	// automatically convert to a float3
	operator float3() const
	{
		assert(m_c == 3);
		return float3(m_v[0], m_v[1], m_v[2]);
	}

	float3 ConvertToFloat3() const
	{
		assert(m_c >= 3);
		return float3(m_v[0], m_v[1], m_v[2]);
	}

	// automatically convert to a float4
	operator float4() const
	{
		assert(m_c == 4);
		return float4(m_v[0], m_v[1], m_v[2], m_v[3]);
	}
};


}

}

#endif
