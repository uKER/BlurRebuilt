#include "stdafx.h"
#include "exception.h"
#include "block.h"
#include "datafile.h"
//#include "xmlparser.h"
#include "minmax.h"

#ifdef USE_BIZFILE
#include "../system/resource/bizfile.h"
#endif

//------------------------------------------------------------------------------
//  constructor
//------------------------------------------------------------------------------
DataFile::DataFile()
{
#ifdef USE_PC_FILE_SYSTEM
    m_pFileIn = NULL;
#endif // USE_PC_FILE_SYSTEM

#ifdef USE_BIZFILE
    m_pBizFile = new  BizFile;
	m_pBizFile->SetBuffered(true);
#endif // USE_BIZFILE
}

//------------------------------------------------------------------------------
//  destructor
//------------------------------------------------------------------------------
DataFile::~DataFile()
{
    Release();
}

//------------------------------------------------------------------------------
//  IsData
//      used to specify whether the block contents are more blocks or data
//------------------------------------------------------------------------------
bool DataFile::IsData(DWORD dwBlockID)
{
    if (dwBlockID == 'DATA')
        return true;
    else
        return false;
}

//------------------------------------------------------------------------------
//  Load
//------------------------------------------------------------------------------
bool DataFile::Load(const char* pchFilename, Block** ppBlock)
{
#ifdef THROW_EXCEPTIONS
	try
	{
#endif //THROW_EXCEPTIONS
        if (Open(pchFilename))
        {
		    *ppBlock = new  Block;
		    ReadBlock(*ppBlock);

            if (!Close())
                THROW("Closing file");
        }
        else
            THROW("Opening file");

		return true;
#ifdef THROW_EXCEPTIONS
	}
	catch (Error& err)
	{
		DBPrint("Error: %s\n" , err.What());
		Release();
		system("pause 0");
		return false;
	}
#endif //THROW_EXCEPTIONS
}

//------------------------------------------------------------------------------
//  ReadBlock
//------------------------------------------------------------------------------
void DataFile::ReadBlock(Block* pBlock)
{
	DWORD dwBlockID = 0;
	int iSize = 0;

	//	read block id
	if (ReadBytes(&dwBlockID, 4) < 1)
		THROW("Read past end of file reading block ID");

	pBlock->SetID(dwBlockID);

	//	read block size
	if (ReadBytes(&iSize, 4) < 1)
		THROW("Read past end of file reading block size");

	//	subtract block header size
	iSize -= 8;

//    _ASSERTE(_CrtCheckMemory());

	//	if not skipping this block type
	if (DoBlock(dwBlockID))
	{
//        if (dwBlockID & 0x7fffff00)
//            int z= 0;

		//	if data (attributes, text, etc.)
        if (IsData(dwBlockID))
        {
			ReadBlockData(pBlock, iSize);
        }
		//	else child block
		else
		{
			//	not skipping children
			if (DoChildren(dwBlockID))
			{
				if (dwBlockID == 0x80000012)
					int z=0;

				while (iSize > 0)
				{
					//	create child block
					Block* pChild = new  Block(pBlock);

					assert(pChild);

					pBlock->AddChild(pChild);

					//	read child block
					ReadBlock(pChild);

					//	any remaining children?
					iSize -= pChild->GetBlockSize();
				}
			}
		}
    }
	//	skipping this block
	else 
	{
        if (SkipBytes(iSize) == false)
            THROW("Read past end of file reading block ID");
	}

}

//------------------------------------------------------------------------------
//  ReadBlockData
//	called when a DATA block is found within the data file with the current
//	block and size of data segment
//	parent blocks are navigable thru pBlock->GetParent()
//------------------------------------------------------------------------------
void DataFile::ReadBlockData(Block* pBlock, int iSize)
{
	if (true)
	{
		assert(iSize > 0);
		void* pData = new  BYTE[iSize];
	    
		//	read data
		if (ReadBytes(pData, iSize) < 1)
		{
			delete [] pData;
			pData = NULL;
			THROW("Read past end of file reading block data");
		}

		//	set block data
		pBlock->SetData(pData, iSize);

		delete [] pData;
	}
	else
	{
		//	skip data block
		SkipBytes(iSize);
	}
}

//------------------------------------------------------------------------------
//  Save
//------------------------------------------------------------------------------
bool DataFile::Save(const char* pchFilename, Block* pBlock)
{
#ifdef USE_PC_FILE_SYSTEM
	FILE* fileOut;

	if (fileOut = fopen(pchFilename, "w+b"))
    {
	    WriteBlock(fileOut, pBlock);
    }
	else
		return false;

	fclose(fileOut);

	return true;
#else
    return false;
#endif // USE_PC_FILE_SYSTEM

}

//------------------------------------------------------------------------------
//  WriteBlock
//------------------------------------------------------------------------------
void DataFile::WriteBlock(FILE* pOutFile, Block* pBlock)
{
#ifdef USE_PC_FILE_SYSTEM
//    DWORD dwTag = DoEndian((U32)pBlock->GetID());
    DWORD dwTag = (U32)pBlock->GetID();
    DWORD dwSize = DoEndian((U32)pBlock->GetBlockSize());
    void* pData = pBlock->GetData();
    DWORD dwDataSize = pBlock->GetDataSize();

	fwrite (&dwTag, 1, 4, pOutFile);
	fwrite (&dwSize, 1, 4, pOutFile);

	if (pData)
		fwrite (pData, 1, dwDataSize, pOutFile);

	//	do children
	for (Audio::Iterator<Block*> i = pBlock->GetChildren().first(); !i.isPastEnd(); i.advance())
	{
        Block* pChild = i.retrieve();
		WriteBlock(pOutFile, pChild);
	}
#endif // USE_PC_FILE_SYSTEM
}

//------------------------------------------------------------------------------
//  Show
//------------------------------------------------------------------------------
void DataFile::Show(Block* pBlock, DWORD uIndentation)
{
    DWORD dwTag = pBlock->GetID();
    DWORD uCount = uIndentation;
    char* chTag = (char*)&dwTag;

	U32 iLen = uIndentation * 3;
	char* pchIndent = new  char[iLen + 1];
	memset(pchIndent, ' ', iLen);
	pchIndent[iLen] = '\0';

    DBPrint("%s%c%c%c%c (%d bytes)", pchIndent, chTag[0], chTag[1], chTag[2], chTag[3], pBlock->GetBlockSize());

	delete [] pchIndent;

	//	do children
	for (Audio::Iterator<Block*> i = pBlock->GetChildren().first(); !i.isPastEnd(); i.advance())
	{
		Block* pChild = i.retrieve();
        Show(pChild, uIndentation + 1);
	}
}

//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------
void DataFile::Release()
{
#ifdef USE_PC_FILE_SYSTEM
	if (m_pFileIn)
	{
		fclose(m_pFileIn);
		m_pFileIn = NULL;
	}
#endif // USE_PC_FILE_SYSTEM

#ifdef USE_BIZFILE
    if (m_pBizFile)
    {
        delete m_pBizFile;
        m_pBizFile = NULL;
    }
#endif // USE_BIZFILE
}

#ifdef USE_PC_FILE_SYSTEM

//------------------------------------------------------------------------------
//  Open
//------------------------------------------------------------------------------
bool DataFile::Open(const char* pchFilename)
{
    Close();
    m_pFileIn = fopen(pchFilename, "rb");
    return (m_pFileIn != NULL);
}

//------------------------------------------------------------------------------
//  Read
//------------------------------------------------------------------------------
int DataFile::ReadBytes(void* pData, int nBytes)
{
    return (int)fread(pData, nBytes, 1, m_pFileIn);
}

//------------------------------------------------------------------------------
//  SkipBytes
//------------------------------------------------------------------------------
bool DataFile::SkipBytes(int nBytes)
{
    return (fseek(m_pFileIn, nBytes, ftell(m_pFileIn)) == 0);
}

//------------------------------------------------------------------------------
//  Close
//------------------------------------------------------------------------------
bool DataFile::Close()
{
    int iRet = 0;
    if (m_pFileIn)
    {
		int iRet = fclose(m_pFileIn);
		m_pFileIn = NULL;
    }
    return (iRet != EOF);
}

#endif // USE_PC_FILE_SYSTEM

#ifdef USE_BIZFILE

//------------------------------------------------------------------------------
//  Open
//------------------------------------------------------------------------------
bool DataFile::Open(const char* pchFilename)
{
    return m_pBizFile->OpenDD(pchFilename, true);
}

//------------------------------------------------------------------------------
//  Read
//------------------------------------------------------------------------------
int DataFile::ReadBytes(void* pData, int nBytes)
{
	U32 bytesRead;
    return m_pBizFile->ReadFile(pData, nBytes, bytesRead);
}

//------------------------------------------------------------------------------
//  SkipBytes
//------------------------------------------------------------------------------
bool DataFile::SkipBytes(int nBytes)
{
    return m_pBizFile->SkipBytes(nBytes);
}

//------------------------------------------------------------------------------
//  Close
//------------------------------------------------------------------------------
bool DataFile::Close()
{
    m_pBizFile->Close();
    return true;
}

#endif // USE_BIZFILE

