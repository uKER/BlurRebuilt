#ifndef LITE_MATHS_VEC_3_H
#define LITE_MATHS_VEC_3_H

namespace LiteMaths
{

	template <class T>
	class Vec3
	{
	public:
		enum Axis
		{
			VX,
			VY,
			VZ
		};

		LITE_MATHS_INLINE Vec3()
		{
		}

		LITE_MATHS_INLINE Vec3(const Vec3& rhs)
		{
			*this = rhs;
		}

		LITE_MATHS_INLINE Vec3(T x, T y, T z)
		{
			m_data[VX] = x;
			m_data[VY] = y;
			m_data[VZ] = z;
		}

		const Vec3& operator = (const Vec3& rhs)
		{
			memcpy(m_data, rhs.m_data, sizeof(m_data));
			return *this;
		}

		LITE_MATHS_INLINE bool operator < (const Vec3& rhs) const
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
			return false;
		}

		LITE_MATHS_INLINE void operator += (const Vec3& rhs)
		{
			m_data[VX] += rhs.m_data[VX];
			m_data[VY] += rhs.m_data[VY];
			m_data[VZ] += rhs.m_data[VZ];
		}

		LITE_MATHS_INLINE void operator -= (const Vec3& rhs)
		{
			m_data[VX] -= rhs.m_data[VX];
			m_data[VY] -= rhs.m_data[VY];
			m_data[VZ] -= rhs.m_data[VZ];
		}

		LITE_MATHS_INLINE void operator *= (const Vec3& rhs)
		{
			m_data[VX] *= rhs.m_data[VX];
			m_data[VY] *= rhs.m_data[VY];
			m_data[VZ] *= rhs.m_data[VZ];
		}

		LITE_MATHS_INLINE void operator *= (const T rhs)
		{
			m_data[VX] *= rhs;
			m_data[VY] *= rhs;
			m_data[VZ] *= rhs;
		}

		LITE_MATHS_INLINE const Vec3 operator + (const Vec3& rhs) const
		{
			return Vec3(
				m_data[VX] + rhs.m_data[VX],
				m_data[VY] + rhs.m_data[VY],
				m_data[VZ] + rhs.m_data[VZ]);
		}

		LITE_MATHS_INLINE const Vec3 operator - (const Vec3& rhs) const
		{
			return Vec3(
				m_data[VX] - rhs.m_data[VX],
				m_data[VY] - rhs.m_data[VY],
				m_data[VZ] - rhs.m_data[VZ]);
		}

		LITE_MATHS_INLINE const Vec3 operator * (const Vec3& rhs) const
		{
			return Vec3(
				m_data[VX] * rhs.m_data[VX],
				m_data[VY] * rhs.m_data[VY],
				m_data[VZ] * rhs.m_data[VZ]);
		}

		LITE_MATHS_INLINE const Vec3 operator * (T rhs) const
		{
			return Vec3(
				m_data[VX] * rhs,
				m_data[VY] * rhs,
				m_data[VZ] * rhs);
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
			m_data[2] = v;
		}

		LITE_MATHS_INLINE T Length() const
		{
			return sqrtf(m_data[0] * m_data[0] + m_data[1] * m_data[1] + m_data[2] * m_data[2]);
		}

		LITE_MATHS_INLINE void Normalise()
		{
			T length = Length();
			if (length < 1e-6)
			{
				memset(m_data, 0, sizeof(m_data));
			}
			m_data[0] /= length;
			m_data[1] /= length;
			m_data[2] /= length;
		}

		LITE_MATHS_INLINE Vec3 Normalised() const
		{
			T length = Length();
			if (length < LITE_MATHS_EPSILON) return Vec3(0,0,0);
			return Vec3 (
				m_data[0] / length,
				m_data[1] / length,
				m_data[2] / length);
		}

		LITE_MATHS_INLINE Vec3 CrossProduct(const Vec3<T>& rhs)
		{
			return Vec3(	m_data[VY] * rhs.m_data[VZ] - m_data[VZ] * rhs.m_data[VY],
							m_data[VZ] * rhs.m_data[VX] - m_data[VX] * rhs.m_data[VZ],
							m_data[VX] * rhs.m_data[VY] - m_data[VY] * rhs.m_data[VX]);
		}

		LITE_MATHS_INLINE Vec3 operator ^ (const Vec3<T>& rhs)
		{
			return CrossProduct(rhs);
		}

		LITE_MATHS_INLINE T DotProduct(const Vec3<T>& rhs)
		{
			return (m_data[0] * rhs.m_data[0] + m_data[1] * rhs.m_data[1] + m_data[2] * rhs.m_data[2]);
		}

		LITE_MATHS_INLINE T operator | (const Vec3<T>& rhs)
		{
			return DotProduct(rhs);
		}

		LITE_MATHS_INLINE bool Equals(	const Vec3<T>& rhs,
							T epsilon = LITE_MATHS_EQ_EPSILON) const
		{
			T magnitudeSquared = 0;

			T delta = m_data[VX] - rhs.m_data[VX];
			magnitudeSquared += delta*delta;
			delta = m_data[VY] - rhs.m_data[VY];
			magnitudeSquared += delta*delta;
			delta = m_data[VZ] - rhs.m_data[VZ];
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
		T m_data[3];
	};

	typedef Vec3<F32>	Vec3F32;
	typedef Vec3<S32>	Vec3S32;
}

#endif
