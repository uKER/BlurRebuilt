#include "CorePch.h"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>

namespace Maths
{
/// @addtogroup Mat66
//! @{

/// Determinant of Mat66
NOALIAS const VF32 Determinant( const Mat66& src )
{
    const VF32 det0( Determinant( DeleteRowCol<0,0>( src ) ) );
    const VF32 det1( Determinant( DeleteRowCol<0,1>( src ) ) );
    const VF32 det2( Determinant( DeleteRowCol<0,2>( src ) ) );
    const VF32 det3( Determinant( DeleteRowCol<0,3>( src ) ) );
    const VF32 det4( Determinant( DeleteRowCol<0,4>( src ) ) );
    const VF32 det5( Determinant( DeleteRowCol<0,5>( src ) ) );
    
    const VF32 f0( Element<0,0>( src ) );
    const VF32 f1( Element<0,1>( src ) );
    const VF32 f2( Element<0,2>( src ) );
    const VF32 f3( Element<0,3>( src ) );
    const VF32 f4( Element<0,4>( src ) );
    const VF32 f5( Element<0,5>( src ) );
    
    return NegMulSub( f1, det1, f0 * det0 ) +
           NegMulSub( f3, det3, f2 * det2 ) +
           NegMulSub( f5, det5, f4 * det4 );
}

/// Inverse of Mat66
NOALIAS const Mat66 Inverse( const Mat66& src )
{
    const VF32 epsilon( Const<VF32>::Epsilon() );
    
    const VF32 detA( Determinant( src ) );
    if ( Abs( detA ) >= epsilon )
    {
        // Mat66 Inverse using Mat33 Inverse and blockwise inversion method
        // [ A B ]^-1    [ A^-1 + A^-1 * B * (D - C * A^-1 * B)^-1 * C * A^-1, -A^-1 * B * (D - C * A^-1 * B)^-1 ]
        // [ C D ]    =  [ -(D - C * A^-1 * B)^-1 * C * A^-1,                  (D - C * A^-1 * B)^-1             ]
        // [ A00 A01 A02 B00 B01 B02 ]
        // [ A10 A11 A12 B10 B11 B12 ]
        // [ A20 A21 A22 B20 B21 B22 ]
        // [ C00 C01 C02 D00 D01 D02 ]
        // [ C10 C11 C12 D10 D11 D12 ]
        // [ C20 C21 C22 D20 D21 D22 ]
        {
            Mat33 A, B, C, D;
            Split( src, &A, &B, &C, &D );
            
            if ( Abs( Determinant( A ) ) >= epsilon ) // Algorithm only valid if A is invertible
            {
                const Mat33 Ai( Inverse( A ) );
                const Mat33 Ai_B( Ai * B );
                const Mat33 C_Ai( C * Ai );
                const Mat33 D_minus_C_Ai_B( D - ( C_Ai * B ) );
                
                if ( Abs( Determinant( D_minus_C_Ai_B ) ) >= epsilon ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const Mat33 inverse_D_minus_C_Ai_B( Inverse( D_minus_C_Ai_B ) );
                    const Mat33 Ai_B_inverse_D_minus_C_Ai_B( Ai_B * inverse_D_minus_C_Ai_B );
                    
                    const Mat33 result_A( Ai + ( Ai_B_inverse_D_minus_C_Ai_B * C_Ai ) );
                    const Mat33 result_B( -Ai_B_inverse_D_minus_C_Ai_B );
                    const Mat33 result_C( -( inverse_D_minus_C_Ai_B * C_Ai ) );
                    const Mat33 result_D( inverse_D_minus_C_Ai_B );
                    
                    const Mat66 result( MakeMat66( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= epsilon )
                        return result;
                }
            }
        }
        
        /// Cramer's Rule:
        ///  A.x = c, x[n] = det(An) / det(A), An is A with the n-th column replaced by c
        /// Matrix inverse using Cramer's Rule:
        ///  x = column m of A's inverse (Ai), c = column m of identity (I):
        ///  A.Ai[][m] = I[][m], Ai[n][m] = det(Anm) / det(A), Anm is A with the n-th column replaced by the m-th column of I
        {
            const VF32 rcpDetA( Rcp( detA ) );
            const Mat66 I( Const<Mat66>::Identity() );
            Mat66 Anm( src );
            Mat66 inverseA( Const<Mat66>::Zero() ); // JASOND: PS3 compiler whinges about this being used uninitialised...
            
            ReplaceColumn<0,0>( &Anm, I ); SetElement<0,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=0
            ReplaceColumn<0,1>( &Anm, I ); SetElement<0,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=1
            ReplaceColumn<0,2>( &Anm, I ); SetElement<0,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=2
            ReplaceColumn<0,3>( &Anm, I ); SetElement<0,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=3
            ReplaceColumn<0,4>( &Anm, I ); SetElement<0,4>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=4
            ReplaceColumn<0,5>( &Anm, I ); SetElement<0,5>( &inverseA, Determinant(Anm) * rcpDetA ); // n=0, m=5
            ReplaceColumn<0,0>( &Anm, src ); 
                       
            ReplaceColumn<1,0>( &Anm, I ); SetElement<1,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=0
            ReplaceColumn<1,1>( &Anm, I ); SetElement<1,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=1
            ReplaceColumn<1,2>( &Anm, I ); SetElement<1,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=2
            ReplaceColumn<1,3>( &Anm, I ); SetElement<1,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=3
            ReplaceColumn<1,4>( &Anm, I ); SetElement<1,4>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=4
            ReplaceColumn<1,5>( &Anm, I ); SetElement<1,5>( &inverseA, Determinant(Anm) * rcpDetA ); // n=1, m=5
            ReplaceColumn<1,1>( &Anm, src ); 
                       
            ReplaceColumn<2,0>( &Anm, I ); SetElement<2,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=0
            ReplaceColumn<2,1>( &Anm, I ); SetElement<2,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=1
            ReplaceColumn<2,2>( &Anm, I ); SetElement<2,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=2
            ReplaceColumn<2,3>( &Anm, I ); SetElement<2,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=3
            ReplaceColumn<2,4>( &Anm, I ); SetElement<2,4>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=4
            ReplaceColumn<2,5>( &Anm, I ); SetElement<2,5>( &inverseA, Determinant(Anm) * rcpDetA ); // n=2, m=5
            ReplaceColumn<2,2>( &Anm, src ); 
                       
            ReplaceColumn<3,0>( &Anm, I ); SetElement<3,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=0
            ReplaceColumn<3,1>( &Anm, I ); SetElement<3,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=1
            ReplaceColumn<3,2>( &Anm, I ); SetElement<3,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=2
            ReplaceColumn<3,3>( &Anm, I ); SetElement<3,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=3
            ReplaceColumn<3,4>( &Anm, I ); SetElement<3,4>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=4
            ReplaceColumn<3,5>( &Anm, I ); SetElement<3,5>( &inverseA, Determinant(Anm) * rcpDetA ); // n=3, m=5
            ReplaceColumn<3,3>( &Anm, src ); 
                       
            ReplaceColumn<4,0>( &Anm, I ); SetElement<4,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=4, m=0
            ReplaceColumn<4,1>( &Anm, I ); SetElement<4,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=4, m=1
            ReplaceColumn<4,2>( &Anm, I ); SetElement<4,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=4, m=2
            ReplaceColumn<4,3>( &Anm, I ); SetElement<4,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=4, m=3
            ReplaceColumn<4,4>( &Anm, I ); SetElement<4,4>( &inverseA, Determinant(Anm) * rcpDetA ); // n=4, m=4
            ReplaceColumn<4,5>( &Anm, I ); SetElement<4,5>( &inverseA, Determinant(Anm) * rcpDetA ); // n=4, m=5
            ReplaceColumn<4,4>( &Anm, src ); 
                       
            ReplaceColumn<5,0>( &Anm, I ); SetElement<5,0>( &inverseA, Determinant(Anm) * rcpDetA ); // n=5, m=0
            ReplaceColumn<5,1>( &Anm, I ); SetElement<5,1>( &inverseA, Determinant(Anm) * rcpDetA ); // n=5, m=1
            ReplaceColumn<5,2>( &Anm, I ); SetElement<5,2>( &inverseA, Determinant(Anm) * rcpDetA ); // n=5, m=2
            ReplaceColumn<5,3>( &Anm, I ); SetElement<5,3>( &inverseA, Determinant(Anm) * rcpDetA ); // n=5, m=3
            ReplaceColumn<5,4>( &Anm, I ); SetElement<5,4>( &inverseA, Determinant(Anm) * rcpDetA ); // n=5, m=4
            ReplaceColumn<5,5>( &Anm, I ); SetElement<5,5>( &inverseA, Determinant(Anm) * rcpDetA ); // n=5, m=5
            
            return inverseA;
        }
        
        #if 0
        
        // Mat66 Inverse using Mat55/Mat33 Inverse and blockwise inversion method
        // [ A B ]^-1    [ A^-1 + A^-1 * B * (D - C * A^-1 * B)^-1 * C * A^-1, -A^-1 * B * (D - C * A^-1 * B)^-1 ]
        // [ C D ]    =  [ -(D - C * A^-1 * B)^-1 * C * A^-1,                  (D - C * A^-1 * B)^-1             ]
        // [ A00 A01 A02 A03 A04 B0 ]
        // [ A10 A11 A12 A13 A14 B1 ]
        // [ A20 A21 A22 A23 A24 B2 ]
        // [ A30 A31 A32 A33 A34 B3 ]
        // [ A40 A41 A42 A43 A44 B4 ]
        // [ C0  C1  C2  C3  C4  D  ]
        {
            Mat55   A; Vector5 B;
            Vector5 C; VF32    D;
            Split( src, &A, &B, &C, &D );
            
            if ( Abs( Determinant( A ) ) >= epsilon ) // Algorithm only valid if A is invertible
            {
                const Mat55   Ai( Inverse( A ) );
                const Vector5 Ai_B( Ai * B );
                const Vector5 C_Ai( C * Ai );
                const VF32    D_minus_C_Ai_B( D - ( Dot( C_Ai, B ) ) );
                
                if ( Abs( D_minus_C_Ai_B ) >= epsilon ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const VF32    inverse_D_minus_C_Ai_B( Inverse( D_minus_C_Ai_B ) );
                    const Vector5 Ai_B_inverse_D_minus_C_Ai_B( Ai_B * inverse_D_minus_C_Ai_B );
                    
                    const Mat55   result_A( Ai + ( TensorProduct( Ai_B_inverse_D_minus_C_Ai_B, C_Ai ) ) );
                    const Vector5 result_B( -Ai_B_inverse_D_minus_C_Ai_B );
                    const Vector5 result_C( -inverse_D_minus_C_Ai_B * C_Ai );
                    const VF32    result_D( inverse_D_minus_C_Ai_B );
                    
                    const Mat66 result( MakeMat66( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= epsilon )
                        return result;
                }
            }
        }
        
        // [ A0  A1  A2  A3  A4  B  ]
        // [ C00 C01 C02 C03 C04 D0 ]
        // [ C10 C11 C12 C13 C14 D1 ]
        // [ C20 C21 C22 C23 C24 D2 ]
        // [ C30 C31 C32 C33 C34 D3 ]
        // [ C40 C41 C42 C43 C44 D4 ]
        {
            Vector5 A; VF32    B;
            Mat55   C; Vector5 D;
            Split( src, &A, &B, &C, &D );
            
            if ( LengthSquared( A ) >= ( epsilon * epsilon ) ) // Algorithm only valid if A is invertible
            {
                const Vector5 Ai( Inverse( A ) ); // col vec5
                const Vector5 Ai_B( Ai * B ); // col vec5
                const Vector5 C_Ai( C * Ai ); // col vec5
                const Vector5 D_minus_C_Ai_B( D - ( C_Ai * B ) ); // col vec5
                
                if ( LengthSquared( D_minus_C_Ai_B ) >= ( epsilon * epsilon ) ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const Vector5 inverse_D_minus_C_Ai_B( Inverse( D_minus_C_Ai_B ) ); // row vec5
                    const Mat55   Ai_B_inverse_D_minus_C_Ai_B( TensorProduct( Ai_B, inverse_D_minus_C_Ai_B ) ); // mat55
                    
                    const Vector5 result_A( Ai + ( Ai_B_inverse_D_minus_C_Ai_B * C_Ai ) ); // col vec5
                    const Mat55   result_B( -Ai_B_inverse_D_minus_C_Ai_B ); // mat55
                    const VF32    result_C( -Dot( inverse_D_minus_C_Ai_B, C_Ai ) ); // scalar
                    const Vector5 result_D( inverse_D_minus_C_Ai_B ); // row vec5
                    
                    const Mat66 result( MakeMat66( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= epsilon )
                        return result;
                }
            }
        }
        
        #endif
    }
    
    return Const<Mat66>::Zero();
}






//! @}
}; // namespace Maths


