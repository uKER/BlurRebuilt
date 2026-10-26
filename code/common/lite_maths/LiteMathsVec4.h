#ifndef LITE_MATHS_VEC_4_H
#define LITE_MATHS_VEC_4_H

namespace LiteMaths
{

	template <class T>
	class Vec4
	{
	public:
		enum Axis
		{
			VX,
			VY,
			VZ,
			VW
		};

		LITE_MATHS_INLINE Vec4()
		{
		}

		LITE_MATHS_INLINE Vec4(const Vec4& rhs)
		{
			*this = rhs;
		}

		LITE_MATHS_INLINE Vec4(T x, T y, T z, T w)
		{
			m_data[VX] = x;
			m_data[VY] = y;
			m_data[VZ] = z;
			m_data[VW] = w;
		}

		const Vec4& operator = (const Vec4& rhs)
		{
			memcpy(m_data, rhs.m_data, sizeof(m_data));
			return *this;
		}

		LITE_MATHS_INLINE bool operator < (const Vec4& rhs) const
		{
			if (lhs.X() < rhs.X())
			{
				return true;
			}
			if (lhs.Y() < rhs.Y())
			{
				return true;
			}
			if (lhs.Z() < rhs.Z())
			{
				return true;
			}
			if (lhs.W() < rhs.W())
			{
				return true;
			}
			return false;
		}

		LITE_MATHS_INLINE void operator += (const Vec4& rhs)
		{
			m_data[VX] += rhs.m_data[VX];
			m_data[VY] += rhs.m_data[VY];
			m_data[VZ] += rhs.m_data[VZ];
			m_data[VW] += rhs.m_data[VW];
		}

		LITE_MATHS_INLINE void operator -= (const Vec4& rhs)
		{
			m_data[VX] -= rhs.m_data[VX];
			m_data[VY] -= rhs.m_data[VY];
			m_data[VZ] -= rhs.m_data[VZ];
			m_data[VW] -= rhs.m_data[VW];
		}

		LITE_MATHS_INLINE void operator *= (const Vec4& rhs)
		{
			m_data[VX] *= rhs.m_data[VX];
			m_data[VY] *= rhs.m_data[VY];
			m_data[VZ] *= rhs.m_data[VZ];
			m_data[VW] *= rhs.m_data[VW];
		}

		LITE_MATHS_INLINE void operator *= (const T rhs)
		{
			m_data[VX] *= rhs;
			m_data[VY] *= rhs;
			m_data[VZ] *= rhs;
			m_data[VW] *= rhs;
		}

		LITE_MATHS_INLINE const Vec4 operator + (const Vec4& rhs) const
		{
			return Vec4(
				m_data[VX] + rhs.m_data[VX],
				m_data[VY] + rhs.m_data[VY],
				m_data[VZ] + rhs.m_data[VZ],
				m_data[VW] + rhs.m_data[VW]);
		}

		LITE_MATHS_INLINE const Vec4 operator - (const Vec4& rhs) const
		{
			return Vec4(
				m_data[VX] - rhs.m_data[VX],
				m_data[VY] - rhs.m_data[VY],
				m_data[VZ] - rhs.m_data[VZ],
				m_data[VW] - rhs.m_data[VW]);
		}

		LITE_MATHS_INLINE const Vec4 operator * (const Vec4& rhs) const
		{
			return Vec4(
				m_data[VX] * rhs.m_data[VX],
				m_data[VY] * rhs.m_data[VY],
				m_data[VZ] * rhs.m_data[VZ],
				m_data[VW] * rhs.m_data[VW]);
		}

		LITE_MATHS_INLINE const Vec4 operator * (T rhs) const
		{
			return Vec4(
				m_data[VX] * rhs,
				m_data[VY] * rhs,
				m_data[VZ] * rhs,
				m_data[VW] * rhs);
		}

		LITE_MATHS_INLINE T X() const
		{
			return m_data[VX];
		}

		LITE_MATHS_INLINE T Y() const
		{
			return m_data[VY];
		}

		LITE_MATHS_INLINE T Z() const
		{
			return m_data[VZ];
		}

		LITE_MATHS_INLINE T W() const
		{
			return m_data[VW];
		}

		LITE_MATHS_INLINE void X(T v)
		{
			m_data[VX] = v;
		}

		LITE_MATHS_INLINE void Y(T v)
		{
			m_data[VY] = v;
		}

		LITE_MATHS_INLINE void Z(T v)
		{
			m_data[VZ] = v;
		}

		LITE_MATHS_INLINE void W(T v)
		{
			m_data[VW] = v;
		}

		LITE_MATHS_INLINE T Length() const
		{
			return sqrtf(
				m_data[VX] * m_data[VX] +
				m_data[VY] * m_data[VY] + 
				m_data[VZ] * m_data[VZ] +
				m_data[VW] * m_data[VW]);
		}

		LITE_MATHS_INLINE bool Equals(	const Vec4<T>& rhs,
							T epsilon = LITE_MATHS_EQ_EPSILON) const
		{
			T magnitudeSquared = 0;

			T delta = m_data[VX] - rhs.m_data[VX];
			magnitudeSquared += delta*delta;
			delta = m_data[VY] - rhs.m_data[VY];
			magnitudeSquared += delta*delta;
			delta = m_data[VZ] - rhs.m_data[VZ];
			magnitudeSquared += delta*delta;
			delta = m_data[VW] - rhs.m_data[VW];
			magnitudeSquared += delta*delta;

			if (magnitudeSquared >= epsilon)
				return false;

			return true;
		}

		void Zero()
		{
			memset(&m_data, 0, sizeof(m_data));
		}

	private:
		T m_data[4];
	};

	typedef Vec4<F32>	Vec4F32;
	typedef Vec4<S32>	Vec4S32;
	typedef Vec4<U8>	Vec4U8;
}

#endif
