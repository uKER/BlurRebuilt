#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_GEOMETRIC_PRODUCT_INL
#define SOFT_MATHS_HVEC_NEW_G3V_GEOMETRIC_PRODUCT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_GEOMETRIC_PRODUCT_INL
#define MATHS_HVEC_NEW_G3V_GEOMETRIC_PRODUCT_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_GeometricProduct
/// @{

/// Geometric Product Operator: VF32 * Vector
NOALIAS __forceinline const Vector operator * ( const VF32 VECREF a, const Vector PCREF b )
{
    /// @code
    ///               |           V           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS * RHS | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    /// S | 1         | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    return MakeVector( a * b.m_V );
}

/// Geometric Product Operator: Vector * VF32
NOALIAS __forceinline const Vector operator * ( const Vector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///               |  S    |
    ///   +-----------+-------+
    ///   | LHS * RHS | 1     |
    /// --+-----------+-------+
    ///   | e1        | e1    |
    ///   +-----------+-------+
    /// V | e2        | e2    |
    ///   +-----------+-------+
    ///   | e3        | e3    |
    /// --+-----------+-------+
    /// @endcode
    
    return MakeVector( a.m_V * b );
}

/// Geometric Product Operator: VF32 * BiVector
NOALIAS __forceinline const BiVector operator * ( const VF32 VECREF a, const BiVector PCREF b )
{
    /// @code
    ///               |           B           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS * RHS | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    /// S | 1         | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    return MakeBiVector( a * b.m_B );
}

/// Geometric Product Operator: BiVector * VF32
NOALIAS __forceinline const BiVector operator * ( const BiVector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///               |  S    |
    ///   +-----------+-------+
    ///   | LHS * RHS | 1     |
    /// --+-----------+-------+
    ///   | e12       | e12   |
    ///   +-----------+-------+
    /// B | e23       | e23   |
    ///   +-----------+-------+
    ///   | e31       | e31   |
    /// --+-----------+-------+
    /// @endcode
    
    return MakeBiVector( a.m_B * b );
}

/// Geometric Product Operator: VF32 * TriVector
NOALIAS __forceinline const TriVector operator * ( const VF32 VECREF a, const TriVector PCREF b )
{
    /// @code
    ///               |   T   |
    ///   +-----------+-------+
    ///   | LHS * RHS | e123  |
    /// --+-----------+-------+
    /// S | 1         | e123  |
    /// --+-----------+-------+
    /// @endcode
    
    return MakeTriVector( a * b.m_T );
}

/// Geometric Product Operator: TriVector * VF32
NOALIAS __forceinline const TriVector operator * ( const TriVector PCREF a, const VF32 VECREF b )
{
    /// @code
    ///               |  S    |
    ///   +-----------+-------+
    ///   | LHS * RHS | 1     |
    /// --+-----------+-------+
    /// T | e123      | e123  |
    /// --+-----------+-------+
    /// @endcode
    
    return MakeTriVector( a.m_T * b );
}

/// Geometric Product Operator: Vector * Vector
NOALIAS __forceinline const MultiVecSB operator * ( const Vector PCREF a, const Vector PCREF b )
{
    /// @code
    ///               |           V           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS * RHS | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    ///   | e1        | 1     | +e12  | -e31  |
    ///   +-----------+-------+-------+-------+
    /// V | e2        | -e12  | 1     | +e23  |
    ///   +-----------+-------+-------+-------+
    ///   | e3        | +e31  | -e23  | 1     |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    // result.e12 = a.e1 * b.e2 - a.e2 * b.e1
    // result.e23 = a.e2 * b.e3 - a.e3 * b.e2
    // result.e31 = a.e3 * b.e1 - a.e1 * b.e3
    // result.1   = a.e1 * b.e1 + a.e2 * b.e2 + a.e3 * b.e3
    
    MultiVecSB result;
    //result.m_SB = ( a.m_V * Permute<e2,e3,e1>( b ) )
    //            - ( b.m_V * Permute<e2,e3,e1>( a ) );
    //result.m_SB = NegMulSub( b.m_V,  VPermute<e2,e3,e1>( a ),
    //                         a.m_V * VPermute<e2,e3,e1>( b ) );
    //SetW( &result.m_SB, Dot3( a.m_V, b.m_V ) );
    result.m_SB = MakeVector4( NegMulSub( b.m_V,  VPermute<e2,e3,e1>( a ),
                                          a.m_V * VPermute<e2,e3,e1>( b ) ),
                               Dot( a.m_V, b.m_V ) );
    return result;
}

/// Geometric Product Operator: Vector * BiVector
NOALIAS __forceinline const MultiVecVT operator * ( const Vector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///               |           B           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS * RHS | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    ///   | e1        | +e2   | e123  | -e3   |
    ///   +-----------+-------+-------+-------+
    /// V | e2        | -e1   | +e3   | e123  |
    ///   +-----------+-------+-------+-------+
    ///   | e3        | e123  | -e2   | e1    |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    // result.e1   = (a.e3 * b.e31) - (a.e2 * b.e12)
    // result.e2   = (a.e1 * b.e12) - (a.e3 * b.e23)
    // result.e3   = (a.e2 * b.e23) - (a.e1 * b.e31)
    // result.e123 = a.e1 * b.e23
    //             + a.e2 * b.e31
    //             + a.e3 * b.e12
    
    MultiVecVT result;
    //result.m_VT = ( VPermute<e3,e1,e2>( a ) * BPermute<e31,e12,e23>( b ) )
    //            - ( VPermute<e2,e3,e1>( a ) * BPermute<e12,e23,e31>( b ) ); // TODO: Use NegMulSub()!
    //SetW( &result.m_VT, Dot3( VPermute<e1,e2,e3>( a ), BPermute<e23,e31,e12>( b ) ) );
    result.m_VT = MakeVector4( ( VPermute<e3,e1,e2>( a ) * BPermute<e31,e12,e23>( b ) ) -
                               ( VPermute<e2,e3,e1>( a ) * BPermute<e12,e23,e31>( b ) ),
                               Dot( VPermute<e1,e2,e3>( a ), BPermute<e23,e31,e12>( b ) ) ); // TODO: Use NegMulSub()!
    return result;
}

/// Geometric Product Operator: BiVector * Vector
NOALIAS __forceinline const MultiVecVT operator * ( const BiVector PCREF a, const Vector PCREF b )
{
    /// @code
    ///               |           V           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS * RHS | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    ///   | e12       | -e2   | +e1   | e123  |
    ///   +-----------+-------+-------+-------+
    /// B | e23       | e123  | -e3   | +e2   |
    ///   +-----------+-------+-------+-------+
    ///   | e31       | +e3   | e123  | -e1   |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    // result.e1   = (a.e12 * b.e2) - (a.e31 * b.e3)
    // result.e2   = (a.e23 * b.e3) - (a.e12 * b.e1)
    // result.e3   = (a.e31 * b.e1) - (a.e23 * b.e2)
    // result.e123 = a.e12 * b.e3
    //             + a.e23 * b.e1
    //             + a.e31 * b.e2
    
    MultiVecVT result;
    //result.m_VT = ( BPermute<e12,e23,e31>( a ) * VPermute<e2,e3,e1>( b ) )
    //            - ( BPermute<e31,e12,e23>( a ) * VPermute<e3,e1,e2>( b ) ); // TODO: Use NegMulSub()!
    //SetW( &result.m_VT, Dot3( BPermute<e12,e23,e31>( a ), VPermute<e3,e1,e2>( b ) ) );
    result.m_VT = MakeVector4( ( BPermute<e12,e23,e31>( a ) * VPermute<e2,e3,e1>( b ) ) -
                               ( BPermute<e31,e12,e23>( a ) * VPermute<e3,e1,e2>( b ) ),
                               Dot( BPermute<e12,e23,e31>( a ), VPermute<e3,e1,e2>( b ) ) ); // TODO: Use NegMulSub()!
    return result;
}

/// Geometric Product Operator: Vector * TriVector
NOALIAS __forceinline const BiVector operator * ( const Vector PCREF a, const TriVector PCREF b )
{
    /// @code
    ///               |   T   |
    ///   +-----------+-------+
    ///   | LHS * RHS | e123  |
    /// --+-----------+-------+
    ///   | e1        | e23   |
    ///   +-----------+-------+
    /// V | e2        | e31   |
    ///   +-----------+-------+
    ///   | e3        | e12   |
    /// --+-----------+-------+
    /// @endcode
    
    return MakeBiVector( VPermute<e3,e1,e2>( a ) * b.m_T );
}

/// Geometric Product Operator: TriVector * Vector
NOALIAS __forceinline const BiVector operator * ( const TriVector PCREF a, const Vector PCREF b )
{
    /// @code
    ///               |           V           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS * RHS | e1    | e2    | e3    |
    /// --+-----------+-------+-------+-------+
    /// T | e123      | e23   | e31   | e12   |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    return MakeBiVector( a.m_T * VPermute<e3,e1,e2>( b ) );
}

/// Geometric Product Operator: BiVector * BiVector
NOALIAS __forceinline const MultiVecSB operator * ( const BiVector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///               |           B           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS * RHS | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    ///   | e12       | -1    | -e31  | e23   |
    ///   +-----------+-------+-------+-------+
    /// B | e23       | +e31  | -1    | -e12  |
    ///   +-----------+-------+-------+-------+
    ///   | e31       | -e23  | +e12  | -1    |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    // result.e12 = (a.e31 * b.e23) - (a.e23 * b.e31)
    // result.e23 = (a.e12 * b.e31) - (a.e31 * b.e12)
    // result.e31 = (a.e23 * b.e12) - (a.e12 * b.e23)
    // result.1   = -( a.e12 * b.e12
    //               + a.e23 * b.e23
    //               + a.e31 * b.e31 )
    
    MultiVecSB result;
    //result.m_SB = ( BPermute<e31,e12,e23>( a ) * BPermute<e23,e31,e12>( b ) )
    //            - ( BPermute<e23,e31,e12>( a ) * BPermute<e31,e12,e23>( b ) ); // TODO: Use NegMulSub()!
    //SetW( &result.m_SB, -Dot3( a.m_B, b.m_B ) );
    result.m_SB = MakeVector4( ( BPermute<e31,e12,e23>( a ) * BPermute<e23,e31,e12>( b ) ) -
                               ( BPermute<e23,e31,e12>( a ) * BPermute<e31,e12,e23>( b ) ),
                              -Dot( a.m_B, b.m_B ) ); // TODO: Use NegMulSub()!
    return result;
}

/// Geometric Product Operator: BiVector * TriVector
NOALIAS __forceinline const Vector operator * ( const BiVector PCREF a, const TriVector PCREF b )
{
    /// @code
    ///               |   T   |
    ///   +-----------+-------+
    ///   | LHS * RHS | e123  |
    /// --+-----------+-------+
    ///   | e12       | -e3   |
    ///   +-----------+-------+
    /// B | e23       | -e1   |
    ///   +-----------+-------+
    ///   | e31       | -e2   |
    /// --+-----------+-------+
    /// @endcode
    
    return MakeVector( -BPermute<e23,e31,e12>( a ) * b.m_T );
}

/// Geometric Product Operator: TriVector * BiVector
NOALIAS __forceinline const Vector operator * ( const TriVector PCREF a, const BiVector PCREF b )
{
    /// @code
    ///               |           B           |
    ///   +-----------+-------+-------+-------+
    ///   | LHS * RHS | e12   | e23   | e31   |
    /// --+-----------+-------+-------+-------+
    /// T | e123      | -e3   | -e1   | -e2   |
    /// --+-----------+-------+-------+-------+
    /// @endcode
    
    return MakeVector( -a.m_T * BPermute<e23,e31,e12>( b ) );
}

/// Geometric Product Operator: TriVector * TriVector
NOALIAS __forceinline const VF32 operator * ( const TriVector PCREF a, const TriVector PCREF b )
{
    /// @code
    ///               |   T   |
    ///   +-----------+-------+
    ///   | LHS * RHS | e123  |
    /// --+-----------+-------+
    /// T | e123      | -1    |
    /// --+-----------+-------+
    /// @endcode
    
    return -( a.m_T * b.m_T );
}


/// Geometric Product Operator: MultiVector * MultiVector
NOALIAS __forceinline const MultiVector operator * ( const MultiVector& l, const MultiVector& r )
{
    /// @code
    ///               |  S    |           V           |           B           |   T   |
    ///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    ///   | LHS * RHS | 1     | e1    | e2    | e3    | e12   | e23   | e31   | e123  |
    /// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    /// S | 1         | 1     | e1    | e2    | e3    | e12   | e23   | e31   | e123  |
    /// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    ///   | e1        | e1    | 1     | +e12  | -e31  | +e2   | e123  | -e3   | e23   |
    ///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    /// V | e2        | e2    | -e12  | 1     | +e23  | -e1   | +e3   | e123  | e31   |
    ///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    ///   | e3        | e3    | +e31  | -e23  | 1     | e123  | -e2   | e1    | e12   |
    /// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    ///   | e12       | e12   | -e2   | +e1   | e123  | -1    | -e31  | e23   | -e3   |
    ///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    /// B | e23       | e23   | e123  | -e3   | +e2   | +e31  | -1    | -e12  | -e1   |
    ///   +-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    ///   | e31       | e31   | +e3   | e123  | -e1   | -e23  | +e12  | -1    | -e2   |
    /// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    /// T | e123      | e123  | e23   | e31   | e12   | -e3   | -e1   | -e2   | -1    |
    /// --+-----------+-------+-------+-------+-------+-------+-------+-------+-------+
    /// @endcode
    
    // result.e1   = l.1 * r.e1   + l.e1 *  r.1    + l.e2 * -r.e12  + l.e3 *  r.e31  + l.e12 *  r.e2   + l.e23 * -r.e123 + l.e31 * -r.e3   + l.e123 * -r.e23
    // result.e2   = l.1 * r.e2   + l.e1 *  r.e12  + l.e2 *  r.1    + l.e3 * -r.e23  + l.e12 * -r.e1   + l.e23 *  r.e3   + l.e31 * -r.e123 + l.e123 * -r.e31
    // result.e3   = l.1 * r.e3   + l.e1 * -r.e31  + l.e2 *  r.e23  + l.e3 *  r.1    + l.e12 * -r.e123 + l.e23 * -r.e2   + l.e31 *  r.e1   + l.e123 * -r.e12
    // result.1    = l.1 * r.1    + l.e1 *  r.e1   + l.e2 *  r.e2   + l.e3 *  r.e3   + l.e12 *  r.e12  + l.e23 *  r.e23  + l.e31 *  r.e31  + l.e123 *  r.e123
    // result.e12  = l.1 * r.e12  + l.e1 *  r.e2   + l.e2 * -r.e1   + l.e3 *  r.e123 + l.e12 *  r.1    + l.e23 * -r.e31  + l.e31 *  r.e23  + l.e123 *  r.e3
    // result.e23  = l.1 * r.e23  + l.e1 *  r.e123 + l.e2 *  r.e3   + l.e3 * -r.e2   + l.e12 *  r.e31  + l.e23 *  r.1    + l.e31 * -r.e12  + l.e123 *  r.e23
    // result.e31  = l.1 * r.e31  + l.e1 * -r.e3   + l.e2 *  r.e123 + l.e3 *  r.e1   + l.e12 * -r.e23  + l.e23 *  r.e12  + l.e31 *  r.1    + l.e123 *  r.e2
    // result.e123 = l.1 * r.e123 + l.e1 *  r.e23  + l.e2 *  r.e31  + l.e3 *  r.e12  + l.e12 *  r.e3   + l.e23 *  r.e1   + l.e31 *  r.e2   + l.e123 *  r.1
    
    const Vector8 v8( ( ( Element< mv_1    >( l ) * Negate<0,0,0,0,0,0,0,0>( MV_Permute< mv_e1,   mv_e2,   mv_e3,   mv_1,    mv_e12,  mv_e23,  mv_e31,  mv_e123 >( r ) ) +
                          Element< mv_e1   >( l ) * Negate<0,0,1,0,0,0,1,0>( MV_Permute< mv_1,    mv_e12,  mv_e31,  mv_e1,   mv_e2,   mv_e123, mv_e3,   mv_e23  >( r ) ) ) +
                        ( Element< mv_e2   >( l ) * Negate<1,0,0,0,1,0,0,0>( MV_Permute< mv_e12,  mv_1,    mv_e23,  mv_e2,   mv_e1,   mv_e3,   mv_e123, mv_e31  >( r ) ) +
                          Element< mv_e3   >( l ) * Negate<0,1,0,0,0,1,0,0>( MV_Permute< mv_e31,  mv_e23,  mv_1,    mv_e3,   mv_e123, mv_e2,   mv_e1,   mv_e12  >( r ) ) ) ) +
                      ( ( Element< mv_e12  >( l ) * Negate<0,1,1,0,0,0,1,0>( MV_Permute< mv_e2,   mv_e1,   mv_e123, mv_e12,  mv_1,    mv_e31,  mv_e23,  mv_e3   >( r ) ) +
                          Element< mv_e23  >( l ) * Negate<1,0,1,0,1,0,0,0>( MV_Permute< mv_e123, mv_e3,   mv_e2,   mv_e23,  mv_e31,  mv_1,    mv_e12,  mv_e1   >( r ) ) ) +
                        ( Element< mv_e31  >( l ) * Negate<1,1,0,0,0,1,0,0>( MV_Permute< mv_e3,   mv_e123, mv_e1,   mv_e31,  mv_e23,  mv_e12,  mv_1,    mv_e2   >( r ) ) +
                          Element< mv_e123 >( l ) * Negate<1,1,1,0,0,0,0,0>( MV_Permute< mv_e23,  mv_e31,  mv_e12,  mv_e123, mv_e3,   mv_e23,  mv_e2,   mv_1    >( r ) ) ) ) );
    
    MultiVector result;
    result.m_VS.m_V = v8.m_V.m_V0;
    result.m_BT.m_V = v8.m_V.m_V1;
    return result;
}



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


