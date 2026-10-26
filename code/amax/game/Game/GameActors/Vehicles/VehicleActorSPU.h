/////////////////////////////////////////////////////////////
// VehicleActor
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_ACTOR_SPU_H__
#define __VEHICLE_ACTOR_SPU_H__

#include <System/DebugLog.h>
#include "Vehicle/Component.h"
#include <shared_actors/Utility/VisibilityDetection.h>

#if 1

//namespace Spu {

template < typename T >
class PPU_Ptr
{
public:
    PPU_Ptr() {}
    PPU_Ptr( T* pT ) : m_pT( pT ) {}
    PPU_Ptr( PPU_Ptr<T>& copy ) : m_pT( copy.m_pT ) {}
    ~PPU_Ptr() {}
    
    T*          operator = ( T* pT )            { m_pT = pT; return pT; }
    PPU_Ptr<T>& operator = ( PPU_Ptr<T>& copy ) { m_pT = copy.m_pT; return copy; }
    
    #if !defined( PLATFORM_SPU )
    operator T* () { return m_pT; }
    operator T const * () const { return m_pT; }
    #endif
    
    #if !defined( PLATFORM_SPU )
    T*        operator -> ()       { return m_pT; }
    T const * operator -> () const { return m_pT; }
    T&        operator *  ()       { return *m_pT; }
    const T&  operator *  () const { return *m_pT; }
    #else
    T*        operator -> ()       { BIZ_ASSERT_FAIL( "Can't dereference a PPU pointer on the SPU!" ); return NULL; }
    T const * operator -> () const { BIZ_ASSERT_FAIL( "Can't dereference a PPU pointer on the SPU!" ); return NULL; }
    T&        operator *  ()       { BIZ_ASSERT_FAIL( "Can't dereference a PPU pointer on the SPU!" ); return *m_pT; }
    const T&  operator *  () const { BIZ_ASSERT_FAIL( "Can't dereference a PPU pointer on the SPU!" ); return *m_pT; }
    #endif
    
    PPU_Ptr<T>& operator ++ ()      { m_pT++; return *this; }
    PPU_Ptr<T>  operator ++ ( int ) { PPU_Ptr<T> result( *this ); m_pT++; return result; }
    PPU_Ptr<T>& operator -- ()      { m_pT--; return *this; }
    PPU_Ptr<T>  operator -- ( int ) { PPU_Ptr<T> result( *this ); m_pT--; return result; }
    
template < typename S > friend class DMA;
private:
    T*  m_pT;
};

template < typename T >
struct SPU_Ptr
{
public:
    
    //inline SPU_Ptr()                   __attribute__(( always_inline )) {}
    //inline SPU_Ptr( T* pT )            __attribute__(( always_inline )) : m_pT( pT ) {}
    //inline SPU_Ptr( SPU_Ptr<T>& copy ) __attribute__(( always_inline )) : m_pT( copy.m_pT ) {}
    //inline ~SPU_Ptr()                  __attribute__(( always_inline )) {}
    inline SPU_Ptr()                   __attribute__(( always_inline )) {}
    inline SPU_Ptr( T* pT )            __attribute__(( always_inline )) { m_pT = pT; }
    inline SPU_Ptr( SPU_Ptr<T>& copy ) __attribute__(( always_inline )) { m_pT = copy.m_pT; }
    inline ~SPU_Ptr()                  __attribute__(( always_inline )) {}
    //inline SPU_Ptr()                   __attribute__(( always_inline ));
    //inline SPU_Ptr( T* pT )            __attribute__(( always_inline ));
    //inline SPU_Ptr( SPU_Ptr<T>& copy ) __attribute__(( always_inline ));
    //inline ~SPU_Ptr()                  __attribute__(( always_inline ));
    
    //inline T*          operator = ( T* pT )            __attribute__(( always_inline )) { m_pT = pT; return pT; }
    //inline SPU_Ptr<T>& operator = ( SPU_Ptr<T>& copy ) __attribute__(( always_inline )) { m_pT = copy.m_pT; return copy; }
    
    //inline T*        operator -> ()       __attribute__(( always_inline )) { return m_pT; }
    //inline T const * operator -> () const __attribute__(( always_inline )) { return m_pT; }
    //inline T&        operator *  ()       __attribute__(( always_inline )) { return *m_pT; }
    //inline const T&  operator *  () const __attribute__(( always_inline )) { return *m_pT; }
    //
    //inline SPU_Ptr<T>& operator ++ ()      __attribute__(( always_inline )) { m_pT++; return *this; }
    //inline SPU_Ptr<T>  operator ++ ( int ) __attribute__(( always_inline )) { SPU_Ptr<T> result( *this ); m_pT++; return result; }
    //inline SPU_Ptr<T>& operator -- ()      __attribute__(( always_inline )) { m_pT--; return *this; }
    //inline SPU_Ptr<T>  operator -- ( int ) __attribute__(( always_inline )) { SPU_Ptr<T> result( *this ); m_pT--; return result; }
    
//template < typename S > friend class DMA;
//private:
    T*  m_pT;
};

//template < typename T > inline SPU_Ptr<T>::SPU_Ptr() {}
//template < typename T > inline SPU_Ptr<T>::SPU_Ptr( T* pT ) : m_pT( pT ) {}
//template < typename T > inline SPU_Ptr<T>::SPU_Ptr( SPU_Ptr<T>& copy ) : m_pT( copy.m_pT ) {}
//template < typename T > inline SPU_Ptr<T>::~SPU_Ptr() {}

#define SMALL_DMA_LIMIT (16)
#define LARGE_DMA_LIMIT (16*1024)

void DMA_Wait( const U32 dmaTag = 1 )
{
    #if defined( PLATFORM_SPU )
    cellDmaWaitTagStatusAll( 1 << dmaTag );
    #endif
}

template < typename T >
class DMA
{
public:
    
    inline DMA()  __attribute__(( always_inline )) {}
    inline ~DMA() __attribute__(( always_inline )) {}
    
    //inline SPU_Ptr<T> Fetch( PPU_Ptr<T>& src, const U32 dmaTag = 1 ) __attribute__(( always_inline ));
    inline T* Fetch( PPU_Ptr<T>& src, const U32 dmaTag = 1 ) __attribute__(( always_inline ));
    
private:
    #if defined( PLATFORM_SPU )
    U8  m_Data[ sizeof( T ) >= SMALL_DMA_LIMIT ? sizeof( T ) : SMALL_DMA_LIMIT ] __attribute__(( aligned( 16 ) ));
    #endif
};

template < typename T >
//inline SPU_Ptr<T> DMA<T>::Fetch( PPU_Ptr<T>& src, const U32 dmaTag )
inline T* DMA<T>::Fetch( PPU_Ptr<T>& src, const U32 dmaTag )
{
    #if defined( PLATFORM_SPU )
    
    if ( sizeof( T ) < SMALL_DMA_LIMIT )
    {
        BIZ_ASSERT( ( sizeof( T ) == 1 ) || ( sizeof( T ) == 2 ) || ( sizeof( T ) == 4 ) || ( sizeof( T ) == 8 ) );
        //BIZ_ASSERT( ( (U32)m_Data & 0xF ) == ( (U32)src.m_pT & 0xF ) );
        const U32 offset( (U32)src.m_pT & 0xF );
        T* pDst( (T*)( m_Data + offset ) );
        cellDmaSmallGet( pDst, (uint64_t)src.m_pT, sizeof( T ), dmaTag, 0, 0 );
        //return SPU_Ptr< T >( pDst );
        //SPU_Ptr< T > result( pDst );
        //return result;
        return pDst;
    }
    else
    {
        if ( sizeof( T ) <= LARGE_DMA_LIMIT )
        {
            BIZ_ASSERT( ( (U32)m_Data & 0xF ) == 0 );
            BIZ_ASSERT( ( (U32)src.m_pT & 0xF ) == 0 );
            BIZ_ASSERT( ( sizeof( T ) & 0xF ) == 0 );
            cellDmaGet( m_Data, (uint64_t)src.m_pT, sizeof( T ), dmaTag, 0, 0 );
        }
        else
        {
            BIZ_ASSERT( ( (U32)m_Data & 0xF ) == 0 );
            BIZ_ASSERT( ( (U32)src.m_pT & 0xF ) == 0 );
            BIZ_ASSERT( ( sizeof( T ) & 0xF ) == 0 );
            cellDmaLargeGet( m_Data, (uint64_t)src.m_pT, sizeof( T ), dmaTag, 0, 0 );
        }
        //return SPU_Ptr< T >( (T*)m_Data );
        //SPU_Ptr< T > result( (T*)m_Data );
        //return result;
        return (T*)m_Data;
    }
    
    #else
    
    //return SPU_Ptr< T >( src.m_pT );
    //SPU_Ptr< T > result( src.m_pT );
    //return result;
    return src.m_pT;
    
    #endif
}

//} // namespace Spu

#else

#if defined( PLATFORM_SPU )

namespace Spu
{

template < typename T_Object > class SPU_Ptr;
template < typename T_Object > class PPU_Ptr;

template < typename T_Object, unsigned int m_DmaTag, unsigned int m_MaxSize > class AutoDmaObject_Aligned;
template < typename T_Object, unsigned int m_DmaTag, unsigned int m_MaxSize > class AutoDmaObject_Unaligned;
template < typename T_Element, unsigned int m_DmaTag > class AutoDmaArray_Unaligned;

template < typename T > inline const T SmallDMA( T* PPU_ptr )
{
    BIZ_STATIC_ASSERT( sizeof( T ) == 1 || sizeof( T ) == 2 || sizeof( T ) == 4 || sizeof( T ) == 8 );
    
    spu_assert_ge( (U32)PPU_ptr, 0x40000 );
    
    U8 buffer[16] __attribute__(( aligned( 16 ) ));
    //vector unsigned char buffer;
    
    const U32 src = (U32)PPU_ptr;
    const U32 offset = src & 0xF;
    const U32 dst = (U32)buffer + offset;
    const U32 tag = 0;
    
    cellDmaSmallGet( (void*)dst, (U64)src, sizeof( T ), tag, 0, 0 );
    cellDmaWaitTagStatusAll( 1 << tag );
    
    T const * const SPU_ptr = (T*)dst;
    const T result = *SPU_ptr;
    return result;
}

/// SPU Pointer class
template < typename T_Object > class SPU_Ptr
{
public:
    
    inline SPU_Ptr() __attribute__(( always_inline )) : m_pObject( NULL ) {}
    
    inline SPU_Ptr( T_Object* const pObject ) __attribute__(( always_inline ))
        : m_pObject( pObject )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_lt( (U32)pObject, 0x40000 );
        #endif
    }
    
    inline SPU_Ptr( const SPU_Ptr< T_Object > & pObject ) __attribute__(( always_inline ))
        : m_pObject( pObject.m_pObject )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_lt( (U32)pObject.m_pObject, 0x40000 );
        #endif
    }
    
    inline ~SPU_Ptr() __attribute__(( always_inline )) {}
    
    inline void operator = ( T_Object* const pObject ) __attribute__(( always_inline ))
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_lt( (U32)pObject, 0x40000 );
        #endif
        m_pObject = pObject;
    }
    
    inline void operator = ( const SPU_Ptr< T_Object > & pObject ) __attribute__(( always_inline ))
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_lt( (U32)pObject.m_pObject, 0x40000 );
        #endif
        m_pObject = pObject.m_pObject;
    }
    
    inline T_Object * operator -> () const __attribute__(( /* pure, */ always_inline )) { return m_pObject; }
    
    inline T_Object & operator * () const __attribute__(( /* pure, */ always_inline )) { return *m_pObject; }
    
    inline operator T_Object * () const __attribute__(( /* pure, */ always_inline )) { return m_pObject; }
    
    inline SPU_Ptr< T_Object > operator ++ ( int ) /* postfix */ __attribute__(( always_inline )) { return SPU_Ptr< T_Object >( m_pObject++ ); }
    inline SPU_Ptr< T_Object > operator ++ ()      /* prefix */  __attribute__(( always_inline )) { return SPU_Ptr< T_Object >( ++m_pObject ); }
    
    inline SPU_Ptr< T_Object > operator + ( const U32 n ) const __attribute__(( always_inline )) { return SPU_Ptr< T_Object >( m_pObject + n ); }
    inline SPU_Ptr< T_Object > operator + ( const S32 n ) const __attribute__(( always_inline )) { return SPU_Ptr< T_Object >( m_pObject + n ); }
    
    inline void operator += ( const U32 n ) __attribute__(( always_inline )) { m_pObject += n; }
    inline void operator += ( const S32 n ) __attribute__(( always_inline )) { m_pObject += n; }
    
    inline operator bool () const __attribute__(( /* pure, */ always_inline )) { return m_pObject != NULL; }
    
    template < typename T > inline SPU_Ptr<T> Cast() const { return SPU_Ptr<T>( (T*)m_pObject ); }
    
private:
    
    template < typename T > friend class SPU_Ptr;
    
    T_Object*   m_pObject;
};

enum _PPU_NULL { PPU_NULL = NULL };

/// PPU Pointer class
template < typename T_Object > class PPU_Ptr
{
public:
    
    inline PPU_Ptr() __attribute__(( always_inline )) {}
    
    inline PPU_Ptr( const _PPU_NULL null ) __attribute__(( always_inline )) : m_pObject( NULL ) {}
    
    inline PPU_Ptr( const PPU_Ptr< T_Object > & pObject ) __attribute__(( always_inline ))
        : m_pObject( pObject.m_pObject )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_ge( (U32)pObject.m_pObject, 0x40000 );
        #endif
    }
    
    inline ~PPU_Ptr() __attribute__(( always_inline )) {}
    
    inline void operator = ( const PPU_Ptr< T_Object > & pObject ) __attribute__(( always_inline )) { m_pObject = pObject.m_pObject; }
    
    inline void operator = ( const _PPU_NULL null ) __attribute__(( always_inline )) { m_pObject = NULL; }
    
    inline operator bool () const __attribute__(( /* pure, */ always_inline )) { return m_pObject != NULL; }
    
    inline const bool operator == ( const _PPU_NULL null ) __attribute__(( /* pure, */ always_inline )) { return m_pObject == NULL; }
    inline const bool operator != ( const _PPU_NULL null ) __attribute__(( /* pure, */ always_inline )) { return m_pObject != NULL; }
    
    inline PPU_Ptr< T_Object > operator + ( const U32 offset ) const __attribute__(( always_inline )) { return PPU_Ptr< T_Object >( m_pObject + offset ); }
    
    template < typename T > inline PPU_Ptr<T> Cast() const { return PPU_Ptr<T>( (T*)m_pObject ); }
    
    inline void       SetRawPtr( T_Object * pObject ) __attribute__(( always_inline ))  { m_pObject = pObject; }
    inline T_Object * GetRawPtr() const __attribute__(( /* pure, */ always_inline ))    { return m_pObject; }
    
private:
    
    inline explicit PPU_Ptr( T_Object* const pObject ) __attribute__(( always_inline ))
        : m_pObject( pObject )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_ge( (U32)pObject, 0x40000 );
        #endif
    }
    
    template < typename T > friend class PPU_Ptr;
    
    template < typename T, unsigned int m_DmaTag, unsigned int m_MaxSize > friend class AutoDmaObject_Aligned;
    template < typename T, unsigned int m_DmaTag, unsigned int m_MaxSize > friend class AutoDmaObject_Unaligned;
    template < typename T, unsigned int m_DmaTag > friend class AutoDmaArray_Unaligned;
    
    T_Object*   m_pObject;
};

#define PPU_ADDRESS_OF_MEMBER( ptr, member ) (&((ptr).GetRawPtr()->member))

/// SPU Aligned Auto-DMA Ptr
template < typename T_Object, unsigned int m_DmaTag = 1, unsigned int m_MaxSize = sizeof( T_Object ) >
class AutoDmaObject_Aligned
{
public:
    
    AutoDmaObject_Aligned()
        : m_SPU_pBuffer( memalign( 0x10, m_MaxSize ) )
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        , m_SyncRequired( true )
        #endif
    {
        BIZ_STATIC_ASSERT( __alignof( T_Object ) >= 16 );
    }
    
    void DMA( T_Object * const PPU_pObject, const U32 size = sizeof( T_Object ) )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( ( (U32)m_SPU_pBuffer & 0xF ) == 0 );
        spu_assert( ( (U32)PPU_pObject & 0xF ) == 0 );
        spu_assert_lt( (U32)m_SPU_pBuffer, 0x40000 );
        spu_assert_ge( (U32)PPU_pObject, 0x40000 );
        spu_assert( size );
        spu_assert_le( size, m_MaxSize );
        #endif
        
        cellDmaGet( (void*)m_SPU_pBuffer, (uint64_t)PPU_pObject, size, m_DmaTag, 0, 0 );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        m_SyncRequired = true;
        #endif
        
        //spu_printf( "%s: %08X, %05X\n", __DEBUG_FUNC__, PPU_pObject, m_SPU_pBuffer );
    }
    
    AutoDmaObject_Aligned( T_Object * const PPU_pObject, const U32 size = sizeof( T_Object ) )
        : m_SPU_pBuffer( memalign( 0x10, m_MaxSize ) )
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        , m_SyncRequired( true )
        #endif
    {
        BIZ_STATIC_ASSERT( __alignof( T_Object ) >= 16 );
        
        DMA( PPU_pObject, size );
    }
    
    AutoDmaObject_Aligned( const PPU_Ptr< T_Object > PPU_pObject, const U32 size = sizeof( T_Object ) )
        : m_SPU_pBuffer( memalign( 0x10, m_MaxSize ) )
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        , m_SyncRequired( true )
        #endif
    {
        BIZ_STATIC_ASSERT( __alignof( T_Object ) >= 16 );
        
        DMA( PPU_pObject.m_pObject, size );
    }
    
    ~AutoDmaObject_Aligned()
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        #endif
        
        free( m_SPU_pBuffer );
    }
    
    void operator = ( T_Object * const PPU_pObject )
    {
        DMA( PPU_pObject, sizeof( T_Object ) );
    }
    
    void operator = ( const PPU_Ptr< T_Object > PPU_pObject )
    {
        DMA( PPU_pObject.m_pObject, sizeof( T_Object ) );
    }
    
    SPU_Ptr< T_Object > Sync() const
    {
        cellDmaWaitTagStatusAll( 1 << m_DmaTag );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        m_SyncRequired = false;
        #endif
        
        return SPU_Ptr< T_Object >( (T_Object*)m_SPU_pBuffer );
    }
    
    //operator SPU_Ptr< T_Object > () const
    //{
    //    #if defined( PHYSICS_DEBUG_SPU_DMA )
    //    spu_assert( !m_SyncRequired );
    //    #endif
    //    
    //    return SPU_Ptr< T_Object >( (T_Object*)m_SPU_pBuffer );
    //}
    
    T_Object * operator -> () const
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        #endif
        
        return (T_Object*)m_SPU_pBuffer;
    }
    
    //T_Object & operator * () const
    //{
    //    #if defined( PHYSICS_DEBUG_SPU_DMA )
    //    spu_assert( !m_SyncRequired );
    //    #endif
    //    
    //    return *(T_Object*)m_SPU_pBuffer;
    //}
    
    SPU_Ptr< T_Object > operator & () const
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        #endif
        
        return SPU_Ptr< T_Object >( (T_Object*)m_SPU_pBuffer );
    }
    
private:
    
    #if defined( PHYSICS_DEBUG_SPU_DMA )
    public : mutable bool   m_SyncRequired;
    #endif
    
    void*                   m_SPU_pBuffer;
};

/// SPU Unligned Auto-DMA Ptr
template < typename T_Object, unsigned int m_DmaTag = 1, unsigned int m_MaxSize = sizeof( T_Object ) >
class AutoDmaObject_Unaligned
{
public:
    
    AutoDmaObject_Unaligned()
        : m_SPU_pObject( NULL )
        , m_SPU_pBuffer( memalign( 0x10, m_BufferSize ) )
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        , m_SyncRequired( true )
        #endif
    {
        BIZ_STATIC_ASSERT( __alignof( T_Object ) < 16 );
    }
    
    void DMA( T_Object * const PPU_pObject, const U32 size = sizeof( T_Object ) )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_ge( (U32)PPU_pObject, 0x40000 );
        spu_assert( size );
        spu_assert_le( size, m_MaxSize );
        #endif
        
        const U32 src = (U32)PPU_pObject & ~0xF;
        const U32 offset = (U32)PPU_pObject & 0xF;
        cellDmaGet( m_SPU_pBuffer, (uint64_t)src, m_BufferSize, m_DmaTag, 0, 0 );
        m_SPU_pObject = (T_Object*)( (U32)m_SPU_pBuffer + offset );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        m_SyncRequired = true;
        #endif
        
        //spu_printf( "%s: %08X, %05X\n", __DEBUG_FUNC__, PPU_pObject, m_SPU_pBuffer );
    }
    
    AutoDmaObject_Unaligned( T_Object * const PPU_pObject )
        : m_SPU_pBuffer( memalign( 0x10, m_BufferSize ) )
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        , m_SyncRequired( true )
        #endif
    {
        BIZ_STATIC_ASSERT( __alignof( T_Object ) < 16 );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_ge( (U32)PPU_pObject, 0x40000 );
        #endif
        
        DMA( PPU_pObject );
    }
    
    AutoDmaObject_Unaligned( const PPU_Ptr< T_Object > PPU_pObject )
        : m_SPU_pBuffer( memalign( 0x10, m_BufferSize ) )
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        , m_SyncRequired( true )
        #endif
    {
        BIZ_STATIC_ASSERT( __alignof( T_Object ) < 16 );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_ge( (U32)PPU_pObject.m_pObject, 0x40000 );
        #endif
        
        DMA( PPU_pObject.m_pObject );
    }
    
    ~AutoDmaObject_Unaligned()
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        #endif
        
        free( m_SPU_pBuffer );
    }
    
    void operator = ( T_Object * const PPU_pObject )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_ge( (U32)PPU_pObject, 0x40000 );
        #endif
        
        DMA( PPU_pObject );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        m_SyncRequired = true;
        #endif
    }
    
    void operator = ( const PPU_Ptr< T_Object > PPU_pObject )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_ge( (U32)PPU_pObject.m_pObject, 0x40000 );
        #endif
        
        DMA( PPU_pObject.m_pObject );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        m_SyncRequired = true;
        #endif
    }
    
    SPU_Ptr< T_Object > Sync() const
    {
        cellDmaWaitTagStatusAll( 1 << m_DmaTag );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        m_SyncRequired = false;
        #endif
        
        return SPU_Ptr< T_Object >( m_SPU_pObject );
    }
    
    //operator SPU_Ptr< T_Object > () const
    //{
    //    #if defined( PHYSICS_DEBUG_SPU_DMA )
    //    spu_assert( !m_SyncRequired );
    //    #endif
    //    
    //    return SPU_Ptr< T_Object >( m_SPU_pObject );
    //}
    
    T_Object * operator -> () const
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        #endif
        
        return m_SPU_pObject;
    }
    
    //T_Object & operator * () const
    //{
    //    #if defined( PHYSICS_DEBUG_SPU_DMA )
    //    spu_assert( !m_SyncRequired );
    //    #endif
    //    
    //    return *m_SPU_pObject;
    //}
    
    SPU_Ptr< T_Object > operator & () const
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        #endif
        
        return SPU_Ptr< T_Object >( m_SPU_pObject );
    }
    
private:
    
    static const U32        m_BufferSize = ( ( m_MaxSize + 0xF ) & ~0xF ) + 0x10;
    
    T_Object const *        m_SPU_pObject;
    void*                   m_SPU_pBuffer;
    
    #if defined( PHYSICS_DEBUG_SPU_DMA )
    public : mutable bool   m_SyncRequired;
    #endif
};

/// SPU Unligned Auto-DMA Array
template < typename T_Element, unsigned int m_DmaTag = 1 >
class AutoDmaArray_Unaligned
{
private:
    
    void DMA( T_Element * const PPU_pArray, const U32 elements )
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert_ge( (U32)PPU_pArray, 0x40000 );
        spu_assert( elements );
        #endif
        
        const U32 arraySize = sizeof( T_Element ) * elements;
        U32 bufferSize = ( ( arraySize + 0xF ) & ~0xF ) + 0x10;
        
        m_SPU_pBuffer = memalign( 0x10, bufferSize ); // Align spu buffer to 16 byte boundary
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( m_SPU_pBuffer );
        spu_assert_lt( (U32)m_SPU_pBuffer, 0x40000 );
        #endif
        
        char* SPU_pDst = (char*)m_SPU_pBuffer;
        
        U32 PPU_pSrc = (U32)PPU_pArray & ~0xF;
        const U32 offset = (U32)PPU_pArray & 0xF;
        
        while ( bufferSize > 0x4000 )
        {
            cellDmaGet( (void*)SPU_pDst, (uint64_t)PPU_pSrc, 0x4000, m_DmaTag, 0, 0 );
            SPU_pDst += 0x4000;
            PPU_pSrc += 0x4000;
            bufferSize -= 0x4000;
        }
        
        cellDmaGet( (void*)SPU_pDst, (uint64_t)PPU_pSrc, bufferSize, m_DmaTag, 0, 0 );
        
        m_SPU_pArray = (T_Element*)( (char*)m_SPU_pBuffer + offset );
    }
    
public:
    
    AutoDmaArray_Unaligned( T_Element * const PPU_pArray, const U32 elements )
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        : m_Elements( elements )
        , m_SyncRequired( true )
        #endif
    {
        DMA( PPU_pArray, elements );
    }
    
    AutoDmaArray_Unaligned( const PPU_Ptr< T_Element > PPU_pArray, const U32 elements )
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        : m_Elements( elements )
        , m_SyncRequired( true )
        #endif
    {
        DMA( PPU_pArray.m_pObject, elements );
    }
    
    ~AutoDmaArray_Unaligned()
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        #endif
        
        free( m_SPU_pBuffer );
    }
    
    SPU_Ptr< T_Element > Sync() const
    {
        cellDmaWaitTagStatusAll( 1 << m_DmaTag );
        
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        m_SyncRequired = false;
        #endif
        
        return SPU_Ptr< T_Element >( m_SPU_pArray );
    }
    
    operator SPU_Ptr< T_Element > () const
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        #endif
        
        return SPU_Ptr< T_Element >( m_SPU_pArray );
    }
    
    T_Element & operator [] ( const U32 index ) const
    {
        #if defined( PHYSICS_DEBUG_SPU_DMA )
        spu_assert( !m_SyncRequired );
        spu_assert_lt( index, m_Elements );
        #endif
        
        return m_SPU_pArray[ index ];
    }
    
private:
    
    void*                   m_SPU_pBuffer;
    T_Element const *       m_SPU_pArray;
    
    #if defined( PHYSICS_DEBUG_SPU_DMA )
    const U32               m_Elements;
    public : mutable bool   m_SyncRequired;
    #endif
};


/// Multiple Object Sync Functions [ Add more as required... ]

template < typename T0, typename T1, unsigned int DmaTag0, unsigned int DmaTag1 >
void Sync( const AutoDmaObject_Unaligned< T0, DmaTag0 >& object0, 
           const AutoDmaObject_Unaligned< T1, DmaTag1 >& object1 )
{
    cellDmaWaitTagStatusAll( ( 1 << DmaTag0 ) | ( 1 << DmaTag1 ) );
    
    #if defined( PHYSICS_DEBUG_SPU_DMA )
    object0.m_SyncRequired = false;
    object1.m_SyncRequired = false;
    #endif
}

template < typename T0, typename T1, unsigned int DmaTag0, unsigned int DmaTag1 >
void Sync( const AutoDmaObject_Aligned< T0, DmaTag0 >& object0, 
           const AutoDmaObject_Aligned< T1, DmaTag1 >& object1 )
{
    cellDmaWaitTagStatusAll( ( 1 << DmaTag0 ) | ( 1 << DmaTag1 ) );
    
    #if defined( PHYSICS_DEBUG_SPU_DMA )
    object0.m_SyncRequired = false;
    object1.m_SyncRequired = false;
    #endif
}

template < typename T, unsigned int DmaTag >
void Sync( const AutoDmaObject_Aligned< T, DmaTag >& object0, 
           const AutoDmaObject_Aligned< T, DmaTag >& object1, 
           const AutoDmaObject_Aligned< T, DmaTag >& object2, 
           const AutoDmaObject_Aligned< T, DmaTag >& object3, 
           const AutoDmaObject_Aligned< T, DmaTag >& object4, 
           const AutoDmaObject_Aligned< T, DmaTag >& object5, 
           const AutoDmaObject_Aligned< T, DmaTag >& object6, 
           const AutoDmaObject_Aligned< T, DmaTag >& object7 )
{
    cellDmaWaitTagStatusAll( 1 << DmaTag );
    
    #if defined( PHYSICS_DEBUG_SPU_DMA )
    object0.m_SyncRequired = false;
    object1.m_SyncRequired = false;
    object2.m_SyncRequired = false;
    object3.m_SyncRequired = false;
    object4.m_SyncRequired = false;
    object5.m_SyncRequired = false;
    object6.m_SyncRequired = false;
    object7.m_SyncRequired = false;
    #endif
}

template < typename T, unsigned int DmaTag >
void Sync( const AutoDmaObject_Unaligned< T, DmaTag >& object0, 
           const AutoDmaObject_Unaligned< T, DmaTag >& object1, 
           const AutoDmaObject_Unaligned< T, DmaTag >& object2, 
           const AutoDmaObject_Unaligned< T, DmaTag >& object3, 
           const AutoDmaObject_Unaligned< T, DmaTag >& object4, 
           const AutoDmaObject_Unaligned< T, DmaTag >& object5, 
           const AutoDmaObject_Unaligned< T, DmaTag >& object6, 
           const AutoDmaObject_Unaligned< T, DmaTag >& object7 )
{
    cellDmaWaitTagStatusAll( 1 << DmaTag );
    
    #if defined( PHYSICS_DEBUG_SPU_DMA )
    object0.m_SyncRequired = false;
    object1.m_SyncRequired = false;
    object2.m_SyncRequired = false;
    object3.m_SyncRequired = false;
    object4.m_SyncRequired = false;
    object5.m_SyncRequired = false;
    object6.m_SyncRequired = false;
    object7.m_SyncRequired = false;
    #endif
}

}; // namespace Spu

#else // #if defined( PLATFORM_SPU )

namespace Spu
{

/// PPU Pointer class
template < typename T_Object >
class PPU_Ptr
{
public:
    
    inline PPU_Ptr() __attribute__(( always_inline )) /* : m_pObject( NULL ) */ {}
    
    inline PPU_Ptr( T_Object* const pObject ) __attribute__(( always_inline )) : m_pObject( pObject ) {}
    
    inline PPU_Ptr( const PPU_Ptr< T_Object > & pObject ) __attribute__(( always_inline )) : m_pObject( pObject.m_pObject ) {}
    
    inline ~PPU_Ptr() __attribute__(( always_inline )) {}
    
    inline void operator = ( T_Object* const pObject ) __attribute__(( always_inline )) { m_pObject = pObject; }
    
    inline void operator = ( const PPU_Ptr< T_Object > & pObject ) __attribute__(( always_inline )) { m_pObject = pObject.m_pObject; }
    
    inline T_Object * operator -> () const __attribute__(( /* pure, */ always_inline )) { return m_pObject; }
    
    inline T_Object & operator * () const __attribute__(( /* pure, */ always_inline )) { return *m_pObject; }
    
    inline operator T_Object * () const __attribute__(( /* pure, */ always_inline )) { return m_pObject; }
    
    inline PPU_Ptr< T_Object > operator ++ ( int ) /* postfix */ __attribute__(( always_inline )) { return PPU_Ptr< T_Object >( m_pObject++ ); }
    inline PPU_Ptr< T_Object > operator ++ ()      /* prefix */  __attribute__(( always_inline )) { return PPU_Ptr< T_Object >( ++m_pObject ); }
    
    inline PPU_Ptr< T_Object > operator + ( const U32 n ) const __attribute__(( always_inline )) { return PPU_Ptr< T_Object >( m_pObject + n ); }
    inline PPU_Ptr< T_Object > operator + ( const S32 n ) const __attribute__(( always_inline )) { return PPU_Ptr< T_Object >( m_pObject + n ); }
    
    inline void operator += ( const U32 n ) __attribute__(( always_inline )) { m_pObject += n; }
    inline void operator += ( const S32 n ) __attribute__(( always_inline )) { m_pObject += n; }
    
    inline operator bool () const __attribute__(( /* pure, */ always_inline )) { return m_pObject != NULL; }
    
    template < typename T > inline PPU_Ptr<T> Cast() const { return PPU_Ptr<T>( (T*)m_pObject ); }
    
private:
    
    template < typename T > friend class PPU_Ptr;
    
    T_Object*   m_pObject;
};

/// SPU Pointer class
template < typename T_Object >
class SPU_Ptr
{
public:
    
    inline SPU_Ptr() {}
    
    template < typename T > inline SPU_Ptr<T> Cast() const { return SPU_Ptr<T>(); }
    
private:
    
    T_Object*   m_pObject;
};

}; // namespace Spu

#endif // #if defined( PLATFORM_SPU )

#endif

namespace Game {
namespace Actors {

struct DeferredVehicleUpdateTaskData
{
    //Spu::PPU_Ptr< Vehicle::C_PartList >         m_pPartList;
    //Spu::PPU_Ptr< ::Actors::C_VisibleDetect >   m_pVisDetect;
    //Spu::PPU_Ptr< Maths::HMat43 >               m_pMatrix;
    //Spu::PPU_Ptr< Maths::HMat43 >               m_pRawMatrix;
    PPU_Ptr< Vehicle::C_PartList >         m_pPartList;
    PPU_Ptr< ::Actors::C_VisibleDetect >   m_pVisDetect;
    PPU_Ptr< Maths::HMat43 >               m_pMatrix;
    PPU_Ptr< Maths::HMat43 >               m_pRawMatrix;
};

} // namespace Actors
} // namespace Game

#endif
