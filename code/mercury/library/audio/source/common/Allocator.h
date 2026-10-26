#ifndef _AALLOCATOR_H_
#define _AALLOCATOR_H_

#pragma once

#if defined(PLATFORM_PC)
#include "Windows.h"
#endif


#if defined(PLATFORM_PS3)
#include <stdio.h>
#endif

#include "Types.h"

namespace Audio
{
//------------------------------------------------------------------------------
//  IndexedAllocator
//		One use (cannot delete individual items) allocator for creating blobs.
//		Returns an index into the blob to allow relocation.
//		Can have different alignment options for the blob and for individual
//		allocs within it. Auto-resizes when full (x2 or by item size).
//		Can be packed to minimum size allowable with specified alignment.
//==============================================================================
    class IndexedAllocator
    {
	private:
		void Create( U32 uSize, U32 uBaseAlign, U32 uAlign)
		{
			U32 uAdjust = uBaseAlign - 1;
			m_uSize = ( uSize + uAdjust ) & ~uAdjust;
			m_uUsed = 0;

			m_uBaseAlign = uBaseAlign;
			m_uAlign = uAlign;

			m_pBase = new U8[ uSize + uBaseAlign ];
			m_pData = (U8*)( ( (INTPTR)m_pBase + uAdjust ) & ~(INTPTR)uAdjust );	// [Blur2PC][x64] align the full address

#if defined (PLATFORM_PS3) && defined (PLATFORM_SPU)
			m_pSPUBase = 0;
			m_pSPUData = 0;
#endif
		}

		void Destroy()
		{
			delete [] m_pBase;

			m_pBase = NULL;
			m_pData = NULL;

			m_uBaseAlign = 0;
			m_uAlign = 0;

			m_uSize = 0;
			m_uUsed = 0;

#if defined (PLATFORM_PS3) && defined (PLATFORM_SPU)
			m_pSPUBase = 0;
			m_pSPUData = 0;
#endif
		}

    public:
		IndexedAllocator()
		{
			Create( 128, 128, 16 );
		}

		IndexedAllocator( U32 uSize )
		{
			Create( uSize, 128, 16 );
		}

		IndexedAllocator( U32 uSize, U32 uBaseAlign, U32 uAlign )
		{
			Create( uSize, uBaseAlign, uAlign );
		}

		~IndexedAllocator()
		{
			Destroy( );
		}

        inline void Reset()
		{
			m_uUsed = 0;
		}

        void Resize( U32 uNewSize )
		{
			U8* pOldData = m_pBase;

			U32 uAdjust = m_uBaseAlign - 1;
			U32 uSize = ( uNewSize + uAdjust ) & ~uAdjust;

			if( uSize != m_uSize )
			{
				m_pBase = new U8[ uSize + m_uBaseAlign ];
				m_pData = (U8*)( ( (INTPTR)m_pBase + uAdjust ) & ~(INTPTR)uAdjust );	// [Blur2PC][x64] align the full address

				m_uSize = uSize;

				U32 uCopyTo;
				if( uNewSize < m_uUsed )
				{
					//Warn( "IndexedAllocator::Resize() - New size smaller than used memory" );
					uCopyTo = uSize;
					m_uUsed = uSize;
				}
				else
					uCopyTo = m_uUsed;

				for( U32 i = 0; i < uCopyTo; ++i )
					m_pData[i] = pOldData[i];

				delete [] pOldData;
			}
		}

		inline U32 Allocate( U32 uSize )
		{
			U32 uIndex = m_uUsed;

			U32 uAdjust = m_uAlign - 1;
			U32 uUsed = ( ( m_uUsed + uSize + uAdjust ) & ~uAdjust );

			//	grow
			if( uUsed > m_uSize )
			{
				if( uUsed < m_uSize * 2 )
					Resize( m_uSize * 2 );
				else
					Resize( uUsed );
			}

			m_uUsed = uUsed;

			m_uNumAllocs++;

			return uIndex;
		}

		inline void* AllocatePtr( U32 uSize )
		{
			return &m_pData[ Allocate( uSize ) ];
		}

		inline void Free()
		{
			m_uNumAllocs--;
		}

		inline void Pack()
		{
			Resize( m_uUsed );
		}

		//	index -> ptr
		inline void* operator [] ( U32 uIndex )
		{
			Assert( uIndex < m_uUsed );

			return &m_pData[ uIndex ];
		}

		//	ptr -> index
		//	won't work if base address has changed since obtaining the pointer
		inline U32 GetIndex( void* pData )
		{
			return (U32)((U8*)pData - m_pData);	// [Blur2PC][x64] byte index
		}

		inline void* Start()
		{
			return m_pData;
		}

		inline U32 Size()
		{
			return m_uSize;
		}

		inline U32 Used()
		{
			return m_uUsed;
		}

		inline U32 Allocations()
		{
			return m_uNumAllocs;
		}

#if defined (PLATFORM_PS3) && defined (PLATFORM_SPU)
		void SPUFetch()
		{
			//	dma to spu mem, deal with alignment
		}

		void SPUWriteBack()
		{
			//	write back to main mem
		}
#endif

	private:
		U8* m_pBase;
		U8* m_pData;
		U32 m_uNumAllocs;
		U32 m_uBaseAlign;
		U32 m_uAlign;
		U32 m_uSize;
		U32 m_uUsed;
#if defined (PLATFORM_PS3) && defined (PLATFORM_SPU)
		U8* m_pSPUBase;
		U8* m_pSPUData;
#endif
	};
}

#endif
