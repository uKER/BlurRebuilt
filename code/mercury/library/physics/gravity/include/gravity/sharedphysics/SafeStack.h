#ifndef SAFESTACK_H
#define SAFESTACK_H

namespace SharedPhysics
{

template <bool bFixedSize>
class C_SafeStack
{
public:
	void** m_pData;
	U32 m_Size;
	U32 m_OffsetValue;
	bool m_bDestruct;

	C_SafeStack(U32 size) : m_Size(size), m_OffsetValue(0), m_bDestruct(true)
	{
		m_pData = new void*[size];
	}

	C_SafeStack(void* data, const U32 size) : m_Size(size), m_OffsetValue(0), m_pData((void**)data), m_bDestruct(false)
	{
	}

	~C_SafeStack()
	{
		if(m_bDestruct)
			delete [] m_pData;
	}

	NOALIAS inline void Push(void* data, const U32 index)
	{
		const U32 ind(index + m_OffsetValue);
		if(bFixedSize)
		{
			BIZ_ASSERT(m_Size > ind);
		}
		else
		{
			void** oldData =m_pData;
			m_Size = (m_Size * 2) + 1;
			m_pData = new void*[m_Size];
			for(U32 a = 0; a < ind; ++a)
			{
				m_pData[a] = oldData[a];
			}
			delete [] oldData;
		}
		m_pData[ind] = data;
	}

	NOALIAS inline void PushU32(const U32 data, const U32 index)
	{
		const U32 ind(index + m_OffsetValue);
		if(bFixedSize)
		{
			BIZ_ASSERT(m_Size > ind);
		}
		else
		{
			void** oldData =m_pData;
			m_Size = (m_Size * 2) + 1;
			m_pData = new void*[m_Size];
			for(U32 a = 0; a < ind; ++a)
			{
				m_pData[a] = oldData[a];
			}
			delete [] oldData;
		}
		m_pData[ind] = (void*)data;
	}

	NOALIAS inline void PushOffset(U32 index)
	{
		m_OffsetValue = index;
	}

	template <class Elem>
	NOALIAS inline Elem* Pop(U32 index)
	{
		const U32 ind(index + m_OffsetValue);
		BIZ_ASSERT(m_Size > ind);
		return (Elem*)m_pData[ind];
	}

	NOALIAS inline U32 PopU32(U32 index)
	{
		const U32 ind(index + m_OffsetValue);
		BIZ_ASSERT(m_Size > ind);
		return (U32)(m_pData[ind]);
	}
};

class C_S32SafeStack
{
public:
	S32* m_pData;
	U32 m_Size;
	U32 m_OffsetValue;
	bool m_bDestruct;

	C_S32SafeStack(S32* data, U32 size) : m_pData(data), m_Size(size), m_OffsetValue(0), m_bDestruct(false)
	{
	}
#ifndef PLATFORM_SPU
	C_S32SafeStack(U32 size) : m_Size(size), m_OffsetValue(0), m_bDestruct(true)
	{
		m_pData = new S32[size];
	}
	~C_S32SafeStack()
	{
		if(m_bDestruct)
			delete [] m_pData;
	}
#endif

	NOALIAS inline void Push(S32 data, const U32 index)
	{
		const U32 ind(index + m_OffsetValue);
		BIZ_ASSERT(m_Size > ind);
		m_pData[ind] = data;
	}

	NOALIAS inline void PushOffset(U32 index)
	{
		m_OffsetValue = index;
	}

	NOALIAS inline S32 Pop(U32 index)
	{
		const U32 ind(index + m_OffsetValue);
		BIZ_ASSERT(m_Size > ind);
		return m_pData[ind];
	}
};

};

#endif
