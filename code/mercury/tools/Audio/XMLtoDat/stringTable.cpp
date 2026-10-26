#include "AudioPch.h"
#include "string.h"
#include "stringTable.h"


//------------------------------------------------------------------------------
//  FoundText
//------------------------------------------------------------------------------
int Audio::StringTable::AddString(const char* pchString)
{
	for (int index = 0; index != m_vStrings.Size(); index++)
	{
        const char* strEntry = m_vStrings[index];

        if (strcmp(pchString, strEntry) == 0)
            return index;
    }

	// [Rog] We now only allocate it if we need to, rather than before the check :)
    char* pstrString = new  char[strlen(pchString)+1];
    strcpy(pstrString, pchString);

    m_vStrings.PushBack(pstrString);

    m_iLength += (int)strlen(pstrString) + 1;

    return (int)m_vStrings.Size()-1;
}

//------------------------------------------------------------------------------
//  Create
//------------------------------------------------------------------------------

// [Rog] This function is unsafe due to it's buffer usage and as such generates Prefix errors. It's not needed in-game, so if it's need in the tool chain
// it needs to be wrapped with something so that it only gets included in the tool-chain. It's not to come back into the game else it'll just generate more bugs

void Audio::StringTable::Create(void* pStrings, U32 uBufferLength)
{
    char* pchCurr = (char*)pStrings;
	U32 uBytesWritten = 0;

	for (int i = 0; i != m_vStrings.Size(); ++i)
	{
//		std::string* pEntry = m_vStrings[i];
		char* pEntry = m_vStrings[i];

		assert (strlen(m_vStrings[i]) < uBufferLength - uBytesWritten);
     
        strcpy(pchCurr, pEntry);
		uBytesWritten += (U32)(strlen(pchCurr));

        pchCurr += (int)strlen(pEntry) + 1;
	}
}

//------------------------------------------------------------------------------
//  GetString
//------------------------------------------------------------------------------
const char* Audio::StringTable::GetString(int iOffset) const
{
    return m_vStrings[iOffset];
}

//------------------------------------------------------------------------------
//  GetString
//------------------------------------------------------------------------------
void Audio::StringTable::Populate(void* pStrings, int iSize)
{
    char* pchCurr = (char*)pStrings;
//    m_iLength = iSize;
    int iLength;

    while (iSize)
    {
        AddString(pchCurr);

        iLength = (int)strlen(pchCurr) + 1;

        iSize -= iLength;

        pchCurr += iLength;
    }
}

//------------------------------------------------------------------------------
//  GetString
//------------------------------------------------------------------------------
void Audio::StringTable::Release()
{
	for (int i = 0; i != m_vStrings.Size(); i++)
        delete m_vStrings[i];

    m_vStrings.Clear();
}






