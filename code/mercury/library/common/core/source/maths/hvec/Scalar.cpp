#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{

#if defined( MATHS_DEBUG_MAX )

//void HF32::DebugValidate() const
//{
//    #if defined( _VMX128_ )
//    
//    U32 CR1, CR2, CR3;
//    
//    __vcmpeqfpR( __vspltw( m_F, VX ), __vspltw( m_F, VY ), &CR1 );
//    __vcmpeqfpR( __vspltw( m_F, VX ), __vspltw( m_F, VZ ), &CR2 );
//    __vcmpeqfpR( __vspltw( m_F, VX ), __vspltw( m_F, VW ), &CR3 );
//    
//    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
//    {
//        DebugBreak();
//    }
//    
//    #elif defined( _SSE_ )
//    
//    if ( ( m_F.m128_f32[VY] != 0.0f ) ||
//         ( m_F.m128_f32[VZ] != 0.0f ) ||
//         ( m_F.m128_f32[VW] != 0.0f ) )
//    {
//        DebugBreak();
//    }
//    
//    #endif
//}

//void HVecX::DebugValidate() const
//{
//    #if defined( _VMX128_ )
//    U32 CR1, CR2, CR3;
//    __vcmpeqfpR( __vzero(), __vspltw( m_X, VY ), &CR1 );
//    __vcmpeqfpR( __vzero(), __vspltw( m_X, VZ ), &CR2 );
//    __vcmpeqfpR( __vzero(), __vspltw( m_X, VW ), &CR3 );
//    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
//    {
//        DebugBreak();
//    }
//    #elif defined( _SSE_ )
//    if ( ( m_X.m128_f32[VY] != 0.0f ) ||
//         ( m_X.m128_f32[VZ] != 0.0f ) ||
//         ( m_X.m128_f32[VW] != 0.0f ) )
//    {
//        DebugBreak();
//    }
//    #endif
//}

//void HVecY::DebugValidate() const
//{
//    #if defined( _VMX128_ )
//    U32 CR1, CR2, CR3;
//    __vcmpeqfpR( __vzero(), __vspltw( m_Y, VX ), &CR1 );
//    __vcmpeqfpR( __vzero(), __vspltw( m_Y, VZ ), &CR2 );
//    __vcmpeqfpR( __vzero(), __vspltw( m_Y, VW ), &CR3 );
//    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
//    {
//        DebugBreak();
//    }
//    #elif defined( _SSE_ )
//    if ( ( m_Y.m128_f32[VX] != 0.0f ) ||
//         ( m_Y.m128_f32[VZ] != 0.0f ) ||
//         ( m_Y.m128_f32[VW] != 0.0f ) )
//    {
//        DebugBreak();
//    }
//    #endif
//}

//void HVecZ::DebugValidate() const
//{
//    #if defined( _VMX128_ )
//    U32 CR1, CR2, CR3;
//    __vcmpeqfpR( __vzero(), __vspltw( m_Z, VX ), &CR1 );
//    __vcmpeqfpR( __vzero(), __vspltw( m_Z, VY ), &CR2 );
//    __vcmpeqfpR( __vzero(), __vspltw( m_Z, VW ), &CR3 );
//    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
//    {
//        DebugBreak();
//    }
//    #elif defined( _SSE_ )
//    if ( ( m_Z.m128_f32[VX] != 0.0f ) ||
//         ( m_Z.m128_f32[VY] != 0.0f ) ||
//         ( m_Z.m128_f32[VW] != 0.0f ) )
//    {
//        DebugBreak();
//    }
//    #endif
//}

//void HVecW::DebugValidate() const
//{
//    #if defined( _VMX128_ )
//    U32 CR1, CR2, CR3;
//    __vcmpeqfpR( __vzero(), __vspltw( m_W, VX ), &CR1 );
//    __vcmpeqfpR( __vzero(), __vspltw( m_W, VY ), &CR2 );
//    __vcmpeqfpR( __vzero(), __vspltw( m_W, VZ ), &CR3 );
//    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
//    {
//        DebugBreak();
//    }
//    #elif defined( _SSE_ )
//    if ( ( m_W.m128_f32[VX] != 0.0f ) ||
//         ( m_W.m128_f32[VY] != 0.0f ) ||
//         ( m_W.m128_f32[VZ] != 0.0f ) )
//    {
//        DebugBreak();
//    }
//    #endif
//}

#endif





//#ifdef SHOOTER_VECMAT_DEBUG_PRINT
//
//void HF32::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s HF32( %+8.3f )\n", text, operator const F32() );
//    OutputDebugString( buffer );
//}
//
//void HVecX::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s HVecX( %+8.3f )\n", text, (F32)X() );
//    OutputDebugString( buffer );
//}
//
//void HVecY::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s HVecY( %+8.3f )\n", text, (F32)Y() );
//    OutputDebugString( buffer );
//}
//
//void HVecZ::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s HVecZ( %+8.3f )\n", text, (F32)Z() );
//    OutputDebugString( buffer );
//}
//
//void HVecW::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s HVecW( %+8.3f )\n", text, (F32)W() );
//    OutputDebugString( buffer );
//}
//
//#endif


}; // namespace Maths



