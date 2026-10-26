#pragma once

#ifndef BYTE
typedef unsigned char BYTE;
#endif	//	BYTE

#define MEMORY_BLOCK_DATA_SIZE 4096

class MemoryBlock
{
public:
	MemoryBlock()
	{
		pNextAlloc = pData;
		pNextBlock = NULL;
	}
public:
	BYTE pData[MEMORY_BLOCK_DATA_SIZE];
	BYTE* pNextAlloc;
	MemoryBlock* pNextBlock;
};

class MemoryManager
{
public:
	MemoryManager()
	{
		m_iNumBlocks = 0;
		m_iNumAllocs = 0;
		m_pFirst = NULL;
		m_pCurr = NULL;
	}
	~MemoryManager()
	{
		Release();
	}

	BYTE* Alloc(int iSize)
	{
		int iAlign = (iSize + 3) & 0xfffffffc;

		if (m_pFirst == NULL)
		{
			m_pFirst = new MemoryBlock;
			m_pCurr = m_pFirst;
		}

		int iRem = iAlign + ((int)m_pCurr->pNextAlloc - (int)m_pCurr->pData);
		if (iRem > MEMORY_BLOCK_DATA_SIZE)
		{
			//	no room in curr block
			MemoryBlock* pNew = new MemoryBlock;
			m_pCurr->pNextBlock = pNew;
			m_pCurr = pNew;

			m_iNumBlocks++;
		}

		ZeroMemory(m_pCurr->pNextAlloc, iAlign);
		BYTE* pAlloc = m_pCurr->pNextAlloc;
		m_pCurr->pNextAlloc += iAlign;

		m_iNumAllocs++;

		return pAlloc;
	}

	void Free(BYTE* pData)
	{
		m_iNumAllocs--;
	}

	void Release()
	{
		if (m_iNumAllocs)
		{
			OutputDebugString("[ERROR] Memory still allocated");
			exit(0);
		}
		MemoryBlock* pCurr = m_pFirst;

		while (pCurr)
		{
			MemoryBlock* pNext = pCurr->pNextBlock;
			delete pCurr;
			pCurr = pNext;
		}
	}

private:
	int m_iNumBlocks;
	int m_iNumAllocs;

	MemoryBlock* m_pFirst;
	MemoryBlock* m_pCurr;
};


