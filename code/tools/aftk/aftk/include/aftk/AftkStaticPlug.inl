template<class T>
C_StaticPlug<T>::C_StaticPlug(E_GenericAttributeType type, void(*pOnChangedCallback)(void* pThis)) 
: C_NullPlug(pOnChangedCallback)
{
	m_dataType = type;
}

template<class T>
C_StaticPlug<T>::~C_StaticPlug()
{
}

template<class T>
const C_StaticPlug<T>& C_StaticPlug<T>::operator=(const C_StaticPlug<T> &rhs)
{
	C_NullPlug::operator =(rhs);

	m_value = rhs.m_value;
	return *this;
}

template<class T>
bool C_StaticPlug<T>::operator==(const C_StaticPlug<T> &rhs) const
{
	return IsEqualToAll(rhs);
}

template<class T>
bool C_StaticPlug<T>::operator!= (const C_StaticPlug<T> &rhs) const
{
	return !((*this) == rhs);
}

template<class T>
bool C_StaticPlug<T>::operator<(const C_StaticPlug<T> &rhs) const
{
	if (C_NullPlug::operator <(rhs))
		return true;

	return false;
}

template<class T>
void C_StaticPlug<T>::SetValue(const T& value, bool evaluateDependants)
{
	m_value = value;

	if (evaluateDependants)
		OnChanged();
}

template<class T>
bool C_StaticPlug<T>::GetValue(U8& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_U8)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((U8*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(U16& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_U16)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((U16*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(U32& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_U32)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((U32*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(U64& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_U64)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((U64*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(S8& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_S8)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((S8*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(S16& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_S16)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((S16*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(S32& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_S32)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((S32*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(S64& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_S64)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((S64*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(F32& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_F32)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((F32*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(F64& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_F64)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((F64*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(bool& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_Boolean)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((bool*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(std::string& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_String)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((std::string*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::GetValue(Matrix44& value, bool evaluate) const
{
	if (m_dataType != Aftk::E_GenericAttributeType_String)
		return false;

	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	value = *((Matrix44*)((void*)&m_value));
	return true;
}

template<class T>
bool C_StaticPlug<T>::IsNull() const
{
	return false;
}

template<class T>
E_GenericAttributeType C_StaticPlug<T>::GetDataType() const
{
	return m_dataType;
}

template<class T>
bool C_StaticPlug<T>::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

template<class T>
bool C_StaticPlug<T>::IsEqualToThis(const I_Object& _rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_StaticPlug))
		return false;
	const C_StaticPlug& rhs = (const C_StaticPlug&)_rhs;

	if (m_value != rhs.m_value)
		return false;

	return true;
}

template<class T>
bool C_StaticPlug<T>::IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (!IsEqualToThis(rhs, flags))
		return false;

	return C_NullPlug::IsEqualTo(rhs, classToAbortTest, flags);
}


template<class T>
U32 C_StaticPlug<T>::GetHash() const
{
	return 0;
}

template<class T>
E_ObjType C_StaticPlug<T>::GetType() const
{
	return Aftk::E_ObjType_StaticPlug;
}

template<class T>
bool C_StaticPlug<T>::HasType(E_ObjType type) const
{
	if (C_NullPlug::HasType(type))
		return true;
	if (type == C_StaticPlug<T>::GetType()) 
		return true;

	return false;
}

template<class T>
I_Plug* C_StaticPlug<T>::Clone() const
{
	C_StaticPlug<T>* p = new C_StaticPlug<T>(m_dataType, m_pOnChangedCallback);
	if (p)
	{
		*p = *this;
	}
	return p;
}

template<class T>
void C_StaticPlug<T>::Clear()
{
	ClearThis();
	C_NullPlug::Clear();
}

template<class T>
void C_StaticPlug<T>::OneTimeInit()
{
}

template<class T>
void C_StaticPlug<T>::ClearThis()
{
}

template<class T>
void* C_StaticPlug<T>::operator new (size_t size)
{
	return C_MemoryManager::PoolAllocate((U32)size);
}

template<class T>
void C_StaticPlug<T>::operator delete(void* p)
{
	C_MemoryManager::PoolFree(sizeof(C_StaticPlug<T>), p);
}
