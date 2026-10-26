#include "AudioPch.h"
#include "SystemHeaders.h"
//#include <xtl.h>

//using namespace Audio;
/*
Audio::MemoryManager Memory;

////////////////////////////////////////////////////////////////////////////////
//	MemoryManager
////////////////////////////////////////////////////////////////////////////////
MemoryManager::MemoryManager()
{
    for (U32 uCount = 0; uCount < AUDIO_MEMORY_MAX_PAGES; uCount++)
        m_apPages[uCount] = NULL;

    m_apPages[0] = XPhysicalAlloc((size_t)AUDIO_MEMORY_PAGE_SIZE, MAXULONG_PTR, 0, MEM_LARGE_PAGES | PAGE_READWRITE);
    //  TODO: debug only
    ZeroMemory(m_apPages[0], (size_t)AUDIO_MEMORY_PAGE_SIZE);

    //  prime block
    U32 uSize = AUDIO_MEMORY_PAGE_SIZE - 8;
    uSize &= 0xfffffff8;    //  sanity check
    U8* pBlock = (U8*)m_apPages[0];
    U32* pSizeHeader = (U32*)pBlock;
    U32* pSizeFooter = (U32*)&pBlock[AUDIO_MEMORY_PAGE_SIZE - 4];
    *pSizeHeader = uSize;
    *pSizeFooter = uSize;

    //  put in bin
    U32 uBin = 0;
    U32 uBinSize = 8;
    while (uBinSize < uSize)
    {
        uBin++;
        uBinSize *= 2;
    }
    m_apBins[uBin] = pBlock + 4;
}

////////////////////////////////////////////////////////////////////////////////
//	Allocate
////////////////////////////////////////////////////////////////////////////////
void* MemoryManager::Allocate(U32 uSize)
{*/
/*
    //  8 byte alignment
    uSize += 7;
    uSize &= 0xfffffff8;

    //  get nearest bin
    U32 uBestBin = 0;
    U32 uBinSize = 8;
    while (uBinSize < uSize)
    {
        uBestBin++;
        uBinSize *= 2;
    }
    
    //  get 1st block in bin
    U8* pBlock = m_apBins[uBestBin];

    U32 uUseBin = uBestBin;
    //  move to next bin, if empty
    while ((pBlock == NULL) && (uUseBin < 18))
    {
        uUseBin++;
        pBlock = m_apBins[uBin];
    }

    //  out of mem
    if (uUseBin == 18)
        AUDIO_SYS_ERROR("OutOfMemory");

    //  need to split
    if (uUseBin != uBestBin)
    {
    }
*/

/*
    return malloc(uSize);
}

////////////////////////////////////////////////////////////////////////////////
//	Allocate
////////////////////////////////////////////////////////////////////////////////
void MemoryManager::Set(void* pData, U32 uValue, U32 uSize)
{
    memset(pData, uValue, uSize);
}

////////////////////////////////////////////////////////////////////////////////
//	Allocate
////////////////////////////////////////////////////////////////////////////////
void MemoryManager::Zero(void* pData, U32 uSize)
{
    memset(pData, 0, uSize);
}

////////////////////////////////////////////////////////////////////////////////
//	Free
////////////////////////////////////////////////////////////////////////////////
void MemoryManager::Free(void* pData)
{
    free(pData);
}
*/
