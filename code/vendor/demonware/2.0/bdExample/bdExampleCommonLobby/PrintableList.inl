// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

template <typename T, bdUInt N>
PrintableList<T, N>::PrintableList()
: m_size(0)
{
}

template <typename T, bdUInt N>
PrintableList<T, N>::~PrintableList()
{
}


template <typename T, bdUInt N>
bdBool PrintableList<T, N>::add(const T& entry)
{
	const bdBool added = (m_size < N);
	if(added)
	{
		m_entries[m_size] = entry;
		m_size++;
	}
	return added;
}

template <typename T, bdUInt N>
void PrintableList<T, N>::clear()
{
	m_size = 0;
}

template <typename T, bdUInt N>
bdUInt PrintableList<T, N>::getSize() const
{
	return m_size;
}

template <typename T, bdUInt N>
T& PrintableList<T, N>::operator[] (const bdUInt i)
{
	BD_ASSERT((i < N) && (i < m_size), "Rangecheck failed");
	return m_entries[i];
}

template <typename T, bdUInt N>
void PrintableList<T, N>::print(bdString& string) const
{
	if(m_size > 0)
	{
		for(bdUInt i = 0; i < m_size; i++)
		{
			bdNChar8 buf[128];
			printInfo(m_entries[i], buf, sizeof(buf));

			string += "\n";
			string += buf;			
		}
	}
	else
	{
		string += "-- empty --";
	}
}
