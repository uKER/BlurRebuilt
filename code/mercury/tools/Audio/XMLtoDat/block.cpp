#include "stdafx.h"
#include "block.h"

//------------------------------------------------------------------------------
//  constructor
//------------------------------------------------------------------------------
Block::Block(Block* pParent)
{
    m_dwID = 'LLUN';
    m_pData = 0;
    m_dwDataSize = 0;
    m_pParent = pParent;
}

//------------------------------------------------------------------------------
//  destructor
//------------------------------------------------------------------------------
Block::~Block()
{
	//	delete children
	for (Audio::Iterator<Block*> i = GetChildren().first(); !i.isPastEnd(); i.advance())
        delete i.retrieve();

    GetChildren().makeEmpty();

	//	delete data
	if (m_pData)
		delete [] m_pData;
}

//------------------------------------------------------------------------------
//  Make
//------------------------------------------------------------------------------
void Block::AddChild(Block* pBlock)
{
    if (pBlock == 0)
	    GetChildren().insert(pBlock, GetChildren().last());
    else
	    GetChildren().insert(pBlock, GetChildren().last());
}

//------------------------------------------------------------------------------
//  Make
//------------------------------------------------------------------------------
void Block::SetData(const void* pData, DWORD dwDataSize)
{
	//	delete previous, if necc
	if (m_pData)
		delete [] m_pData;

	m_pData = new  BYTE[dwDataSize];
	memcpy(m_pData, pData, dwDataSize);
	m_dwDataSize = dwDataSize;
}

//------------------------------------------------------------------------------
//  GetBlockSize
//------------------------------------------------------------------------------
DWORD Block::GetBlockSize()	
{
    DWORD dwSize = 8 + GetDataSize();

    for (Audio::Iterator<Block*> i = GetChildren().first(); !i.isPastEnd(); i.advance())
	{
        Block* pBlock = i.retrieve();
        dwSize += pBlock->GetBlockSize();
	}

	return dwSize;
}

