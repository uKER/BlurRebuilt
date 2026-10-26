#include "CorePch.h"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>

namespace Maths
{
/// @addtogroup Mat55
//! @{

/// Determinant of Mat55
NOALIAS const VF32 Determinant( const Mat55& src )
{
    const VF32 det0( Determinant( DeleteRowCol<0,0>( src ) ) );
    const VF32 det1( Determinant( DeleteRowCol<0,1>( src ) ) );
    const VF32 det2( Determinant( DeleteRowCol<0,2>( src ) ) );
    const VF32 det3( Determinant( DeleteRowCol<0,3>( src ) ) );
    const VF32 det4( Determinant( DeleteRowCol<0,4>( src ) ) );
    
    const VF32 f0( Element<0,0>( src ) );
    const VF32 f1( Element<0,1>( src ) );
    const VF32 f2( Element<0,2>( src ) );
    const VF32 f3( Element<0,3>( src ) );
    const VF32 f4( Element<0,4>( src ) );
    
    return NegMulSub( f1, det1, f0 * det0 ) +
           NegMulSub( f3, det3, f2 * det2 ) +
                                f4 * det4;
}

/// Inverse of Mat55
NOALIAS const Mat55 Inverse( const Mat55& src )
{
    // Mat55 Inverse using Mat44 Inverse and blockwise inversion method
    
    // [ A B ]^-1    [ A^-1 + A^-1 * B * (D - C * A^-1 * B)^-1 * C * A^-1, -A^-1 * B * (D - C * A^-1 * B)^-1 ]
    // [ C D ]    =  [ -(D - C * A^-1 * B)^-1 * C * A^-1,                  (D - C * A^-1 * B)^-1             ]
    
    const VF32 epsilon( Const<VF32>::Epsilon() );
    
    if ( Abs( Determinant( src ) ) >= epsilon )
    {
        // [ A00 A01 A02 A03 B0 ]
        // [ A10 A11 A12 A13 B1 ]
        // [ A20 A21 A22 A23 B2 ]
        // [ A30 A31 A32 A33 B3 ]
        // [ C0  C1  C2  C3  D  ]
        {
            Mat44   A; Vector4 B;
            Vector4 C; VF32    D;
            Split( src, &A, &B, &C, &D );
            
            if ( Abs( Determinant( A ) ) >= epsilon ) // Algorithm only valid if A is invertible
            {
                const Mat44   Ai( Inverse( A ) );
                const Vector4 Ai_B( Ai * B );
                const Vector4 C_Ai( C * Ai );
                const VF32    D_minus_C_Ai_B( D - ( Dot( C_Ai, B ) ) );
                
                if ( Abs( D_minus_C_Ai_B ) >= epsilon ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const VF32    inverse_D_minus_C_Ai_B( Inverse( D_minus_C_Ai_B ) );
                    const Vector4 Ai_B_inverse_D_minus_C_Ai_B( Ai_B * inverse_D_minus_C_Ai_B );
                    
                    const Mat44   result_A( Ai + ( TensorProduct( Ai_B_inverse_D_minus_C_Ai_B, C_Ai ) ) );
                    const Vector4 result_B( -Ai_B_inverse_D_minus_C_Ai_B );
                    const Vector4 result_C( -inverse_D_minus_C_Ai_B * C_Ai );
                    const VF32    result_D( inverse_D_minus_C_Ai_B );
                    
                    const Mat55 result( MakeMat55( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= epsilon )
                        return result;
                }
            }
        }
        
        // [ A0  A1  A2  A3  B  ]
        // [ C00 C01 C02 C03 D0 ]
        // [ C10 C11 C12 C13 D1 ]
        // [ C20 C21 C22 C23 D2 ]
        // [ C30 C31 C32 C33 D3 ]
        {
            Vector4 A; VF32    B;
            Mat44   C; Vector4 D;
            Split( src, &A, &B, &C, &D );
            
            if ( LengthSquared( A ) >= ( epsilon * epsilon ) ) // Algorithm only valid if A is invertible
            {
                const Vector4 Ai( Inverse( A ) ); // col vec4
                const Vector4 Ai_B( Ai * B ); // col vec4
                const Vector4 C_Ai( C * Ai ); // col vec4
                const Vector4 D_minus_C_Ai_B( D - ( C_Ai * B ) ); // col vec4
                
                if ( LengthSquared( D_minus_C_Ai_B ) >= ( epsilon * epsilon ) ) // Algorithm only valid if D_minus_C_Ai_B is invertible
                {
                    const Vector4 inverse_D_minus_C_Ai_B( Inverse( D_minus_C_Ai_B ) ); // row vec4
                    const Mat44   Ai_B_inverse_D_minus_C_Ai_B( TensorProduct( Ai_B, inverse_D_minus_C_Ai_B ) ); // mat44
                    
                    const Vector4 result_A( Ai + ( Ai_B_inverse_D_minus_C_Ai_B * C_Ai ) ); // col vec4
                    const Mat44   result_B( -Ai_B_inverse_D_minus_C_Ai_B ); // mat44
                    const VF32    result_C( -Dot( inverse_D_minus_C_Ai_B, C_Ai ) ); // scalar
                    const Vector4 result_D( inverse_D_minus_C_Ai_B ); // row vec4
                    
                    const Mat55 result( MakeMat55( result_A, result_B,
                                                   result_C, result_D ) );
                    
                    //if ( Abs( Determinant( result ) ) >= epsilon )
                        return result;
                }
            }
        }
    }
    
    return Const<Mat55>::Zero();
}





//! @}
}; // namespace Maths


