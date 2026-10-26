#ifndef BIGMATRIX_H
#define BIGMATRIX_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

//#if defined( USER_JDENTON )
//#define BIG_MATRIX_DEBUG
//#define BIG_MATRIX_ENABLE_ASSERT
//#endif


#define BIG_MATRIX_NEARLY_ZERO 1.0e-5


template < typename T_Element > class C_BigVector;
template < typename T_Element > class C_BigMatrix;


NOALIAS __forceinline const F32 VectorSumOfSquares(
    F32 const * const __restrict pA,
    const S32 elements );

NOALIAS __forceinline const F64 VectorSumOfSquares(
    F64 const * const __restrict pA,
    const S32 elements );

NOALIAS __forceinline const F32 VectorDotProduct(
    F32 const * __restrict pA,
    F32 const * __restrict pB,
    S32 elements );

NOALIAS __forceinline const F64 VectorDotProduct(
    F64 const * __restrict pA,
    F64 const * __restrict pB,
    S32 elements );

NOALIAS const HF32 VectorSumOfSquares_Aligned(
    F32 const * const __restrict pA,
    const S32& elements );

#if defined( _VMX128_ )
NOALIAS const __vector4 VectorDotProduct_Aligned(
#else
NOALIAS const HF32 VectorDotProduct_Aligned(
#endif
    F32 const * __restrict pA,
    F32 const * __restrict pB,
    const S32& elements );


template < typename T_Element >
class C_BigVector
{
public:
    
    // Pre-Calculate the raw data size of a vector [ elements ]
    static const S32            Sizeof( const S32 elements );
    
    // Default Constructor: Call Initialise() to set number of elements
    C_BigVector();
    
    C_BigVector( const S32 elements );
    C_BigVector( const S32 elements, T_Element* pVector );
    C_BigVector( const S32 elements, char* pMemory, const S32 size );
    ~C_BigVector();
    
    NOALIAS operator const T_Element* () const __restrict;
    NOALIAS operator T_Element* () __restrict;
    
    NOALIAS void                Initialise( const S32 elements, char* pMemory, const S32 size ) __restrict;
    
    NOALIAS const T_Element*    Vector() const __restrict;
    NOALIAS T_Element*          Vector() __restrict;
    
    NOALIAS const T_Element     operator [] ( const U32 element ) const __restrict;
    NOALIAS const T_Element     operator [] ( const S32 element ) const __restrict;
    NOALIAS T_Element&          operator [] ( const U32 element ) __restrict;
    NOALIAS T_Element&          operator [] ( const S32 element ) __restrict;
    
    NOALIAS void                SwapElements( const S32 a, const S32 b ) __restrict;
    
    template < typename T_Float >
    friend NOALIAS void         Add(
                                    C_BigVector<T_Float> const * __restrict pA,   // In
                                    C_BigVector<T_Float> const * __restrict pB,   // In
                                    C_BigVector<T_Float>       * __restrict pC ); // Out
    
    template < typename T_Float >
    friend NOALIAS void         Multiply(
                                    C_BigMatrix<T_Float> const * __restrict pA,   // In
                                    C_BigVector<T_Float> const * __restrict pB,   // In
                                    C_BigVector<T_Float>       * __restrict pC ); // Out
    
    template < typename T_Float >
    friend NOALIAS void         Multiply(
                                    C_BigVector<T_Float> const * __restrict pA,   // In
                                    C_BigMatrix<T_Float> const * __restrict pB,   // In
                                    C_BigVector<T_Float>       * __restrict pC ); // Out
    
    template < typename T_Float >
    friend NOALIAS const bool   Solve_Rx_eq_y(
                                    C_BigMatrix<T_Float> const * __restrict pR,   // In
                                    C_BigVector<T_Float>       * __restrict pX,   // Out
                                    C_BigVector<T_Float> const * __restrict pY ); // In
    
    template < typename T_Float >
    friend NOALIAS const bool   GaussianElimination(
                                    C_BigMatrix<T_Float>       * __restrict pA,   // In/Out
                                    C_BigVector<T_Float>       * __restrict pX,   // Out
                                    C_BigVector<T_Float>       * __restrict pY ); // In/Out
    
private:
    
    static const S32            m_DataAlignment = 128;
    static const S32            m_ElementPad = m_DataAlignment / sizeof( T_Element ); // NOTE: This must be a power of 2!
    
    S32                         m_Elements;
    T_Element* __restrict       m_pVector;
    char*                       m_pMemory;
};


template < typename T_Element >
class C_BigMatrix
{
public:
    
    // Pre-Calculate the raw data size of a matrix [ rows, columns ]
    static const S32            Sizeof( const S32 rows, const S32 columns );
    
    // Default Constructor: Call Initialise() to set number of rows & columns
    C_BigMatrix();
    
    // Constructor: rows & columns
    C_BigMatrix( const S32 rows, const S32 columns );
    
    // Constructor: rows & columns, with pre-allocated memory
    C_BigMatrix( const S32 rows, const S32 columns, char* pMemory, const S32 memorySize );
    
    // Destructor
    ~C_BigMatrix();
    
    void                        Initialise( const S32 rows, const S32 columns );
    void                        Initialise( const S32 rows, const S32 columns, char* pMemory, const S32 memorySize );
    
    NOALIAS T_Element const *   operator [] ( const U32 row ) const __restrict;
    NOALIAS T_Element const *   operator [] ( const S32 row ) const __restrict;
    NOALIAS T_Element*          operator [] ( const U32 row ) __restrict;
    NOALIAS T_Element*          operator [] ( const S32 row ) __restrict;
    
    NOALIAS void                operator = ( const C_BigMatrix<T_Element>& B );
    
    NOALIAS void                SwapRows( const S32 a, const S32 b ) __restrict;
    NOALIAS void                SwapColumns( const S32 a, const S32 b ) __restrict;
    
    template < typename T_Float >
    friend NOALIAS void         Transpose(
                                    C_BigMatrix<T_Float> const * __restrict pA,   // In
                                    C_BigMatrix<T_Float>       * __restrict pB ); // Out
    
    template < typename T_Float >
    friend NOALIAS void         Multiply(
                                    C_BigMatrix<T_Float> const * __restrict pA,   // In
                                    C_BigMatrix<T_Float> const * __restrict pB,   // In
                                    C_BigMatrix<T_Float>       * __restrict pC ); // Out
    
    template < typename T_Float >
    friend NOALIAS void         Multiply(
                                    C_BigMatrix<T_Float> const * __restrict pA,   // In
                                    C_BigVector<T_Float> const * __restrict pB,   // In
                                    C_BigVector<T_Float>       * __restrict pC ); // Out
    
    template < typename T_Float >
    friend NOALIAS void         Multiply(
                                    C_BigVector<T_Float> const * __restrict pA,   // In
                                    C_BigMatrix<T_Float> const * __restrict pB,   // In
                                    C_BigVector<T_Float>       * __restrict pC ); // Out
    
    template < typename T_Float >
    friend NOALIAS void         RQ_Decomposition(
                                    C_BigMatrix<T_Float> const * __restrict pA,   // In
                                    C_BigMatrix<T_Float>       * __restrict pR,   // Out
                                    C_BigMatrix<T_Float>       * __restrict pQ ); // Out
    
    template < typename T_Float >
    friend NOALIAS void         CholeskyDecomposition(
                                    C_BigMatrix<T_Float> const * __restrict pA,   // In
                                    C_BigMatrix<T_Float>       * __restrict pL ); // Out
    
    template < typename T_Float >
    friend NOALIAS const bool   Solve_Rx_eq_y(
                                    C_BigMatrix<T_Float> const * __restrict pR,   // In
                                    C_BigVector<T_Float>       * __restrict pX,   // Out
                                    C_BigVector<T_Float> const * __restrict pY ); // In
    
    template < typename T_Float >
    friend NOALIAS const bool   GaussianElimination(
                                    C_BigMatrix<T_Float>       * __restrict pA,   // In/Out
                                    C_BigVector<T_Float>       * __restrict pX,   // Out
                                    C_BigVector<T_Float>       * __restrict pY ); // In/Out
    
private:
    
    static const S32                    m_DataAlignment = 128;
    static const S32                    m_RowPad = m_DataAlignment / sizeof( T_Element ); // NOTE: This must be a power of 2!
    
    S32                                 m_Rows;
    S32                                 m_Columns;
    S32                                 m_RowStep;
    T_Element* __restrict * __restrict  m_pRowArray;
    T_Element*                          m_pMatrix;
    char*                               m_pMemory;
    
    // TODO: Add additional vector to store the reciprocal of the diagonal
    T_Element* __restrict               m_pRcpDiagonal;
    bool                                m_RcpDiagonalValid;
};

template < typename T_Float >
NOALIAS void Add(
    C_BigVector<T_Float> const * __restrict pA,   // In
    C_BigVector<T_Float> const * __restrict pB,   // In
    C_BigVector<T_Float>       * __restrict pC ); // Out

template < typename T_Float >
NOALIAS void Transpose(
    C_BigMatrix<T_Float> const * __restrict pA,   // In
    C_BigMatrix<T_Float>       * __restrict pB ); // Out

template < typename T_Float >
NOALIAS void Multiply(
    C_BigMatrix<T_Float> const * __restrict pA,   // In
    C_BigMatrix<T_Float> const * __restrict pB,   // In
    C_BigMatrix<T_Float>       * __restrict pC ); // Out

template < typename T_Float >
NOALIAS void Multiply(
    C_BigMatrix<T_Float> const * __restrict pA,   // In
    C_BigVector<T_Float> const * __restrict pB,   // In
    C_BigVector<T_Float>       * __restrict pC ); // Out

template < typename T_Float >
NOALIAS void Multiply(
    C_BigVector<T_Float> const * __restrict pA,   // In
    C_BigMatrix<T_Float> const * __restrict pB,   // In
    C_BigVector<T_Float>       * __restrict pC ); // Out

template < typename T_Float >
NOALIAS void RQ_Decomposition(
    C_BigMatrix<T_Float> const * __restrict pA,   // In
    C_BigMatrix<T_Float>       * __restrict pR,   // Out
    C_BigMatrix<T_Float>       * __restrict pQ ); // Out

template < typename T_Float >
NOALIAS void CholeskyDecomposition(
    C_BigMatrix<T_Float> const * __restrict pA,   // In
    C_BigMatrix<T_Float>       * __restrict pL ); // Out

template < typename T_Float >
NOALIAS const bool Solve_Rx_eq_y(
    C_BigMatrix<T_Float> const * __restrict pR,   // In
    C_BigVector<T_Float>       * __restrict pX,   // Out
    C_BigVector<T_Float> const * __restrict pY ); // In

template < typename T_Float >
NOALIAS const bool GaussianElimination(
    C_BigMatrix<T_Float>       * __restrict pA,   // In/Out
    C_BigVector<T_Float>       * __restrict pX,   // Out
    C_BigVector<T_Float>       * __restrict pY ); // In/Out

//#if defined( PLATFORM_XENON )
template <> // Xenon SIMD F32 Specialisation?
NOALIAS const bool GaussianElimination(
    C_BigMatrix<F32>           * __restrict pA,   // In/Out
    C_BigVector<F32>           * __restrict pX,   // Out
    C_BigVector<F32>           * __restrict pY ); // In/Out
//#endif

}; // namespace Maths

#endif // #ifndef BIGMATRIX_H
