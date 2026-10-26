#include "CorePch.h"
#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

namespace Maths
{
/// @addtogroup Mat43
//! @{

const Mat43 Const< Mat43 >::m_Zero =
    MakeMat43( 0.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f );

const Mat43 Const< Mat43 >::m_Identity =
    MakeMat43( 1.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f,
               0.0f, 0.0f, 1.0f,
               0.0f, 0.0f, 0.0f );


/// Inverse of Mat43
NOALIAS const Mat43 Inverse( const Mat43 MATREF src )
{
    // Mat43 Inverse using Mat33 Inverse and blockwise inversion method
    // [ A00 A01 A02 0 ]
    // [ A10 A11 A12 0 ]
    // [ A20 A21 A22 0 ]
    // [ C0  C1  C2  1 ]
    
    // [ A B ]^-1    [ A^-1 + A^-1 * B * (D - C * A^-1 * B)^-1 * C * A^-1, -A^-1 * B * (D - C * A^-1 * B)^-1 ]
    // [ C D ]    =  [ -(D - C * A^-1 * B)^-1 * C * A^-1,                  (D - C * A^-1 * B)^-1             ]
    
    const Mat33   A( MakeMat33( src ) );
    const Mat33   iA( Inverse( A ) );
    const Vector3 C( RowW( src ) );
    return MakeMat43( iA, -C * iA );
}



//! @}
}; // namespace Maths


