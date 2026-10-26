#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{



NOALIAS const HF32 VectorSumOfSquares_Aligned(
    F32 const * const __restrict pA,
    const S32& input_elements )
{
    // TODO: Optimise this!
    S32 elements( input_elements );
    HF32 sos; // Zero
    for ( S32 i=0; i<elements; i++ )
    {
        const HF32 a( Load<HF32>( &pA[i] ) );
        sos += a * a;
    }
    return sos;
}


#if defined( _VMX128_ )

NOALIAS const __vector4 VectorDotProduct_Aligned(
    F32 const * __restrict pA,
    F32 const * __restrict pB,
    const S32& input_elements )
{
    BIG_MATRIX_ASSERT( ( (S32)pA & 0xF ) == 0 );
    BIG_MATRIX_ASSERT( ( (S32)pB & 0xF ) == 0 );
    
    __dcbt( 0x0, pA );
    __dcbt( 0x0, pB );
    
    const __vector4 vi_4( __vspltisw( 4 ) );
    const __vector4 vi_8( __vspltisw( 8 ) );
    const __vector4 vi_12( __vaddsws( vi_4, vi_8 ) );
    const __vector4 vi_16( __vaddsws( vi_8, vi_8 ) );
    const __vector4 vi_20( __vaddsws( vi_8, vi_12 ) );
    const __vector4 vi_24( __vaddsws( vi_12, vi_12 ) );
    const __vector4 vi_28( __vaddsws( vi_12, vi_16 ) );
    const __vector4 vi_32( __vaddsws( vi_16, vi_16 ) );
    
    const __vector4 vi_dec0( __vmrghw( __vmrghw( __vspltisw( 0 ), __vspltisw( 2 ) ),
                                       __vmrghw( __vspltisw( 1 ), __vspltisw( 3 ) ) ) );
    
    const __vector4 vi_dec1( __vaddsws( vi_dec0, vi_4 ) );
    const __vector4 vi_dec2( __vaddsws( vi_dec0, vi_8 ) );
    const __vector4 vi_dec3( __vaddsws( vi_dec0, vi_12 ) );
    const __vector4 vi_dec4( __vaddsws( vi_dec0, vi_16 ) );
    const __vector4 vi_dec5( __vaddsws( vi_dec0, vi_20 ) );
    const __vector4 vi_dec6( __vaddsws( vi_dec0, vi_24 ) );
    const __vector4 vi_dec7( __vaddsws( vi_dec0, vi_28 ) );
    
    const __vector4 vi_elements( __vspltw( __lvlx( &input_elements, 0x0 ), 0 ) );
    
    __vector4 vi_elements0( __vsubsws( vi_elements, vi_dec0 ) );
    __vector4 vi_elements1( __vsubsws( vi_elements, vi_dec1 ) );
    __vector4 vi_elements2( __vsubsws( vi_elements, vi_dec2 ) );
    __vector4 vi_elements3( __vsubsws( vi_elements, vi_dec3 ) );
    __vector4 vi_elements4( __vsubsws( vi_elements, vi_dec4 ) );
    __vector4 vi_elements5( __vsubsws( vi_elements, vi_dec5 ) );
    __vector4 vi_elements6( __vsubsws( vi_elements, vi_dec6 ) );
    __vector4 vi_elements7( __vsubsws( vi_elements, vi_dec7 ) );
    
    __vector4 v_dp0( __vzero() );
    __vector4 v_dp1( __vzero() );
    __vector4 v_dp2( __vzero() );
    __vector4 v_dp3( __vzero() );
    __vector4 v_dp4( __vzero() );
    __vector4 v_dp5( __vzero() );
    __vector4 v_dp6( __vzero() );
    __vector4 v_dp7( __vzero() );
    
    S32 elements( input_elements );
    while ( elements > 0 )
    {
        __dcbt( 0x80, pA );
        __dcbt( 0x80, pB );
        
        const __vector4 v_mask0( __vcmpgtsw( vi_elements0, __vzero() ) );
        const __vector4 v_mask1( __vcmpgtsw( vi_elements1, __vzero() ) );
        const __vector4 v_mask2( __vcmpgtsw( vi_elements2, __vzero() ) );
        const __vector4 v_mask3( __vcmpgtsw( vi_elements3, __vzero() ) );
        const __vector4 v_mask4( __vcmpgtsw( vi_elements4, __vzero() ) );
        const __vector4 v_mask5( __vcmpgtsw( vi_elements5, __vzero() ) );
        const __vector4 v_mask6( __vcmpgtsw( vi_elements6, __vzero() ) );
        const __vector4 v_mask7( __vcmpgtsw( vi_elements7, __vzero() ) );
        
        const __vector4 v_a0( __vand( __lvx( pA, 0x00 ), v_mask0 ) );
        const __vector4 v_b0( __vand( __lvx( pB, 0x00 ), v_mask0 ) );
        const __vector4 v_a1( __vand( __lvx( pA, 0x10 ), v_mask1 ) );
        const __vector4 v_b1( __vand( __lvx( pB, 0x10 ), v_mask1 ) );
        const __vector4 v_a2( __vand( __lvx( pA, 0x20 ), v_mask2 ) );
        const __vector4 v_b2( __vand( __lvx( pB, 0x20 ), v_mask2 ) );
        const __vector4 v_a3( __vand( __lvx( pA, 0x30 ), v_mask3 ) );
        const __vector4 v_b3( __vand( __lvx( pB, 0x30 ), v_mask3 ) );
        const __vector4 v_a4( __vand( __lvx( pA, 0x40 ), v_mask4 ) );
        const __vector4 v_b4( __vand( __lvx( pB, 0x40 ), v_mask4 ) );
        const __vector4 v_a5( __vand( __lvx( pA, 0x50 ), v_mask5 ) );
        const __vector4 v_b5( __vand( __lvx( pB, 0x50 ), v_mask5 ) );
        const __vector4 v_a6( __vand( __lvx( pA, 0x60 ), v_mask6 ) );
        const __vector4 v_b6( __vand( __lvx( pB, 0x60 ), v_mask6 ) );
        const __vector4 v_a7( __vand( __lvx( pA, 0x70 ), v_mask7 ) );
        const __vector4 v_b7( __vand( __lvx( pB, 0x70 ), v_mask7 ) );
        
        v_dp0 = __vmaddfp( v_a0, v_b0, v_dp0 );
        v_dp1 = __vmaddfp( v_a1, v_b1, v_dp1 );
        v_dp2 = __vmaddfp( v_a2, v_b2, v_dp2 );
        v_dp3 = __vmaddfp( v_a3, v_b3, v_dp3 );
        v_dp4 = __vmaddfp( v_a4, v_b4, v_dp4 );
        v_dp5 = __vmaddfp( v_a5, v_b5, v_dp5 );
        v_dp6 = __vmaddfp( v_a6, v_b6, v_dp6 );
        v_dp7 = __vmaddfp( v_a7, v_b7, v_dp7 );
        
        vi_elements0 = __vsubsws( vi_elements0, vi_32 );
        vi_elements1 = __vsubsws( vi_elements1, vi_32 );
        vi_elements2 = __vsubsws( vi_elements2, vi_32 );
        vi_elements3 = __vsubsws( vi_elements3, vi_32 );
        vi_elements4 = __vsubsws( vi_elements4, vi_32 );
        vi_elements5 = __vsubsws( vi_elements5, vi_32 );
        vi_elements6 = __vsubsws( vi_elements6, vi_32 );
        vi_elements7 = __vsubsws( vi_elements7, vi_32 );
        
        elements -= 32;
        
        pA += 32;
        pB += 32;
    }
    
    v_dp0 = __vaddfp( v_dp0, v_dp1 );
    v_dp1 = __vaddfp( v_dp2, v_dp3 );
    v_dp2 = __vaddfp( v_dp4, v_dp5 );
    v_dp3 = __vaddfp( v_dp6, v_dp7 );
    
    v_dp0 = __vaddfp( v_dp0, v_dp1 );
    v_dp1 = __vaddfp( v_dp2, v_dp3 );
    
    v_dp0 = __vaddfp( v_dp0, v_dp1 );
    
    const __vector4 v_one( c_VectorOne );
    v_dp0 = __vmsum4fp( v_dp0, v_one );
    
    return v_dp0;
}

#else

NOALIAS const HF32 VectorDotProduct_Aligned(
    F32 const * __restrict pA,
    F32 const * __restrict pB,
    const S32& input_elements )
{
    HF32 dp; // Zero
    S32 elements( input_elements );
    for ( S32 i=0; i<elements; i++ )
    {
        const HF32 a( Load<HF32>( &pA[i] ) );
        const HF32 b( Load<HF32>( &pB[i] ) );
        dp += a * b;
    }
    return dp;
}

#endif






}; // namespace Maths
