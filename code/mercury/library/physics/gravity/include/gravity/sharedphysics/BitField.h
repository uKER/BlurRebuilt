#ifndef GRAVITY_BITFIELD_H
#define GRAVITY_BITFIELD_H

namespace SharedPhysics
{
	class C_BitField
	{
		U32* m_pBits;
		U32 m_Size;
		bool m_ExternallyOwned;
	public:
		C_BitField(const U32 numBits) : m_Size((numBits >> 5) + 1), m_ExternallyOwned(false)
		{
			m_pBits = new U32[m_Size];
		}

		C_BitField(U32* bits, const U32 size) : m_pBits(bits), m_Size(size), m_ExternallyOwned(true)
		{
		}

		~C_BitField()
		{
#ifndef PLATFORM_SPU
			if(!m_ExternallyOwned)
				delete [] m_pBits;
#endif
			m_pBits = NULL;
		}

		void ClearAllBits()
		{
			for(U32 a = 0; a < m_Size; ++a)
			{
				m_pBits[a] = 0;
			}
		}

		void SetBit(const U32& bit)
		{
			U32 index = (bit >> 5);
			U32 bitIndex = bit & 0x1f;
			m_pBits[index] |= (1 << bitIndex);
		}

		bool TestBit(const U32& bit) const
		{
			U32 index = (bit >> 5);
			U32 bitIndex = bit & 0x1f;
			U32 res = (m_pBits[index] & (1 << bitIndex));
			return res != 0;
		}

		bool TestAndSetBit(const U32& bit)
		{
			U32 index = (bit >> 5);
			U32 bitIndex = (1 << (bit & 0x1f));
			U32 res = (m_pBits[index] & bitIndex);
			m_pBits[index] |= bitIndex;
			return res != 0;
		}

	};
}

#endif //GRAVITY_BITFIELD_H