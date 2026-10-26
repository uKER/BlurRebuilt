template<class T>
C_ProxyPlug<T>::C_ProxyPlug(
	E_GenericAttributeType type, 
	T(*pOnValueQueryCallback)(const void* pThis, bool evaluate)) 
: C_NullPlug(0)
{
	m_dataType = type;
	m_pOnValueQueryCallback = pOnValueQueryCallback;
}

template<class T>
C_ProxyPlug<T>::~C_ProxyPlug()
{
}

template<class T>
const C_ProxyPlug<T>& C_ProxyPlug<T>::operator=(const C_ProxyPlug<T> &rhs)
{
	C_NullPlug::operator =(rhs);

	return *this;
}

template<class T>
bool C_ProxyPlug<T>::operator==(const C_ProxyPlug<T> &rhs) const
{
	return IsEqualToAll(rhs);
}

template<class T>
bool C_ProxyPlug<T>::operator!= (const C_ProxyPlug<T> &rhs) const
{
	return !((*this) == rhs);
}

template<class T>
bool C_ProxyPlug<T>::operator<(const C_ProxyPlug<T> &rhs) const
{
	if (C_NullPlug::operator <(rhs))
		return true;

	return false;
}

template<class T>
bool C_ProxyPlug<T>::GetValue(T& value, bool evaluate) const
{
	AFTK_ASSERT(m_pOnValueQueryCallback);
	if (!m_pOnValueQueryCallback)
		return false;
	value = m_pOnValueQueryCallback(this, evaluate);
	return true;
}

template<class T>
bool C_ProxyPlug<T>::IsNull() const
{
	return false;
}

template<class T>
E_GenericAttributeType C_ProxyPlug<T>::GetDataType() const
{
	return m_dataType;
}

template<class T>
bool C_ProxyPlug<T>::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

template<class T>
bool C_ProxyPlug<T>::IsEqualToThis(const I_Object& _rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_ProxyPlug))
		return false;

	return true;
}

template<class T>
bool C_ProxyPlug<T>::IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (!IsEqualToThis(rhs, flags))
		return false;

	return C_NullPlug::IsEqualTo(rhs, classToAbortTest, flags);
}


template<class T>
U32 C_ProxyPlug<T>::GetHash() const
{
	return 0;
}

template<class T>
E_ObjType C_ProxyPlug<T>::GetType() const
{
	return Aftk::E_ObjType_ProxyPlug;
}

template<class T>
bool C_ProxyPlug<T>::HasType(E_ObjType type) const
{
	if (C_NullPlug::HasType(type))
		return true;
	if (type == C_ProxyPlug<T>::GetType()) 
		return true;

	return false;
}

template<class T>
I_Plug* C_ProxyPlug<T>::Clone() const
{
	return new C_ProxyPlug<T>(*this);
}

template<class T>
void C_ProxyPlug<T>::Clear()
{
	ClearThis();
	C_NullPlug::Clear();
}

template<class T>
void C_ProxyPlug<T>::OneTimeInit()
{
}

template<class T>
void C_ProxyPlug<T>::ClearThis()
{
}

template<class T>
void* C_ProxyPlug<T>::operator new (size_t size)
{
	return C_MemoryManager::PoolAllocate((U32)size);
}

template<class T>
void C_ProxyPlug<T>::operator delete(void* p)
{
	C_MemoryManager::PoolFree(sizeof(C_ProxyPlug<T>), p);
}


