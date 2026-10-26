#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{

//
// ---------------------------------------------------
// |-- HVec3 : Hardware 3 Element F32 Vector Class --|
// ---------------------------------------------------
//

//#if defined( MATHS_DEBUG_MAX )
///// Validate that the elements are all finite and that the W element is zero
//void HVec3::DebugValidate() const
//{
//    #if defined( _VMX128_ )
//    
//    U32 CR1, CR2, CR3, CR4;
//    
//    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VX ), c_VectorFltMax ), __vzero(), &CR1 );
//    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VY ), c_VectorFltMax ), __vzero(), &CR2 );
//    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VZ ), c_VectorFltMax ), __vzero(), &CR3 );
//    
//    __vcmpeqfpR( __vspltw( m_V, VW ), __vzero(), &CR4 );
//    
//    if ( !( CR1 & CR2 & CR3 & CR4 & VMX_CR6_ALL_TRUE ) )
//    {
//        DebugBreak();
//    }
//    
//    #else
//    
//    if ( !_finite( X() ) || !_finite( Y() ) || !_finite( Z() ) )
//    {
//        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
//        DbgPrint( "HVec3::DebugValidate(): Non-Finite HVec3!" );
//        #endif
//    }
//    
//    #if defined( _SIMD_ )
//    if ( HVec4( m_V ).W() != 0.0f )
//    {
//        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
//        DbgPrint( "HVec3::DebugValidate(): Invalid HVec3!" );
//        #endif
//    }
//    #endif
//    
//    #endif
//}
//#endif


/// Finite Test
//const S32 HVec3::Finite() const
//{
//    #if defined( _VMX128_ )
//    U32 CR;
//    __vcmpequwR( __vcmpbfp( __vrlimi( m_V, __vzero(), VMX_MASK_INSERT_W, 0 ), c_VectorFltMax ), __vzero(), &CR );
//    return ( CR & VMX_CR6_ALL_TRUE ) >> VMX_CR6_ALL_TRUE_BIT;
//    #else
//    return _finite( X() ) &&
//           _finite( Y() ) &&
//           _finite( Z() );
//    #endif
//}



//
// ---------------------------------------------------
// |-- HVec4 : Hardware 4 Element F32 Vector Class --|
// ---------------------------------------------------
//

//#if defined( MATHS_DEBUG_MAX )
///// Validate that elements are all finite
//void HVec4::DebugValidate() const
//{
//    #if defined( _VMX128_ )
//    
//    U32 CR1, CR2, CR3, CR4;
//    
//    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VX ), c_VectorFltMax ), __vzero(), &CR1 );
//    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VY ), c_VectorFltMax ), __vzero(), &CR2 );
//    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VZ ), c_VectorFltMax ), __vzero(), &CR3 );
//    __vcmpequwR( __vcmpbfp( __vspltw( m_V, VW ), c_VectorFltMax ), __vzero(), &CR4 );
//    
//    if ( !( CR1 & CR2 & CR3 & CR4 & VMX_CR6_ALL_TRUE ) )
//    {
//        DebugBreak();
//    }
//    
//    #else
//    
//    if ( !_finite( X() ) || !_finite( Y() ) || !_finite( Z() ) || !_finite( W() ) )
//    {
//        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
//        DbgPrint( "HVec4::DebugValidate(): Non-Finite HVec4!" );
//        #endif
//    }
//    
//    #endif
//}
//#endif


/// Finite Test
//const S32 HVec4::Finite() const
//{
//    return _finite( X() ) &&
//           _finite( Y() ) &&
//           _finite( Z() ) &&
//           _finite( W() );
//}


//
// ----------------------------------------------
// |-- C_Vertex : HVec3 with vertex semantics --|
// ----------------------------------------------
//


//
// ----------------------------------------------
// |-- C_Vector : HVec3 with vector semantics --|
// ----------------------------------------------
//





//
// ----------------------------------------------------------
// |-- C_Normal : HVec3 with unit length normal semantics --|
// ----------------------------------------------------------
//

//#if defined( _SIMD_ )
//const C_Normal C_Normal::m_UnitNormalX( c_UnitVectorX );
//const C_Normal C_Normal::m_UnitNormalY( c_UnitVectorY );
//const C_Normal C_Normal::m_UnitNormalZ( c_UnitVectorZ );
//#else
//const C_Normal C_Normal::m_UnitNormalX( 1.0f, 0.0f, 0.0f );
//const C_Normal C_Normal::m_UnitNormalY( 0.0f, 1.0f, 0.0f );
//const C_Normal C_Normal::m_UnitNormalZ( 0.0f, 0.0f, 1.0f );
//#endif





//
// Debug Print Functions
//

//#ifdef SHOOTER_VECMAT_DEBUG_PRINT
//
//void HVec2::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s HVec2( %+8.3f, %+8.3f )\n", text, (F32)X(), (F32)Y() );
//    OutputDebugString( buffer );
//}
//
//
//void HVec3::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    
//    #if defined( _SIMD_ )
//    if ( TestW() != 0.0f )
//    {
//        sprintf_s( buffer, 256, "%s HVec3( %+8.3f, %+8.3f, %+8.3f ) [ %+18.15f !!! ]\n", text, (F32)X(), (F32)Y(), (F32)Z(), (F32)TestW() );
//    }
//    else
//    {
//        sprintf_s( buffer, 256, "%s HVec3( %+8.3f, %+8.3f, %+8.3f )\n", text, (F32)X(), (F32)Y(), (F32)Z() );
//    }
//    #else
//    sprintf_s( buffer, 256, "%s HVec3( %+8.3f, %+8.3f, %+8.3f )\n", text, (F32)X(), (F32)Y(), (F32)Z() );
//    #endif
//    
//    OutputDebugString( buffer );
//}
//
//
//void HVec4::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s HVec4( %+8.3f, %+8.3f, %+8.3f, %+8.3f )\n", text, (F32)X(), (F32)Y(), (F32)Z(), (F32)W() );
//    OutputDebugString( buffer );
//}
//
//void C_Vertex::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s C_Vertex( %+11.6f, %+11.6f, %+11.6f )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z() );
//    OutputDebugString( buffer );
//}
//
//void C_Vector::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s C_Vector( %+11.6f, %+11.6f, %+11.6f )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z() );
//    OutputDebugString( buffer );
//}
//
//void C_Normal::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s C_Normal( %+11.6f, %+11.6f, %+11.6f )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z() );
//    OutputDebugString( buffer );
//}
//
//void C_Trilinear::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s C_Trilinear( %+11.6f, %+11.6f, %+11.6f )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z() );
//    OutputDebugString( buffer );
//}
//
//void C_Areal::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s C_Areal( %+11.6f, %+11.6f, %+11.6f )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z() );
//    OutputDebugString( buffer );
//}
//
//#endif




}; // namespace Maths




