// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline
bdMatrix4::bdMatrix4()
{
	for (bdUInt32 i = 0; i < 16; i++)
	{
		m_matrix[i] = 0;
	}
}

inline
bdMatrix4::bdMatrix4(bdFloat32 *const matrix)
{
	for (bdUInt32 i = 0; i < 16; i++)
	{
		m_matrix[i] = matrix[i];
	}
}

inline
void bdMatrix4::operator= (const bdMatrix4 &other)
{
	for (bdUInt32 i = 0; i < 16; i++)
	{
		m_matrix[i] = other.m_matrix[i];
	}
}

inline
void bdMatrix4::operator= (const bdFloat32 *other)
{
	for (bdUInt32 i = 0; i < 16; i++)
	{
		m_matrix[i] = other[i];
	}
}

inline
bdFloat32& bdMatrix4::operator[] (const bdUInt32 i)
{
	BD_ASSERT(i < 16, "bdMatrix4::set index is out of range.");

	return m_matrix[i];
}

inline
const bdFloat32* bdMatrix4::getData() const
{
	return m_matrix;
}

inline
void bdMatrix4::set(bdFloat32 *const matrix)
{
	for (bdUInt32 i = 0; i < 16; i++)
	{
		m_matrix[i] = matrix[i];
	}
}

inline
void bdMatrix4::get(bdFloat32 *const matrix) const
{
	for (bdUInt32 i = 0; i < 16; i++)
	{
		matrix[i] = m_matrix[i];
	}
}

