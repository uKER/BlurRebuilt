#ifndef _MEMORYMANAGER_H_
#define _MEMORYMANAGER_H_

#define AUDIO_MEMORY_PAGE_SIZE 1024 * 1024
#define AUDIO_MEMORY_MAX_PAGES 20
#define AUDIO_MEMORY_ALIGNMENT 8
/*
namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	MemoryManager
////////////////////////////////////////////////////////////////////////////////
    class MemoryManager
    {
    public:
        MemoryManager();
        ~MemoryManager() {}

        void* Allocate(U32 uSize);
        void Set(void* pData, U32 uValue, U32 uSize);
        void Zero(void* pData, U32 uSize);
        void Free(void* pData);

    private:
        void* m_apPages[AUDIO_MEMORY_MAX_PAGES];
        void* m_apBins[18];
    };

    static MemoryManager GlobalMemory;

    inline void* operator new(size_t sz)
    {
        return GlobalMemory.Allocate(sz);
    }

    inline void operator delete(void* ptr)
    {
        GlobalMemory.Free(ptr);
    }
}
*/
#endif // _MEMORYMANAGER_H_

