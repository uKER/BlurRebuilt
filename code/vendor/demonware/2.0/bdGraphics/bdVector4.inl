// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline
bdVector4::bdVector4()
{
	m_v[3] = 0;
}

inline
bdVector4::bdVector4(const bdFloat32 x,
					 const bdFloat32 y,
					 const bdFloat32 z,
					 const bdFloat32 w)
{
	m_v[0] = x;
	m_v[1] = y;
	m_v[2] = z;
	m_v[3] = w;
}

inline
bdVector4::bdVector4(bdFloat32 *const v)
{
	m_v[0] = v[0];
	m_v[1] = v[1];
	m_v[2] = v[2];
	m_v[3] = v[3];
}

inline
void bdVector4::operator= (const bdVector4 &other)
{
	m_v[0] = other.m_v[0];
	m_v[1] = other.m_v[1];
	m_v[2] = other.m_v[2];
	m_v[3] = other.m_v[3];
}

inline
bdFloat32& bdVector4::operator[] (const bdUInt32 i)
{
	BD_ASSERT(i < 4, "bdVector4::operator[] index is out of range.");

	return m_v[i];
}

inline 
const bdFloat32& bdVector4::operator[] (const bdUInt32 i) const
{
	BD_ASSERT(i < 4, "bdVector4::operator[] index is out of range.");

	return m_v[i];
}

inline bdVector4 bdVector4::operator- ()
{
	const bdVector4 v(-m_v[0], -m_v[1], -m_v[2], -m_v[3]);
	return v;
}

inline
const bdFloat32* bdVector4::getData() const
{
	return m_v;
}

inline
void bdVector4::set(bdFloat32 *const v)
{
	m_v[0] = v[0];
	m_v[1] = v[1];
	m_v[2] = v[2];
	m_v[3] = v[3];
}

inline
void bdVector4::set(const bdUInt32 i,
					const bdFloat32 v)
{
	BD_ASSERT(i < 4, "bdVector4::set index is out of range.");
	m_v[i] = v;
}

inline
void bdVector4::set(const bdFloat32 x,
					const bdFloat32 y,
					const bdFloat32 z,
					const bdFloat32 w)
{
	m_v[0] = x;
	m_v[1] = y;
	m_v[2] = z;
	m_v[3] = w;
}


inline
void bdVector4::setX(const bdFloat32 x)
{
	m_v[0] = x;
}

inline
void bdVector4::setY(const bdFloat32 y)
{
	m_v[1] = y;
}

inline
void bdVector4::setZ(const bdFloat32 z)
{
	m_v[2] = z;
}

inline
void bdVector4::setW(const bdFloat32 w)
{
	m_v[3] = w;
}

inline
void bdVector4::get(bdFloat32 *const v) const
{
	v[0] = m_v[0];
	v[1] = m_v[1];
	v[2] = m_v[2];
	v[3] = m_v[3];
}

inline
bdFloat32 bdVector4::get(const bdUInt32 i) const
{
	BD_ASSERT(i < 4, "bdVector4::get index is out of range.");
	return m_v[i];
}

inline
bdFloat32 bdVector4::getX() const
{
	return m_v[0];
}

inline
bdFloat32 bdVector4::getY() const
{
	return m_v[1];
}

inline
bdFloat32 bdVector4::getZ() const
{
	return m_v[2];
}

inline
bdFloat32 bdVector4::getW() const
{
	return m_v[3];
}

inline
void bdVector4::zero()
{
	m_v[0] = 0;
	m_v[1] = 0;
	m_v[2] = 0;
	m_v[3] = 0;
}

inline
void bdVector4::add4(const bdVector4 &v)
{
	m_v[0] += v.m_v[0];
	m_v[1] += v.m_v[1];
	m_v[2] += v.m_v[2];
	m_v[3] += v.m_v[3];
}

inline
void bdVector4::add3(const bdVector4 &v)
{
	m_v[0] += v.m_v[0];
	m_v[1] += v.m_v[1];
	m_v[2] += v.m_v[2];
}

inline
void bdVector4::add4(const bdVector4 &v0,
					 const bdVector4 &v1)
{
	m_v[0] = v0.m_v[0] + v1.m_v[0];
	m_v[1] = v0.m_v[1] + v1.m_v[1];
	m_v[2] = v0.m_v[2] + v1.m_v[2];
	m_v[3] = v0.m_v[3] + v1.m_v[3];
}

inline
void bdVector4::add3(const bdVector4 &v0,
					 const bdVector4 &v1)
{
	m_v[0] = v0.m_v[0] + v1.m_v[0];
	m_v[1] = v0.m_v[1] + v1.m_v[1];
	m_v[2] = v0.m_v[2] + v1.m_v[2];
}

inline
void bdVector4::subtract4(const bdVector4 &v)
{
	m_v[0] -= v.m_v[0];
	m_v[1] -= v.m_v[1];
	m_v[2] -= v.m_v[2];
	m_v[3] -= v.m_v[3];
}

inline
void bdVector4::subtract3(const bdVector4 &v)
{
	m_v[0] -= v.m_v[0];
	m_v[1] -= v.m_v[1];
	m_v[2] -= v.m_v[2];
}

inline
void bdVector4::subtract4(const bdVector4 &v0,
						  const bdVector4 &v1)
{
	m_v[0] = v0.m_v[0] - v1.m_v[0];
	m_v[1] = v0.m_v[1] - v1.m_v[1];
	m_v[2] = v0.m_v[2] - v1.m_v[2];
	m_v[3] = v0.m_v[3] - v1.m_v[3];
}

inline
void bdVector4::subtract3(const bdVector4 &v0,
						  const bdVector4 &v1)
{
	m_v[0] = v0.m_v[0] - v1.m_v[0];
	m_v[1] = v0.m_v[1] - v1.m_v[1];
	m_v[2] = v0.m_v[2] - v1.m_v[2];
}

inline
void bdVector4::multiply3(const bdFloat32 s)
{
	m_v[0] *= s;
	m_v[1] *= s;
	m_v[2] *= s;
}


inline
void bdVector4::multiply3(const bdVector4 &s)
{
	m_v[0] *= s.m_v[0];
	m_v[1] *= s.m_v[1];
	m_v[2] *= s.m_v[2];
}

inline
bdFloat32 bdVector4::getLengthSq() const
{
	return (m_v[0] * m_v[0]) + (m_v[1] * m_v[1]) + (m_v[2] * m_v[2]) + (m_v[3] * m_v[3]);
}


inline
bdFloat32 bdVector4::getLength() const
{
	const bdFloat32 lengthSq = getLengthSq();
	const bdFloat32 length = bdSqrtf32(lengthSq);
	return length;
}


inline
void bdVector4::cross3(const bdVector4 &a,
					   const bdVector4 &b)
{
	if ((&a != this) && (&b != this))
	{
		m_v[0] = (a.m_v[1] * b.m_v[2]) - (a.m_v[2] * b.m_v[1]);
		m_v[1] = (a.m_v[2] * b.m_v[0]) - (a.m_v[0] * b.m_v[2]);
		m_v[2] = (a.m_v[0] * b.m_v[1]) - (a.m_v[1] * b.m_v[0]);
	}
	else
	{
		bdFloat32 temp[3];
		temp[0] = (a.m_v[1] * b.m_v[2]) - (a.m_v[2] * b.m_v[1]);
		temp[1] = (a.m_v[2] * b.m_v[0]) - (a.m_v[0] * b.m_v[2]);
		temp[2] = (a.m_v[0] * b.m_v[1]) - (a.m_v[1] * b.m_v[0]);

		m_v[0] = temp[0];
		m_v[1] = temp[1];
		m_v[2] = temp[2];
	}
}

inline
bdFloat32 bdVector4::dot3(const bdVector4 &a)
{
	const bdFloat32 dot = (m_v[0] * a.m_v[0]) +
						  (m_v[1] * a.m_v[1]) +
						  (m_v[2] * a.m_v[2]);
	return dot;
}

inline
void bdVector4::normalize()
{
	const bdFloat32 length = getLength();
	if(length != 0)
	{
		m_v[0] = m_v[0] / length;
		m_v[1] = m_v[1] / length;
		m_v[2] = m_v[2] / length;
		m_v[3] = m_v[3] / length;
	}
	else
	{
		m_v[0] = 1;
		m_v[1] = 0;
		m_v[2] = 0;
		m_v[3] = 0;
		BD_WARN("vector4", "vector with length ZERO was normalised, this is potentially a bad usage of a vector");
	}
}

inline
void bdVector4::postMatrixMultiply(bdMatrix4 &s)
{
	bdVector4 copy(*this);
	bdFloat32 temp;
	for (bdUInt i = 0; i < 4; i++)
	{
		temp = 0;
		for (bdUInt j = 0; j < 4; j++)
		{
			temp = temp + s[i+4*j] * copy[j];
		}
        m_v[i] = temp;
	}
}

