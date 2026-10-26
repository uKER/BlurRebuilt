#ifndef ANIMATION_ARRAYPROXY_H
#define ANIMATION_ARRAYPROXY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{

	/**
		Simple wrapper for passing arrays to functions.
		
		The MakeArrayProxy functions allow you to avoid explicitly
		specifying the array type when calling a function. 
		
		@see MakeArrayProxy
	*/
	template <typename T>
	class C_ArrayProxy
	{
	public:
		/// Construct from a base pointer and a size.
		explicit C_ArrayProxy( T* base, U32 size ):
			m_base(base),
			m_size(size)
		{
		}
		
		//explicit C_ArrayProxy( const T* base, U32 size ):
		//	m_base(const_cast<T*>(base)),
		//	m_size(size)
		//{
		//}
		
		/// Construct an array from a given range.
		explicit C_ArrayProxy( T* begin, T* end ):
			m_base(begin),
			m_size(end - begin)
		{
		}
			
		/// Non-const index operator.
		FORCE_INLINE T& operator []( U32 index )
		{
		#ifdef CFG_DEBUG
			assert( index <= GetSize() );
		#endif
			return m_base[ index ];
		}
		
		/// Const index operator.
		FORCE_INLINE T& operator []( U32 index ) const
		{
		#ifdef CFG_DEBUG
			assert( index <= GetSize() );
		#endif
			return m_base[ index ];
		}
		
		/// Get size of array.
		U32 GetSize() const
		{
			return m_size;
		}
		
		/// Returns true if size of array is zero.
		bool IsEmpty() const
		{
			return GetSize() == 0;
		}
	
	private:
		C_ArrayProxy& operator = ( const C_ArrayProxy& rhs );
	
		T* const m_base;
		const U32 m_size;
	};
	
	/// Make an array proxy from a base pointer and size.
	template <typename T>
	inline C_ArrayProxy<T> MakeArrayProxy( T* base, U32 size )
	{
		return C_ArrayProxy<T>( base, size );
	}

	/// Make an array proxy from a range.
	template <typename T>
	inline C_ArrayProxy<T> MakeArrayProxy( T* base, T* end )
	{
		return C_ArrayProxy<T>( base, end );
	}

}



#endif
