#include "stdafx.h"
#include <animation/common/MemoryBlock.h>
#include <system/Scheduler.h>

namespace Animation
{

C_MemoryBlock::C_MemoryBlock( U32 size ):
	m_data(NULL),
	m_size(0),
	m_capacity(0)
{
	Create( size );
}

#ifdef PLATFORM_SPU
void C_MemoryBlock::SpuFetchAsync( U32 tag ) const
{
	BIZ_ASSERT(!IsInPlace());
	C_MemoryBlock* nonConstThis = const_cast<C_MemoryBlock*>(this);
	Spu::FetchAllocAsync<U8>(nonConstThis->m_data, m_size, tag);
}

void C_MemoryBlock::SpuFetch() const
{
	SpuFetchAsync( 0 );
	Spu::Wait( 0 );
}

void C_MemoryBlock::SpuPartialFetchAsync( U32 numBytes, U32 tag ) const
{
	BIZ_ASSERT(numBytes <= m_size);
	C_MemoryBlock* nonConstThis = const_cast<C_MemoryBlock*>(this);
	Spu::FetchAllocAsync<U8>(nonConstThis->m_data, numBytes, tag);
}

void C_MemoryBlock::SpuPartialFetch( U32 numBytes ) const
{
	SpuPartialFetchAsync(numBytes, 0);
	Spu::Wait(0);
}

void C_MemoryBlock::SpuStoreAsync( U32 tag ) const
{
	BIZ_ASSERT(!IsInPlace());
	Spu::StoreAsync<U8>( m_data, m_size, tag );
}

void C_MemoryBlock::SpuStore() const
{
	SpuStoreAsync( 0 );
	Spu::Wait( 0 );
}

#endif // PLATFORM_SPU

void C_MemoryBlock::Create( const C_MemoryBlockSizer& sizer )
{
	Create(sizer.GetSize());
}

void C_MemoryBlock::Create( U32 size )
{
	const U32 kAlign = 16;
	U32 alignedSize = ( ( size + kAlign - 1 ) & ~( kAlign - 1 ) );

	if (alignedSize > m_capacity)
	{
		Destroy();
	
	#ifdef PLATFORM_SPU
		m_data = (U8*) Spu::Alloc( alignedSize );
	#else
		m_data = (U8*) BizAlignedMalloc( alignedSize, kAlign );
	#endif

		BIZ_ASSERT(0 == (((U32)(m_data.RawPtr())) & (kAlign - 1)));

		m_capacity = alignedSize;
	}
	m_size = alignedSize;
}

void C_MemoryBlock::CreateInPlace(U8* data, U32 size)
{
	Destroy();
	m_data = data;
	m_size = size;
	m_capacity = 0;
	BIZ_ASSERT(IsInPlace());
}

void C_MemoryBlock::Resize( U32 newSize )
{
	const U32 kAlign = 16;
	U32 alignedSize = ( ( newSize + kAlign - 1 ) & ~( kAlign - 1 ) );

	if (alignedSize > m_capacity)
	{
	#ifdef PLATFORM_SPU
		BIZ_ASSERT(alignedSize <= m_capacity);
	#else
		if (!IsInPlace())
		{
			U8* oldData = m_data;
			m_data = (U8*) BizAlignedMalloc( alignedSize, kAlign );
			BIZ_ASSERT(0 == (((U32)(m_data.RawPtr())) & (kAlign - 1)));

			if (oldData && m_capacity > 0)
			{
				memcpy(m_data, oldData, m_size);
			}
			m_capacity = alignedSize;
			
			BizAlignedFree(oldData);
		}
		else
		{
			BIZ_ASSERT(IsInPlace());
		}

	#endif
	}
	m_size = alignedSize;
}


void C_MemoryBlock::Destroy()
{
#ifndef PLATFORM_SPU
	if (m_data && !IsInPlace())
	{
		BizAlignedFree(m_data);
	}
#endif
	m_data = 0;
	m_size = 0;
	m_capacity = 0;
}

} // namespace Animation
