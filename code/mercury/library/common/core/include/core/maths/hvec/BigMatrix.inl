#ifndef BIGMATRIX_INL
#define BIGMATRIX_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{




#if defined( BIG_MATRIX_DEBUG )
NOALIAS inline void BIG_MATRIX_ASSERT( const S32 x )
{
    if ( !x )
    {
        DebugBreak();
    }
}
#else
#define BIG_MATRIX_ASSERT(x)
#endif

#if defined( BIG_MATRIX_DEBUG )
NOALIAS inline void BIG_MATRIX_ASSERT_FINITE( const F64 x )
{
    if ( !_finite(x) || ( Maths::Abs(x) > 3.0e30 ) )
    {
        DebugBreak();
    }
}
#else
#define BIG_MATRIX_ASSERT_FINITE(x)
#endif

#if defined( BIG_MATRIX_DEBUG )
NOALIAS inline void BIG_MATRIX_ASSERT_ZERO( const F64 x )
{
    if ( Maths::Abs(x) > BIG_MATRIX_NEARLY_ZERO )
    {
        DebugBreak();
    }
}
#else
#define BIG_MATRIX_ASSERT_ZERO(x)
#endif

#if defined( BIG_MATRIX_DEBUG )
NOALIAS inline void BIG_MATRIX_ASSERT_NONZERO( const F64 x )
{
    if ( Maths::Abs(x) <= BIG_MATRIX_NEARLY_ZERO )
    {
        DebugBreak();
    }
}
#else
#define BIG_MATRIX_ASSERT_NONZERO(x)
#endif

#if defined( BIG_MATRIX_DEBUG )
NOALIAS inline void BIG_MATRIX_ASSERT_POSITIVE( const F64 x )
{
    if ( x < -BIG_MATRIX_NEARLY_ZERO )
    {
        DebugBreak();
    }
}
#else
#define BIG_MATRIX_ASSERT_POSITIVE(x)
#endif

#if defined( BIG_MATRIX_DEBUG )
NOALIAS inline void BIG_MATRIX_ASSERT_EQUAL( const F64 x, const F64 y )
{
    if ( Maths::Abs( x - y ) > BIG_MATRIX_NEARLY_ZERO )
    {
        DebugBreak();
    }
}
#else
#define BIG_MATRIX_ASSERT_EQUAL(x,y)
#endif






//NOALIAS __forceinline const F32 Reciprocal( const F32 B )
//{
//    #if defined( PLATFORM_XENON )
//    
//    const F32 one( 1.0 );
//    const F32 y0( __fres( B ) );                        // 8 bit accuracy
//    const F32 y1( y0 + ( y0 * ( one - ( B * y0 ) ) ) ); // 16 bit accuracy
//    const F32 y2( y1 + ( y1 * ( one - ( B * y1 ) ) ) ); // 24 bit accuracy
//    return y2;
//    
//    #else
//    
//    return 1.0f / B;
//    
//    #endif
//}

NOALIAS __forceinline const F32 Divide( const F32 A, const F32 B )
{
    //#if defined( PLATFORM_XENON )
    //
    //const F32 one( 1.0 );
    //const F32 y0( __fres( B ) );                        // 8 bit accuracy
    //const F32 y1( y0 + ( y0 * ( one - ( B * y0 ) ) ) ); // 16 bit accuracy
    //const F32 y2( y1 + ( y1 * ( one - ( B * y1 ) ) ) ); // 24 bit accuracy
    //return A * y2;
    //
    //#else
    //
    //return A / B;
    //
    //#endif
    
    return A * Reciprocal( B );
}

NOALIAS __forceinline const F64 Divide( const F64 A, const F64 B )
{
    //#if defined( PLATFORM_XENON )
    //
    //const F64 one( 1.0 );
    //const F64 y0( __fres( B ) );                        // 8 bit accuracy
    //const F64 y1( y0 + ( y0 * ( one - ( B * y0 ) ) ) ); // 16 bit accuracy
    //const F64 y2( y1 + ( y1 * ( one - ( B * y1 ) ) ) ); // 32 bit accuracy
    //const F64 y3( y2 + ( y2 * ( one - ( B * y2 ) ) ) ); // 53 bit accuracy
    //return A * y3;
    //
    //#else
    //
    //return A / B;
    //
    //#endif
    
    return A * Reciprocal( B );
}


NOALIAS __forceinline const F32 VectorSumOfSquares(
    F32 const * const __restrict pA,
    const S32 elements )
{
    F32 sos = 0.0;
    for ( S32 i=0; i<elements; i++ )
    {
        sos += pA[i] * pA[i];
    }
    return sos;
}

NOALIAS __forceinline const F64 VectorSumOfSquares(
    F64 const * const __restrict pA,
    const S32 elements )
{
    F64 sos = 0.0;
    for ( S32 i=0; i<elements; i++ )
    {
        sos += pA[i] * pA[i];
    }
    return sos;
}

NOALIAS __forceinline const F32 VectorDotProduct(
    F32 const * __restrict pA,
    F32 const * __restrict pB,
    S32 elements )
{
    F32 dp = 0.0;
    for ( S32 i=0; i<elements; i++ )
    {
        dp += pA[i] * pB[i];
    }
    return dp;
}

NOALIAS __forceinline const F64 VectorDotProduct(
    F64 const * __restrict pA,
    F64 const * __restrict pB,
    S32 elements )
{
    F64 dp = 0.0;
    for ( S32 i=0; i<elements; i++ )
    {
        dp += pA[i] * pB[i];
    }
    return dp;
}




// Pre-Calculate the size of a vector [ elements ]
template < typename T_Element >
const S32 C_BigVector<T_Element>::Sizeof( const S32 elements )
{
    const S32 paddedElements( ( elements + (m_ElementPad-1) ) & ~(m_ElementPad-1) );
    return sizeof( T_Element ) * paddedElements;
}

template < typename T_Element >
C_BigVector<T_Element>::C_BigVector() :
    m_Elements( 0 ),
    m_pVector( NULL ),
    m_pMemory( NULL )
{
}

template < typename T_Element >
C_BigVector<T_Element>::C_BigVector( const S32 elements ) :
    m_Elements( elements )
{
    const S32 paddedElements( ( elements + (m_ElementPad-1) ) & ~(m_ElementPad-1) );
    m_pVector = new T_Element[ paddedElements ];
    m_pMemory = (char*)m_pVector;
}

template < typename T_Element >
C_BigVector<T_Element>::C_BigVector( const S32 elements, T_Element* pVector ) :
    m_Elements( elements ),
    m_pVector( pVector ),
    m_pMemory( NULL )
{
}

template < typename T_Element >
C_BigVector<T_Element>::C_BigVector( const S32 elements, char* pMemory, const S32 memorySize ) :
    m_Elements( elements ),
    m_pVector( (T_Element*)pMemory ),
    m_pMemory( NULL )
{
    BIG_MATRIX_ASSERT( Sizeof( elements ) <= memorySize );
}

template < typename T_Element >
C_BigVector<T_Element>::~C_BigVector()
{
    if ( m_pMemory )
    {
        delete [] m_pMemory;
    }
}

template < typename T_Element >
NOALIAS C_BigVector<T_Element>::operator const T_Element* () const __restrict
{
    return m_pVector;
}

template < typename T_Element >
NOALIAS C_BigVector<T_Element>::operator T_Element* () __restrict
{
    return m_pVector;
}

template < typename T_Element >
NOALIAS const T_Element* C_BigVector<T_Element>::Vector() const __restrict
{
    return m_pVector;
}

template < typename T_Element >
NOALIAS T_Element* C_BigVector<T_Element>::Vector() __restrict
{
    return m_pVector;
}

template < typename T_Element >
NOALIAS __forceinline void C_BigVector<T_Element>::Initialise( const S32 elements, char* pMemory, const S32 memorySize ) __restrict
{
    if ( m_pMemory )
    {
        delete [] m_pMemory;
    }
    
    BIG_MATRIX_ASSERT( Sizeof( elements ) <= memorySize );
    m_Elements = elements;
    m_pVector = (T_Element*)pMemory;
    m_pMemory = NULL;
}

template < typename T_Element >
NOALIAS __forceinline const T_Element C_BigVector<T_Element>::operator [] ( const U32 element ) const __restrict
{
    return m_pVector[element];
}

template < typename T_Element >
NOALIAS __forceinline const T_Element C_BigVector<T_Element>::operator [] ( const S32 element ) const __restrict
{
    return m_pVector[element];
}

template < typename T_Element >
NOALIAS __forceinline T_Element& C_BigVector<T_Element>::operator [] ( const U32 element ) __restrict
{
    return m_pVector[element];
}

template < typename T_Element >
NOALIAS __forceinline T_Element& C_BigVector<T_Element>::operator [] ( const S32 element ) __restrict
{
    return m_pVector[element];
}

template < typename T_Element >
NOALIAS __forceinline void C_BigVector<T_Element>::SwapElements( const S32 a, const S32 b ) __restrict
{
    const T_Element A( m_pVector[a] );
    const T_Element B( m_pVector[b] );
    m_pVector[b] = A;
    m_pVector[a] = B;
}

















// Pre-Calculate the size of a matrix [ rows, columns ]
template < typename T_Element >
const S32 C_BigMatrix<T_Element>::Sizeof( const S32 rows, const S32 columns )
{
    const S32 rowStep( ( columns + (m_RowPad-1) ) & ~(m_RowPad-1) );
    const S32 rowSize( rowStep * sizeof( T_Element ) );
    const S32 matrixSize( rows * rowSize );
    const S32 rowArraySize( ( ( rows * sizeof( T_Element* ) ) + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
    return matrixSize + rowArraySize + m_DataAlignment; // Plus m_DataAlignment bytes for alignment!
}

template < typename T_Element >
C_BigMatrix<T_Element>::C_BigMatrix() :
    m_Rows( 0 ),
    m_Columns( 0 ),
    m_RowStep( 0 ),
    m_pRowArray( NULL ),
    m_pMatrix( NULL ),
    m_pMemory( NULL )
{
}

template < typename T_Element >
C_BigMatrix<T_Element>::C_BigMatrix( const S32 rows, const S32 columns ) :
    m_Rows( rows ),
    m_Columns( columns ),
    m_RowStep( ( columns + (m_RowPad-1) ) & ~(m_RowPad-1) )
{
    const S32 rowSize( m_RowStep * sizeof( T_Element ) );
    const S32 matrixSize( rows * rowSize );
    const S32 rowArraySize( ( ( rows * sizeof( T_Element* ) ) + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
    const S32 rawDataSize( rowArraySize + matrixSize + m_DataAlignment ); // Plus m_DataAlignment bytes for alignment!
    
    char* rawPtr( new char[ rawDataSize ] );
    m_pMemory = rawPtr;
    
    rawPtr = (char*)( ( (S32)rawPtr + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
    
    m_pMatrix = (T_Element*)rawPtr; rawPtr += matrixSize;
    
    m_pRowArray = (T_Element**)rawPtr; rawPtr += rowArraySize;
    
    T_Element* pRow = m_pMatrix;
    for ( S32 i=0; i<rows; i++ )
    {
        m_pRowArray[i] = (T_Element*)pRow; pRow += m_RowStep;
    }
}

// Constructor: rows & columns, with pre-allocated memory
template < typename T_Element >
C_BigMatrix<T_Element>::C_BigMatrix( const S32 rows, const S32 columns, char* pMemory, const S32 size ) :
    m_Rows( rows ),
    m_Columns( columns ),
    m_RowStep( ( columns + (m_RowPad-1) ) & ~(m_RowPad-1) ),
    m_pRowArray( NULL ),
    m_pMatrix( NULL ),
    m_pMemory( NULL )
{
    BIG_MATRIX_ASSERT( Sizeof( rows, columns ) <= size );
    
    const S32 rowSize( m_RowStep * sizeof( T_Element ) );
    const S32 matrixSize( rows * rowSize );
    const S32 rowArraySize( ( ( rows * sizeof( T_Element* ) ) + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
    
    pMemory = (char*)( ( (S32)pMemory + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
    
    m_pMatrix = (T_Element*)pMemory; pMemory += matrixSize;
    
    m_pRowArray = (T_Element**)pMemory; pMemory += rowArraySize;
    
    T_Element* pRow = m_pMatrix;
    for ( S32 i=0; i<rows; i++ )
    {
        m_pRowArray[i] = (T_Element*)pRow; pRow += m_RowStep;
    }
}

template < typename T_Element >
C_BigMatrix<T_Element>::~C_BigMatrix()
{
    if ( m_pMemory )
    {
        delete [] m_pMemory;
    }
}

template < typename T_Element >
void C_BigMatrix<T_Element>::Initialise( const S32 rows, const S32 columns )
{
    if ( !m_pMemory || ( ( rows != m_Rows ) && ( columns != m_Columns ) ) )
    {
        if ( m_pMemory )
        {
            delete [] m_pMemory;
        }
        
        m_Rows = rows;
        m_Columns = columns;
        m_RowStep = ( columns + (m_RowPad-1) ) & ~(m_RowPad-1);
        
        const S32 rowSize( m_RowStep * sizeof( T_Element ) );
        const S32 matrixSize( rows * rowSize );
        const S32 rowArraySize( ( ( rows * sizeof( T_Element* ) ) + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
        const S32 rawDataSize( rowArraySize + matrixSize + m_DataAlignment );
        
        char* rawPtr( new char[ rawDataSize ] );
        m_pMemory = rawPtr;
        
        rawPtr = (char*)( ( (S32)rawPtr + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
        
        m_pMatrix = (T_Element*)rawPtr; rawPtr += matrixSize;
        
        m_pRowArray = (T_Element**)rawPtr; rawPtr += rowArraySize;
        
        T_Element* pRow = m_pMatrix;
        for ( S32 i=0; i<rows; i++ )
        {
            m_pRowArray[i] = (T_Element*)pRow; pRow += m_RowStep;
        }
    }
}

template < typename T_Element >
void C_BigMatrix<T_Element>::Initialise( const S32 rows, const S32 columns, char* pMemory, const S32 memorySize )
{
    if ( m_pMemory )
    {
        delete [] m_pMemory;
    }
    
    m_Rows = rows;
    m_Columns = columns;
    m_RowStep = ( columns + (m_RowPad-1) ) & ~(m_RowPad-1);
    
    const S32 rowSize( m_RowStep * sizeof( T_Element ) );
    const S32 matrixSize( rows * rowSize );
    const S32 rowArraySize( ( ( rows * sizeof( T_Element* ) ) + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
    const S32 rawDataSize( rowArraySize + matrixSize + m_DataAlignment );
    
    BIG_MATRIX_ASSERT( rawDataSize <= memorySize );
    char* rawPtr( pMemory );
    m_pMemory = NULL;
    
    rawPtr = (char*)( ( (S32)rawPtr + (m_DataAlignment-1) ) & ~(m_DataAlignment-1) );
    
    m_pMatrix = (T_Element*)rawPtr; rawPtr += matrixSize;
    
    m_pRowArray = (T_Element**)rawPtr; rawPtr += rowArraySize;
    
    T_Element* pRow = m_pMatrix;
    for ( S32 i=0; i<rows; i++ )
    {
        m_pRowArray[i] = (T_Element*)pRow; pRow += m_RowStep;
    }
}

template < typename T_Element >
NOALIAS __forceinline T_Element const * C_BigMatrix<T_Element>::operator [] ( const U32 row ) const __restrict
{
    BIG_MATRIX_ASSERT( row < m_Rows );
    return m_pRowArray[ row ];
}

template < typename T_Element >
NOALIAS __forceinline T_Element const * C_BigMatrix<T_Element>::operator [] ( const S32 row ) const __restrict
{
    BIG_MATRIX_ASSERT( row < m_Rows );
    return m_pRowArray[ row ];
}

template < typename T_Element >
NOALIAS __forceinline T_Element* C_BigMatrix<T_Element>::operator [] ( const U32 row ) __restrict
{
    BIG_MATRIX_ASSERT( (S32)row < m_Rows );
    return m_pRowArray[ row ];
}

template < typename T_Element >
NOALIAS __forceinline T_Element* C_BigMatrix<T_Element>::operator [] ( const S32 row ) __restrict
{
    BIG_MATRIX_ASSERT( row < m_Rows );
    return m_pRowArray[ row ];
}

template < typename T_Element >
NOALIAS __forceinline void C_BigMatrix<T_Element>::SwapRows( const S32 a, const S32 b ) __restrict
{
    BIG_MATRIX_ASSERT( a < m_Rows );
    BIG_MATRIX_ASSERT( b < m_Rows );
    
    T_Element* __restrict * const __restrict ppRowA = &m_pRowArray[a];
    T_Element* __restrict * const __restrict ppRowB = &m_pRowArray[b];
    
    T_Element* const __restrict pRowA = *ppRowA; ReadWriteBarrier();
    T_Element* const __restrict pRowB = *ppRowB; ReadWriteBarrier();
    *ppRowB = pRowA; ReadWriteBarrier();
    *ppRowA = pRowB;
}

template < typename T_Element >
//NOALIAS __forceinline void C_BigMatrix<T_Element>::SwapColumns( const S32 a, const S32 b ) __restrict
NOALIAS /*__forceinline*/ void C_BigMatrix<T_Element>::SwapColumns( const S32 a, const S32 b ) __restrict
{
    BIG_MATRIX_ASSERT( a < m_Columns );
    BIG_MATRIX_ASSERT( b < m_Columns );
    
    #if 1
    
    #if defined( COMPILER_VC )
    
    //const S32 rowSize( m_RowStep * sizeof( T_Element ) );
    //const S32 rowSize_x4( m_RowStep * 4 * sizeof( T_Element ) );
    const S32 rowStep( m_RowStep );
    const S32 rowStep_x4( rowStep * 4 );
    const S32 rowSize_x4( rowStep_x4 * sizeof( T_Element ) );
    
    T_Element* __restrict pA0 = &m_pMatrix[a];
    T_Element* __restrict pB0 = &m_pMatrix[b];
    //T_Element* __restrict pA1 = (T_Element*)( (S32)pA0 + rowSize );
    //T_Element* __restrict pB1 = (T_Element*)( (S32)pB0 + rowSize );
    //T_Element* __restrict pA2 = (T_Element*)( (S32)pA1 + rowSize );
    //T_Element* __restrict pB2 = (T_Element*)( (S32)pB1 + rowSize );
    //T_Element* __restrict pA3 = (T_Element*)( (S32)pA2 + rowSize );
    //T_Element* __restrict pB3 = (T_Element*)( (S32)pB2 + rowSize );
    T_Element* __restrict pA1 = pA0 + rowStep;
    T_Element* __restrict pB1 = pB0 + rowStep;
    T_Element* __restrict pA2 = pA1 + rowStep;
    T_Element* __restrict pB2 = pB1 + rowStep;
    T_Element* __restrict pA3 = pA2 + rowStep;
    T_Element* __restrict pB3 = pB2 + rowStep;
    
    #if defined( _VMX128_ )
    __dcbt( 0x0, pA0 );
    __dcbt( 0x0, pB0 );
    __dcbt( 0x0, pA1 );
    __dcbt( 0x0, pB1 );
    __dcbt( 0x0, pA2 );
    __dcbt( 0x0, pB2 );
    __dcbt( 0x0, pA3 );
    __dcbt( 0x0, pB3 );
    #endif
    
    S32 rows( m_Rows );
    
    while ( rows >= 4 )
    {
        const T_Element elementA0( *pA0 ); ReadWriteBarrier(); // *** TRACE ANALYSIS: L1-MISS HERE! ***
        const T_Element elementB0( *pB0 ); ReadWriteBarrier();
        const T_Element elementA1( *pA1 ); ReadWriteBarrier();
        const T_Element elementB1( *pB1 ); ReadWriteBarrier();
        const T_Element elementA2( *pA2 ); ReadWriteBarrier();
        const T_Element elementB2( *pB2 ); ReadWriteBarrier();
        const T_Element elementA3( *pA3 ); ReadWriteBarrier();
        const T_Element elementB3( *pB3 ); ReadWriteBarrier();
        
        #if defined( _VMX128_ )
        __dcbt( rowSize_x4, pA0 );
        __dcbt( rowSize_x4, pB0 );
        __dcbt( rowSize_x4, pA1 );
        __dcbt( rowSize_x4, pB1 );
        __dcbt( rowSize_x4, pA2 );
        __dcbt( rowSize_x4, pB2 );
        __dcbt( rowSize_x4, pA3 );
        __dcbt( rowSize_x4, pB3 );
        #endif
        
        *pB0 = elementA0; ReadWriteBarrier();
        *pA0 = elementB0; ReadWriteBarrier();
        *pB1 = elementA1; ReadWriteBarrier();
        *pA1 = elementB1; ReadWriteBarrier();
        *pB2 = elementA2; ReadWriteBarrier();
        *pA2 = elementB2; ReadWriteBarrier();
        *pB3 = elementA3; ReadWriteBarrier();
        *pA3 = elementB3;
        
        //pA0 = (T_Element*)( (S32)pA0 + rowSize_x4 );
        //pB0 = (T_Element*)( (S32)pB0 + rowSize_x4 );
        //pA1 = (T_Element*)( (S32)pA1 + rowSize_x4 );
        //pB1 = (T_Element*)( (S32)pB1 + rowSize_x4 );
        //pA2 = (T_Element*)( (S32)pA2 + rowSize_x4 );
        //pB2 = (T_Element*)( (S32)pB2 + rowSize_x4 );
        //pA3 = (T_Element*)( (S32)pA3 + rowSize_x4 );
        //pB3 = (T_Element*)( (S32)pB3 + rowSize_x4 );
        pA0 += rowStep_x4;
        pB0 += rowStep_x4;
        pA1 += rowStep_x4;
        pB1 += rowStep_x4;
        pA2 += rowStep_x4;
        pB2 += rowStep_x4;
        pA3 += rowStep_x4;
        pB3 += rowStep_x4;
        
        rows -= 4;
    }
    
    __assume( rows < 4 );
    while ( rows > 0 )
    {
        const T_Element elementA( *pA0 ); ReadWriteBarrier();
        const T_Element elementB( *pB0 ); ReadWriteBarrier();
        *pB0 = elementA; ReadWriteBarrier();
        *pA0 = elementB;
        
        //pA0 = (T_Element*)( (S32)pA0 + rowSize );
        //pB0 = (T_Element*)( (S32)pB0 + rowSize );
        pA0 += rowStep;
        pB0 += rowStep;
        
        rows--;
    }
    
    #else
    
    T_Element* __restrict pA = &m_pMatrix[a];
    T_Element* __restrict pB = &m_pMatrix[b];
    
    S32 rows( m_Rows );
    while ( rows > 0 )
    {
        const T_Element elementA( *pA );
        const T_Element elementB( *pB );
        
        *pB = elementA;
        *pA = elementB;
        
        pA += m_RowStep;
        pB += m_RowStep;
        
        rows--;
    }
    
    #endif
    
    #else
    
    for ( S32 i=0; i<m_Rows; i++ )
    {
        T_Element* const __restrict pRow_i = m_pRowArray[i];
        
        const T_Element A = pRow_i[a];
        const T_Element B = pRow_i[b];
        
        pRow_i[b] = A;
        pRow_i[a] = B;
    }
    
    #endif
}

template < typename T_Element >
NOALIAS void C_BigMatrix<T_Element>::operator = ( const C_BigMatrix<T_Element>& B )
{
    C_BigMatrix<T_Element>       * const __restrict pA = this;
    C_BigMatrix<T_Element> const * const __restrict pB = &B;
    
    BIG_MATRIX_ASSERT( ( pA->m_Rows <= pB->m_Rows ) && ( pA->m_Columns <= pB->m_Columns ) );
    
    const S32 rows( pA->m_Rows );
    const S32 cols( pA->m_Columns );
    
    for ( S32 i=0; i<rows; i++ )
    {
        T_Element       * const __restrict pAi = (*pA)[i];
        T_Element const * const __restrict pBi = (*pB)[i];
        
        for ( S32 j=0; j<cols; j++ )
        {
            pAi[j] = pBi[j];
        }
    }
}


template < typename T_Float >
NOALIAS void Add(
    C_BigVector<T_Float> const * __restrict pA,  // In
    C_BigVector<T_Float> const * __restrict pB,  // In
    C_BigVector<T_Float>       * __restrict pC ) // Out
{
    BIG_MATRIX_ASSERT( ( pA->m_Elements == pB->m_Elements ) &&
                       ( pA->m_Elements == pC->m_Elements ) );
    
    const S32 elements( pA->m_Elements );
    
    T_Float const * __restrict pAi = pA->m_pVector;
    T_Float const * __restrict pBi = pB->m_pVector;
    T_Float       * __restrict pCi = pC->m_pVector;
    
    for ( S32 i=0; i<elements; i++, pAi++, pBi++, pCi++ )
    {
        *pCi = *pAi + *pBi;
    }
}

template < typename T_Float >
NOALIAS void Transpose(
    C_BigMatrix<T_Float> const * const __restrict pA,  // In
    C_BigMatrix<T_Float>       * const __restrict pB ) // Out
{
    BIG_MATRIX_ASSERT( ( pA->m_Rows == pB->m_Columns ) &&
                       ( pA->m_Columns == pB->m_Rows ) );
    
    const S32 rows( pA->m_Rows );
    const S32 columns( pA->m_Columns );
    
    for ( S32 row=0; row<rows; row++ )
    {
        T_Float const * __restrict pRowA = (*pA)[row];
        for ( S32 col=0; col<columns; col++, pRowA++ )
        {
            (*pB)[col][row] = *pRowA;
        }
    }
}

template < typename T_Float >
NOALIAS void Multiply(
    C_BigMatrix<T_Float> const * const __restrict pA,  // In
    C_BigVector<T_Float> const * const __restrict pB,  // In
    C_BigVector<T_Float>       * const __restrict pC ) // Out
{
    BIG_MATRIX_ASSERT( ( pA->m_Columns == pB->m_Elements ) &&
                       ( pA->m_Rows    == pC->m_Elements ) );
    
    const S32 rows( pA->m_Rows );
    const S32 cols( pA->m_Columns );
    
    for ( S32 i=0; i<rows; i++ )
    {
        //pC->m_pVector[i] = VectorDotProduct( pA->m_pRowArray[i], pB->m_pVector, cols );
        Store( VectorDotProduct_Aligned( pA->m_pRowArray[i], pB->m_pVector, cols ), &pC->m_pVector[i] );
    }
}

template < typename T_Float >
NOALIAS void Multiply(
    C_BigVector<T_Float> const * const __restrict pA,  // In
    C_BigMatrix<T_Float> const * const __restrict pB,  // In
    C_BigVector<T_Float>       * const __restrict pC ) // Out
{
    BIG_MATRIX_ASSERT( ( pA->m_Elements == pB->m_Rows ) &&
                       ( pC->m_Elements == pB->m_Columns ) );
    
    const S32 rows( pB->m_Rows );
    const S32 cols( pB->m_Columns );
    
    for ( S32 i=0; i<cols; i++ )
    {
        T_Float c = 0.0;
        for ( S32 j=0; j<rows; j++ )
        {
            c += (*pA)[j] * (*pB)[j][i];
        }
        (*pC)[i] = c;
    }
}

template < typename T_Float >
NOALIAS void Multiply(
    C_BigMatrix<T_Float> const * const __restrict pA,  // In
    C_BigMatrix<T_Float> const * const __restrict pB,  // In
    C_BigMatrix<T_Float>       * const __restrict pC ) // Out
{
    BIG_MATRIX_ASSERT( ( pA->m_Rows == pB->m_Columns ) &&
                       ( pA->m_Columns == pB->m_Rows ) &&
                       ( pA->m_Rows == pC->m_Rows ) &&
                       ( pA->m_Rows == pC->m_Columns ) );
    
    const S32 dimensions( pA->m_Rows );
    const S32 elements( pA->m_Columns );
    
    C_BigMatrix<T_Float> BT( pB->m_Columns, pB->m_Rows );
    C_BigMatrix<T_Float>* const __restrict pBT = &BT;
    
    Transpose( pB, pBT );
    
    for ( S32 row=0; row<dimensions; row++ )
    {
        for ( S32 col=0; col<dimensions; col++ )
        {
            //(*pC)[row][col] = VectorDotProduct( (*pA)[row], (*pBT)[col], elements );
            const HF32 dp( VectorDotProduct_Aligned( (*pA)[row], (*pBT)[col], elements ) );
            Store( dp, &(*pC)[row][col] );
        }
    }
}

template < typename T_Float >
NOALIAS void RQ_Decomposition(
    C_BigMatrix<T_Float> const * const __restrict pA,  // In
    C_BigMatrix<T_Float>       * const __restrict pR,  // Out
    C_BigMatrix<T_Float>       * const __restrict pQ ) // Out
{
    // Solve R * Q = A with R upper triangular and Q ortho-normalised
    
    const S32 dimensions( pA->m_Rows );
    
    BIG_MATRIX_ASSERT( ( dimensions == pA->m_Columns ) &&
                       ( dimensions == pR->m_Rows ) &&
                       ( dimensions == pQ->m_Rows ) &&
                       ( dimensions == pR->m_Columns ) &&
                       ( dimensions == pQ->m_Columns ) );
    
    #if 0
    
    // Gram-Schmidt process
    // NOTE: This is far too numerically unstable!!!
    
    //for ( S32 row=0; row<dimensions; row++ )
    //{
    //    T_Float const * const __restrict pRowA = (*pA)[row];
    //    T_Float       * const __restrict pRowQ = (*pQ)[row];
    //    
    //    for ( S32 col=0; col<dimensions; col++ )
    //    {
    //        pRowQ[col] = pRowA[col];
    //    }
    //    
    //    for ( S32 row2=0; row2<row; row2++ )
    //    {
    //        T_Float const * const __restrict pRowQ2 = (*pQ)[row2];
    //        const T_Float proj( VectorDotProduct( pRowQ2, pRowA, dimensions ) );
    //        
    //        for ( S32 col=0; col<dimensions; col++ )
    //        {
    //            pRowQ[col] -= pRowQ2[col] * proj;
    //        }
    //    }
    //    
    //    const T_Float length_squared( VectorSumOfSquares( pRowQ, dimensions ) );
    //    const T_Float length_reciprocal( ReciprocalSqrt( length_squared ) );
    //    
    //    for ( S32 col=0; col<dimensions; col++ )
    //    {
    //        pRowQ[col] *= length_reciprocal;
    //        
    //        #if defined( BIG_MATRIX_DEBUG )
    //        if ( !_finite( pRowQ[col] ) )
    //            DebugBreak();
    //        #endif
    //    }
    //}
    
    #else
    
    S32 row( dimensions - 1 );
    
    #if 1 // Faster specialised version for solving bottom row!
    {
        T_Float const * __restrict pRowA = (*pA)[row];
        T_Float       * __restrict pRowR = (*pR)[row];
        T_Float       * __restrict pRowQ = (*pQ)[row];
        
        // Solve bottom row first
        
        const T_Float R_br_squared( VectorSumOfSquares( pRowA, dimensions ) );
        const T_Float R_br_reciprocal( ReciprocalSqrt( R_br_squared ) );
        
        for ( S32 col=0; col<dimensions; col++ )
        {
            pRowR[col] = (T_Float)0.0; // last one gets overwritten!
            pRowQ[col] = (T_Float)( pRowA[col] * R_br_reciprocal );
            
            #if defined( BIG_MATRIX_DEBUG )
            if ( !_finite( pRowQ[col] ) )
                DebugBreak();
            #endif
        }
        
        pRowR[dimensions-1] = R_br_squared * R_br_reciprocal;
        
        #if defined( BIG_MATRIX_DEBUG )
        if ( !_finite( pRowR[dimensions-1] ) )
            DebugBreak();
        #endif
        
        row--;
    }
    #endif
    
    // Solve each row from bottom to top
    while ( row >= 0 )
    {
        // BUG: Q is not properly ortho-normalised!
        // Rows are normalised but columns are not.
        // Dot-products between different pairs of rows or pairs of columns are incorrect (should be zero).
        // HOWEVER: R is upper triangular and QR=A is correct and this is acceptable for the LCP solver for now...
        // ADDENDUM: This bug may only occur if A is not positive semi-definite - need to verify this!
        
        T_Float const * __restrict pRowA = (*pA)[row];
        T_Float       * __restrict pRowR = (*pR)[row];
        T_Float       * __restrict pRowQ = (*pQ)[row];
        
        // R: Left of diagonal - all zeros
        for ( S32 col=0; col<row; col++ )
        {
            pRowR[col] = 0.0;
        }
        
        // R: Right of diagonal
        for ( S32 col=row+1; col<dimensions; col++ )
        {
            pRowR[col] = VectorDotProduct( (*pQ)[col], pRowA, dimensions );
            
            #if defined( BIG_MATRIX_DEBUG )
            if ( !_finite( pRowR[col] ) )
                DebugBreak();
            #endif
        }
        
        // R: Diagonal - depends on R's to the right of the diagonal
        T_Float r_diagonal_squared( 0.0 );
        for ( S32 col=0; col<dimensions; col++ )
        {
            T_Float temp = pRowA[col];
            for ( S32 i=row+1; i<dimensions; i++ )
            {
                temp -= pRowR[i] * (*pQ)[i][col];
            }
            r_diagonal_squared += temp * temp;
        }
        
        const T_Float r_diagonal_reciprocal( ReciprocalSqrt( r_diagonal_squared ) );
        
        pRowR[row] = r_diagonal_squared * r_diagonal_reciprocal;
        
        #if defined( BIG_MATRIX_DEBUG )
        if ( !_finite( pRowR[row] ) )
            DebugBreak();
        #endif
        
        // Q
        for ( S32 col=0; col<dimensions; col++ )
        {
            T_Float q( pRowA[col] );
            for ( S32 i=row+1; i<dimensions; i++ )
            {
                q -= pRowR[i] * (*pQ)[i][col];
            }
            pRowQ[col] = q * r_diagonal_reciprocal;
            
            #if defined( BIG_MATRIX_DEBUG )
            if ( !_finite( pRowQ[col] ) )
                DebugBreak();
            #endif
        }
        
        #if 0 && defined( BIG_MATRIX_DEBUG )
        // Debug Validate
        {
            const T_Float sos( VectorSumOfSquares( pRowQ, dimensions ) );
            
            if ( Abs( sos - 1.0 ) > 0.001f )
                DebugBreak();
            
            for ( S32 row2=row+1; row2<dimensions; row2++ )
            {
                T_Float const * const __restrict pRowQ2 = (*pQ)[row2];
                
                const T_Float dp( VectorDotProduct( pRowQ, pRowQ2, dimensions ) );
                
                if ( Abs( dp ) > 0.001f )
                    DebugBreak();
            }
        }
        #endif
        
        row--;
    }
    
    #endif
    
    #if 0 && defined( BIG_MATRIX_DEBUG )
    // Debug Validate
    {
        for ( S32 rowQ1=0; rowQ1<dimensions; rowQ1++ )
        {
            T_Float const * const __restrict pRowQ1 = (*pQ)[rowQ1];
            
            const T_Float sos( VectorSumOfSquares( pRowQ1, dimensions ) );
            
            if ( Abs( sos - 1.0 ) > 0.001f )
                DebugBreak();
            
            for ( S32 rowQ2=rowQ1+1; rowQ2<dimensions; rowQ2++ )
            {
                T_Float const * const __restrict pRowQ2 = (*pQ)[rowQ2];
                
                const T_Float dp( VectorDotProduct( pRowQ1, pRowQ2, dimensions ) );
                
                if ( Abs( dp ) > 0.001f )
                    DebugBreak();
            }
        }
        
        //C_BigMatrix QT( dimensions, dimensions );
        //C_BigMatrix* const __restrict pQT = &QT;
        //Transpose( pQ, pQT );
        //
        //for ( S32 rowQ1=0; rowQ1<dimensions; rowQ1++ )
        //{
        //    T_Float const * const __restrict pRowQ1 = (*pQT)[rowQ1];
        //    
        //    const T_Float sos( VectorSumOfSquares( pRowQ1, dimensions ) );
        //    
        //    if ( Abs( sos - 1.0 ) > 0.001f )
        //        DebugBreak();
        //    
        //    for ( S32 rowQ2=rowQ1+1; rowQ2<dimensions; rowQ2++ )
        //    {
        //        T_Float const * const __restrict pRowQ2 = (*pQT)[rowQ2];
        //        
        //        const T_Float dp( VectorDotProduct( pRowQ1, pRowQ2, dimensions ) );
        //        
        //        if ( Abs( dp ) > 0.001f )
        //            DebugBreak();
        //    }
        //}
    }
    #endif
}

template < typename T_Float >
NOALIAS void CholeskyDecomposition(
    C_BigMatrix<T_Float> const * const __restrict pA,
    C_BigMatrix<T_Float>       * const __restrict pL )
{
    // Solve A = LL* with L lower triangular
    
    const S32 dimensions( pA->m_Rows );
    
    BIG_MATRIX_ASSERT( ( dimensions == pA->m_Columns ) &&
                       ( dimensions == pL->m_Rows ) &&
                       ( dimensions == pL->m_Columns ) );
    
    #if 1
    
    for ( S32 i=0; i<dimensions; i++ )
    {
        T_Float const * const __restrict pAi = (*pA)[i];
        T_Float       * const __restrict pLi = (*pL)[i];
        
        T_Float Lii_squared = pAi[i];
        
        // Left of diagonal
        for ( S32 j=0; j<i; j++ )
        {
            T_Float const * const __restrict pLj = (*pL)[j];
            //pLi[j] = ( pAi[j] - VectorDotProduct( pLi, pLj, j ) ) * Reciprocal( pLj[j] );
            //const T_Float Lij = ( pAi[j] - VectorDotProduct( pLi, pLj, j ) ) / pLj[j];
            const T_Float Lij = Divide( pAi[j] - VectorDotProduct( pLi, pLj, j ), pLj[j] );
            pLi[j] = Lij;
            
            #if defined( BIG_MATRIX_DEBUG )
            if ( !_finite( pLi[j] ) )
                DebugBreak();
            #endif
            
            Lii_squared -= Lij * Lij;
        }
        
        // Diagonal
        //const T_Float Lii_squared = pAi[i] - VectorSumOfSquares( pLi, i );
        //const T_Float Lii_reciprocal = ReciprocalSqrt( Lii_squared );
        //pLi[i] = Lii_squared * Lii_reciprocal;
        pLi[i] = __fsqrts( Lii_squared );
        
        #if defined( BIG_MATRIX_DEBUG )
        if ( !_finite( pLi[i] ) )
            DebugBreak();
        #endif
        
        // Right of diagonal
        for ( S32 j=i+1; j<dimensions; j++ )
        {
            pLi[j] = 0.0;
        }
    }
    
    #else
    
    for ( S32 i=0; i<dimensions; i++ )
    {
        T_Float const * const __restrict pAi = (*pA)[i];
        T_Float       * const __restrict pLi = (*pL)[i];
        
        // Diagonal
        const T_Float Lii_squared = pAi[i] - VectorSumOfSquares( pLi, i );
        const T_Float Lii_reciprocal = ReciprocalSqrt( Lii_squared );
        pLi[i] = Lii_squared * Lii_reciprocal;
        
        #if defined( BIG_MATRIX_DEBUG )
        if ( !_finite( pLi[i] ) )
            DebugBreak();
        #endif
        
        // Below diagonal
        for ( S32 j=i+1; j<dimensions; j++ )
        {
            T_Float const * const __restrict pAj = (*pA)[j];
            T_Float       * const __restrict pLj = (*pL)[j];
            pLj[i] = ( pAj[i] - VectorDotProduct( pLi, pLj, i ) ) * Lii_reciprocal;
            
            #if defined( BIG_MATRIX_DEBUG )
            if ( !_finite( pLi[j] ) )
                DebugBreak();
            #endif
        }
    }
    
    #endif
}

template < typename T_Float >
NOALIAS const bool Solve_Rx_eq_y(
    C_BigMatrix<T_Float> const * const __restrict pR,  // In
    C_BigVector<T_Float>       * const __restrict pX,  // Out
    C_BigVector<T_Float> const * const __restrict pY ) // In
{
    BIG_MATRIX_ASSERT( ( pR->m_Rows == pR->m_Columns ) &&
                       ( pR->m_Columns == pX->m_Elements ) &&
                       ( pR->m_Rows == pY->m_Elements ) );
    
    const S32 rows( pR->m_Rows );
    const S32 cols( pR->m_Columns );
    
    const T_Float nearlyZero = BIG_MATRIX_NEARLY_ZERO;
    
    T_Float Rii_min = FLT_MAX;
    
    bool result = true;
    
    #if defined( BIG_MATRIX_DEBUG )
    bool debug = false;
    #endif
    
    for ( S32 i=rows-1; i>=0; i-- )
    {
        T_Float const * const __restrict pRi = pR->m_pRowArray[i];
        
        T_Float Rii_xi = pY->m_pVector[i];
        
        for ( S32 j=i+1; j<cols; j++ )
        {
            Rii_xi -= pRi[j] * pX->m_pVector[j];
        }
        
        const T_Float Rii( pRi[i] );
        
        Rii_min = Min( Abs( Rii ), Rii_min );
        
        if ( Abs( Rii ) > nearlyZero )
        {
            //pX->m_pVector[i] = Rii_xi / Rii;
            pX->m_pVector[i] = Divide( Rii_xi, Rii );
        }
        else if ( Abs( Rii_xi ) < nearlyZero )
        {
            pX->m_pVector[i] = 0.0;
            
            #if defined( BIG_MATRIX_DEBUG )
            printf( "Setting x to zero for x * %+15.9f = %+15.9f\n", Rii, Rii_xi );
            debug = true;
            #endif
        }
        else
        {
            // No solution for 0.0 * x == y, where y != 0.0
            pX->m_pVector[i] = 0.0;
            //result = false;
            
            #if defined( BIG_MATRIX_DEBUG )
            printf( "No solution for x * %+15.9f = %+15.9f\n", Rii, Rii_xi );
            debug = true;
            #endif
        }
    }
    
    #if defined( BIG_MATRIX_DEBUG )
    if ( 1 )
    {
        for ( S32 i=0; i<rows; i++ )
        {
            T_Float const * const __restrict pRi = pR->m_pRowArray[i];
            
            T_Float y = 0.0;
            
            for ( S32 j=0; j<cols; j++ )
            {
                y += pRi[j] * pX->m_pVector[j];
            }
            
            BIG_MATRIX_ASSERT_EQUAL( y, pY->m_pVector[i] );
            
            if ( Abs( y - pY->m_pVector[i] ) > nearlyZero )
            {
                printf( "Solve_Rx_eq_y() failed: Rx != y!\n" );
                result = false;
                debug = true;
            }
        }
    }
    #endif
    
    #if defined( BIG_MATRIX_DEBUG )
    if ( debug )
    {
        for ( S32 i=0; i<rows; i++ )
        {
            T_Float const * const __restrict pRi = pR->m_pRowArray[i];
            
            T_Float y = 0.0;
            
            for ( S32 j=0; j<cols; j++ )
            {
                printf( "R[%d][%d] = %+15.9f\n", i, j, pRi[j] );
                y += pRi[j] * pX->m_pVector[j];
            }
            
            printf( "X[%d] = %+15.9f\n", i, pX->m_pVector[i] );
            printf( "Y[%d] = %+15.9f\n", i, pY->m_pVector[i] );
            printf( "y = %+15.9f\n", y );
        }
        DebugBreak();
    }
    #endif
    
    return result;
}

template < typename T_Float >
NOALIAS const bool GaussianElimination(
    C_BigMatrix<T_Float>* __restrict pA,  // In/Out
    C_BigVector<T_Float>* __restrict pX,  // Out
    C_BigVector<T_Float>* __restrict pY ) // In/Out
{
    // Solve Ax=y, by putting augmented matrix A:y into upper triangular form
    // using elementary row operations and then solving rows from bottom to top.
    // NOTE: Only X is unpermuted at the end, A & Y are 'trashed'!
    
    BIG_MATRIX_ASSERT( ( pA->m_Rows == pA->m_Columns ) &&
                       ( pA->m_Columns == pX->m_Elements ) &&
                       ( pA->m_Rows == pY->m_Elements ) );
    
    const S32 rows( pA->m_Rows );
    const S32 cols( pA->m_Columns );
    
    const T_Float nearlyZero( BIG_MATRIX_NEARLY_ZERO );
    
    #if defined( BIG_MATRIX_DEBUG )
    printf( "GaussianElimination(): solve A*X=Y for X:\n" );
    for ( S32 i=0; i<rows; i++ )
    {
        printf( "A [ " );
        for ( S32 j=0; j<cols; j++ )
        {
            //printf( "%+15.9f ", (*pA)[i][j] );
            Util::C_String text;
            text << (*pA)[i][j] << " ";
            printf( text.Chars() );
        }
        //printf( "] * [ X ] = [ %+15.9f ]\n", (*pY)[i] );
        Util::C_String text;
        text << "] * [ X ] = [ " << (*pY)[i] << " ]\n";
        printf( text.Chars() );
    }
    printf( "\n" );
    #endif
    
    BIG_MATRIX_ASSERT( rows < 1024 ); // TODO: dynamic allocation for this array!
    S16 unitColumnIndex[ 1024 ];
    
    // Put matrix A into row echelon form
    
    const F32 f_zero( 0.0f );
    const F32 f_one( 1.0f );
    
    S32 row = 0;
    S32 col = 0;
    F32 f_row = f_zero;
    
    //while ( ( row < rows ) && ( col < cols ) )
    while ( ( row < rows ) & ( col < cols ) )
    {
        // Find pivot in column 'col' starting in row 'row'
        
        #if defined( _VMX128_ )
        
        T_Float pivotValue = pA->m_pRowArray[row][col];
        F32 f_pivotIndex = f_row;
        F32 f_r = f_pivotIndex + f_one;
        for ( S32 r=row+1; r<rows; r++, f_r += f_one )
        {
            const T_Float value = pA->m_pRowArray[r][col];
            const T_Float selector = Abs( value ) - Abs( pivotValue );
            pivotValue = (T_Float)__fsel( selector, value, pivotValue );
            f_pivotIndex = (F32)__fsel( selector, f_r, f_pivotIndex );
        }
        
        const S32 pivotIndex = (S32)f_pivotIndex;
        
        #else
        
        T_Float pivotValue = pA->m_pRowArray[row][col];
        S32 pivotIndex = row;
        for ( S32 r=row+1; r<rows; r++ )
        {
            const T_Float value = pA->m_pRowArray[r][col];
            if ( Abs( pivotValue ) < Abs( value ) )
            {
                pivotValue = value;
                pivotIndex = r;
            }
        }
        
        #endif
        
        if ( Abs( pivotValue ) > nearlyZero )
        {
            if ( row != pivotIndex )
            {
                pA->SwapRows( row, pivotIndex );
                pY->SwapElements( row, pivotIndex );
            }
            
            T_Float* const __restrict pA_row = pA->m_pRowArray[row];
            
            // Divide row 'row' by pivotValue
            
            const T_Float rcpPivotValue( Reciprocal( pivotValue ) );
            
            pA_row[col] = (T_Float)f_one;
            
            const T_Float Y_row = pY->m_pVector[row] * rcpPivotValue;
            pY->m_pVector[row] = Y_row;
            
            for ( S32 c=col+1; c<cols; c++ )
            {
                pA_row[c] *= rcpPivotValue;
            }
            
            // Clear column 'col' for rows r > row
            for ( S32 r=row+1; r<rows; r++ )
            {
                T_Float* const __restrict pA_r = pA->m_pRowArray[r];
                
                const T_Float multiplier( pA_r[col] );
                pA_r[col] = (T_Float)f_zero;
                
                pY->m_pVector[r] -= Y_row * multiplier;
                
                for ( S32 c=col+1; c<cols; c++ )
                {
                    pA_r[c] -= pA_row[c] * multiplier;
                }
            }
            
            unitColumnIndex[row] = col;
            
            row++;
            f_row += f_one;
        }
        else
        {
            // Set column 'col' to zero
            for ( S32 r=row; r<rows; r++ )
            {
                BIG_MATRIX_ASSERT_ZERO( pA->m_pRowArray[r][col] );
                pA->m_pRowArray[r][col] = (T_Float)f_zero; // Just to make sure...
            }
        }
        
        col++;
        
        #if defined( BIG_MATRIX_DEBUG )
        printf( "GaussianElimination(): solve A*X=Y for X:\n" );
        for ( S32 i=0; i<rows; i++ )
        {
            printf( "A [ " );
            for ( S32 j=0; j<cols; j++ )
            {
                //printf( "%+15.9f ", (*pA)[i][j] );
                Util::C_String text;
                text << (*pA)[i][j] << " ";
                printf( text.Chars() );
            }
            //printf( "] * [ X ] = [ %+15.9f ]\n", (*pY)[i] );
            Util::C_String text;
            text << "] * [ X ] = [ " << (*pY)[i] << " ]\n";
            printf( text.Chars() );
        }
        printf( "\n" );
        #endif
    }
    
    const S32 unitRows = row;
    
    // Solve for X
    
    bool result = true;
    //for ( S32 r=unitRows; r<rows; r++ )
    //{
    //    BIG_MATRIX_ASSERT_ZERO( (*pY)[r] );
    //    //if ( Abs( (*pY)[r] ) > nearlyZero )
    //    if ( Abs( (*pY)[r] ) > 1.0e-5 )
    //    {
    //        result = false;
    //    }
    //}
    
    for ( S32 r=0; r<rows; r++ )
    {
        pX->m_pVector[r] = (T_Float)f_zero;
    }
    
    for ( S32 r=unitRows-1; r>=0; r-- )
    {
        const S32 uci = unitColumnIndex[r];
        
        T_Float const * const __restrict pA_r = pA->m_pRowArray[r];
        
        T_Float X_uci = pY->m_pVector[r];
        
        for ( S32 c=uci+1; c<cols; c++ )
        {
            X_uci -= pA_r[c] * pX->m_pVector[c];
        }
        
        pX->m_pVector[uci] = X_uci;
    }
    
    #if defined( BIG_MATRIX_DEBUG )
    for ( S32 i=0; i<rows; i++ )
    {
        printf( "A [ " );
        for ( S32 j=0; j<cols; j++ )
        {
            //printf( "%+15.9f ", (*pA)[i][j] );
            Util::C_String text;
            text << (*pA)[i][j] << " ";
            printf( text.Chars() );
        }
        //printf( "] * [ %+15.9f ] = [ %+15.9f ]\n", (*pX)[i], (*pY)[i] );
        Util::C_String text;
        text << "] * [ " << (*pX)[i] << " ] = [ " << (*pY)[i] << " ]\n";
        printf( text.Chars() );
    }
    printf( "\n" );
    #endif
    
    #if defined( BIG_MATRIX_DEBUG )
    for ( S32 i=0; i<rows; i++ )
    {
        T_Float y = 0.0f;
        for ( S32 j=0; j<cols; j++ )
        {
            y += (*pA)[i][j] * (*pX)[j];
        }
        
        BIG_MATRIX_ASSERT_EQUAL( y, (*pY)[i] );
    }
    #endif
    
    return result;
}

template < typename T_Float >
NOALIAS void RowEchelonForm( C_BigMatrix<T_Float>* __restrict pA ) // In/Out
{
    // Put matrix A into row echelon form using elementary row operations.
    
    const S32 rows( pA->m_Rows );
    const S32 cols( pA->m_Columns );
    
    const T_Float nearlyZero( BIG_MATRIX_NEARLY_ZERO );
    
    S32 row = 0;
    S32 col = 0;
    while ( ( row < rows ) && ( col < cols ) )
    {
        // Find pivot in column 'col' starting in row 'row'
        
        T_Float pivotValue = (*pA)[row][col];
        S32 pivotIndex = row;
        for ( S32 r=row+1; r<rows; r++ )
        {
            const T_Float value = (*pA)[r][col];
            if ( Abs( pivotValue ) < Abs( value ) )
            {
                pivotValue = value;
                pivotIndex = r;
            }
        }
        
        if ( Abs( pivotValue ) > nearlyZero )
        {
            if ( row != pivotIndex )
            {
                pA->SwapRows( row, pivotIndex );
            }
            
            // Divide row 'row' by pivotValue
            //const T_Float rcpPivotValue( 1.0 / pivotValue );
            const T_Float rcpPivotValue( Reciprocal( pivotValue ) );
            for ( S32 c=col+1; c<cols; c++ )
            {
                (*pA)[row][c] *= rcpPivotValue;
            }
            (*pA)[row][col] = 1.0;
            
            // Clear column 'col' for rows r > row
            for ( S32 r=row+1; r<rows; r++ )
            {
                const T_Float multiplier( (*pA)[r][col] );
                for ( S32 c=col+1; c<cols; r++ )
                {
                    (*pA)[r][c] -= (*pA)[row][c] * multiplier;
                }
                (*pA)[r][col] = 0.0;
            }
            
            row++;
        }
        else
        {
            // Set column 'col' to zero
            for ( S32 r=row; r<rows; r++ )
            {
                (*pA)[r][col] = 0.0; // Just to make sure...
            }
        }
        
        col++;
    }
}





}; // namespace Maths

#endif // #ifndef BIGMATRIX_INL
