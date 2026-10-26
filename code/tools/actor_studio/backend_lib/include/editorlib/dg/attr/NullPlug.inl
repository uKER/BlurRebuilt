//From I_Attribute

const char* C_NullPlug::GetName() const
{
	return C_NamedPlugPolicy::_GetName();
}

void C_NullPlug::GetValue(U32& value, bool update) const
{
}

void C_NullPlug::GetValue(U64& value, bool update) const
{
}

void C_NullPlug::GetValue(S32& value, bool update) const
{
}

void C_NullPlug::GetValue(S64& value, bool update) const
{
}

void C_NullPlug::GetValue(F32& value, bool update) const
{
}

void C_NullPlug::GetValue(F64& value, bool update) const
{
}

void C_NullPlug::GetValue(bool& value, bool update) const
{
}

void C_NullPlug::GetValue(Btl::String& value, bool update) const
{
}

void C_NullPlug::GetValue(const Btl::String*& value, bool update) const
{
}

void C_NullPlug::GetValue(MathsStorage::Matrix44& value, bool update) const
{
}

void C_NullPlug::GetValue(const MathsStorage::Matrix44*& value, bool update) const
{
}

Util::E_VarType C_NullPlug::GetDataType() const
{
	return Util::E_VarType_Null;
}
