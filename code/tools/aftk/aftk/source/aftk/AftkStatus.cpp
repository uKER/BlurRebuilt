#include <stdafx.h>

using namespace Aftk;

C_Status::C_Status()
{
	m_severity = 0;
	m_errorCode = C_Status::E_Status_Success;
}

C_Status::C_Status(E_Status statusCode, const std::string& errorString)
{
	m_severity = 0;
	m_errorCode = statusCode;
	m_errorString = errorString;
}

C_Status::C_Status(E_Status statusCode, const std::string& errorString, U32 severity)
{
	m_severity = severity;
	m_errorCode = statusCode;
	m_errorString = errorString;
}

C_Status::C_Status(E_Status statusCode)
{
	m_severity = 0;
	m_errorCode = statusCode;
}

C_Status::C_Status(const C_Status& rhs)
{
	*this = rhs;
}

C_Status::~C_Status()
{
}

const C_Status& C_Status::operator=(const C_Status& rhs)
{
	m_errorCode = rhs.m_errorCode;
	m_errorString = rhs.m_errorString;
	m_severity = rhs.m_severity;

	return *this;
}

bool C_Status::operator==(const C_Status& rhs) const
{
	if ((m_errorCode == rhs.m_errorCode) &&
		(m_errorString == rhs.m_errorString) &&
		(m_severity == rhs.m_severity))
	{
		return true;
	}

	return false;
}

bool C_Status::operator==(E_Status statusCode) const
{
	if (m_errorCode == statusCode)
	{
		return true;
	}

	return false;
}

bool C_Status::operator!=(const C_Status& rhs) const
{
	return !((*this) == rhs);
}

bool C_Status::operator!=(E_Status statusCode) const
{
	return !((*this) == statusCode);
}

C_Status::operator bool() const
{
	if (m_errorCode ==	C_Status::E_Status_Success)
	{
		return true;
	}

	return false;
}

C_Status::E_Status C_Status::GetErrorCode() const
{
	return m_errorCode;
}

const std::string& C_Status::GetErrorString() const
{
	return m_errorString;
}

void C_Status::SetErrorCode(E_Status statusCode)
{
	m_errorCode = statusCode;
}

void C_Status::SetErrorString(const std::string& errorString)
{
	m_errorString = errorString;
}

void C_Status::Set(E_Status statusCode, const std::string& errorString, U32 severity)
{
	m_errorCode = statusCode;
	m_errorString = errorString;
	m_severity = severity;
}


C_Status& C_Status::operator<<(const char* string) 
{
	m_errorString += string;
	return *this;
}

C_Status& C_Status::operator<<(const std::string& string) 
{
	m_errorString += string;
	return *this;
}

C_Status& C_Status::operator<<(const U8 var) 
{
	std::ostringstream stream;
	stream << var;
	m_errorString += stream.str();

	return *this;
}

C_Status& C_Status::operator<<(const U16 var) 
{
	std::ostringstream stream;
	stream << var;
	m_errorString += stream.str();

	return *this;
}

C_Status& C_Status::operator<<(const U32 var) 
{
	std::ostringstream stream;
	stream << var;
	m_errorString += stream.str();

	return *this;
}

C_Status& C_Status::operator<<(const S8 var) 
{
	std::ostringstream stream;
	stream << var;
	m_errorString += stream.str();

	return *this;
}

C_Status& C_Status::operator<<(const S16 var) 
{
	std::ostringstream stream;
	stream << var;
	m_errorString += stream.str();

	return *this;
}

C_Status& C_Status::operator<<(const S32 var) 
{
	std::ostringstream stream;
	stream << var;
	m_errorString += stream.str();

	return *this;
}

C_Status& C_Status::operator<<(const F32 var) 
{
	std::ostringstream stream;
	stream << var;
	m_errorString += stream.str();

	return *this;
}

C_Status& C_Status::operator<<(const F64 var) 
{
	std::ostringstream stream;
	stream << var;
	m_errorString += stream.str();

	return *this;
}

void C_Status::Clear()
{
	m_errorCode = C_Status::E_Status_Success;
	m_errorString.clear();
}
	
