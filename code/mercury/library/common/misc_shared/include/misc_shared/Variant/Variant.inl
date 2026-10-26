template<typename DATA_TYPE>
INLINE E_VarType GetVarType()
{
	return E_VarType_Unknown;
}

template<>
INLINE E_VarType GetVarType<U32>()
{
	return E_VarType_U32;
}
template<>
INLINE E_VarType GetVarType<U64>()
{
	return E_VarType_U64;
}
template<>
INLINE E_VarType GetVarType<S32>()
{
	return E_VarType_S32;
}
template<>
INLINE E_VarType GetVarType<S64>()
{
	return E_VarType_S64;
}
template<>
INLINE E_VarType GetVarType<F32>()
{
	return E_VarType_F32;
}
template<>
INLINE E_VarType GetVarType<F64>()
{
	return E_VarType_F64;
}
template<>
INLINE E_VarType GetVarType<bool>()
{
	return E_VarType_Boolean;
}
template<>
INLINE E_VarType GetVarType<Btl::String>()
{
	return E_VarType_String;
}
template<>
INLINE E_VarType GetVarType<MathsStorage::Matrix44>()
{
	return E_VarType_Matrix;
}


C_Variant::C_Variant()
{
	m_type = E_VarType_Null;
}

C_Variant::C_Variant(const C_Variant& rhs)
{
	*this = rhs;
}

C_Variant::~C_Variant()
{
	ClearHeapData();
}

const C_Variant& C_Variant::operator =(const C_Variant& rhs)
{
	if (m_type == E_VarType_String)
	{
		//If no need to realloc new string.
		if (rhs.m_type == E_VarType_String)
		{
			*GetData()->pString = *rhs.GetData()->pString;
			return *this;
		}
		else
			//Clear existing string, will be overwritten with no string type.
			ClearHeapData();
	}
	else if (rhs.m_type == E_VarType_String)
	{
		//We're not of type string, but we need to be.
		GetData()->pString = new Btl::String(*rhs.GetData()->pString);
		m_type = E_VarType_String;
		return *this;
	}
	if (m_type == E_VarType_Matrix)
	{
		//If no need to realloc new string.
		if (rhs.m_type == E_VarType_Matrix)
		{
			*GetData()->pMatrix = *rhs.GetData()->pMatrix;
			return *this;
		}
		else
			ClearHeapData();
	}
	else if (rhs.m_type == E_VarType_Matrix)
	{
		GetData()->pMatrix = new MathsStorage::Matrix44(*rhs.GetData()->pMatrix);
		m_type = E_VarType_Matrix;
		return *this;
	}
	//bitwise copy. heap data cleared by this point.
	memcpy(GetData(), rhs.GetData(), sizeof(C_Data));
	m_type = rhs.m_type;
	return *this;
}

bool C_Variant::operator==(const C_Variant &rhs) const
{
	if (m_type != rhs.m_type)
		return false;

	switch(m_type)
	{
		//Heap data...
		case E_VarType_String:
			return *GetData()->pString == *rhs.GetData()->pString;
		case E_VarType_Matrix:
			return memcmp(GetData()->pMatrix, rhs.GetData()->pMatrix, sizeof(MathsStorage::Matrix44)) == 0;

		case E_VarType_U32:
		case E_VarType_S32:
		case E_VarType_F32:
			//Compare first 4 bytes.
			return memcmp(GetData(), rhs.GetData(), 4) == 0;

		case E_VarType_S64:
		case E_VarType_U64:
		case E_VarType_F64:
			//Compare first 8 bytes.
			return memcmp(GetData(), rhs.GetData(), 8) == 0;

		default:
			BIZ_ASSERT(!"Invalid data type!");
			break;
	}

	//lhs and rhs are the same invalid type.
	return true;
}

bool C_Variant::operator!= (const C_Variant &rhs) const 
{
	return !(*this == rhs);
}

bool C_Variant::operator<(const C_Variant &rhs) const 
{
	//Note that this is going to compare uninitialised memory, so results will be
	//inconsistent between runs. For a little more cost we could truncate the comparison.
	return memcmp(GetData(), rhs.GetData(), sizeof(C_Data)) < 0;
}

void C_Variant::SetValue(const U32 value)
{
	ClearHeapData();
	GetData()->u32 = value;
	m_type = E_VarType_U32;
}

void C_Variant::SetValue(const U64 value)
{
	ClearHeapData();
	GetData()->u64 = value;
	m_type = E_VarType_U64;
}

void C_Variant::SetValue(const S32 value)
{
	ClearHeapData();
	GetData()->s32 = value;
	m_type = E_VarType_S32;
}

void C_Variant::SetValue(const S64 value)
{
	ClearHeapData();
	GetData()->s64 = value;
	m_type = E_VarType_S64;
}

void C_Variant::SetValue(const F32 value)
{
	ClearHeapData();
	GetData()->f32 = value;
	m_type = E_VarType_F32;
}

void C_Variant::SetValue(const F64 value)
{
	ClearHeapData();
	GetData()->f64 = value;
	m_type = E_VarType_F64;
}

void C_Variant::SetValue(const bool value)
{
	ClearHeapData();
	GetData()->boolean = value;
	m_type = E_VarType_Boolean;
}

void C_Variant::SetValue(const Btl::String& value)
{
	if (m_type == E_VarType_String)
	{
		*GetData()->pString = value;
		return;
	}
	GetData()->pString = new Btl::String(value);
	m_type = E_VarType_String;
}

void C_Variant::SetValue(const MathsStorage::Matrix44& value)
{
	if (m_type == E_VarType_Matrix)
	{
		*GetData()->pMatrix = value;
		return;
	}
	GetData()->pMatrix = new MathsStorage::Matrix44(value);
	m_type = E_VarType_Matrix;
}

bool C_Variant::GetValue(U32& value) const
{
	if (m_type != E_VarType_U32) return false;
	value = GetData()->u32;
	return true;
}

bool C_Variant::GetValue(U64& value) const
{
	if (m_type != E_VarType_U64) return false;
	value = GetData()->u64;
	return true;
}

bool C_Variant::GetValue(S32& value) const
{
	if (m_type != E_VarType_S32) return false;
	value = GetData()->s32;
	return true;
}

bool C_Variant::GetValue(S64& value) const
{
	if (m_type != E_VarType_S64) return false;
	value = GetData()->s64;
	return true;
}

bool C_Variant::GetValue(F32& value) const
{
	if (m_type != E_VarType_F32) return false;
	value = GetData()->f32;
	return true;
}

bool C_Variant::GetValue(F64& value) const
{
	if (m_type != E_VarType_F64) return false;
	value = GetData()->f64;
	return true;
}

bool C_Variant::GetValue(bool& value) const
{
	if (m_type != E_VarType_Boolean) return false;
	value = GetData()->boolean;
	return true;
}

bool C_Variant::GetValue(Btl::String& value) const
{
	if (m_type != E_VarType_String) return false;
	value = *GetData()->pString;
	return true;
}

bool C_Variant::GetValue(MathsStorage::Matrix44& value) const
{
	if (m_type != E_VarType_Matrix) return false;
	value = *GetData()->pMatrix;
	return true;
}

E_VarType C_Variant::GetDataType() const
{
	return m_type;
}

void C_Variant::Clear()
{
	ClearHeapData();
	m_type = E_VarType_Null;
}

void C_Variant::ClearHeapData()
{
	if (m_type == E_VarType_String)
		SAFE_DELETE_PTR(GetData()->pString);
	if (m_type == E_VarType_Matrix)
		SAFE_DELETE_PTR(GetData()->pMatrix);
}

C_Variant::C_Data*	C_Variant::GetData()
{
	return (C_Variant::C_Data*)m_data;
}

const C_Variant::C_Data* C_Variant::GetData() const
{
	return (C_Variant::C_Data*)m_data;
}
