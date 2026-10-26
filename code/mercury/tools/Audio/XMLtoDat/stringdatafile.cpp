#include "stdafx.h"
#include "block.h"
#include "stringdatafile.h"


//------------------------------------------------------------------------------
//  IsData
//      used to specify whether the block contents are more blocks or data
//------------------------------------------------------------------------------
bool StringDataFile::IsData(DWORD dwBlockID)
{
    if ((dwBlockID == 'SRTS') ||
        (IsString(dwBlockID) && (strcmp(GetString(dwBlockID), "Data") == 0)))
        return true;
    else
        return false;
}

//------------------------------------------------------------------------------
//  Load
//------------------------------------------------------------------------------
bool StringDataFile::Load(const char* pchFilename, Block** ppBlock)
{
#ifdef THROW_EXCEPTIONS
	try
	{
#endif // THROW_EXCEPTIONS
        if (Open(pchFilename))
        {
            *ppBlock = new  Block;
            Block* pStrings = new  Block;
            Block* pData = new  Block;

	        DWORD dwBlockID = 0;
	        int iSize = 0;

	        //	read block id
	        if (ReadBytes(&dwBlockID, 4) < 1)
		        THROW("Read past end of file reading block ID");

            //  check header DWORD

	        (*ppBlock)->SetID(dwBlockID);

	        //	read file size
	        if (ReadBytes(&iSize, 4) < 1)
		        THROW("Read past end of file reading block size");

		    ReadBlock(pStrings);
            (*ppBlock)->AddChild(pStrings);

            m_StringTable.Populate(pStrings->GetData(), pStrings->GetDataSize());

		    ReadBlock(pData);
            (*ppBlock)->AddChild(pData);

            Close();
        }
        else
        {
#ifdef THROW_EXCEPTIONS
			THROW("Could not open Dat file");
#else
            return false;
#endif // THROW_EXCEPTIONS
        }

        return true;
#ifdef THROW_EXCEPTIONS
	}
	catch (Error& err)
	{
		printf("Error: %s\n" , err.What());
		Release();
		system("pause 0");
		return false;
	}
#endif // THROW_EXCEPTIONS
}

//------------------------------------------------------------------------------
//  Save
//------------------------------------------------------------------------------
bool StringDataFile::Save(const char* pchFilename, Block* pBlock)
{
	FILE* fileOut;

	if (fileOut = fopen(pchFilename, "w+b"))
    {
		WriteBlock(fileOut, pBlock);
/*
        WriteBlock(fileOut, pRoot);

        Block* pRoot = new Block;
        Block* pStrings = new Block;

        void* pStringTable = new char[m_StringTable.GetLength()];

        if (pStringTable)
        {
            m_StringTable.Create(pStringTable);
            pStrings->SetData(pStringTable, m_StringTable.GetLength());
            
            pRoot->SetID(GetFileTypeID());

            pStrings->SetID((U32)'SRTS');
            pBlock->SetID((U32)'ATAD');

            pRoot->AddChild(pStrings);
            pRoot->AddChild(pBlock);

		    WriteBlock(fileOut, pRoot);

            //  change block ptr to new root
            pBlock = pRoot;

            delete pStringTable;
        }
        else
            THROW("Out of memory");
*/
    }
	else
		return false;

	fclose(fileOut);

	return true;
}

//------------------------------------------------------------------------------
//  Show
//------------------------------------------------------------------------------
void StringDataFile::Show(Block* pBlock, DWORD uIndentation)
{
    DWORD dwTag = DoEndian((U32)pBlock->GetID());
    DWORD uCount = uIndentation;
    char* chTag = (char*)&dwTag;

	int iLen = uIndentation * 3;
	char* pchIndent = new char[iLen + 1];
	memset(pchIndent, ' ', iLen);
	pchIndent[iLen] = '\0';

    if (IsString(dwTag))
        DBPrint("%s%s ", pchIndent, GetString(dwTag));
    else
        DBPrint("%s%c%c%c%c ", pchIndent, chTag[0], chTag[1], chTag[2], chTag[3]);

	DBPrint("(%d bytes)", pBlock->GetBlockSize());

	delete [] pchIndent;

	//	do children
	for (Audio::Iterator<Block*> i = pBlock->GetChildren().first(); !i.isPastEnd(); i.advance())
	{
		Block* pChild = i.retrieve();
        Show(pChild, uIndentation + 1);
	}
}



