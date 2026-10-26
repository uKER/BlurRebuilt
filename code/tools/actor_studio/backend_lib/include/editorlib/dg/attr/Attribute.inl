C_Attribute::C_Attribute() 
{
}

C_Attribute::~C_Attribute()
{
	// Free any allocated memory
	Clear();
}

// Perform deep copying of this node;
C_Attribute::C_Attribute(const C_Attribute &rhs) 
{
	*this = rhs;
}

const C_Attribute& C_Attribute::operator =(const C_Attribute &rhs)
{
	m_name = rhs.m_name;
	m_variant = rhs.m_variant;
	return *this;
}

bool C_Attribute::operator==(const C_Attribute &rhs) const
{
	return m_name == rhs.m_name && m_variant == rhs.m_variant;
}

bool C_Attribute::operator!= (const C_Attribute &rhs) const 
{
	return !(*this==rhs);
}

bool C_Attribute::operator<(const C_Attribute &rhs) const 
{
	return m_name < rhs.m_name && m_variant < rhs.m_variant;
}

void C_Attribute::SetName(const Btl::String& name) 
{
	m_name = name;
}

void C_Attribute::SetValue(U32 value)
{
	m_variant.SetValue(value);
}

void C_Attribute::SetValue(U64 value)
{
	m_variant.SetValue(value);
}

void C_Attribute::SetValue(S32 value)
{
	m_variant.SetValue(value);
}

void C_Attribute::SetValue(S64 value)
{
	m_variant.SetValue(value);
}

void C_Attribute::SetValue(F32 value) 
{
	m_variant.SetValue(value);
}

void C_Attribute::SetValue(F64 value)
{
	m_variant.SetValue(value);
}

void C_Attribute::SetValue(bool value) 
{
	m_variant.SetValue(value);
}

void C_Attribute::SetValue(const Btl::String& value) 
{
	m_variant.SetValue(value);
}

const Btl::String& C_Attribute::GetName() const 
{
	return m_name;
}

Util::E_VarType C_Attribute::GetDataType() const 
{
	return m_variant.GetDataType();
}

void C_Attribute::Clear() 
{
	m_variant.Clear();
}

void C_Attribute::GetValue(U32& value) const
{
	m_variant.GetValue(value);
}

void C_Attribute::GetValue(U64& value) const
{
	m_variant.GetValue(value);
}

void C_Attribute::GetValue(S32& value) const
{
	m_variant.GetValue(value);
}

void C_Attribute::GetValue(S64& value) const
{
	m_variant.GetValue(value);
}

void C_Attribute::GetValue(F32& value) const
{
	m_variant.GetValue(value);
}

void C_Attribute::GetValue(F64& value) const
{
	m_variant.GetValue(value);
}

void C_Attribute::GetValue(bool& value) const
{
	m_variant.GetValue(value);
}

void C_Attribute::GetValue(Btl::String& value) const
{
	m_variant.GetValue(value);
}

void C_Attribute::GetValue(const Btl::String*& value) const
{
	m_variant.GetValue(value);
}
