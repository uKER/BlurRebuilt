#ifndef AFTK_HERMITE_KEY_H
#define AFTK_HERMITE_KEY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeHermiteKey;

	///Stores a sample for animation, or any other purpose where a 
	///value varies over x.
	class C_HermiteKey : public C_BaseKey
	{
	public:
		///\brief Constructor.
		///
		C_HermiteKey()
		{
			m_inTangentAngle = 0;
			m_outTangentAngle = 0;
			m_inTangentType = E_CurveTangentType_Unknown;
			m_outTangentType = E_CurveTangentType_Unknown;
		}

		///\brief Copy constructor.
		///
		///@param rhs The key to copy.
		C_HermiteKey(const C_HermiteKey& rhs)
		{
			*this = rhs;
		}

		///\brief Asignment operator.
		///
		///@param rhs The key to copy.
		///@return A reference to this key.
		const C_HermiteKey& operator=(const C_HermiteKey& rhs)
		{
			C_BaseKey::operator =(rhs);

			m_inTangentAngle = rhs.m_inTangentAngle;
			m_outTangentAngle = rhs.m_outTangentAngle;
			m_inTangentType = rhs.m_inTangentType;
			m_outTangentType = rhs.m_outTangentType;
			return *this;
		}

		void SetInTangentAngle(F32 inTangentAngle)
		{
			m_inTangentAngle = inTangentAngle;
		}

		void SetInTangentType(E_CurveTangentType type)
		{
			m_inTangentType = type;
		}

		Vector2F32 GetInTangentVector()
		{
			return Vector2F32(sinf(m_inTangentAngle), cosf(m_inTangentAngle));
		}

		const F32& GetInTangentAngle() const
		{
			return m_inTangentAngle;
		}

		F32& GetInTangentAngle()
		{
			return m_inTangentAngle;
		}

		E_CurveTangentType GetInTangentType() const
		{
			return m_inTangentType;
		}

		void SetOutTangentAngle(F32 outTangentAngle)
		{
			m_outTangentAngle = outTangentAngle;
		}

		void SetOutTangentType(E_CurveTangentType type)
		{
			m_outTangentType = type;
		}

		Vector2F32 GetOutTangentVector()
		{
			return Vector2F32(sinf(m_outTangentAngle), cosf(m_outTangentAngle));
		}

		const F32& GetOutTangentAngle() const
		{
			return m_outTangentAngle;
		}

		F32& GetOutTangentAngle()
		{
			return m_outTangentAngle;
		}

		E_CurveTangentType GetOutTangentType() const
		{
			return m_outTangentType;
		}

	private:
		F32 m_inTangentAngle;
		F32 m_outTangentAngle;
		E_CurveTangentType m_inTangentType;
		E_CurveTangentType m_outTangentType;

		FRIEND_CLASS C_NativeHermiteKey;
	};
}

#endif
