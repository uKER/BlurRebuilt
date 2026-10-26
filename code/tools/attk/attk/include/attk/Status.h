#ifndef ATTK_STATUS_H
#define ATTK_STATUS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <string>

namespace Attk
{
	class C_Status
	{
	public:
		///\brief Default constructor.
		///
		C_Status();
		
		///\brief Construct from a given status code. The error string will be null.
		///
		///@param ErrorCode The status code value.
		C_Status(E_Status ErrorCode);

		///\brief Construct from a given status code and error string.
		///
		///@param ErrorCode The status code value.
		///@param errorString The error string.
		C_Status(E_Status ErrorCode, const std::string& errorString);
		
		///\brief Construct from a given status code and error string.
		///
		///@param ErrorCode The status code value.
		///@param severity The severity of the error, not currently used - always 0.
		///@param errorString The error string.
		C_Status(E_Status ErrorCode, U32 severity, const std::string& errorString);

		///\brief Copy constructor.
		///
		///@param rhs The class to copy.
		C_Status(const C_Status& rhs);

		///\brief Destructor.
		///
		~C_Status();

		///\brief Asignment operator.
		///
		///@param rhs The class to copy.
		///@return A reference to this class.
		const C_Status& operator=(const C_Status& rhs);

		///\brief Equality test operator.
		///
		///Note that both the status value _AND_ the error string are compared.
		///
		///@param rhs The class to compare with.
		///@return true or false.
		bool operator==(const C_Status& rhs) const;

		///\brief Equality test operator.
		///
		///The error string value is ignored here.
		///
		///@param errorCode The status code to compare against.
		///@return true or false.
		bool operator==(E_Status errorCode) const;

		///
		///\brief Inequality test operator.
		///
		///Note that both the status value _AND_ the error string are compared.
		///
		///@param rhs The class to compare with.
		///@return true or false.
		bool operator!=(const C_Status& rhs) const;

		///\brief Inequality test operator.
		///
		///The error string value is ignored here.
		///
		///@param errorCode The status code to compare against.
		///@return true or false.
		bool operator!=(E_Status errorCode) const;

		///\brief Boolean query operator.
		///
		///This allows the status class to be queried as if it were a bool. This returns true
		///if the status code equals SUCCESS.
		///
		///@return true or false.
		operator bool() const;

		///\brief Returns the status code of this class.
		///
		///@return The status code.
		E_Status GetErrorCode() const;

		///\brief Returns the error string of this class.
		///
		///@return The error string.
		const std::string& GetErrorString() const;

		void SetErrorString(const std::string& errorString);

		///\brief Sets the status code of this class.
		///
		///Note that the error string is reset to null.
		///
		///@param ErrorCode The status code value.
		void SetErrorCode(E_Status ErrorCode);

		///\brief Sets the status code of this class.
		///
		///@param ErrorCode The status code value.
		///@param errorString The error string.
		///@param severity Error severity.
		void Set(E_Status ErrorCode, const std::string& errorString="", U32 severity=0);

		void SetSeverity(U32 severity)
		{
			m_severity = severity;
		}

		///\brief Returns the severity of any error information.
		///
		///@return Error severity.
		U32 GetSeverity() const
		{
			return m_severity;
		}

		///\brief Clears the contents of this class to its initial state.
		///
		///After a clear the status code is set to SUCCESS and the error string is set to null.
		void Clear();
		
	private:
		E_Status		m_errorCode;
		std::string		m_errorString;
		U32				m_severity;
	};


	inline C_Status::C_Status()
	{
		m_severity = 0;
		m_errorCode = E_Status_Success;
	}

	inline C_Status::C_Status(E_Status statusCode, const std::string& errorString)
	{
		m_severity = 0;
		m_errorCode = statusCode;
		m_errorString = errorString;
	}

	inline C_Status::C_Status(E_Status statusCode, U32 severity, const std::string& errorString)
	{
		m_severity = severity;
		m_errorCode = statusCode;
		m_errorString = errorString;
	}

	inline C_Status::C_Status(E_Status statusCode)
	{
		m_severity = 0;
		m_errorCode = statusCode;
	}

	inline C_Status::C_Status(const C_Status& rhs)
	{
		*this = rhs;
	}

	inline C_Status::~C_Status()
	{
	}

	inline const C_Status& C_Status::operator=(const C_Status& rhs)
	{
		m_errorCode = rhs.m_errorCode;
		m_errorString = rhs.m_errorString;
		m_severity = rhs.m_severity;

		return *this;
	}

	inline bool C_Status::operator==(const C_Status& rhs) const
	{
		if ((m_errorCode == rhs.m_errorCode) &&
			(m_errorString == rhs.m_errorString) &&
			(m_severity == rhs.m_severity))
		{
			return true;
		}

		return false;
	}

	inline bool C_Status::operator==(E_Status statusCode) const
	{
		if (m_errorCode == statusCode)
		{
			return true;
		}

		return false;
	}

	inline bool C_Status::operator!=(const C_Status& rhs) const
	{
		return !((*this) == rhs);
	}

	inline bool C_Status::operator!=(E_Status statusCode) const
	{
		return !((*this) == statusCode);
	}

	inline C_Status::operator bool() const
	{
		if (m_errorCode ==	E_Status_Success)
		{
			return true;
		}

		return false;
	}

	inline E_Status C_Status::GetErrorCode() const
	{
		return m_errorCode;
	}

	inline const std::string& C_Status::GetErrorString() const
	{
		return m_errorString;
	}

	inline void C_Status::SetErrorCode(E_Status statusCode)
	{
		m_errorCode = statusCode;
	}

	inline void C_Status::SetErrorString(const std::string& errorString)
	{
		m_errorString = errorString;
	}

	inline void C_Status::Set(E_Status statusCode, const std::string& errorString, U32 severity)
	{
		m_errorCode = statusCode;
		m_errorString = errorString;
		m_severity = severity;
	}

	inline void C_Status::Clear()
	{
		m_errorCode = E_Status_Success;
		m_errorString.clear();
	}
}

#endif
