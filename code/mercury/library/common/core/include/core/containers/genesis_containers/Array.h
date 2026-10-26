#ifndef GENESIS_ARRAY_H
#define GENESIS_ARRAY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\containers\genesis_containers\Sort.h>

#if !defined( PLACEMENT_NEW_DEFINED )
#define PLACEMENT_NEW_DEFINED

class placement_new
{
public:
    placement_new( void* pMemory, U32 size ) : m_pMemory( pMemory ), m_Size( size ) {}
    void* m_pMemory;
    U32   m_Size;
};

inline void* operator new ( size_t size, placement_new pn )
{
    return pn.m_pMemory;
}

inline void operator delete ( void* ptr, placement_new pn )
{
}

inline void* operator new[] ( size_t size, placement_new pn )
{
    return pn.m_pMemory;
}

inline void operator delete[]( void* ptr, placement_new pn )
{
}

#endif // #if !defined( PLACEMENT_NEW_DEFINED )




namespace Util
{

//////////////////////////////////////////////
// FixedArray Class
//////////////////////////////////////////////

template < class T_Element, U32 T_ArrayCapacity >
class C_Array
{
public:
    
    C_Array() {}
    ~C_Array() {}
    
    const U32           Capacity() const    { return T_ArrayCapacity; }
    const U32           Elements() const    { return T_ArrayCapacity; }
    
    const T_Element*    Data() const        { return m_ElementArray; }
    T_Element*          Data()              { return m_ElementArray; }
    
    const T_Element& operator [] ( const U32 index ) const
    {
        //assertError( index < T_ArrayCapacity, "Array index out of range\n" );
        assert( index < T_ArrayCapacity );
        return m_ElementArray[ index ];
    }
    
    T_Element& operator [] ( const U32 index )
    {
        //assertError( index < T_ArrayCapacity, "Array index out of range\n" );
        assert( index < T_ArrayCapacity );
        return m_ElementArray[ index ];
    }
    
    const U32 ItemIndex( const T_Element* pItem ) const
    {
        const S32 itemIndex = pItem - m_ElementArray;
        //assertError( ( itemIndex >= 0 ) && ( (U32)itemIndex < T_ArrayCapacity ), "Item is not in this array!" );
        assert( ( itemIndex >= 0 ) && ( (U32)itemIndex < T_ArrayCapacity ) );
        return (U32)itemIndex;
    }
    
    const U32 ItemIndex( const T_Element& item ) const
    {
        return ItemIndex( &item );
    }
    
	// added to be more like Ut containers
	S32                 Size() const		{ return (S32)Elements(); }

protected:
    
    T_Element           m_ElementArray[ T_ArrayCapacity ];
};

template< class T_Stream, class T_Element, U32 T_ArrayCapacity >
inline T_Stream& operator << (T_Stream& stream, const C_Array<T_Element, T_ArrayCapacity>& in)
{
    // TODO : use array iterator when we have one
    U32 count = in.Elements();
    stream << count;
    for(U32 i=0; i<count; i++)
    {
        stream << in[i];
    }
    return stream;
}

template< class T_Stream, class T_Element, U32 T_ArrayCapacity >
inline T_Stream& operator >> (T_Stream& stream, C_Array<T_Element, T_ArrayCapacity>& out)
{
    U32 count;
    stream >> count;
    
    assert( count == out.Elements() );
    //out.Expand(count);
    for(U32 i=0; i<count; i++)
    {
        stream >> out[i];
    }
    return stream;
}


/// C_DynamicArray
template < typename T_Element,
           bool     T_SafeToMemCpy = true > // WARNING: Set this to FALSE if T_Element has a copy constructor which is not a pure byte-for-byte copy!!!
class C_DynamicArray
{
public:
    
    /// Default Constructor: Zero capacity
    NOALIAS __forceinline C_DynamicArray() :
        m_ArrayCapacity( 0 ),
        m_pElementArray( NULL )
    {
    }
    
    /// Constructor: Initialises capacity
    NOALIAS __forceinline C_DynamicArray( const U32 capacity ) :
        m_ArrayCapacity( capacity ),
        m_pElementArray( capacity ? new T_Element[ capacity ] : NULL )
    {
    }
    
    /// Destructor
    NOALIAS __forceinline ~C_DynamicArray()
    {
        if ( m_pElementArray )
        {
            delete [] m_pElementArray;
            m_pElementArray = NULL;
        }
        
        m_ArrayCapacity = 0;
    }
    
    /// Copy Constructor
    NOALIAS inline C_DynamicArray( const C_DynamicArray& copy ) :
        m_ArrayCapacity( copy.Capacity() )
    {
        const U32 elementsToCopy( copy.Capacity() );
        if ( elementsToCopy )
        {
            if ( T_SafeToMemCpy )
            {
                m_pElementArray = new T_Element[ elementsToCopy ];
                
                T_Element const * __restrict pSrc = copy.m_pElementArray;
                T_Element       * __restrict pDst = m_pElementArray;
                
                memcpy( pDst, pSrc, sizeof( T_Element ) * elementsToCopy );
            }
            else
            {
                m_pElementArray = new T_Element[ elementsToCopy ];
                
                T_Element const * __restrict pSrc = copy.m_pElementArray;
                T_Element       * __restrict pDst = m_pElementArray;
                
                for ( U32 i=0; i<elementsToCopy; i++, pSrc++, pDst++ )
                {
                    *pDst = *pSrc;
                }
            }
        }
        else
        {
            m_pElementArray = NULL;
        }
    }
    
    /// Copy Assignment
    NOALIAS inline const C_DynamicArray& operator = ( const C_DynamicArray& copy )
    {
        if ( m_pElementArray )
        {
            delete [] m_pElementArray;
            m_pElementArray = NULL;
        }
        
        const U32 elementsToCopy( copy.Capacity() );
        
        m_ArrayCapacity = elementsToCopy;
        
        if ( elementsToCopy )
        {
            if ( T_SafeToMemCpy )
            {
                m_pElementArray = new T_Element[ elementsToCopy ];
                
                T_Element const * __restrict pSrc = copy.m_pElementArray;
                T_Element       * __restrict pDst = m_pElementArray;
                
                memcpy( pDst, pSrc, sizeof( T_Element ) * elementsToCopy );
            }
            else
            {
                m_pElementArray = new T_Element[ elementsToCopy ];
                
                T_Element const * __restrict pSrc = copy.m_pElementArray;
                T_Element       * __restrict pDst = m_pElementArray;
                
                for ( U32 i=0; i<elementsToCopy; i++, pSrc++, pDst++ )
                {
                    *pDst = *pSrc;
                }
            }
        }
        else
        {
            m_pElementArray = NULL;
        }
        
        return copy;
    }
    
    /// Accessor: Capacity
    NOALIAS __forceinline const U32 Capacity() const { return m_ArrayCapacity; }
    
    /// Accessor: Elements
    NOALIAS __forceinline const U32 Elements() const { return m_ArrayCapacity; }
    
    /// Accessor: Array data const pointer
    NOALIAS __forceinline const T_Element* Data() const { return m_pElementArray; }
    
    /// Accessor: Array data pointer
    NOALIAS __forceinline T_Element*       Data()       { return m_pElementArray; }
    
    /// Accessor: Indexed const element
    //NOALIAS __forceinline const T_Element& operator [] ( const U32 index ) const
    //{
    //    #if defined( _DEBUG )
    //    assert( m_pElementArray != NULL );
    //    assert( index < m_ArrayCapacity );
    //    #endif
    //    
    //    return m_pElementArray[ index ];
    //}
    
    /// Accessor: Indexed element
    NOALIAS __forceinline T_Element& operator [] ( const U32 index ) const
    {
        #if defined( _DEBUG )
        assert( m_pElementArray != NULL );
        assert( index < m_ArrayCapacity );
        #endif
        
        return m_pElementArray[ index ];
    }

    NOALIAS __forceinline T_Element& UnsafeGetElement( const U32 index ) const
    {
        return m_pElementArray[ index ];
    }
    

    /// Get array index of item pointer
    NOALIAS __forceinline const U32 ItemIndex( T_Element const * const __restrict pItem ) const
    {
        const S32 itemIndex = (S32)( pItem - m_pElementArray );
        
        #if defined( _DEBUG )
        assert( m_pElementArray != NULL );
        assert( ( itemIndex >= 0 ) && ( (U32)itemIndex < m_ArrayCapacity ) );
        #endif
        
        return (U32)itemIndex;
    }
    
    /// Get array index of item reference
    NOALIAS __forceinline const U32 ItemIndex( const T_Element& item ) const
    {
        return ItemIndex( &item );
    }
    
    /// Increase capacity of array
    NOALIAS __forceinline void Expand( const U32 newCapacity )
    {
        if ( newCapacity > m_ArrayCapacity )
        {
            Capacity( newCapacity );
        }
    }
    
    /// Decrease capacity of array
    NOALIAS __forceinline void Shrink( const U32 newCapacity )
    {
        if ( newCapacity < m_ArrayCapacity )
        {
            Capacity( newCapacity );
        }
    }
    
    /// Increase/Decrease capacity of array
    NOALIAS inline void Capacity( const U32 newCapacity )
    {
        if ( newCapacity )
        {
            if ( m_pElementArray )
            {
                T_Element* __restrict pNewElementArray = NULL;
                
                const U32 elementsToCopy( Maths::Min( newCapacity, m_ArrayCapacity ) );
                
                if ( T_SafeToMemCpy )
                {
                    pNewElementArray = new T_Element[ newCapacity ];
                    
                    T_Element const * __restrict pSrc = m_pElementArray;
                    T_Element       * __restrict pDst = pNewElementArray;
                    
                    // Memcpy original items!
                    memcpy( pDst, pSrc, sizeof( T_Element ) * elementsToCopy );
                }
                else
                {
                    pNewElementArray = new T_Element[ newCapacity ];
                    
                    T_Element const * __restrict pSrc = m_pElementArray;
                    T_Element       * __restrict pDst = pNewElementArray;
                    
                    for ( U32 i=0; i<elementsToCopy; i++, pSrc++, pDst++ )
                    {
                        *pDst = *pSrc;
                    }
                }
                
                delete [] m_pElementArray;
                m_pElementArray = pNewElementArray;
            }
            else
            {
                // Nothing to copy from original array
                m_pElementArray = new T_Element[ newCapacity ];
            }
            
            m_ArrayCapacity = newCapacity;
        }
        else
        {
            if ( m_pElementArray )
            {
                delete [] m_pElementArray;
            }
            m_pElementArray = NULL;
            m_ArrayCapacity = 0;
        }
    }

	// added to be more like Ut containers
	S32                 Size() const		{ return (S32)Elements(); }
	void				Resize( const S32 newCapacity ) { Capacity(newCapacity); }

protected:
    
    U32             m_ArrayCapacity;
    T_Element*      m_pElementArray;
};


template< class T_Stream, class T_Element >
inline T_Stream& operator << (T_Stream& stream, const C_DynamicArray<T_Element>& in)
{
    // TODO : use array iterator when we have one
    U32 count = in.Elements();
    stream << count;
    for(U32 i=0; i<count; i++)
    {
        stream << in[i];
    }
    return stream;
}

template< class T_Stream, class T_Element >
inline T_Stream& operator >> (T_Stream& stream, C_DynamicArray<T_Element>& out)
{
    U32 count;
    stream >> count;
    out.Expand(count);
    for(U32 i=0; i<count; i++)
    {
        stream >> out[i];
    }
    return stream;
}



}; // namespace Util


#endif
