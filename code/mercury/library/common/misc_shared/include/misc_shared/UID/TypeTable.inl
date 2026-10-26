namespace UID
{

template <class T_Entry, size_t t_Size>
inline C_TypeTableIteratorBase<T_Entry, t_Size>::C_TypeTableIteratorBase()
	: m_pEntry(NULL)
	, m_Hash(t_Size)
	, m_ppTable(NULL) 
{
}

template <class T_Entry, size_t t_Size>
inline C_TypeTableIteratorBase<T_Entry, t_Size>::C_TypeTableIteratorBase(const C_TypeTableIteratorBase<T_Entry, t_Size>& copy)
	: m_pEntry(copy.m_pEntry)
	, m_Hash(copy.m_Hash)
	, m_ppTable(copy.m_ppTable) 
{
}

template <class T_Entry, size_t t_Size>
inline C_TypeTableIteratorBase<T_Entry, t_Size>::C_TypeTableIteratorBase(T_Entry* pEntry, size_t hash, T_Entry* const* ppTable)
	: m_pEntry(pEntry)
	, m_Hash(hash)
	, m_ppTable(ppTable) 
{
	BIZ_ASSERT(hash <= t_Size); 
	BIZ_ASSERT((pEntry == NULL) == (hash == t_Size)); 
	BIZ_ASSERT((pEntry == NULL) || (ppTable != NULL));
}

template <class T_Entry, size_t t_Size>
inline C_TypeTableIteratorBase<T_Entry, t_Size>::~C_TypeTableIteratorBase() 
{
}

template <class T_Entry, size_t t_Size>
inline C_TypeTableIteratorBase<T_Entry, t_Size>& C_TypeTableIteratorBase<T_Entry, t_Size>::operator = (const C_TypeTableIteratorBase<T_Entry, t_Size>& copy) 
{
	m_pEntry = copy.m_pEntry; 
	m_Hash = copy.m_Hash; 
	m_ppTable = copy.m_ppTable; 
	return *this;
}

template <class T_Entry, size_t t_Size>
inline bool C_TypeTableIteratorBase<T_Entry, t_Size>::operator == (const C_TypeTableIteratorBase<T_Entry, t_Size>& other) const 
{
	return (m_pEntry == other.m_pEntry);
}

template <class T_Entry, size_t t_Size>
inline bool C_TypeTableIteratorBase<T_Entry, t_Size>::operator != (const C_TypeTableIteratorBase<T_Entry, t_Size>& other) const 
{
	return (m_pEntry != other.m_pEntry);
}

template <class T_Entry, size_t t_Size>
inline T_Entry& C_TypeTableIteratorBase<T_Entry, t_Size>::GetEntry() const 
{
	BIZ_ASSERT(m_pEntry != NULL); 
	return *m_pEntry;
}

template <class T_Entry, size_t t_Size>
inline C_TypeTableIteratorBase<T_Entry, t_Size>& C_TypeTableIteratorBase<T_Entry, t_Size>::operator ++ ()
{
	if (m_pEntry != NULL)
	{
		m_pEntry = m_pEntry->m_pNext;

		while (m_pEntry == NULL)
		{
			++m_Hash;

			if (m_Hash == t_Size)
				break;

			m_pEntry = m_ppTable[m_Hash];
		}
	}

	return *this;
}

template <class T_Entry, size_t t_Size>
inline C_TypeTableIteratorBase<T_Entry, t_Size>& C_TypeTableIteratorBase<T_Entry, t_Size>::operator += (size_t count)
{
	for (; count > 0; --count)
		++(*this);

	return *this;
}





template <class T_Entry, size_t t_Size>
inline const typename C_TypeTableIterator<T_Entry, t_Size>::Iterator C_TypeTableIterator<T_Entry, t_Size>::operator ++ (int)
{
	const Iterator copy(*this); 
	++(*this); 
	return copy;
}

template <class T_Entry, size_t t_Size>
inline const typename C_TypeTableIterator<T_Entry, t_Size>::Iterator C_TypeTableIterator<T_Entry, t_Size>::operator + (size_t count) const
{
	Iterator result;
	result += count;
	return result;
}





template <class T_Entry, size_t t_Size>
inline const typename C_TypeTableIteratorConst<T_Entry, t_Size>::ConstIterator C_TypeTableIteratorConst<T_Entry, t_Size>::operator ++ (int)
{
	const ConstIterator copy(*this); 
	++(*this); 
	return copy;
}

template <class T_Entry, size_t t_Size>
inline const typename C_TypeTableIteratorConst<T_Entry, t_Size>::ConstIterator C_TypeTableIteratorConst<T_Entry, t_Size>::operator + (size_t count) const
{
	ConstIterator result;
	result += count;
	return result;
}




template <typename T_UID, size_t t_HashSize>
inline size_t C_TypeTableHashPolicyDefault<T_UID, t_HashSize>::CalcHash(const T_UID uid)
{
	return static_cast<size_t>(uid) % t_HashSize;
}




template <typename T_UID, size_t t_HashSize>
inline size_t C_TypeTableHashPolicyHi20<T_UID, t_HashSize>::CalcHash(const T_UID uid)
{
	StaticAssert(t_HashSize <= (1 << 12), HI20_UIDs_can_only_generate_at_most_2_to_the_12_unique_values);
	return static_cast<size_t>(uid ^ (uid >> 12) ^ (uid >> 22)) % t_HashSize;
}




template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator>
C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::C_TypeTable()
	: m_Allocator()
	, m_Size(0)
{
	memset(m_pTable, 0, sizeof(m_pTable));
}

template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator>
C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::~C_TypeTable()
{
	Clear();
}

template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator>
void C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::Clear()
{
	for (unsigned int hash = 0; hash < kHashSize; ++hash)
	{
		C_Entry* pEntry = m_pTable[hash];
		
		while (pEntry != NULL)
		{
			C_Entry* pNext = pEntry->m_pNext;
			m_Allocator.Free(pEntry);
			pEntry = pNext;
			BIZ_ASSERT(m_Size > 0);
			--m_Size;
		}
	}

	BIZ_ASSERT(m_Size == 0);
	memset(m_pTable, 0, sizeof(m_pTable));
}

template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator>
typename C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::InsertionResult C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::Insert(C_UID uid)
{
	const size_t hash = T_HashPolicy::CalcHash(uid);
	BIZ_ASSERT(hash < kHashSize);

	C_Entry* pEntry = NULL;
	// check if this hash already exists
	for (pEntry = m_pTable[hash]; pEntry != NULL; pEntry = pEntry->m_pNext)
	{
		if (pEntry->m_Uid == uid)
		{
			return InsertionResult(false, Iterator(pEntry, hash, m_pTable));
		}
	}

	// create new entry and insert into hash table
	pEntry = m_Allocator.Allocate();

	pEntry->m_Uid = uid;
	pEntry->m_pNext = m_pTable[hash];

	m_pTable[hash] = pEntry;
	++m_Size;
	
	return InsertionResult(true, Iterator(pEntry, hash, m_pTable));
}

template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator>
const typename C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::Iterator C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::Erase(const Iterator& it)
{
	const Iterator end = End();

	if (it != end)
	{
		const T_UID uid = it.GetUID();
		const unsigned int hash = T_HashPolicy::CalcHash(uid);
		BIZ_ASSERT(hash < kHashSize);

		C_Entry** ppNext = &m_pTable[hash];
		
		while (*ppNext != NULL)
		{
			C_Entry* pEntry = *ppNext;

			if (pEntry->m_Uid == uid)
			{
				Iterator next = it;
				++next;

				*ppNext = pEntry->m_pNext;
				--m_Size;
				
				m_Allocator.Free(pEntry);

				return next;
			}
			else
			{
				ppNext = &pEntry->m_pNext;
			}
		}
	}

	return end;
}

template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator>
const typename C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::Iterator C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::BeginImpl() const
{
	for (unsigned int hash = 0; hash < kHashSize; ++hash)
	{
		C_Entry* pEntry = m_pTable[hash];

		if (pEntry != NULL)
		{
			return Iterator(pEntry, hash, m_pTable);
		}
	}

	return EndImpl();
}

template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator>
const typename C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::Iterator C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::EndImpl() const
{
	return Iterator(NULL, kHashSize, m_pTable);
}

template <typename T_UID, typename T_Data, class T_HashPolicy, class T_Allocator>
const typename C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::Iterator C_TypeTable<T_UID, T_Data, T_HashPolicy, T_Allocator>::FindImpl(C_UID uid) const
{
	const size_t hash = T_HashPolicy::CalcHash(uid);
	BIZ_ASSERT(hash < kHashSize);

	for (C_Entry* pEntry = m_pTable[hash]; pEntry != NULL; pEntry = pEntry->m_pNext)
	{
		if (pEntry->m_Uid == uid)
		{
			return Iterator(pEntry, hash, m_pTable);
		}
	}

	return EndImpl();
}

} // namespace UID
