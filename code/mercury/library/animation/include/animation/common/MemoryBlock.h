#ifndef ANIMATION_MEMORYBLOCK_H
#define ANIMATION_MEMORYBLOCK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/utility/types.h>
#include <system/Scheduler.h>

namespace Animation
{

	template <typename T>
	union MustBePod
	{
		T x;
		U32 tmp;
		
		enum
		{
			kIsValid	= 1
		};
	};
	
	class C_MemoryBlockSizer;
	
	class C_MemoryBlock
	{
	public:
		C_MemoryBlock();
		
		explicit C_MemoryBlock( U32 size );
				
		~C_MemoryBlock();
		
		void Create( const C_MemoryBlockSizer& sizer );
		void Create( U32 size );
		void CreateInPlace( U8* data, U32 size );
		void Resize( U32 newSize );
		
		void Destroy();
		
		U8& operator []( U32 index )
		{
			return m_data[ index ];
		}
		
		const U8& operator [] ( U32 index ) const
		{
			return m_data[ index ];
		}
		
		U32 GetSize() const;
		
		void SpuFetchAsync( U32 tag ) const;
		
		void SpuFetch() const;

		void SpuPartialFetchAsync( U32 numBytes, U32 tag ) const;

		void SpuPartialFetch( U32 numBytes ) const;
		
		void SpuStoreAsync( U32 tag ) const;
		
		void SpuStore() const;

		bool IsInPlace() const	{ return m_data.RawPtr() && 0 == m_capacity; }

		bool IsValid() const	{ return m_data.RawPtr() != 0; }
			
	private:
		// Not to be implemented.
		C_MemoryBlock& operator = ( const C_MemoryBlock& rhs );
	
	private:
		Spu::Ptr<U8>	m_data;
		U32				m_size;
		U32				m_capacity;	// Zero if in place.
	};
	
	///
	/// Allocates contiguous arrays of simple data types inside a pre-allocated memory block.
	/// \warning Constructors and destructors of the allocated data will be not called.
	///
	class C_MemoryBlockWriter
	{
	public:
		explicit C_MemoryBlockWriter( U8* basePtr, U32 size ):
			m_basePtr( basePtr ),
			m_offset( 0 ),
			m_size( size )
		{
						
		}
		
		explicit C_MemoryBlockWriter( C_MemoryBlock& memoryBlock, U32 offset = 0 ):
			m_basePtr( &memoryBlock[ 0 ] ),
			m_offset( offset ),
			m_size( memoryBlock.GetSize() )
		{
			BIZ_ASSERT(m_offset <= m_size);
		}		
		
		C_MemoryBlockWriter():
			m_basePtr( 0 ),
			m_offset( 0 ),
			m_size( (U32)~0 )
		{
		}
		
		
		template < typename T>
		bool HasSpaceFor( U32 numItems = 1 ) const
		{
			U32 alignment = BTL_ALIGNMENT_OF( T );
			U32 startOffset = ( m_offset + ( alignment - 1 ) ) & ~( alignment - 1 );
			U32 endOffset = startOffset + sizeof( T ) * numItems;
			return endOffset <= m_size;
		}
		
		template < typename T>
		T* Allocate( U32 numItems )
		{
			//BIZ_STATIC_ASSERT( MustBePod< T >::kIsValid );
			U32 prevOffset = m_offset;
			if ( numItems > 0 )
			{
				Align( BTL_ALIGNMENT_OF( T ) );
				prevOffset = m_offset;
				m_offset += sizeof( T ) * numItems;
			}
			BIZ_ASSERT( m_offset <= m_size );
			return reinterpret_cast< T* >( m_basePtr + prevOffset );
		}
		
		template < typename T>
		void Allocate( T*& outBasePtr, U32 numItems = 1 )
		{
			U32 prevOffset = m_offset;
			if ( numItems > 0 )
			{
				Align( BTL_ALIGNMENT_OF( T ) );
				prevOffset = m_offset;
				m_offset += sizeof( T ) * numItems;
			}
			BIZ_ASSERT( m_offset <= m_size );
			outBasePtr = reinterpret_cast< T* >( m_basePtr + prevOffset );
		}
		
		template <typename T>
		C_MemoryBlockWriter& operator << (const T& value)
		{
			T* p;
			Allocate(p);
			*p = value;
			return *this;
		}
		
		U8* GetTop() const
		{
			return m_basePtr + m_offset;
		}
		
		void Align( U32 alignment )
		{
			BIZ_ASSERT( ( alignment & ( alignment - 1 ) ) == 0 ); // must be power of 2
			m_offset = ( m_offset + ( alignment - 1 ) ) & ~( alignment - 1 );
		}
		
		U32 BytesUsed() const
		{
			return m_offset;
		}
		
		U32 BytesRemaining() const
		{
			return m_size - m_offset;
		}
		
	private:
		U8* m_basePtr;
		U32 m_offset;
		const U32 m_size;
	};
	
	///
	/// Returns pointers to arrays of simple data types previously allocated inside a pre-allocated contiguous memory block.
	//
	class C_MemoryBlockReader
	{
	public:
		explicit C_MemoryBlockReader( const U8* basePtr, U32 size ):
			m_basePtr( basePtr ),
			m_offset( 0 ),
			m_size( size )
		{
		}
		
		explicit C_MemoryBlockReader( const C_MemoryBlock& memoryBlock ):
			m_basePtr( &memoryBlock[ 0 ] ),
			m_offset( 0 ),
			m_size( memoryBlock.GetSize() )
		{
		}		
		
		const U8* ReadBytes( U32 size, U32 align )
		{
			U32 prevOffset = m_offset;
			// remove branch. if size == 0, set alignment to 1, otherwise keep it.
			if ( size > 0 )
			{
				Align( align );
				prevOffset = m_offset;
				m_offset += size;
			}
		#ifdef CFG_DEBUG
			BIZ_ASSERT( m_offset <= m_size );
		#endif
			
			return m_basePtr + prevOffset;
		}
		
		// return a pointer to an array.
		template < typename T>
		const T* Read( U32 numItems = 1 )
		{
			return reinterpret_cast< const T* >( ReadBytes( numItems * sizeof( T ), BTL_ALIGNMENT_OF( T ) ) );
		}
		
		template <typename T>
		void Read( T*& destPtr, U32 numItems = 1 )
		{
			destPtr = reinterpret_cast< const T* >( ReadBytes( numItems * sizeof( T ), BTL_ALIGNMENT_OF( T ) ) );
		}

		template <typename T>
		C_MemoryBlockReader& operator >> (T& value)
		{
			const T* p;
			Read(p);
			value = *p;
			return *this;
		}

		
		/// Align current pointer to specified boundary.
		void Align( U32 alignment )
		{
		#ifdef CFG_DEBUG
			BIZ_ASSERT( ( alignment & ( alignment - 1 ) ) == 0 ); // must be power of 2
		#endif
			m_offset = ( m_offset + ( alignment - 1 ) ) & ~( alignment - 1 );
		}
		
		bool AtEnd() const
		{
			return m_offset >= m_size;
		}
		
		
	private:
		const U8* const m_basePtr;
		U32 m_offset;
		const U32 m_size;
	};

	class C_MemoryBlockOffset
	{
	public:
		C_MemoryBlockOffset():
			m_offset(0) {}
		
		explicit C_MemoryBlockOffset( U16 offset ):
			m_offset(offset)
		{
		}
		
		C_MemoryBlockOffset( const C_MemoryBlockOffset& rhs ):
			m_offset( rhs.m_offset )
		{
		}
		
		void SetOffset(U32 offset)
		{
			m_offset = offset;
		}
						
		U32 GetOffset() const
		{
			return m_offset;
		}
		
		const U16* operator[] ( S32 index ) const
		{
			BIZ_STATIC_ASSERT(sizeof(C_MemoryBlockOffset) == sizeof(U16));
			return &m_offset + index;
		}
		
		void operator += (S32 offset)
		{
			BIZ_ASSERT(offset >= -m_offset);	// check for underflow
			m_offset += offset;
		}
		
		void operator -= (S32 offset)
		{
			BIZ_ASSERT(offset <= m_offset);
			m_offset -= offset;
		}
	
	protected:
		U16 m_offset;
	};


	// C_RelativePtr
	template <typename T>
	class C_RelativePtr: public C_MemoryBlockOffset
	{
	public:
		C_RelativePtr() {}
		
		explicit C_RelativePtr( U16 offset ): C_MemoryBlockOffset(offset) {}
		
		C_RelativePtr( const C_RelativePtr& rhs ):
			C_MemoryBlockOffset( rhs.m_offset )
		{
		}
	
		T* operator() ( C_MemoryBlock& memBlock ) const
		{
			return reinterpret_cast<T*>( &memBlock[ m_offset ] );
		}
		
		const T* operator() ( const C_MemoryBlock& memBlock ) const
		{
			return reinterpret_cast<const T*>( &memBlock[ m_offset ] );
		}
		
		T* operator() ( void* basePtr ) const
		{
			U8* p = static_cast<U8*>(basePtr);
			return reinterpret_cast<T*>( p + m_offset );
		}

		const T* operator() ( const void* basePtr ) const
		{
			const U8* p = static_cast<const U8*>(basePtr);
			return reinterpret_cast<const T*>( p + m_offset );
		}

		C_RelativePtr<T> operator + (const U32 count) const
		{
			return C_RelativePtr(m_offset + sizeof(T) * count);
		}
	};
	
	class C_MemoryBlockSizer
	{
	public:
		C_MemoryBlockSizer():
			m_size(0)
		{
		}

		explicit C_MemoryBlockSizer( U32 size ):
			m_size(size)
		{
		}

		
		void Reset( U32 offset = 0 )
		{
			m_size = offset;
		}
		
		template <typename T>
		U32 Allocate( U32 numItems )
		{
			//BIZ_STATIC_ASSERT( MustBePod< T >::kIsValid );
			return Allocate( sizeof(T) * numItems, BTL_ALIGNMENT_OF(T) );
		}
		
		template <typename T>
		void Allocate( C_RelativePtr<T>& ptr, U32 numItems = 1 )
		{
			//BIZ_STATIC_ASSERT( MustBePod< T >::kIsValid );
			ptr = C_RelativePtr<T>( Allocate( sizeof(T) * numItems, BTL_ALIGNMENT_OF(T) ) );
		}
		
		U32 Allocate( U32 numBytes, U32 alignment )
		{
			U32 offset = m_size;
			if ( numBytes > 0 )
			{
				Align( alignment );
				offset = m_size;
				m_size += numBytes;
			}	
			return offset;
		}
		
		void Align( U32 alignment )
		{
			m_size = ( m_size + ( alignment - 1 ) ) & ~( alignment - 1 );
		}
		
		U32 GetSize() const
		{
			return m_size;
		}
		
	private:
		U32 m_size;
	};

}

#include "MemoryBlock.inl"

#endif
