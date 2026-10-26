
namespace Maths
{

const F32 Constants<HF32>::m_Sixth( 1.0f / 6.0f );
const F32 Constants<HF32>::m_Min( FLT_MIN );
const F32 Constants<HF32>::m_Max( FLT_MAX );
const F32 Constants<HF32>::m_Epsilon( FLT_EPSILON );
const F32 Constants<HF32>::m_Epsilon2(0.01f);
const F32 Constants<HF32>::m_Epsilon3(1e-12f);
const F32 Constants<HF32>::m_Pi( 3.1415926535897932384626433832795f );
const F32 Constants<HF32>::m_Sqrt2( 1.4142135623730950488016887242097f );
const F32 Constants<HF32>::m_Sqrt2div2( 0.70710678118654752440084436210485f );
const F32 Constants<HF32>::m_e( 2.71828182845904523536028747135266249775724709369995957496696762772407663035354759457138f );


const HMat33 Constants< HMat33 >::m_Zero( ZERO_MATRIX );
const HMat33 Constants< HMat33 >::m_Identity;

const HMat43 Constants< HMat43 >::m_Zero( ZERO_MATRIX );
const HMat43 Constants< HMat43 >::m_Identity;

const HMat44 Constants< HMat44 >::m_Zero( ZERO_MATRIX );
const HMat44 Constants< HMat44 >::m_Identity;

}; // namespace Maths

