#ifndef _STATIC_BUFFER_ALLOCATOR_H_
#define _STATIC_BUFFER_ALLOCATOR_H_


namespace Audio
{
	class StaticBufferAllocator
	{
	public:
		StaticBufferAllocator();
		~StaticBufferAllocator();

		bool		Init(const S32 s32BufferSize, const S32 s32MaxNumBuffers, const S32 s32Alignment = 16);
		void*		GetBuffer(const S32 s32Size);
		S32			GetBufferSize() { return m_s32BufferSize; }
		bool		FreeBuffer(const void* pBuffer);
		bool		FreeAllBuffers();

	private:
		Core::Lock	m_Lock;
		void*		m_pBufferMemory;
		bool*		m_pbBufferInUse;
		S32			m_s32BufferSize;
		S32			m_s32NumBuffers;
		bool		m_bInited;
	};
};

#endif	//_STATIC_BUFFER_ALLOCATOR_H_
