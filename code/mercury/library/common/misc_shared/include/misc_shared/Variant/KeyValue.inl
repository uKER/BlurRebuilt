C_KeyValue::C_KeyValue() : C_Variant()
{
}

C_KeyValue::~C_KeyValue()
{
}

C_KeyValue::C_KeyValue(const C_KeyValue &rhs)
{
	*this = rhs;
}

const C_KeyValue& C_KeyValue::operator=(const C_KeyValue &rhs)
{
	C_Variant::operator=(rhs);
	m_name = rhs.m_name;
	return *this;
}

bool C_KeyValue::operator==(const C_KeyValue &rhs) const
{
	return C_Variant::operator==(rhs) && m_name == rhs.m_name;
}

bool C_KeyValue::operator!= (const C_KeyValue &rhs) const
{
	return !(*this==rhs);
}

bool C_KeyValue::operator<(const C_KeyValue &rhs) const
{
	return C_Variant::operator<(rhs) && m_name < rhs.m_name;
}

void C_KeyValue::SetName(const Btl::String& name)
{
	m_name = name;
}

const Btl::String& C_KeyValue::GetName() const
{
	return m_name;
}
