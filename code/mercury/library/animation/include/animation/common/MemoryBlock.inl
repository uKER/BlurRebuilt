#ifndef ANIMATION_MEMORYBLOCK_INL
#define ANIMATION_MEMORYBLOCK_INL

namespace Animation
{

	//
	// C_MemoryBlock
	//
	inline C_MemoryBlock::C_MemoryBlock():
			m_data(0),
			m_size(0),
			m_capacity(0)
	{
	}
	
	inline C_MemoryBlock::~C_MemoryBlock()
	{
		Destroy();
	}
	
	inline U32 C_MemoryBlock::GetSize() const
	{
		return m_size;
	}
	
	#ifndef PLATFORM_SPU
	inline void C_MemoryBlock::SpuFetchAsync( U32 tag ) const {}
	inline void C_MemoryBlock::SpuFetch() const {}
	inline void C_MemoryBlock::SpuPartialFetchAsync( U32 size, U32 tag ) const {}
	inline void C_MemoryBlock::SpuPartialFetch( U32 size ) const {}
	inline void C_MemoryBlock::SpuStoreAsync( U32 tag ) const {}
	inline void C_MemoryBlock::SpuStore() const {}
	#endif
}


#endif
