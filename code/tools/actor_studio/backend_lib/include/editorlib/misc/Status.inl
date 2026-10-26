C_Status::C_Status()
{
	m_errorState = E_Status_Success;
}

C_Status::C_Status(const C_Status& rhs)
{
	*this = rhs;
}

C_Status::C_Status(E_Status status)
{
	m_errorState = status;
}

const C_Status& C_Status::operator=(const C_Status& rhs)
{
	m_errorState = rhs.m_errorState;
	return *this;
}
	
const C_Status& C_Status::operator=(E_Status status)
{
	m_errorState = status;
	return *this;
}

C_Status::operator bool() const
{
	return m_errorState == E_Status_Success;
}

C_Status::operator E_Status() const
{
	return m_errorState;
}
