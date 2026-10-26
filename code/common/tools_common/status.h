#ifndef TU_STATUS_H
#define TU_STATUS_H


namespace Tu
{
class Status
{
public:
	enum StatusEnum
	{
		FAILURE = 0,
		SUCCESS = 1,
		NOT_IMPLEMENTED = 2,
		OUT_OF_MEMORY = 3,
		CANCELLED = 4,

		COUNT
	};

	///\brief Default constructor.
	///
	Status();
	
	///\brief Construct from a given status code. The error string will be null.
	///
	///@param ErrorCode The status code value.
	Status(StatusEnum ErrorCode);

	///\brief Construct from a given status code and error string.
	///
	///@param ErrorCode The status code value.
	///@param errorString The error string.
	Status(StatusEnum ErrorCode, const std::string& errorString);
	
	///\brief Construct from a given status code and error string.
	///
	///@param ErrorCode The status code value.
	///@param errorString The error string.
	Status(StatusEnum ErrorCode, U32 severity, const std::string& errorString);

	///\brief Copy constructor.
	///
	///@param rhs The class to copy.
	Status(const Status& rhs);

	///\brief Destructor.
	///
	~Status();

	///\brief Asignment operator.
	///
	///@param rhs The class to copy.
	///@return A reference to this class.
	const Status& operator=(const Status& rhs);

	///\brief Equality test operator.
	///
	///Note that both the status value _AND_ the error string are compared.
	///
	///@param rhs The class to compare with.
	///@return true or false.
	bool operator==(const Status& rhs) const;

	///\brief Equality test operator.
	///
	///The error string value is ignored here.
	///
	///@param ErrorCode The status code to compare against.
	///@return true or false.
	bool operator==(StatusEnum ErrorCode) const;

	///
	///\brief Inequality test operator.
	///
	///Note that both the status value _AND_ the error string are compared.
	///
	///@param rhs The class to compare with.
	///@return true or false.
	bool operator!=(const Status& rhs) const;

	///\brief Inequality test operator.
	///
	///The error string value is ignored here.
	///
	///@param ErrorCode The status code to compare against.
	///@return true or false.
	bool operator!=(StatusEnum ErrorCode) const;

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
	StatusEnum ErrorCode() const;

	///\brief Returns the error string of this class.
	///
	///@return The error string.
	const std::string& ErrorString() const;

	void SetErrorString(const std::string& errorString);

	///\brief Sets the status code of this class.
	///
	///Note that the error string is reset to null.
	///
	///@param ErrorCode The status code value.
	void SetErrorCode(StatusEnum ErrorCode);

	///\brief Sets the status code of this class.
	///
	///@param ErrorCode The status code value.
	///@param errorString The error string.
	void Set(StatusEnum ErrorCode, const std::string& errorString="", U32 severity=0);

	void SetSeverity(U32 severity)
	{
		m_severity = severity;
	}

	U32 Severity() const
	{
		return m_severity;
	}

	///\brief Clears the contents of this class to its initial state.
	///
	///After a clear the status code is set to SUCCESS and the error string is set to null.
	void Clear();
	
private:
	StatusEnum		m_errorCode;
	std::string		m_errorString;
	U32				m_severity;
};


inline Status::Status()
{
	m_severity = 0;
	m_errorCode = Status::SUCCESS;
}

inline Status::Status(StatusEnum statusCode, const std::string& errorString)
{
	m_severity = 0;
	m_errorCode = statusCode;
	m_errorString = errorString;
}

inline Status::Status(StatusEnum statusCode, U32 severity, const std::string& errorString)
{
	m_severity = severity;
	m_errorCode = statusCode;
	m_errorString = errorString;
}

inline Status::Status(StatusEnum statusCode)
{
	m_severity = 0;
	m_errorCode = statusCode;
}

inline Status::Status(const Status& rhs)
{
	*this = rhs;
}

inline Status::~Status()
{
}

inline const Status& Status::operator=(const Status& rhs)
{
	m_errorCode = rhs.m_errorCode;
	m_errorString = rhs.m_errorString;
	m_severity = rhs.m_severity;

	return *this;
}

inline bool Status::operator==(const Status& rhs) const
{
	if ((m_errorCode == rhs.m_errorCode) &&
		(m_errorString == rhs.m_errorString) &&
		(m_severity == rhs.m_severity))
	{
		return true;
	}

	return false;
}

inline bool Status::operator==(StatusEnum statusCode) const
{
	if (m_errorCode == statusCode)
	{
		return true;
	}

	return false;
}

inline bool Status::operator!=(const Status& rhs) const
{
	return !((*this) == rhs);
}

inline bool Status::operator!=(StatusEnum statusCode) const
{
	return !((*this) == statusCode);
}

inline Status::operator bool() const
{
	if (m_errorCode ==	Status::SUCCESS)
	{
		return true;
	}

	return false;
}

inline Status::StatusEnum Status::ErrorCode() const
{
	return m_errorCode;
}

inline const std::string& Status::ErrorString() const
{
	return m_errorString;
}

inline void Status::SetErrorCode(StatusEnum statusCode)
{
	m_errorCode = statusCode;
}

inline void Status::SetErrorString(const std::string& errorString)
{
	m_errorString = errorString;
}

inline void Status::Set(StatusEnum statusCode, const std::string& errorString, U32 severity)
{
	m_errorCode = statusCode;
	m_errorString = errorString;
	m_severity = severity;
}

inline void Status::Clear()
{
	m_errorCode = Status::SUCCESS;
	m_errorString.clear();
}
}

#endif
