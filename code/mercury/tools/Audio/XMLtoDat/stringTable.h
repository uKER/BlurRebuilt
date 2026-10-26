#ifndef _STRINGTABLE_H_
#define _STRINGTABLE_H_

//#include <string>
//#include "common/Vector.h"

namespace Audio
{
    //------------------------------------------------------------------------------
    //  class StringTable
    //------------------------------------------------------------------------------
    class StringTable
    {
    public:
        StringTable() { m_iLength = 0; }
        ~StringTable() { Release(); }

        int AddString(const char* pchString);

#ifdef AUDIO_PACK_DATA
        int GetLength() const { return m_iLength; }
#else
        int GetLength() const { return (m_iLength + 3) & 0xfffffffc; }
#endif // AUDIO_PACK_DATA

        void Create(void* pStrings, U32 uBufferLength);    // creates data block

        const char* GetString(int iIndex) const;

        void Populate(void* pStrings, int iSize);    // fills array vector

        void Release();

    private:
        int m_iLength;
		Audio::Core::Array<char*> m_vStrings;
    };
}

#endif // _STRINGTABLE_H_
