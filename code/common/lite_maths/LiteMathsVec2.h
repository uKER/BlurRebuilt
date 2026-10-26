#ifndef LITE_MATHS_VEC_2_H
#define LITE_MATHS_VEC_2_H

namespace LiteMaths
{
	template <class T>
	class Vec2
	{
	public:
		enum Axis
		{
			VX,
			VY,
		};

		LITE_MATHS_INLINE Vec2()
		{
		}

		LITE_MATHS_INLINE Vec2(const Vec2& rhs)
		{
			*this = rhs;
		}

		LITE_MATHS_INLINE Vec2(T x, T y)
		{
			m_data[VX] = x;
			m_data[VY] = y;
		}

		bool operator ==(const Vec2& rhs) const
		{
			if (
				(m_data[VX] == rhs.m_data[VX]) &&
				(m_data[VY] == rhs.m_data[VY])
				)
			{
				return true;
			}

			return false;
		}

		const Vec2& operator = (const Vec2& rhs)
		{
			memcpy(m_data, rhs.m_data, sizeof(m_data));
			return *this;
		}

		LITE_MATHS_INLINE bool operator < (const Vec2& rhs) const
		{
			if (lhs.X() < rhs.X())
			{
				return true;
			}
			if (lhs.Y() < rhs.Y())
			{
				return true;
			}
			return false;
		}

		LITE_MATHS_INLINE void operator += (const Vec2& rhs)
		{
			m_data[VX] += rhs.m_data[VX];
			m_data[VY] += rhs.m_data[VY];
		}

		LITE_MATHS_INLINE void operator -= (const Vec2& rhs)
		{
			m_data[VX] -= rhs.m_data[VX];
			m_data[VY] -= rhs.m_data[VY];
		}

		LITE_MATHS_INLINE void operator *= (const Vec2& rhs)
		{
			m_data[VX] *= rhs.m_data[VX];
			m_data[VY] *= rhs.m_data[VY];
		}

		LITE_MATHS_INLINE void operator *= (const T rhs)
		{
			m_data[VX] *= rhs;
			m_data[VY] *= rhs;
		}

		LITE_MATHS_INLINE const Vec2 operator + (const Vec2& rhs) const
		{
			return Vec2(
				m_data[VX] + rhs.m_data[VX],
				m_data[VY] + rhs.m_data[VY]);
		}

		LITE_MATHS_INLINE const Vec2 operator - (const Vec2& rhs) const
		{
			return Vec2(
				m_data[VX] - rhs.m_data[VX],
				m_data[VY] - rhs.m_data[VY]);
		}

		LITE_MATHS_INLINE const Vec2 operator * (const Vec2& rhs) const
		{
			return Vec2(
				m_data[VX] * rhs.m_data[VX],
				m_data[VY] * rhs.m_data[VY]);
		}

		LITE_MATHS_INLINE const Vec2 operator * (T rhs) const
		{
			return Vec2(
				m_data[VX] * rhs,
				m_data[VY] * rhs);
		}

		LITE_MATHS_INLINE T X() const
		{
			return m_data[VX];
		}

		LITE_MATHS_INLINE T Y() const
		{
			return m_data[VY];
		}

		LITE_MATHS_INLINE void X(T v)
		{
			m_data[VX] = v;
		}

		LITE_MATHS_INLINE void Y(T v)
		{
			m_data[VY] = v;
		}

		LITE_MATHS_INLINE T Length() const
		{
			return sqrtf(m_data[0] * m_data[0] + m_data[1] * m_data[1]);
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
		}

		LITE_MATHS_INLINE Vec2 Normalised() const
		{
			T length = Length();
			if (length < LITE_MATHS_EPSILON) return Vec2(0,0,0);
			return Vec2 (
				m_data[0] / length,
				m_data[1] / length);
		}

		LITE_MATHS_INLINE T DotProduct(const Vec2<T>& rhs)
		{
			return (m_data[0] * rhs.m_data[0] + m_data[1] * rhs.m_data[1]);
		}

		LITE_MATHS_INLINE T operator | (const Vec2<T>& rhs)
		{
			return DotProduct(rhs);
		}

		LITE_MATHS_INLINE bool Equals(	const Vec2<T>& rhs,
							T epsilon = LITE_MATHS_EQ_EPSILON) const
		{
			T magnitudeSquared = 0;

			T delta = m_data[VX] - rhs.m_data[VX];
			magnitudeSquared += delta*delta;
			delta = m_data[VY] - rhs.m_data[VY];
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
		T m_data[2];
	};

	typedef Vec2<F32>	Vec2F32;
	typedef Vec2<S32>	Vec2S32;
}

#endif
