//From I_DgObject
template<typename DATA_TYPE>
const char* C_StaticPlug<DATA_TYPE>::GetName() const
{
	return C_NamedPlugPolicy::_GetName();
}

//From I_Attribute
template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(U32& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_U32)
		{
			value = (U32&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(U64& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_U64)
		{
			value = (U64&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}


template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(S32& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_S32)
		{
			value = (S32&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}


template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(S64& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_S64)
		{
			value = (S64&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(F32& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_F32)
		{
			value = (F32&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(F64& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_F64)
		{
			value = (F64&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(bool& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_Boolean)
		{
			value = (bool&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(Btl::String& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_String)
		{
			value = (Btl::String&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(const Btl::String*& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_String)
		{
			value = (const Btl::String*)&m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(MathsStorage::Matrix44& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_Matrix)
		{
			value = (MathsStorage::Matrix44&)m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::GetValue(const MathsStorage::Matrix44*& value, bool update) const
{
	if (!GetSourceConnection()) 
	{ 
		if (GetDataType() == Util::E_VarType_Matrix)
		{
			value = (const MathsStorage::Matrix44*)&m_value; 
		}
	}
	else GetSourceConnection()->GetValue(value);
}


template<typename DATA_TYPE>
Util::E_VarType C_StaticPlug<DATA_TYPE>::GetDataType() const
{
	return Util::GetVarType<DATA_TYPE>();
}

//C_StaticPlug
template<typename DATA_TYPE>
C_StaticPlug<DATA_TYPE>::C_StaticPlug(
	I_DgNode* pParent,
	void(*pOnInputChangedCallback)(I_Plug* pThis),
	E_PlugType type,
	const char* name) : 
	C_PlugBase<C_NamedPlugPolicy, C_ChildPolicy, C_ConnectablePolicy>(pParent, pOnInputChangedCallback, type, name)
{
}

template<typename DATA_TYPE>
C_StaticPlug<DATA_TYPE>::C_StaticPlug(
	I_Plug* pParent,
	void(*pOnInputChangedCallback)(I_Plug* pThis),
	E_PlugType type,
	const char* name) : 
	C_PlugBase<C_NamedPlugPolicy, C_ChildPolicy, C_ConnectablePolicy>(pParent, pOnInputChangedCallback, type, name)
{
}

template<typename DATA_TYPE>
C_StaticPlug<DATA_TYPE>::~C_StaticPlug()
{
}

template<typename DATA_TYPE>
void C_StaticPlug<DATA_TYPE>::SetValue(const DATA_TYPE& value, bool triggerTopLevelParentOnChanged)
{
	m_value = value;
	m_flags.isDirtyOutput = false;
	InvokeDependentOnChanged(triggerTopLevelParentOnChanged);
}

template<typename DATA_TYPE>
const DATA_TYPE& C_StaticPlug<DATA_TYPE>::GetValue(bool update) const
{
	//Invoke update of parent.
	if (IsNodeDirty() & update)
	{
		const I_DgObject* pTlp = PlugHelpers::GetTopLevelParent(*this);
		BIZ_ASSERT(pTlp->IsA(E_ObjType_IDgNode));
		((I_DgNode*)pTlp)->Update();
	}

	return m_value;
}
