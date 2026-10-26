#ifndef AFTK_STATUS_H
#define AFTK_STATUS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	///A container to return more information than a simple boolean
	///return value for a function.
	class C_Status
	{
	public:
		enum E_Status
		{
			E_Status_Failure,
			E_Status_Success,
			E_Status_NotImplemented,
			E_Status_OutOfMemory,
			E_Status_Cancelled
		};

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
		///@param errorString The error string.
		C_Status(E_Status ErrorCode, const std::string& errorString, U32 severity);

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
		///@param ErrorCode The status code to compare against.
		///@return true or false.
		bool operator==(E_Status ErrorCode) const;

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
		///@param ErrorCode The status code to compare against.
		///@return true or false.
		bool operator!=(E_Status ErrorCode) const;

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
		void Set(E_Status ErrorCode, const std::string& errorString, U32 severity);

		///\brief This operator appends a char* string to the current error string.
		///
		///@param string The string to append.
		C_Status& operator<<(const char* string);

		///\brief This operator appends an std::string to the current error string.
		///
		///@param string The string to append.
		C_Status& operator<<(const std::string& string);

		///\brief This operator appends the given number to the current error string.
		///
		///@param var The number to append.
		C_Status& operator<<(const U8 var);

		///\brief This operator appends the given number to the current error string.
		///
		///@param var The number to append.
		C_Status& operator<<(const U16 var);

		///\brief This operator appends the given number to the current error string.
		///
		///@param var The number to append.
		C_Status& operator<<(const U32 var);

		///\brief This operator appends the given number to the current error string.
		///
		///@param var The number to append.
		C_Status& operator<<(const S8 var);

		///\brief This operator appends the given number to the current error string.
		///
		///@param var The number to append.
		C_Status& operator<<(const S16 var);

		///\brief This operator appends the given number to the current error string.
		///
		///@param var The number to append.
		C_Status& operator<<(const S32 var);

		///\brief This operator appends the given number to the current error string.
		///
		///@param var The number to append.
		C_Status& operator<<(const F32 var);

		///\brief This operator appends the given number to the current error string.
		///
		///@param var The number to append.
		C_Status& operator<<(const F64 var);

		///\brief This operator appends the given vector to the current error string.
		///
		///@param value The vector to append.
		template <class T, int N>
		C_Status& operator <<(const UtMaths::BaseVector<T, N>& value)
		{
			for (U32 i = 0; i < value.SIZE; i++)
			{
				*this << value[i];
				if (i < value.SIZE-1)
				{
					*this << " ";
				}
			}

			return*	this;
		}

		///\brief This operator appends the given matrix to the current error string.
		///
		///@param value The matrix to append.
		template <class T, int N>
		C_Status& operator <<(const UtMaths::BasicMatrix<T, N>& value) 
		{
			for (U32 i = 0; i < value.SIZE; i++)
			{
				*this << "Row ";
				*this << i;
				*this << " = [";
				*this << value[i];
				if (i < value.SIZE-1)
				{
					*this << "]\n";
				}
			}
			*this << "]\n";

			return*	this;
		}

		void SetSeverity(U32 severity)
		{
			m_severity = severity;
		}

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
}

#endif
