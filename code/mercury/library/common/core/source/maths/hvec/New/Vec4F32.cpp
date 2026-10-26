#include "CorePch.h"
#include <core/maths/hvec/New/IncludeMinimal.h>
#include <core/maths/hvec/New/IncludeMinimal.inl>

namespace Maths
{
/// @addtogroup Vec4F32
//! @{

const Vec4F32 Const< Vec4F32 >::m_XorNegate[16] =
{
    MakeVec4F32(  0.0f,  0.0f,  0.0f,  0.0f ),
    MakeVec4F32(  0.0f,  0.0f,  0.0f, -0.0f ),
    MakeVec4F32(  0.0f,  0.0f, -0.0f,  0.0f ),
    MakeVec4F32(  0.0f,  0.0f, -0.0f, -0.0f ),
    MakeVec4F32(  0.0f, -0.0f,  0.0f,  0.0f ),
    MakeVec4F32(  0.0f, -0.0f,  0.0f, -0.0f ),
    MakeVec4F32(  0.0f, -0.0f, -0.0f,  0.0f ),
    MakeVec4F32(  0.0f, -0.0f, -0.0f, -0.0f ),
    MakeVec4F32( -0.0f,  0.0f,  0.0f,  0.0f ),
    MakeVec4F32( -0.0f,  0.0f,  0.0f, -0.0f ),
    MakeVec4F32( -0.0f,  0.0f, -0.0f,  0.0f ),
    MakeVec4F32( -0.0f,  0.0f, -0.0f, -0.0f ),
    MakeVec4F32( -0.0f, -0.0f,  0.0f,  0.0f ),
    MakeVec4F32( -0.0f, -0.0f,  0.0f, -0.0f ),
    MakeVec4F32( -0.0f, -0.0f, -0.0f,  0.0f ),
    MakeVec4F32( -0.0f, -0.0f, -0.0f, -0.0f )
};

const Vec4F32 Const< Vec4F32 >::m_XorNegateX( MakeVec4F32( -0.0f,  0.0f,  0.0f,  0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateY( MakeVec4F32(  0.0f, -0.0f,  0.0f,  0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateZ( MakeVec4F32(  0.0f,  0.0f, -0.0f,  0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateW( MakeVec4F32(  0.0f,  0.0f,  0.0f, -0.0f ) );

const Vec4F32 Const< Vec4F32 >::m_XorNegateXY( MakeVec4F32( -0.0f, -0.0f,  0.0f,  0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateXZ( MakeVec4F32( -0.0f,  0.0f, -0.0f,  0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateXW( MakeVec4F32( -0.0f,  0.0f,  0.0f, -0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateYZ( MakeVec4F32(  0.0f, -0.0f, -0.0f,  0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateYW( MakeVec4F32(  0.0f, -0.0f,  0.0f, -0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateZW( MakeVec4F32(  0.0f,  0.0f, -0.0f, -0.0f ) );

const Vec4F32 Const< Vec4F32 >::m_XorNegateXYZ( MakeVec4F32( -0.0f, -0.0f, -0.0f,  0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateXYW( MakeVec4F32( -0.0f, -0.0f,  0.0f, -0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateXZW( MakeVec4F32( -0.0f,  0.0f, -0.0f, -0.0f ) );
const Vec4F32 Const< Vec4F32 >::m_XorNegateYZW( MakeVec4F32(  0.0f, -0.0f, -0.0f, -0.0f ) );

//! @}
}; // namespace Maths


