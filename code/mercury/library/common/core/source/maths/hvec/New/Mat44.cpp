#include "CorePch.h"
#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

namespace Maths
{
/// @addtogroup Mat44
//! @{

const Mat44 Const< Mat44 >::m_Zero =
    MakeMat44( 0.0f, 0.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f, 0.0f );

const Mat44 Const< Mat44 >::m_Identity =
    MakeMat44( 1.0f, 0.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 1.0f, 0.0f,
               0.0f, 0.0f, 0.0f, 1.0f );


/// Determinant of Mat44
NOALIAS const VF32 Determinant( const Mat44 MATREF src )
{
    const Mat33 m0( DeleteRowCol< E_RowX, E_ColX >( src ) );
    const Mat33 m1( DeleteRowCol< E_RowX, E_ColY >( src ) );
    const Mat33 m2( DeleteRowCol< E_RowX, E_ColZ >( src ) );
    const Mat33 m3( DeleteRowCol< E_RowX, E_ColW >( src ) );
    
    const VF32 det0( Determinant( m0 ) );
    const VF32 det1( Determinant( m1 ) );
    const VF32 det2( Determinant( m2 ) );
    const VF32 det3( Determinant( m3 ) );
    
    //const Vec4F32 row_x( RowX( src ) );
    //return X( row_x ) * det0
    //     - Y( row_x ) * det1
    //     + Z( row_x ) * det2
    //     - W( row_x ) * det3;
    //return NegMulSub( Y( row_x ), det1, X( row_x ) * det0 ) +
    //       NegMulSub( W( row_x ), det3, Z( row_x ) * det2 );
    
    const VF32 rx_cx( Element< E_RowX, E_ColX >( src ) );
    const VF32 rx_cy( Element< E_RowX, E_ColY >( src ) );
    const VF32 rx_cz( Element< E_RowX, E_ColZ >( src ) );
    const VF32 rx_cw( Element< E_RowX, E_ColW >( src ) );
    
    return NegMulSub( rx_cy, det1, rx_cx * det0 ) +
           NegMulSub( rx_cw, det3, rx_cz * det2 );
}

/// Inverse of Mat44 (faster but only 22 bit precision)
NOALIAS const Mat44 Inverse( const Mat44 MATREF src )
{
    const VF32 nearlyZero( Const<VF32>::Frac<1,10000>() );
    
    const VF32 detA( Determinant( src ) );
    if ( Abs( detA ) >= nearlyZero )
    {
        /// (1) Mat44 Inverse using Mat33 Inverse and blockwise inversion method:
        /// Faster & works for most common cases due to the top-left 3x3 sub-matrix usually being an invertible rotation matrix
        /// [ A B ]^-1    [ A^-1 + A^-1 * B * (D - C * A^-1 * B)^-1 * C * A^-1, -A^-1 * B * (D - C * A^-1 * B)^-1 ]
        /// [ C D ]    =  [ -(D - C * A^-1 * B)^-1 * C * A^-1,                  (D - C * A^-1 * B)^-1             ]
        /// [ A00 A01 A02 B0 ]
        /// [ A10 A11 A12 B1 ]
        /// [ A20 A21 A22 B2 ]
        /// [ C0  C1  C2  D  ]
        {
            Mat33   A; Vector3 B;
            Vector3 C; VF32    D;
            Split( src, &A, &B, &C, &D );
            
            Mat33 Ai;
            if ( Inverse( A, &Ai ) ) // Algorithm only valid if A is invertible
            {
                const Vector3 Ai_B( Ai * B );
                const Vector3 C_Ai( C * Ai );
                const VF32    D_minus_C_Ai_B( D - ( Dot( C_Ai, B ) ) );
                
                if ( Abs( D_minus_C_Ai_B ) >= nearlyZero ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const VF32    inverse_D_minus_C_Ai_B( Inverse( D_minus_C_Ai_B ) );
                    
                    const Vector3 Ai_B_inverse_D_minus_C_Ai_B( Ai_B * inverse_D_minus_C_Ai_B );
                    
                    const Mat33   result_A( Ai + ( TensorProduct( Ai_B_inverse_D_minus_C_Ai_B, C_Ai ) ) );
                    const Vector3 result_B( -Ai_B_inverse_D_minus_C_Ai_B );
                    const Vector3 result_C( -inverse_D_minus_C_Ai_B * C_Ai );
                    const VF32    result_D( inverse_D_minus_C_Ai_B );
                    
                    const Mat44 result( MakeMat44( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= nearlyZero ) // Double-check that result is invertible (TODO: safe to eliminate this check?)
                        return result;
                }
            }
        }
        
        /// (2) Mat44 Inverse using Cramer's Rule
        /// Slower but can always compute the inverse of an invertible 4x4 matrix
        /// Cramer's Rule
        ///  A.x = c, x[n] = det(An) / det(A), An is A with the n-th column replaced by c
        /// Matrix inverse using Cramer's Rule:
        ///  x = column m of A's inverse (Ai), c = column m of identity (I):
        ///  A.Ai[][m] = I[][m], Ai[n][m] = det(Anm) / det(A), Anm is A with the n-th column replaced by the m-th column of I
        
        {
            const VF32 rcpDetA( Rcp( detA ) );
            const Mat44 I( Const<Mat44>::Identity() );
            Mat44 Anm( src );
            Mat44 inverseA( Const<Mat44>::Zero() );
            
            ReplaceColumn<0,0>( &Anm, I ); SetElement<0,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=0
            ReplaceColumn<0,1>( &Anm, I ); SetElement<0,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=1
            ReplaceColumn<0,2>( &Anm, I ); SetElement<0,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=2
            ReplaceColumn<0,3>( &Anm, I ); SetElement<0,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=3
            ReplaceColumn<0,0>( &Anm, src ); 
                       
            ReplaceColumn<1,0>( &Anm, I ); SetElement<1,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=0
            ReplaceColumn<1,1>( &Anm, I ); SetElement<1,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=1
            ReplaceColumn<1,2>( &Anm, I ); SetElement<1,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=2
            ReplaceColumn<1,3>( &Anm, I ); SetElement<1,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=3
            ReplaceColumn<1,1>( &Anm, src ); 
                       
            ReplaceColumn<2,0>( &Anm, I ); SetElement<2,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=0
            ReplaceColumn<2,1>( &Anm, I ); SetElement<2,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=1
            ReplaceColumn<2,2>( &Anm, I ); SetElement<2,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=2
            ReplaceColumn<2,3>( &Anm, I ); SetElement<2,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=3
            ReplaceColumn<2,2>( &Anm, src ); 
                       
            ReplaceColumn<3,0>( &Anm, I ); SetElement<3,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=0
            ReplaceColumn<3,1>( &Anm, I ); SetElement<3,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=1
            ReplaceColumn<3,2>( &Anm, I ); SetElement<3,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=2
            ReplaceColumn<3,3>( &Anm, I ); SetElement<3,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=3
                       
            return inverseA;
        }
        
        #if 0
        
        // [ A0  A1  A2  B  ]
        // [ C00 C01 C02 D0 ]
        // [ C10 C11 C12 D1 ]
        // [ C20 C21 C22 D2 ]
        {
            Vector3 A; // row vec3
            Vector3 D; // col vec3
            VF32    B; // scalar
            Mat33   C; // mat33
            Split( src, &A, &B, &C, &D );
            
            if ( LengthSquared( A ) >= ( nearlyZero * nearlyZero ) ) // Algorithm only valid if A is invertible
            {
                const Vector3 Ai( Inverse( A ) ); // col vec3
                const Vector3 Ai_B( Ai * B );     // col vec3
                const Vector3 C_Ai( C * Ai );     // col vec3
                const Vector3 D_minus_C_Ai_B( D - ( C_Ai * B ) ); // col vec3
                
                if ( LengthSquared( D_minus_C_Ai_B ) >= ( nearlyZero * nearlyZero ) ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const Vector3 inverse_D_minus_C_Ai_B( Inverse( D_minus_C_Ai_B ) ); // row vec3
                    const Mat33   Ai_B_inverse_D_minus_C_Ai_B( TensorProduct( Ai_B, inverse_D_minus_C_Ai_B ) ); // mat33
                    
                    const Vector3 result_A( Ai + ( Ai_B_inverse_D_minus_C_Ai_B * C_Ai ) ); // col vec3
                    const Mat33   result_B( -Ai_B_inverse_D_minus_C_Ai_B ); // mat33
                    const VF32    result_C( -Dot( inverse_D_minus_C_Ai_B, C_Ai ) ); // scalar
                    const Vector3 result_D( inverse_D_minus_C_Ai_B ); // row vec3
                    
                    const Mat44 result( MakeMat44( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= nearlyZero )
                        return result;
                }
            }
        }
        
        // [ A  B0  B1  B2  ]
        // [ C0 D00 D01 D02 ]
        // [ C1 D10 D11 D12 ]
        // [ C2 D20 D21 D22 ]
        {
            VF32    A; Vector3 B;
            Vector3 C; Mat33   D;
            Split( src, &A, &B, &C, &D );
            
            if ( Abs( A ) >= nearlyZero ) // Algorithm only valid if A is invertible
            {
                const VF32    Ai( Inverse( A ) );
                const Vector3 Ai_B( Ai * B );
                const Vector3 C_Ai( C * Ai );
                const Mat33   D_minus_C_Ai_B( D - ( TensorProduct( C_Ai, B ) ) );
                
                if ( Abs( Determinant( D_minus_C_Ai_B ) ) >= nearlyZero ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const Mat33   inverse_D_minus_C_Ai_B( Inverse( D_minus_C_Ai_B ) );
                    const Vector3 Ai_B_inverse_D_minus_C_Ai_B( Ai_B * inverse_D_minus_C_Ai_B );
                    
                    const VF32    result_A( Ai + ( Dot( Ai_B_inverse_D_minus_C_Ai_B, C_Ai ) ) );
                    const Vector3 result_B( -Ai_B_inverse_D_minus_C_Ai_B );
                    //const Vector3 result_C( -inverse_D_minus_C_Ai_B * C_Ai );
                    const Vector3 result_C( inverse_D_minus_C_Ai_B * -C_Ai );
                    const Mat33   result_D( inverse_D_minus_C_Ai_B );
                    
                    const Mat44 result( MakeMat44( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= nearlyZero )
                        return result;
                }
            }
        }
        
        // [ A0 B00 B01 B02 ]
        // [ A1 B10 B11 B12 ]
        // [ A2 B20 B21 B22 ]
        // [ C  D0  D1  D2  ]
        {
            Vector3 A; // col vec3
            Mat33   B; // mat33
            VF32    C; // scalar
            Vector3 D; // row vec3
            Split( src, &A, &B, &C, &D );
            
            if ( LengthSquared( A ) >= ( nearlyZero * nearlyZero ) ) // Algorithm only valid if A is invertible
            {
                const Vector3 Ai( Inverse( A ) ); // row vec3
                const Vector3 Ai_B( Ai * B ); // row vec3
                const Vector3 C_Ai( C * Ai ); // row vec3
                const Vector3 D_minus_C_Ai_B( D - ( C_Ai * B ) ); // row vec3
                
                if ( LengthSquared( D_minus_C_Ai_B ) >= ( nearlyZero * nearlyZero ) ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const Vector3 inverse_D_minus_C_Ai_B( Inverse( D - ( C_Ai * B ) ) ); // col vec3
                    const VF32    Ai_B_inverse_D_minus_C_Ai_B( Dot( Ai_B, inverse_D_minus_C_Ai_B ) ); // scalar
                    
                    const Vector3 result_A( Ai + ( Ai_B_inverse_D_minus_C_Ai_B * C_Ai ) ); // row vec3
                    const VF32    result_B( -Ai_B_inverse_D_minus_C_Ai_B ); // scalar
                    //const Mat33   result_C( -TensorProduct( inverse_D_minus_C_Ai_B, C_Ai ) ); // mat33
                    const Mat33   result_C( TensorProduct( -inverse_D_minus_C_Ai_B, C_Ai ) ); // mat33
                    const Vector3 result_D( inverse_D_minus_C_Ai_B ); // col vec3
                    
                    const Mat44 result( MakeMat44( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= nearlyZero )
                        return result;
                }
            }
        }
        
        #endif
    }
    
    return Const<Mat44>::Zero();
}

/// Inverse of Mat44 (slower but full 24 bit precision)
NOALIAS const Mat44 Inverse24( const Mat44 MATREF src )
{
    const VF32 nearlyZero( Const<VF32>::Frac<1,10000>() );
    
    const VF32 detA( Determinant( src ) );
    if ( Abs( detA ) >= nearlyZero )
    {
        /// (1) Mat44 Inverse using Mat33 Inverse and blockwise inversion method:
        /// Faster & works for most common cases due to the top-left 3x3 sub-matrix usually being an invertible rotation matrix
        /// [ A B ]^-1    [ A^-1 + A^-1 * B * (D - C * A^-1 * B)^-1 * C * A^-1, -A^-1 * B * (D - C * A^-1 * B)^-1 ]
        /// [ C D ]    =  [ -(D - C * A^-1 * B)^-1 * C * A^-1,                  (D - C * A^-1 * B)^-1             ]
        /// [ A00 A01 A02 B0 ]
        /// [ A10 A11 A12 B1 ]
        /// [ A20 A21 A22 B2 ]
        /// [ C0  C1  C2  D  ]
        {
            Mat33   A; Vector3 B;
            Vector3 C; VF32    D;
            Split( src, &A, &B, &C, &D );
            
            Mat33 Ai;
            if ( Inverse24( A, &Ai ) ) // Algorithm only valid if A is invertible
            {
                const Vector3 Ai_B( Ai * B );
                const Vector3 C_Ai( C * Ai );
                const VF32    D_minus_C_Ai_B( D - ( Dot( C_Ai, B ) ) );
                
                if ( Abs( D_minus_C_Ai_B ) >= nearlyZero ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const VF32    inverse_D_minus_C_Ai_B( Inverse24( D_minus_C_Ai_B ) );
                    
                    const Vector3 Ai_B_inverse_D_minus_C_Ai_B( Ai_B * inverse_D_minus_C_Ai_B );
                    
                    const Mat33   result_A( Ai + ( TensorProduct( Ai_B_inverse_D_minus_C_Ai_B, C_Ai ) ) );
                    const Vector3 result_B( -Ai_B_inverse_D_minus_C_Ai_B );
                    const Vector3 result_C( -inverse_D_minus_C_Ai_B * C_Ai );
                    const VF32    result_D( inverse_D_minus_C_Ai_B );
                    
                    const Mat44 result( MakeMat44( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= nearlyZero ) // Double-check that result is invertible (TODO: safe to eliminate this check?)
                        return result;
                }
            }
        }
        
        /// (2) Mat44 Inverse using Cramer's Rule
        /// Slower but can always compute the inverse of an invertible 4x4 matrix
        /// Cramer's Rule
        ///  A.x = c, x[n] = det(An) / det(A), An is A with the n-th column replaced by c
        /// Matrix inverse using Cramer's Rule:
        ///  x = column m of A's inverse (Ai), c = column m of identity (I):
        ///  A.Ai[][m] = I[][m], Ai[n][m] = det(Anm) / det(A), Anm is A with the n-th column replaced by the m-th column of I
        
        {
            const VF32 rcpDetA( Rcp24( detA ) );
            const Mat44 I( Const<Mat44>::Identity() );
            Mat44 Anm( src );
            Mat44 inverseA( Const<Mat44>::Zero() );
            
            ReplaceColumn<0,0>( &Anm, I ); SetElement<0,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=0
            ReplaceColumn<0,1>( &Anm, I ); SetElement<0,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=1
            ReplaceColumn<0,2>( &Anm, I ); SetElement<0,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=2
            ReplaceColumn<0,3>( &Anm, I ); SetElement<0,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=3
            ReplaceColumn<0,0>( &Anm, src ); 
                       
            ReplaceColumn<1,0>( &Anm, I ); SetElement<1,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=0
            ReplaceColumn<1,1>( &Anm, I ); SetElement<1,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=1
            ReplaceColumn<1,2>( &Anm, I ); SetElement<1,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=2
            ReplaceColumn<1,3>( &Anm, I ); SetElement<1,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=3
            ReplaceColumn<1,1>( &Anm, src ); 
                       
            ReplaceColumn<2,0>( &Anm, I ); SetElement<2,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=0
            ReplaceColumn<2,1>( &Anm, I ); SetElement<2,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=1
            ReplaceColumn<2,2>( &Anm, I ); SetElement<2,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=2
            ReplaceColumn<2,3>( &Anm, I ); SetElement<2,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=3
            ReplaceColumn<2,2>( &Anm, src ); 
                       
            ReplaceColumn<3,0>( &Anm, I ); SetElement<3,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=0
            ReplaceColumn<3,1>( &Anm, I ); SetElement<3,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=1
            ReplaceColumn<3,2>( &Anm, I ); SetElement<3,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=2
            ReplaceColumn<3,3>( &Anm, I ); SetElement<3,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=3
                       
            return inverseA;
        }
    }
    
    return Const<Mat44>::Zero();
}



//! @}
}; // namespace Maths


