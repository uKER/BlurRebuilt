#ifndef AFTK_BASE_KEY_H
#define AFTK_BASE_KEY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeBaseKey;

	///Stores a sample for animation, or any other purpose where a 
	///value varies over x.
	class C_BaseKey
	{
	public:
		///\brief Constructor.
		///
		C_BaseKey()
		{
			m_x = 0;
			m_value = 0;
		}

		///\brief Copy constructor.
		///
		///@param rhs The key to copy.
		C_BaseKey(const C_BaseKey& rhs)
		{
			*this = rhs;
		}

		///\brief Asignment operator.
		///
		///@param rhs The key to copy.
		///@return A reference to this key.
		const C_BaseKey& operator=(const C_BaseKey& rhs)
		{
			m_x = rhs.m_x;
			m_value = rhs.m_value;

			return *this;
		}

		void SetX(F32 x)
		{
			m_x = x;
		}

		///\brief Returns the value of x for this key.
		///
		///Typically x is a time.
		///@return The value of x.
		F32& GetX()
		{
			return m_x;
		}

		///\brief Returns the value of x for this key.
		///
		///Typically x is a time.
		///@return The value of x.
		const F32& GetX() const
		{
			return m_x;
		}

		void SetValue(F32 value)
		{
			m_value = value;
		}

		///\brief Returns the value of this key at x.
		///
		///@return The value of the key at x.
		F32& GetValue()
		{
			return m_value;
		}

		///\brief Returns the value of this key at x.
		///
		///@return The value of the key at x.
		const F32& GetValue() const
		{
			return m_value;
		}


	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		F32	m_value;
		F32 m_x;

		FRIEND_CLASS C_NativeBaseKey;
	#endif
	};
}

#endif
