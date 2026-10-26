#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{

#if defined( USER_JDENTON ) && defined( PLATFORM_PC )

static E_MathsTest g_MathsTestType = E_MathsTest_Create;

static HANDLE g_MathsTestHandle = NULL;

static const char* g_pMathsTestFileName = "z:\\devroot\\code\\mercury\\library\\core\\source\\maths\\hvec\\MathsTest.txt";

static const char* g_pMathsTestPrintFormatS32 = "S32  ( %d )\n";
static const char* g_pMathsTestScanFormatS32  = "S32  ( %d )\n";

static const char* g_pMathsTestPrintFormatU32 = "U32  ( %u )\n";
static const char* g_pMathsTestScanFormatU32  = "U32  ( %u )\n";

static const char* g_pMathsTestPrintFormatF32 = "F32  ( %+16.9f )\n";
static const char* g_pMathsTestScanFormatF32  = "F32  ( %f )\n";

static const char* g_pMathsTestPrintFormatHF32 = "HF32 ( %+16.9f )\n";
static const char* g_pMathsTestScanFormatHF32  = "HF32 ( %f )\n";

static const char* g_pMathsTestPrintFormatHVecX = "HVecX( %+16.9f )\n";
static const char* g_pMathsTestScanFormatHVecX  = "HVecX( %f )\n";

static const char* g_pMathsTestPrintFormatHVecY = "HVecY( %+16.9f )\n";
static const char* g_pMathsTestScanFormatHVecY  = "HVecY( %f )\n";

static const char* g_pMathsTestPrintFormatHVecZ = "HVecZ( %+16.9f )\n";
static const char* g_pMathsTestScanFormatHVecZ  = "HVecZ( %f )\n";

static const char* g_pMathsTestPrintFormatHVecW = "HVecW( %+16.9f )\n";
static const char* g_pMathsTestScanFormatHVecW  = "HVecW( %f )\n";

static const char* g_pMathsTestPrintFormatHVec2 = "HVec2( %+16.9f, %+16.9f )\n";
static const char* g_pMathsTestScanFormatHVec2  = "HVec2( %f, %f )\n";

static const char* g_pMathsTestPrintFormatHVec3 = "HVec3( %+16.9f, %+16.9f, %+16.9f )\n";
static const char* g_pMathsTestScanFormatHVec3  = "HVec3( %f, %f, %f )\n";

static const char* g_pMathsTestPrintFormatHVec4 = "HVec4( %+16.9f, %+16.9f, %+16.9f, %+16.9f )\n";
static const char* g_pMathsTestScanFormatHVec4  = "HVec4( %f, %f, %f, %f )\n";

static const char* g_pMathsTestPrintFormatScalarBool = "Scalar_Bool( %d )\n";
static const char* g_pMathsTestScanFormatScalarBool  = "Scalar_Bool( %d )\n";

static const char* g_pMathsTestPrintFormatVec2Bool = "Vector2_Bool( %d, %d )\n";
static const char* g_pMathsTestScanFormatVec2Bool  = "Vector2_Bool( %d, %d )\n";

static const char* g_pMathsTestPrintFormatVec3Bool = "Vector3_Bool( %d, %d, %d )\n";
static const char* g_pMathsTestScanFormatVec3Bool  = "Vector3_Bool( %d, %d, %d )\n";

static const char* g_pMathsTestPrintFormatVec4Bool = "Vector4_Bool( %d, %d, %d, %d )\n";
static const char* g_pMathsTestScanFormatVec4Bool  = "Vector4_Bool( %d, %d, %d, %d )\n";

static const char* g_pMathsTestPrintFormatHMat33 = "HMat33( %+16.9f, %+16.9f, %+16.9f )\n      ( %+16.9f, %+16.9f, %+16.9f )\n      ( %+16.9f, %+16.9f, %+16.9f )\n";
static const char* g_pMathsTestScanFormatHMat33  = "HMat33( %f, %f, %f )\n      ( %f, %f, %f )\n      ( %f, %f, %f )\n";

static const F32 g_MathsTestToleranceF32    = 0.000001f;
static const F32 g_MathsTestToleranceHF32   = 0.000001f;
static const F32 g_MathsTestToleranceHVecX  = 0.000001f;
static const F32 g_MathsTestToleranceHVecY  = 0.000001f;
static const F32 g_MathsTestToleranceHVecZ  = 0.000001f;
static const F32 g_MathsTestToleranceHVecW  = 0.000001f;
static const F32 g_MathsTestToleranceHVec2  = 0.000001f;
static const F32 g_MathsTestToleranceHVec3  = 0.000001f;
static const F32 g_MathsTestToleranceHVec4  = 0.000001f;
static const F32 g_MathsTestToleranceHMat33 = 0.000001f;

const bool Test( const S32 x )
{
    char buffer[1024];
    char buffer2[1024];
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatS32, x );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        S32 x2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatS32, &x2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const bool valid = ( x2 == x );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const U32 x )
{
    char buffer[1024];
    char buffer2[1024];
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatU32, x );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        U32 x2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatU32, &x2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const bool valid = ( x2 == x );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const F32 x )
{
    char buffer[1024];
    char buffer2[1024];
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatF32, x );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 x2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatF32, &x2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const bool valid = _finite( x ) &&
                           ( Abs( x2 - x ) <= g_MathsTestToleranceF32 );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HF32 PCREF vx )
{
    char buffer[1024];
    char buffer2[1024];
    F32 x;
    Store( vx, &x );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHF32, x );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 x2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHF32, &x2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HF32 vx2( Load<HF32>( &x2 ) );
        const bool valid = _finite( x ) &&
                           ( Abs( vx2 - vx ) <= g_MathsTestToleranceHF32 );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HVecX PCREF input )
{
    const HF32 vx( input );
    char buffer[1024];
    char buffer2[1024];
    F32 x;
    Store( vx, &x );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHVecX, x );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 x2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHVecX, &x2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HF32 vx2( Load<HF32>( &x2 ) );
        const bool valid = _finite( x ) &&
                           ( Abs( vx2 - vx ) <= g_MathsTestToleranceHVecX );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HVecY PCREF input )
{
    const HF32 vy( input );
    char buffer[1024];
    char buffer2[1024];
    F32 y;
    Store( vy, &y );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHVecY, y );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 y2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHVecY, &y2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HF32 vy2( Load<HF32>( &y2 ) );
        const bool valid = _finite( y ) &&
                           ( Abs( vy2 - vy ) <= g_MathsTestToleranceHVecY );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HVecZ PCREF input )
{
    const HF32 vz( input );
    char buffer[1024];
    char buffer2[1024];
    F32 z;
    Store( vz, &z );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHVecZ, z );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 z2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHVecZ, &z2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HF32 vz2( Load<HF32>( &z2 ) );
        const bool valid = _finite( z ) &&
                           ( Abs( vz2 - vz ) <= g_MathsTestToleranceHVecZ );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HVecW PCREF input )
{
    const HF32 vw( input );
    char buffer[1024];
    char buffer2[1024];
    F32 w;
    Store( vw, &w );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHVecW, w );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 w2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHVecW, &w2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HF32 vw2( Load<HF32>( &w2 ) );
        const bool valid = _finite( w ) &&
                           ( Abs( vw2 - vw ) <= g_MathsTestToleranceHVecW );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HVec2 PCREF v )
{
    char buffer[1024];
    char buffer2[1024];
    F32 x, y;
    Store( v.X(), &x );
    Store( v.Y(), &y );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHVec2, x, y );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 x2, y2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHVec2, &x2, &y2 );
        if ( ( fieldsAssigned != 2 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HF32  v2x( Load<HF32>( &x2 ) );
        const HF32  v2y( Load<HF32>( &y2 ) );
        const HVec2 v2( v2x, v2y );
        const bool valid = _finite( x ) &&
                           _finite( y ) &&
                           ( Distance( v, v2 ) <= g_MathsTestToleranceHVec2 );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HVec3 PCREF v )
{
    char buffer[1024];
    char buffer2[1024];
    F32 x, y, z;
    Store( v.X(), &x );
    Store( v.Y(), &y );
    Store( v.Z(), &z );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHVec3, x, y, z );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 x2, y2, z2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHVec3, &x2, &y2, &z2 );
        if ( ( fieldsAssigned != 3 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HF32  v2x( Load<HF32>( &x2 ) );
        const HF32  v2y( Load<HF32>( &y2 ) );
        const HF32  v2z( Load<HF32>( &z2 ) );
        const HVec3 v2( v2x, v2y, v2z );
        const bool valid = _finite( x ) &&
                           _finite( y ) &&
                           _finite( z ) &&
                           ( Distance( v, v2 ) <= g_MathsTestToleranceHVec3 );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HVec4 PCREF v )
{
    char buffer[1024];
    char buffer2[1024];
    F32 x, y, z, w;
    Store( v.X(), &x );
    Store( v.Y(), &y );
    Store( v.Z(), &z );
    Store( v.W(), &w );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHVec4, x, y, z, w );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 x2, y2, z2, w2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHVec4, &x2, &y2, &z2, &w2 );
        if ( ( fieldsAssigned != 4 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HF32  v2x( Load<HF32>( &x2 ) );
        const HF32  v2y( Load<HF32>( &y2 ) );
        const HF32  v2z( Load<HF32>( &z2 ) );
        const HF32  v2w( Load<HF32>( &w2 ) );
        const HVec4 v2( v2x, v2y, v2z, v2w );
        const bool valid = _finite( x ) &&
                           _finite( y ) &&
                           _finite( z ) &&
                           _finite( w ) &&
                           ( ( v - v2 ).Length() <= g_MathsTestToleranceHVec4 );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const Scalar_Bool PCREF v )
{
    char buffer[1024];
    char buffer2[1024];
    const S32 b = (bool)v;
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatScalarBool, b );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        S32 b2;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatScalarBool, &b2 );
        if ( ( fieldsAssigned != 1 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const bool valid = b == b2;
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const Vector2_Bool PCREF v )
{
    char buffer[1024];
    char buffer2[1024];
    const S32 vx = (bool)X( v );
    const S32 vy = (bool)Y( v );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatVec2Bool, vx, vy );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        S32 v2x, v2y;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatVec2Bool, &v2x, &v2y );
        if ( ( fieldsAssigned != 2 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const bool valid = vx == v2x &&
                           vy == v2y;
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const Vector3_Bool PCREF v )
{
    char buffer[1024];
    char buffer2[1024];
    const S32 vx = (bool)X( v );
    const S32 vy = (bool)Y( v );
    const S32 vz = (bool)Z( v );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatVec3Bool, vx, vy, vz );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        S32 v2x, v2y, v2z;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatVec3Bool, &v2x, &v2y, &v2z );
        if ( ( fieldsAssigned != 3 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const bool valid = vx == v2x &&
                           vy == v2y &&
                           vz == v2z;
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const Vector4_Bool PCREF v )
{
    char buffer[1024];
    char buffer2[1024];
    const S32 vx = (bool)X( v );
    const S32 vy = (bool)Y( v );
    const S32 vz = (bool)Z( v );
    const S32 vw = (bool)W( v );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatVec4Bool, vx, vy, vz, vw );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        S32 v2x, v2y, v2z, v2w;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatVec4Bool, &v2x, &v2y, &v2z, &v2w );
        if ( ( fieldsAssigned != 4 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const bool valid = vx == v2x &&
                           vy == v2y &&
                           vz == v2z &&
                           vw == v2w;
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

const bool Test( const HMat33 & m )
{
    char buffer[1024];
    char buffer2[1024];
    F32 m00, m01, m02,
        m10, m11, m12,
        m20, m21, m22;
    Store( m.AX().X(), &m00 );
    Store( m.AX().Y(), &m01 );
    Store( m.AX().Z(), &m02 );
    Store( m.AY().X(), &m10 );
    Store( m.AY().Y(), &m11 );
    Store( m.AY().Z(), &m12 );
    Store( m.AZ().X(), &m20 );
    Store( m.AZ().Y(), &m21 );
    Store( m.AZ().Z(), &m22 );
    const S32 printBytes = sprintf_s( buffer, g_pMathsTestPrintFormatHMat33,
        m00, m01, m02,
        m10, m11, m12,
        m20, m21, m22 );
    assert( printBytes >= 0 );
    U32 bytesInBuffer = (U32)strlen( buffer );
    OutputDebugString( buffer );
    if ( g_MathsTestType == E_MathsTest_Create )
    {
        DWORD bytesWritten = 0;
        if ( !WriteFile( g_MathsTestHandle, buffer, bytesInBuffer, &bytesWritten, NULL ) )
        {
            OutputDebugString( "WriteFile failed!\n" );
        }
        const bool valid = ( bytesInBuffer == bytesWritten );
        assert( valid );
        return valid;
    }
    else
    {
        DWORD bytesRead = 0;
        if ( !ReadFile( g_MathsTestHandle, buffer2, bytesInBuffer, &bytesRead, NULL ) )
        {
            OutputDebugString( "ReadFile failed!\n" );
        }
        buffer2[bytesInBuffer] = 0;
        F32 m2_00, m2_01, m2_02,
            m2_10, m2_11, m2_12,
            m2_20, m2_21, m2_22;
        const S32 fieldsAssigned = sscanf_s( buffer2, g_pMathsTestScanFormatHMat33,
            &m2_00, &m2_01, &m2_02,
            &m2_10, &m2_11, &m2_12,
            &m2_20, &m2_21, &m2_22 );
        if ( ( fieldsAssigned != 9 ) || ( bytesInBuffer != bytesRead ) )
        {
            char string[ 1024 ];
            sprintf_s( string, "fieldsAssigned == %d\n", fieldsAssigned );
            OutputDebugString( string );
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
            return false;
        }
        const HVec3  m2x( Load<HF32>( &m2_00 ), Load<HF32>( &m2_01 ), Load<HF32>( &m2_02 ) );
        const HVec3  m2y( Load<HF32>( &m2_10 ), Load<HF32>( &m2_11 ), Load<HF32>( &m2_12 ) );
        const HVec3  m2z( Load<HF32>( &m2_20 ), Load<HF32>( &m2_21 ), Load<HF32>( &m2_22 ) );
        const HMat33 m2( m2x, m2y, m2z );
        const bool valid = _finite( m2_00 ) && _finite( m2_01 ) && _finite( m2_02 ) &&
                           _finite( m2_10 ) && _finite( m2_11 ) && _finite( m2_12 ) &&
                           _finite( m2_20 ) && _finite( m2_21 ) && _finite( m2_22 ) &&
                           ( Abs( m00 - m2_00 ) <= g_MathsTestToleranceHMat33 ) && ( Abs( m01 - m2_01 ) <= g_MathsTestToleranceHMat33 ) && ( Abs( m02 - m2_02 ) <= g_MathsTestToleranceHMat33 ) &&
                           ( Abs( m10 - m2_10 ) <= g_MathsTestToleranceHMat33 ) && ( Abs( m11 - m2_11 ) <= g_MathsTestToleranceHMat33 ) && ( Abs( m12 - m2_12 ) <= g_MathsTestToleranceHMat33 ) &&
                           ( Abs( m20 - m2_20 ) <= g_MathsTestToleranceHMat33 ) && ( Abs( m21 - m2_21 ) <= g_MathsTestToleranceHMat33 ) && ( Abs( m22 - m2_22 ) <= g_MathsTestToleranceHMat33 );
        assert( valid );
        if ( !valid )
        {
            OutputDebugString( "--> " );
            OutputDebugString( buffer2 );
            __debugbreak();
        }
        return valid;
    }
}

void Test( const E_MathsTest type )
{
    char string[ 1024 ];
    sprintf_s( string, "\nMaths::Test(): Started: create = %d\n\n", type == E_MathsTest_Create );
    OutputDebugString( string );
    
    g_MathsTestType = type;
    g_MathsTestHandle = NULL;
    
    if ( type == E_MathsTest_Create )
    {
        g_MathsTestHandle = CreateFile(
            g_pMathsTestFileName,
            GENERIC_WRITE,
            FILE_SHARE_WRITE,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_POSIX_SEMANTICS,
            NULL );
        
        if ( g_MathsTestHandle == INVALID_HANDLE_VALUE )
        {
            sprintf_s( string, sizeof( string ), "Failed to create file '%s'!\n", g_pMathsTestFileName );
            OutputDebugString( string );
            return;
        }
    }
    else
    {
        g_MathsTestHandle = CreateFile(
            g_pMathsTestFileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_POSIX_SEMANTICS,
            NULL );
        
        if ( g_MathsTestHandle == INVALID_HANDLE_VALUE )
        {
            sprintf_s( string, "Failed to open file '%s'!\n", g_pMathsTestFileName );
            OutputDebugString( string );
            return;
        }
    }
    
    OutputDebugString( "\nTesting F32\n\n" );
    
    const F32 floatZero    = 0.0f;
    const F32 floatQuarter = 0.25f;
    const F32 floatHalf    = 0.5f;
    const F32 floatOne     = 1.0f;
    const F32 floatTwo     = 2.0f;
    const F32 floatThree   = 3.0f;
    const F32 floatFour    = 4.0f;
    const F32 floatFive    = 5.0f;
    const F32 floatSix     = 6.0f;
    const F32 floatSeven   = 7.0f;
    const F32 floatEight   = 8.0f;
    const F32 floatNine    = 9.0f;
    const F32 floatTen     = 10.0f;
    
    Test( -floatTwo );
    Test( -floatOne );
    Test( -floatHalf );
    Test( -floatQuarter );
    Test( floatZero );
    Test( floatQuarter );
    Test( floatHalf );
    Test( floatOne );
    Test( floatTwo );
    Test( floatThree );
    Test( floatFour );
    Test( floatFive );
    Test( floatSix );
    Test( floatSeven );
    Test( floatEight );
    Test( floatNine );
    Test( floatTen );
    
    //
    // HF32 Tests
    //
    
    OutputDebugString( "\nTesting HF32\n\n" );
    
    const HF32 scalarZero   ( floatZero );
    const HF32 scalarQuarter( floatQuarter );
    const HF32 scalarHalf   ( floatHalf );
    const HF32 scalarOne    ( floatOne );
    const HF32 scalarTwo    ( floatTwo );
    const HF32 scalarThree  ( floatThree );
    const HF32 scalarFour   ( floatFour );
    const HF32 scalarFive   ( floatFive );
    const HF32 scalarSix    ( floatSix );
    const HF32 scalarSeven  ( floatSeven );
    const HF32 scalarEight  ( floatEight );
    const HF32 scalarNine   ( floatNine );
    const HF32 scalarTen    ( floatTen );
    
    Test( -scalarTwo );
    Test( -scalarOne );
    Test( -scalarHalf );
    Test( -scalarQuarter );
    Test( scalarZero );
    Test( scalarQuarter );
    Test( scalarHalf );
    Test( scalarOne );
    Test( scalarTwo );
    Test( scalarThree );
    Test( scalarFour );
    Test( scalarFive );
    Test( scalarSix );
    Test( scalarSeven );
    Test( scalarEight );
    Test( scalarNine );
    Test( scalarTen );
    
    Test( HF32( ZERO_INIT ) );
    Test( HF32( 1.0f ) );
    Test( HF32( 2.0 ) );
    Test( HF32( (S32)3 ) );
    Test( HF32( (U32)4 ) );
    Test( HF32( &floatFive ) );
    
    Test( HF32( HVecX( floatOne ) ) );
    Test( HF32( HVecY( floatTwo ) ) );
    Test( HF32( HVecZ( floatThree ) ) );
    Test( HF32( HVecW( floatFour ) ) );
    
    Test( (F32)HF32( floatTen ) );
    
    { const HF32 temp( floatTwo ); HF32 test; test = temp; Test( test ); }
    { const S32  temp( 3 );        HF32 test; test = temp; Test( test ); }
    { const U32  temp( 4 );        HF32 test; test = temp; Test( test ); }
    
    Test( HF32( 1.0f ).F2I() );
    Test( HF32( 1.25f ).F2I() );
    Test( HF32( 1.5f ).F2I() );
    Test( HF32( 1.75f ).F2I() );
    Test( HF32( 1.99f ).F2I() );
    Test( HF32( 2.0f ).F2I() );
    
    Test( HF32( -1.0f ).Abs() );
    Test( HF32( 0.0f ).Abs() );
    Test( HF32( 1.0f ).Abs() );
    Test( Abs( HF32( -1.0f ) ) );
    Test( Abs( HF32( 0.0f ) ) );
    Test( Abs( HF32( 1.0f ) ) );
    
    Test( HF32( -1.0f ).Sign() );
    Test( HF32( 0.0f ).Sign() );
    Test( HF32( 1.0f ).Sign() );
    Test( Sign( HF32( -1.0f ) ) );
    Test( Sign( HF32( 0.0f ) ) );
    Test( Sign( HF32( 1.0f ) ) );
    
    Test( Sqrt( HF32( 1.0f ) ) );
    Test( Sqrt( HF32( 2.0f ) ) );
    Test( Sqrt( HF32( 3.0f ) ) );
    Test( Sqrt( HF32( 4.0f ) ) );
    
    Test( Rcp( HF32( 1.0f ) ) );
    Test( Rcp( HF32( 2.0f ) ) );
    Test( Rcp( HF32( 3.0f ) ) );
    Test( Rcp( HF32( 4.0f ) ) );
    
    Test( RcpSqrt( HF32( 1.0f ) ) );
    Test( RcpSqrt( HF32( 2.0f ) ) );
    Test( RcpSqrt( HF32( 3.0f ) ) );
    Test( RcpSqrt( HF32( 4.0f ) ) );
    
    Test( -HF32( -2.0f ) );
    Test( -HF32( -1.0f ) );
    Test( -HF32( 0.0f ) );
    Test( -HF32( 1.0f ) );
    Test( -HF32( 2.0f ) );
    Test( -HF32( -2.0f ) );
    
    Test( +HF32( -2.0f ) );
    Test( +HF32( -1.0f ) );
    Test( +HF32( 0.0f ) );
    Test( +HF32( 1.0f ) );
    Test( +HF32( 2.0f ) );
    Test( +HF32( -2.0f ) );
    
    Test( HF32( 1.0f ) + HF32( 1.0f ) );
    Test( HF32( 1.0f ) + HF32( 2.0f ) );
    Test( HF32( 2.0f ) + HF32( 1.0f ) );
    Test( HF32( 2.0f ) + HF32( 2.0f ) );
    
    Test( HF32( 1.0f ) - HF32( 1.0f ) );
    Test( HF32( 1.0f ) - HF32( 2.0f ) );
    Test( HF32( 2.0f ) - HF32( 1.0f ) );
    Test( HF32( 2.0f ) - HF32( 2.0f ) );
    
    Test( HF32( 1.0f ) * HF32( 1.0f ) );
    Test( HF32( 1.0f ) * HF32( 2.0f ) );
    Test( HF32( 2.0f ) * HF32( 1.0f ) );
    Test( HF32( 2.0f ) * HF32( 2.0f ) );
    
    Test( HF32(  1.0f ) / HF32(  1.0f ) );
    Test( HF32(  1.0f ) / HF32( -1.0f ) );
    Test( HF32( -1.0f ) / HF32(  1.0f ) );
    Test( HF32( -1.0f ) / HF32( -1.0f ) );
    Test( HF32(  1.0f ) / HF32(  2.0f ) );
    Test( HF32(  1.0f ) / HF32( -2.0f ) );
    Test( HF32( -1.0f ) / HF32(  2.0f ) );
    Test( HF32( -1.0f ) / HF32( -2.0f ) );
    Test( HF32(  2.0f ) / HF32(  1.0f ) );
    Test( HF32(  2.0f ) / HF32( -1.0f ) );
    Test( HF32( -2.0f ) / HF32(  1.0f ) );
    Test( HF32( -2.0f ) / HF32( -1.0f ) );
    Test( HF32(  2.0f ) / HF32(  2.0f ) );
    Test( HF32(  2.0f ) / HF32( -2.0f ) );
    Test( HF32( -2.0f ) / HF32(  2.0f ) );
    Test( HF32( -2.0f ) / HF32( -2.0f ) );
    
    { HF32 test(  1.0f ); test += HF32(  1.0f ); Test( test ); }
    { HF32 test(  1.0f ); test += HF32( -1.0f ); Test( test ); }
    { HF32 test( -1.0f ); test += HF32(  1.0f ); Test( test ); }
    { HF32 test( -1.0f ); test += HF32( -1.0f ); Test( test ); }
    { HF32 test(  1.0f ); test += HF32(  2.0f ); Test( test ); }
    { HF32 test(  1.0f ); test += HF32( -2.0f ); Test( test ); }
    { HF32 test( -1.0f ); test += HF32(  2.0f ); Test( test ); }
    { HF32 test( -1.0f ); test += HF32( -2.0f ); Test( test ); }
    { HF32 test(  2.0f ); test += HF32(  1.0f ); Test( test ); }
    { HF32 test(  2.0f ); test += HF32( -1.0f ); Test( test ); }
    { HF32 test( -2.0f ); test += HF32(  1.0f ); Test( test ); }
    { HF32 test( -2.0f ); test += HF32( -1.0f ); Test( test ); }
    { HF32 test(  2.0f ); test += HF32(  2.0f ); Test( test ); }
    { HF32 test(  2.0f ); test += HF32( -2.0f ); Test( test ); }
    { HF32 test( -2.0f ); test += HF32(  2.0f ); Test( test ); }
    { HF32 test( -2.0f ); test += HF32( -2.0f ); Test( test ); }
    
    { HF32 test(  1.0f ); test -= HF32(  1.0f ); Test( test ); }
    { HF32 test(  1.0f ); test -= HF32( -1.0f ); Test( test ); }
    { HF32 test( -1.0f ); test -= HF32(  1.0f ); Test( test ); }
    { HF32 test( -1.0f ); test -= HF32( -1.0f ); Test( test ); }
    { HF32 test(  1.0f ); test -= HF32(  2.0f ); Test( test ); }
    { HF32 test(  1.0f ); test -= HF32( -2.0f ); Test( test ); }
    { HF32 test( -1.0f ); test -= HF32(  2.0f ); Test( test ); }
    { HF32 test( -1.0f ); test -= HF32( -2.0f ); Test( test ); }
    { HF32 test(  2.0f ); test -= HF32(  1.0f ); Test( test ); }
    { HF32 test(  2.0f ); test -= HF32( -1.0f ); Test( test ); }
    { HF32 test( -2.0f ); test -= HF32(  1.0f ); Test( test ); }
    { HF32 test( -2.0f ); test -= HF32( -1.0f ); Test( test ); }
    { HF32 test(  2.0f ); test -= HF32(  2.0f ); Test( test ); }
    { HF32 test(  2.0f ); test -= HF32( -2.0f ); Test( test ); }
    { HF32 test( -2.0f ); test -= HF32(  2.0f ); Test( test ); }
    { HF32 test( -2.0f ); test -= HF32( -2.0f ); Test( test ); }
    
    { HF32 test(  1.0f ); test *= HF32(  1.0f ); Test( test ); }
    { HF32 test(  1.0f ); test *= HF32( -1.0f ); Test( test ); }
    { HF32 test( -1.0f ); test *= HF32(  1.0f ); Test( test ); }
    { HF32 test( -1.0f ); test *= HF32( -1.0f ); Test( test ); }
    { HF32 test(  1.0f ); test *= HF32(  2.0f ); Test( test ); }
    { HF32 test(  1.0f ); test *= HF32( -2.0f ); Test( test ); }
    { HF32 test( -1.0f ); test *= HF32(  2.0f ); Test( test ); }
    { HF32 test( -1.0f ); test *= HF32( -2.0f ); Test( test ); }
    { HF32 test(  2.0f ); test *= HF32(  1.0f ); Test( test ); }
    { HF32 test(  2.0f ); test *= HF32( -1.0f ); Test( test ); }
    { HF32 test( -2.0f ); test *= HF32(  1.0f ); Test( test ); }
    { HF32 test( -2.0f ); test *= HF32( -1.0f ); Test( test ); }
    { HF32 test(  2.0f ); test *= HF32(  2.0f ); Test( test ); }
    { HF32 test(  2.0f ); test *= HF32( -2.0f ); Test( test ); }
    { HF32 test( -2.0f ); test *= HF32(  2.0f ); Test( test ); }
    { HF32 test( -2.0f ); test *= HF32( -2.0f ); Test( test ); }
    
    { HF32 test(  1.0f ); test /= HF32(  1.0f ); Test( test ); }
    { HF32 test(  1.0f ); test /= HF32( -1.0f ); Test( test ); }
    { HF32 test( -1.0f ); test /= HF32(  1.0f ); Test( test ); }
    { HF32 test( -1.0f ); test /= HF32( -1.0f ); Test( test ); }
    { HF32 test(  1.0f ); test /= HF32(  2.0f ); Test( test ); }
    { HF32 test(  1.0f ); test /= HF32( -2.0f ); Test( test ); }
    { HF32 test( -1.0f ); test /= HF32(  2.0f ); Test( test ); }
    { HF32 test( -1.0f ); test /= HF32( -2.0f ); Test( test ); }
    { HF32 test(  2.0f ); test /= HF32(  1.0f ); Test( test ); }
    { HF32 test(  2.0f ); test /= HF32( -1.0f ); Test( test ); }
    { HF32 test( -2.0f ); test /= HF32(  1.0f ); Test( test ); }
    { HF32 test( -2.0f ); test /= HF32( -1.0f ); Test( test ); }
    { HF32 test(  2.0f ); test /= HF32(  2.0f ); Test( test ); }
    { HF32 test(  2.0f ); test /= HF32( -2.0f ); Test( test ); }
    { HF32 test( -2.0f ); test /= HF32(  2.0f ); Test( test ); }
    { HF32 test( -2.0f ); test /= HF32( -2.0f ); Test( test ); }
    
    Test( HF32( -1.0f ).Min( HF32( -1.0f ) ) );
    Test( HF32( -1.0f ).Min( HF32(  0.0f ) ) );
    Test( HF32( -1.0f ).Min( HF32(  1.0f ) ) );
    Test( HF32(  0.0f ).Min( HF32( -1.0f ) ) );
    Test( HF32(  0.0f ).Min( HF32(  0.0f ) ) );
    Test( HF32(  0.0f ).Min( HF32(  1.0f ) ) );
    Test( HF32(  1.0f ).Min( HF32( -1.0f ) ) );
    Test( HF32(  1.0f ).Min( HF32(  0.0f ) ) );
    Test( HF32(  1.0f ).Min( HF32(  1.0f ) ) );
    
    Test( HF32( -1.0f ).Max( HF32( -1.0f ) ) );
    Test( HF32( -1.0f ).Max( HF32(  0.0f ) ) );
    Test( HF32( -1.0f ).Max( HF32(  1.0f ) ) );
    Test( HF32(  0.0f ).Max( HF32( -1.0f ) ) );
    Test( HF32(  0.0f ).Max( HF32(  0.0f ) ) );
    Test( HF32(  0.0f ).Max( HF32(  1.0f ) ) );
    Test( HF32(  1.0f ).Max( HF32( -1.0f ) ) );
    Test( HF32(  1.0f ).Max( HF32(  0.0f ) ) );
    Test( HF32(  1.0f ).Max( HF32(  1.0f ) ) );
    
    Test( Min( HF32( -1.0f ), HF32( -1.0f ) ) );
    Test( Min( HF32( -1.0f ), HF32(  0.0f ) ) );
    Test( Min( HF32( -1.0f ), HF32(  1.0f ) ) );
    Test( Min( HF32(  0.0f ), HF32( -1.0f ) ) );
    Test( Min( HF32(  0.0f ), HF32(  0.0f ) ) );
    Test( Min( HF32(  0.0f ), HF32(  1.0f ) ) );
    Test( Min( HF32(  1.0f ), HF32( -1.0f ) ) );
    Test( Min( HF32(  1.0f ), HF32(  0.0f ) ) );
    Test( Min( HF32(  1.0f ), HF32(  1.0f ) ) );
    
    Test( Max( HF32( -1.0f ), HF32( -1.0f ) ) );
    Test( Max( HF32( -1.0f ), HF32(  0.0f ) ) );
    Test( Max( HF32( -1.0f ), HF32(  1.0f ) ) );
    Test( Max( HF32(  0.0f ), HF32( -1.0f ) ) );
    Test( Max( HF32(  0.0f ), HF32(  0.0f ) ) );
    Test( Max( HF32(  0.0f ), HF32(  1.0f ) ) );
    Test( Max( HF32(  1.0f ), HF32( -1.0f ) ) );
    Test( Max( HF32(  1.0f ), HF32(  0.0f ) ) );
    Test( Max( HF32(  1.0f ), HF32(  1.0f ) ) );
    
    Test( HF32( -1.0f ).MinEq( HF32( -1.0f ) ) );
    Test( HF32( -1.0f ).MinEq( HF32(  0.0f ) ) );
    Test( HF32( -1.0f ).MinEq( HF32(  1.0f ) ) );
    Test( HF32(  0.0f ).MinEq( HF32( -1.0f ) ) );
    Test( HF32(  0.0f ).MinEq( HF32(  0.0f ) ) );
    Test( HF32(  0.0f ).MinEq( HF32(  1.0f ) ) );
    Test( HF32(  1.0f ).MinEq( HF32( -1.0f ) ) );
    Test( HF32(  1.0f ).MinEq( HF32(  0.0f ) ) );
    Test( HF32(  1.0f ).MinEq( HF32(  1.0f ) ) );
    
    Test( HF32( -1.0f ).MaxEq( HF32( -1.0f ) ) );
    Test( HF32( -1.0f ).MaxEq( HF32(  0.0f ) ) );
    Test( HF32( -1.0f ).MaxEq( HF32(  1.0f ) ) );
    Test( HF32(  0.0f ).MaxEq( HF32( -1.0f ) ) );
    Test( HF32(  0.0f ).MaxEq( HF32(  0.0f ) ) );
    Test( HF32(  0.0f ).MaxEq( HF32(  1.0f ) ) );
    Test( HF32(  1.0f ).MaxEq( HF32( -1.0f ) ) );
    Test( HF32(  1.0f ).MaxEq( HF32(  0.0f ) ) );
    Test( HF32(  1.0f ).MaxEq( HF32(  1.0f ) ) );
    
    for ( HF32 a = -scalarTwo; a <= scalarTwo; a += scalarHalf )
    {
        Test( Clamp( a, -scalarOne, scalarOne ) );
        Test( Clamp( a, -scalarOne, scalarZero ) );
        Test( Clamp( a, scalarZero, scalarOne ) );
    }
    
    for ( HF32 a = -scalarOne; a <= scalarOne; a += scalarHalf )
    {
        for ( HF32 b = -scalarOne; b <= scalarOne; b += scalarHalf )
        {
            Test( a == b );
            Test( a != b );
            Test( a <= b );
            Test( a <  b );
            Test( a >= b );
            Test( a >  b );
        }
    }
    
    for ( HF32 a = -scalarOne; a <= scalarOne; a += scalarHalf )
    {
        for ( HF32 b = -scalarOne; b <= scalarOne; b += scalarHalf )
        {
            Test( EQ( a, b ) );
            Test( NE( a, b ) );
            Test( LE( a, b ) );
            Test( LT( a, b ) );
            Test( GE( a, b ) );
            Test( GT( a, b ) );
        }
    }
    
    for ( HF32 a = -scalarOne; a <= scalarOne; a += scalarHalf )
    {
        for ( HF32 b = -scalarOne; b <= scalarOne; b += scalarHalf )
        {
            Test( a + (F32)b );
            Test( a - (F32)b );
            Test( a * (F32)b );
            Test( a / (F32)b );
            
            Test( a + (U32)Max( b, scalarZero ) );
            Test( a - (U32)Max( b, scalarZero ) );
            Test( a * (U32)Max( b, scalarZero ) );
            Test( a / (U32)Max( b, scalarZero ) );
            
            Test( a + (S32)b );
            Test( a - (S32)b );
            Test( a * (S32)b );
            Test( a / (S32)b );
            
            Test( a + (U16)Max( b, scalarZero ) );
            Test( a - (U16)Max( b, scalarZero ) );
            Test( a * (U16)Max( b, scalarZero ) );
            Test( a / (U16)Max( b, scalarZero ) );
            
            Test( a + (S16)b );
            Test( a - (S16)b );
            Test( a * (S16)b );
            Test( a / (S16)b );
            
            Test( a + (U8)Max( b, scalarZero ) );
            Test( a - (U8)Max( b, scalarZero ) );
            Test( a * (U8)Max( b, scalarZero ) );
            Test( a / (U8)Max( b, scalarZero ) );
            
            Test( a + (S8)b );
            Test( a - (S8)b );
            Test( a * (S8)b );
            Test( a / (S8)b );
            
            { HF32 c( a ); Test( c += (F32)b ); }
            { HF32 c( a ); Test( c -= (F32)b ); }
            { HF32 c( a ); Test( c *= (F32)b ); }
            { HF32 c( a ); Test( c /= (F32)b ); }
                                 
            { HF32 c( a ); Test( c += (U32)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c -= (U32)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c *= (U32)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c /= (U32)Max( b, scalarZero ) ); }
                                 
            { HF32 c( a ); Test( c += (S32)b ); }
            { HF32 c( a ); Test( c -= (S32)b ); }
            { HF32 c( a ); Test( c *= (S32)b ); }
            { HF32 c( a ); Test( c /= (S32)b ); }
                                 
            { HF32 c( a ); Test( c += (U16)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c -= (U16)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c *= (U16)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c /= (U16)Max( b, scalarZero ) ); }
                                 
            { HF32 c( a ); Test( c += (S16)b ); }
            { HF32 c( a ); Test( c -= (S16)b ); }
            { HF32 c( a ); Test( c *= (S16)b ); }
            { HF32 c( a ); Test( c /= (S16)b ); }
                                 
            { HF32 c( a ); Test( c += (U8)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c -= (U8)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c *= (U8)Max( b, scalarZero ) ); }
            { HF32 c( a ); Test( c /= (U8)Max( b, scalarZero ) ); }
                                 
            { HF32 c( a ); Test( c += (S8)b ); }
            { HF32 c( a ); Test( c -= (S8)b ); }
            { HF32 c( a ); Test( c *= (S8)b ); }
            { HF32 c( a ); Test( c /= (S8)b ); }
        }
    }
    
    for ( HF32 a = -scalarOne; a <= scalarOne; a += scalarHalf )
    {
        for ( HF32 b = -scalarOne; b <= scalarOne; b += scalarHalf )
        {
            for ( HF32 c = -scalarOne; c <= scalarOne; c += scalarHalf )
            {
                Test( MulAdd( a, b, c ) );
                Test( NegMulSub( a, b, c ) );
            }
        }
    }
    
    Test( Abs( -scalarTwo ) );
    Test( Abs( -scalarHalf ) );
    Test( Abs( -scalarOne ) );
    Test( Abs( -scalarZero ) );
    Test( Abs(  scalarZero ) );
    Test( Abs(  scalarHalf ) );
    Test( Abs(  scalarOne ) );
    Test( Abs(  scalarTwo ) );
    
    Test( Sqrt( -scalarTwo ) );
    Test( Sqrt( -scalarOne ) );
    Test( Sqrt( -scalarHalf ) );
    Test( Sqrt( -scalarZero ) );
    Test( Sqrt(  scalarZero ) );
    Test( Sqrt(  scalarHalf ) );
    Test( Sqrt(  scalarOne ) );
    Test( Sqrt(  scalarTwo ) );
    
    Test( Rcp( -scalarTwo ) );
    Test( Rcp( -scalarOne ) );
    Test( Rcp( -scalarHalf ) );
    Test( Rcp( -scalarZero ) );
    Test( Rcp(  scalarZero ) );
    Test( Rcp(  scalarHalf ) );
    Test( Rcp(  scalarOne ) );
    Test( Rcp(  scalarTwo ) );
    
    Test( RcpSqrt( -scalarTwo ) );
    Test( RcpSqrt( -scalarOne ) );
    Test( RcpSqrt( -scalarHalf ) );
    Test( RcpSqrt( -scalarZero ) );
    Test( RcpSqrt(  scalarZero ) );
    Test( RcpSqrt(  scalarHalf ) );
    Test( RcpSqrt(  scalarOne ) );
    Test( RcpSqrt(  scalarTwo ) );
    
    Test( IsZero( -scalarTwo ) );
    Test( IsZero( -scalarOne ) );
    Test( IsZero( -scalarHalf ) );
    Test( IsZero( -scalarZero ) );
    Test( IsZero(  scalarZero ) );
    Test( IsZero(  scalarHalf ) );
    Test( IsZero(  scalarOne ) );
    Test( IsZero(  scalarTwo ) );
    
    Test( IsNonZero( -scalarTwo ) );
    Test( IsNonZero( -scalarOne ) );
    Test( IsNonZero( -scalarHalf ) );
    Test( IsNonZero( -scalarZero ) );
    Test( IsNonZero(  scalarZero ) );
    Test( IsNonZero(  scalarHalf ) );
    Test( IsNonZero(  scalarOne ) );
    Test( IsNonZero(  scalarTwo ) );
    
    Test( Finite( -scalarOne ) );
    Test( Finite( -scalarZero ) );
    Test( Finite(  scalarZero ) );
    Test( Finite(  scalarOne ) );
    Test( Finite( Rcp( -scalarOne  ) ) );
    Test( Finite( Rcp( -scalarZero ) ) );
    Test( Finite( Rcp(  scalarZero ) ) );
    Test( Finite( Rcp(  scalarOne  ) ) );
    Test( Finite( RcpSqrt( -scalarOne  ) ) );
    Test( Finite( RcpSqrt( -scalarZero ) ) );
    Test( Finite( RcpSqrt(  scalarZero ) ) );
    Test( Finite( RcpSqrt(  scalarOne  ) ) );
    
    for ( HF32 a = -scalarOne; a <= scalarOne; a += scalarHalf )
    {
        for ( HF32 b = -scalarOne; b <= scalarOne; b += scalarHalf )
        {
            for ( HF32 c = -scalarOne; c <= scalarOne; c += scalarHalf )
            {
                for ( HF32 d = -scalarOne; d <= scalarOne; d += scalarHalf )
                {
                    Test( Select_EQ( a, b, c, d ) );
                    Test( Select_NE( a, b, c, d ) );
                    Test( Select_GE( a, b, c, d ) );
                    Test( Select_LT( a, b, c, d ) );
                    Test( Select_LE( a, b, c, d ) );
                    Test( Select_GT( a, b, c, d ) );
                }
            }
        }
    }
    
    for ( HF32 a = -scalarOne; a <= scalarOne; a += scalarHalf )
    {
        for ( HF32 b = -scalarOne; b <= scalarOne; b += scalarHalf )
        {
            Test( CopySign( a, b ) );
        }
    }
    
    for ( HF32 a = -scalarFour; a <= scalarFour; a += scalarQuarter )
    {
        //sprintf( string, "a = %+16.9f\n", (F32)a );
        //OutputDebugString( string );
        
        Test( RoundToIntMinusInf( a ) );
        Test( RoundToIntPlusInf( a ) );
        Test( RoundToIntNearest( a ) );
        Test( RoundToIntZero( a ) );
        
        Test( Floor( a ) );
        Test( Ceil ( a ) );
        Test( Round( a ) );
        Test( Trunc( a ) );
    }
    
    { F32 storage = 0.0f; HF32 test( floatOne ); Store( test, &storage ); Test( storage ); }
    { F32 storage = 0.0f; HF32 test( floatOne ); Test( Load<HF32>( &storage ) ); }
    
    //{ Storage_HF32 storage; HF32 test( floatOne ); Store( test, &storage ); Test( storage.m_Data ); }
    //{ Storage_HF32 storage( floatOne ); HF32 test( UNDEFINED ); test = Load( &storage ); Test( test ); }
    
    //{ HF32 test( floatOne ); Storage_HF32 storage = test; Test( storage.m_Data ); }
    //{ Storage_HF32 storage( floatOne ); HF32 test = Load( &storage ); Test( test ); }
    
    //
    // HVecX, HVecY, HVecZ & HVecW Tests
    //
    
    OutputDebugString( "\nTesting HVecX, HVecY, HVecZ & HVecW\n\n" );
    {
        const HVecX x0( ZERO_INIT ); Test( x0 );
        const HVecY y0( ZERO_INIT ); Test( y0 );
        const HVecZ z0( ZERO_INIT ); Test( z0 );
        const HVecW w0( ZERO_INIT ); Test( w0 );
        
        const HVecX x1( scalarOne );   Test( x1 );
        const HVecY y2( scalarTwo );   Test( y2 );
        const HVecZ z3( scalarThree ); Test( z3 );
        const HVecW w4( scalarFour );  Test( w4 );
        
        { const HVecX x( x1 ); Test( x ); }
        { const HVecY y( y2 ); Test( y ); }
        { const HVecZ z( z3 ); Test( z ); }
        { const HVecW w( w4 ); Test( w ); }
        
        Test( HVecX( x0 ) );
        Test( HVecY( y0 ) );
        Test( HVecZ( z0 ) );
        Test( HVecW( w0 ) );
        
        Test( HVecX( x1 ) );
        Test( HVecY( y2 ) );
        Test( HVecZ( z3 ) );
        Test( HVecW( w4 ) );
        
        Test( HVecX( y2 ) );
        Test( HVecY( z3 ) );
        Test( HVecZ( w4 ) );
        Test( HVecW( x1 ) );
        
        Test( HVecX( z3 ) );
        Test( HVecY( w4 ) );
        Test( HVecZ( x1 ) );
        Test( HVecW( y2 ) );
        
        Test( HVecX( w4 ) );
        Test( HVecY( x1 ) );
        Test( HVecZ( y2 ) );
        Test( HVecW( z3 ) );
        
        const HVec2 v2( 1.0f, 2.0f );
        const HVec3 v3( 1.0f, 2.0f, 3.0f );
        const HVec4 v4( 1.0f, 2.0f, 3.0f, 4.0f );
        
        Test( HVecX( v2 ) );
        Test( HVecY( v2 ) );
        
        Test( HVecX( v3 ) );
        Test( HVecY( v3 ) );
        Test( HVecZ( v3 ) );
        
        Test( HVecX( v4 ) );
        Test( HVecY( v4 ) );
        Test( HVecZ( v4 ) );
        Test( HVecW( v4 ) );
        
        { HVecX x( ZERO_INIT ); x = x1; Test( x ); }
        { HVecY y( ZERO_INIT ); y = y2; Test( y ); }
        { HVecZ z( ZERO_INIT ); z = z3; Test( z ); }
        { HVecW w( ZERO_INIT ); w = w4; Test( w ); }
        
        { HVecX x( ZERO_INIT ); x = scalarOne;   Test( x ); }
        { HVecY y( ZERO_INIT ); y = scalarTwo;   Test( y ); }
        { HVecZ z( ZERO_INIT ); z = scalarThree; Test( z ); }
        { HVecW w( ZERO_INIT ); w = scalarFour;  Test( w ); }
        
        { HVecX x( ZERO_INIT ); x.X( scalarOne );   Test( x ); }
        { HVecY y( ZERO_INIT ); y.Y( scalarTwo );   Test( y ); }
        { HVecZ z( ZERO_INIT ); z.Z( scalarThree ); Test( z ); }
        { HVecW w( ZERO_INIT ); w.W( scalarFour );  Test( w ); }
        
        { HVecX x( scalarOne );   Test( x.X() ); }
        { HVecY y( scalarTwo );   Test( y.Y() ); }
        { HVecZ z( scalarThree ); Test( z.Z() ); }
        { HVecW w( scalarFour );  Test( w.W() ); }
    }
    
    //
    // HVec2 Tests
    //
    
    OutputDebugString( "\nTesting HVec2\n\n" );
    
    //
    // HVec3 Tests
    //
    
    OutputDebugString( "\nTesting HVec3\n\n" );
    
    #if 0 // TEMPORARY
    
    { HVec3 test( ZERO_INIT ); Test( test ); }
    
    { HVecX x( 1.0f ); HVecY y( 2.0f ); HVecZ z( 3.0f ); HVec3 test( x, y, z ); Test( test ); }
    
    { HVecX xyz( 1.0f ); HVec3 test( xyz ); Test( test ); }
    { HVecY xyz( 2.0f ); HVec3 test( xyz ); Test( test ); }
    { HVecZ xyz( 3.0f ); HVec3 test( xyz ); Test( test ); }
    
    { HVec2 xy( 1.0f, 2.0f ); HVec3 test( xy ); Test( test ); }
    
    { HVec4 xyzw( 1.0f, 2.0f, 3.0f, 4.0f ); HVec3 test( xyzw ); Test( test ); }
    
    { C_Vertex xyz( 1.0f, 2.0f, 3.0f ); HVec3 test( xyz ); Test( test ); }
    { C_Vector xyz( 1.0f, 2.0f, 3.0f ); HVec3 test( xyz ); Test( test ); }
    
    { C_Normal xyz( 1.0f, 0.0f, 0.0f ); HVec3 test( xyz ); Test( test ); }
    { C_Normal xyz( 0.0f, 1.0f, 0.0f ); HVec3 test( xyz ); Test( test ); }
    { C_Normal xyz( 0.0f, 0.0f, 1.0f ); HVec3 test( xyz ); Test( test ); }
    
    { HF32 xyz( 1.0f ); HVec3 test( xyz ); Test( test ); }
    
    { HF32 x( 1.0f ); HF32 y( 2.0f ); HF32 z( 3.0f ); HVec3 test( x, y, z ); Test( test ); }
    
    { F32 xyz[] = { 1.0f, 2.0f, 3.0f }; HVec3 test( xyz ); Test( test ); }
    
    { MathsStorage::Float3 xyz; xyz.x = 1.0f; xyz.y = 2.0f; xyz.z = 3.0f; HVec3 test( &xyz ); Test( test ); }
    
    { HVec3 vec( 1.0f, 2.0f, 3.0f ); MathsStorage::Float3 test = (MathsStorage::Float3)vec; Test( test.x ); Test( test.y ); Test( test.z ); }
    
    { U16 xyz[] = { 1, 2, 3 }; HVec3 test( xyz ); Test( test ); }
    
    { MathsStorage::Fixed3_S16<0> xyz; xyz.x =  1; xyz.y =  2; xyz.z =  3; HVec3 test( &xyz ); Test( test ); }
    { MathsStorage::Fixed3_S16<0> xyz; xyz.x = -1; xyz.y = -2; xyz.z = -3; HVec3 test( &xyz ); Test( test ); }
    { MathsStorage::Fixed3_S16<8> xyz; xyz.x =  1; xyz.y =  2; xyz.z =  3; HVec3 test( &xyz ); Test( test ); }
    { MathsStorage::Fixed3_S16<8> xyz; xyz.x = -1; xyz.y = -2; xyz.z = -3; HVec3 test( &xyz ); Test( test ); }
    
    { HVec3 vec(  1.0f,  2.0f,  3.0f ); MathsStorage::Fixed3_S16<8> xyz = vec; Test( xyz.x ); Test( xyz.y ); Test( xyz.z ); }
    { HVec3 vec( -1.0f, -2.0f, -3.0f ); MathsStorage::Fixed3_S16<8> xyz = vec; Test( xyz.x ); Test( xyz.y ); Test( xyz.z ); }
    
    { HVec3 a( 1.0f, 2.0f, 3.0f ); HVec3 b( 1.0f, 1.0f, 1.0f ); HVec3 test( a == b ); Test( TypePun_FloatToInt( test.X() ) ); Test( TypePun_FloatToInt( test.Y() ) ); Test( TypePun_FloatToInt( test.Z() ) ); }
    { HVec3 a( 1.0f, 2.0f, 3.0f ); HVec3 b( 2.0f, 2.0f, 2.0f ); HVec3 test( a == b ); Test( TypePun_FloatToInt( test.X() ) ); Test( TypePun_FloatToInt( test.Y() ) ); Test( TypePun_FloatToInt( test.Z() ) ); }
    { HVec3 a( 1.0f, 2.0f, 3.0f ); HVec3 b( 3.0f, 3.0f, 3.0f ); HVec3 test( a == b ); Test( TypePun_FloatToInt( test.X() ) ); Test( TypePun_FloatToInt( test.Y() ) ); Test( TypePun_FloatToInt( test.Z() ) ); }
    
    { HVec3 a( 1.0f, 2.0f, 3.0f ); HVec3 b( 1.0f, 1.0f, 1.0f ); HVec3 test( a > b, a, b ); Test( test ); }
    { HVec3 a( 1.0f, 2.0f, 3.0f ); HVec3 b( 2.0f, 2.0f, 2.0f ); HVec3 test( a > b, a, b ); Test( test ); }
    { HVec3 a( 1.0f, 2.0f, 3.0f ); HVec3 b( 3.0f, 3.0f, 3.0f ); HVec3 test( a > b, a, b ); Test( test ); }
    
    { HVec3_Multiply test( HVec3( 1.0f, 2.0f, 3.0f ), HVec3( 3.0f, 2.0f, 1.0f ) ); Test( test ); }
    { HVec3_Add test( HVec3( 1.0f, 2.0f, 3.0f ), HVec3( 3.0f, 2.0f, 1.0f ) ); Test( test ); }
    { HVec3_Subtract test( HVec3( 1.0f, 2.0f, 3.0f ), HVec3( 3.0f, 2.0f, 1.0f ) ); Test( test ); }
    
    { HVec3 vec( 1.0f, 2.0f, 3.0f ); Test( (HVecX)vec ); }
    { HVec3 vec( 1.0f, 2.0f, 3.0f ); Test( (HVecY)vec ); }
    { HVec3 vec( 1.0f, 2.0f, 3.0f ); Test( (HVecZ)vec ); }
    
    { HVec3 test( 1.0f, 2.0f, 3.0f ); HVec3 vec( 4.0f, 5.0f, 6.0f ); test = vec; Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); HVecX x( 4.0f ); test = x; Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); HVecY y( 5.0f ); test = y; Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); HVecZ z( 6.0f ); test = z; Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); HVec2 xy( 4.0f, 5.0f ); test = xy; Test( test ); }
    
    { HVec3 test( 1.0f, 2.0f, 3.0f ); C_Vector xyz( 4.0f, 5.0f, 6.0f ); test = xyz; Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); C_Vertex xyz( 4.0f, 5.0f, 6.0f ); test = xyz; Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); C_Normal xyz( 1.0f, 0.0f, 0.0f ); test = xyz; Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); C_Normal xyz( 0.0f, 1.0f, 0.0f ); test = xyz; Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); C_Normal xyz( 0.0f, 0.0f, 1.0f ); test = xyz; Test( test ); }
    
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.X() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.Y() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.Z() ); }
    
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.X( 4.0f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.Y( 5.0f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.Z( 6.0f ); Test( test ); }
    
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroX(); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroY(); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroZ(); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroXY(); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroXZ(); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroYZ(); Test( test ); }
    
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroIfOutOfBounds( 0.0f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroIfOutOfBounds( 0.5f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroIfOutOfBounds( 1.0f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroIfOutOfBounds( 1.5f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroIfOutOfBounds( 2.0f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroIfOutOfBounds( 2.5f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroIfOutOfBounds( 3.0f ); Test( test ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); test.ZeroIfOutOfBounds( 3.5f ); Test( test ); }
    
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.ZeroedX() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.ZeroedY() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.ZeroedZ() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.ZeroedXY() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.ZeroedXZ() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.ZeroedYZ() ); }
    
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.Shuffle<VX,VY,VZ>() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.Shuffle<VX,VZ,VY>() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.Shuffle<VY,VX,VZ>() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.Shuffle<VY,VZ,VX>() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.Shuffle<VZ,VX,VY>() ); }
    { HVec3 test( 1.0f, 2.0f, 3.0f ); Test( test.Shuffle<VZ,VY,VX>() ); }
    
    for ( F32 az = -1.0f; az <= 1.0f; az += 1.0f )
    {
        for ( F32 ay = -1.0f; ay <= 1.0f; ay += 1.0f )
        {
            for ( F32 ax = -1.0f; ax <= 1.0f; ax += 1.0f )
            {
                HVec3 a( ax, ay, az );
                for ( F32 bz = -1.0f; bz <= 1.0f; bz += 1.0f )
                {
                    for ( F32 by = -1.0f; by <= 1.0f; by += 1.0f )
                    {
                        for ( F32 bx = -1.0f; bx <= 1.0f; bx += 1.0f )
                        {
                            HVec3 b( bx, by, bz );
                            Test( a.Min( b ) );
                            Test( a.Max( b ) );
                        }
                    }
                }
            }
        }
    }
    
    for ( F32 az = -1.0f; az <= 1.0f; az += 1.0f )
    {
        for ( F32 ay = -1.0f; ay <= 1.0f; ay += 1.0f )
        {
            for ( F32 ax = -1.0f; ax <= 1.0f; ax += 1.0f )
            {
                for ( F32 bz = -1.0f; bz <= 1.0f; bz += 1.0f )
                {
                    for ( F32 by = -1.0f; by <= 1.0f; by += 1.0f )
                    {
                        for ( F32 bx = -1.0f; bx <= 1.0f; bx += 1.0f )
                        {
                            {
                                HVec3 a( ax, ay, az );
                                HVec3 b( bx, by, bz );
                                a.MinEq( b );
                                Test( a );
                            }
                            {
                                HVec3 a( ax, ay, az );
                                HVec3 b( bx, by, bz );
                                a.MaxEq( b );
                                Test( a );
                            }
                        }
                    }
                }
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v( x, y, z );
                Test( v.Reciprocal() );
                Test( v.Sum() );
                Test( v.Length() );
                Test( v.Length2() );
                Test( v.LengthSquared() );
                Test( v.RcpLength() );
                Test( v.ReciprocalLength() );
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v( x, y, z );
                Test( v.Star() );
            }
        }
    }
    
    for ( F32 qw = -1.0f; qw <= 1.0f; qw += 1.0f )
    {
        for ( F32 qz = -1.0f; qz <= 1.0f; qz += 1.0f )
        {
            for ( F32 qy = -1.0f; qy <= 1.0f; qy += 1.0f )
            {
                for ( F32 qx = -1.0f; qx <= 1.0f; qx += 1.0f )
                {
                    C_Quaternion q( HVec3( qx, qy, qz ), HF32( qw ) * Constants<HF32>::Pi() * Constants<HF32>::Half() );
                    for ( F32 tz = -1.0f; tz <= 1.0f; tz += 1.0f )
                    {
                        for ( F32 ty = -1.0f; ty <= 1.0f; ty += 1.0f )
                        {
                            for ( F32 tx = -1.0f; tx <= 1.0f; tx += 1.0f )
                            {
                                HVec3 t( tx, ty, tz );
                                for ( F32 vz = -1.0f; vz <= 1.0f; vz += 1.0f )
                                {
                                    for ( F32 vy = -1.0f; vy <= 1.0f; vy += 1.0f )
                                    {
                                        for ( F32 vx = -1.0f; vx <= 1.0f; vx += 1.0f )
                                        {
                                            HVec3 v( vx, vy, vz );
                                            Test( v.Transformed( q, t ) );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    for ( F32 az = -1.0f; az <= 1.0f; az += 1.0f )
    {
        for ( F32 ay = -1.0f; ay <= 1.0f; ay += 1.0f )
        {
            for ( F32 ax = -1.0f; ax <= 1.0f; ax += 1.0f )
            {
                HVec3 a( ax, ay, az );
                for ( F32 bz = -1.0f; bz <= 1.0f; bz += 1.0f )
                {
                    for ( F32 by = -1.0f; by <= 1.0f; by += 1.0f )
                    {
                        for ( F32 bx = -1.0f; bx <= 1.0f; bx += 1.0f )
                        {
                            HVec3 b( bx, by, bz );
                            Test( a.Distance( b ) );
                        }
                    }
                }
            }
        }
    }
    
    for ( F32 az = -1.0f; az <= 1.0f; az += 1.0f )
    {
        for ( F32 ay = -1.0f; ay <= 1.0f; ay += 1.0f )
        {
            for ( F32 ax = -1.0f; ax <= 1.0f; ax += 1.0f )
            {
                HVec3 a( ax, ay, az );
                for ( F32 bz = -1.0f; bz <= 1.0f; bz += 1.0f )
                {
                    for ( F32 by = -1.0f; by <= 1.0f; by += 1.0f )
                    {
                        for ( F32 bx = -1.0f; bx <= 1.0f; bx += 1.0f )
                        {
                            HVec3 b( bx, by, bz );
                            Test( a.DistanceSquared( b ) );
                        }
                    }
                }
            }
        }
    }
    
    for ( F32 az = -1.0f; az <= 1.0f; az += 1.0f )
    {
        for ( F32 ay = -1.0f; ay <= 1.0f; ay += 1.0f )
        {
            for ( F32 ax = -1.0f; ax <= 1.0f; ax += 1.0f )
            {
                HVec3 a( ax, ay, az );
                for ( F32 bz = -1.0f; bz <= 1.0f; bz += 1.0f )
                {
                    for ( F32 by = -1.0f; by <= 1.0f; by += 1.0f )
                    {
                        for ( F32 bx = -1.0f; bx <= 1.0f; bx += 1.0f )
                        {
                            HVec3 b( bx, by, bz );
                            for ( F32 f = 0.0f; f <= 1.0f; f += 0.5f )
                            {
                                Test( a.Lerp( b, f ) );
                            }
                        }
                    }
                }
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v( x, y, z );
                C_Normalised n( v.Normalised() );
                Test( n.Normal() );
                Test( n.Length() );
                Test( n.Length2() );
                Test( n.RcpLength() );
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v( x, y, z );
                v.Normalise();
                Test( v );
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                for ( F32 f = 0.0f; f <= 1.0f; f += 0.5f )
                {
                    HVec3 v( x, y, z );
                    v.LimitLength( f );
                    Test( v );
                }
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v( x, y, z );
                C_Normalised n( v.SafeNormalised( v ) );
                Test( n.Normal() );
                Test( n.Length() );
                Test( n.Length2() );
                Test( n.RcpLength() );
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v( x, y, z );
                v.SafeNormalise( v );
                Test( v );
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v1( x, y, z );
                for ( F32 f = 0.0f; f <= 1.0f; f += 0.5f )
                {
                    HVec3 v2 = v1.VectorOfMagnitude( f );
                    Test( v2 );
                }
            }
        }
    }
    
    for ( F32 vz = -1.0f; vz <= 1.0f; vz += 1.0f )
    {
        for ( F32 vy = -1.0f; vy <= 1.0f; vy += 1.0f )
        {
            for ( F32 vx = -1.0f; vx <= 1.0f; vx += 1.0f )
            {
                HVec3 v( vx, vy, vz );
                for ( F32 nz = -1.0f; nz <= 1.0f; nz += 0.5f )
                {
                    for ( F32 ny = -1.0f; ny <= 1.0f; ny += 0.5f )
                    {
                        for ( F32 nx = -1.0f; nx <= 1.0f; nx += 0.5f )
                        {
                            C_Normal n( nx, ny, nz );
                            Test( v.Parallel( n ) );
                            Test( v.Perpendicular( n ) );
                            HVec3 para, perp;
                            v.ParaPerp( n, para, perp );
                            Test( para );
                            Test( perp );
                            HVec3 u, v;
                            v.PerpendicularAxes( u, v );
                            Test( u );
                            Test( v );
                        }
                    }
                }
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v( x, y, z );
                Test( v.ShuffleXZtoXY() );
            }
        }
    }
    
    for ( F32 z = -1.0f; z <= 1.0f; z += 0.5f )
    {
        for ( F32 y = -1.0f; y <= 1.0f; y += 0.5f )
        {
            for ( F32 x = -1.0f; x <= 1.0f; x += 0.5f )
            {
                HVec3 v( x, y, z );
                Test( v.RoundToIntMinusInf() );
                Test( v.RoundToIntPlusInf() );
                Test( v.RoundToIntNearest() );
                Test( v.RoundToIntZero() );
            }
        }
    }
    
    for ( F32 az = -1.0f; az <= 1.0f; az += 1.0f )
    {
        for ( F32 ay = -1.0f; ay <= 1.0f; ay += 1.0f )
        {
            for ( F32 ax = -1.0f; ax <= 1.0f; ax += 1.0f )
            {
                HVec3 a( ax, ay, az );
                for ( F32 bz = -1.0f; bz <= 1.0f; bz += 1.0f )
                {
                    for ( F32 by = -1.0f; by <= 1.0f; by += 1.0f )
                    {
                        for ( F32 bx = -1.0f; bx <= 1.0f; bx += 1.0f )
                        {
                            HVec3 b( bx, by, bz );
                            Test( a == b );
                            Test( a != b );
                            Test( a >= b );
                            Test( a >  b );
                            Test( a <= b );
                            Test( a <  b );
                            Test( a.All_EQ( b ) );
                            Test( a.All_NE( b ) );
                            Test( a.All_LE( b ) );
                            Test( a.All_LT( b ) );
                            Test( a.All_GE( b ) );
                            Test( a.All_GT( b ) );
                            Test( a.Any_EQ( b ) );
                            Test( a.Any_NE( b ) );
                            Test( a.Any_LE( b ) );
                            Test( a.Any_LT( b ) );
                            Test( a.Any_GE( b ) );
                            Test( a.Any_GT( b ) );
                        }
                    }
                }
            }
        }
    }
    
    #endif // TEMPORARY
    
    //
    // HVec4 Tests
    //
    
    OutputDebugString( "\nTesting HVec4\n\n" );
    
    #if 0 // TEMPORARY
    
    { HVec4 a( ZERO_INIT ); Test( a ); }
    { HVecX x( 1.0f ); HVec4 v( x ); Test( v ); }
    { HVecY y( 2.0f ); HVec4 v( y ); Test( v ); }
    { HVecZ z( 3.0f ); HVec4 v( z ); Test( v ); }
    { HVecW w( 4.0f ); HVec4 v( w ); Test( v ); }
    { HVec2 xy( 1.0f, 2.0f ); HVec4 v( xy ); Test( v ); }
    { HVec3 xyz( 1.0f, 2.0f, 3.0f ); HVec4 v( xyz ); Test( v ); }
    { HVec4 xyzw( 1.0f, 2.0f, 3.0f, 4.0f ); HVec4 v( xyzw ); Test( v ); }
    { HF32 x( 1.0f ), y( 2.0f ), z( 3.0f ), w( 4.0f ); HVec4 v( x, y, z, w ); Test( v ); }
    { HF32 xyzw( 1.0f ); HVec4 v( xyzw ); Test( v ); }
    { F32 xyzw[] = { 1.0f, 2.0f, 3.0f, 4.0f }; HVec4 v( xyzw ); Test( v ); }
    
    {
        HVec4 v( 1.0f, 2.0f, 3.0f, 4.0f );
        HVecX x( v ); Test( x );
        HVecY y( v ); Test( y );
        HVecZ z( v ); Test( z );
        HVecW w( v ); Test( w );
    }
    
    { HVecX x( 1.0f ); HVec4 v( ZERO_INIT ); v = x; Test( v ); }
    { HVecY y( 2.0f ); HVec4 v( ZERO_INIT ); v = y; Test( v ); }
    { HVecZ z( 3.0f ); HVec4 v( ZERO_INIT ); v = z; Test( v ); }
    { HVecW w( 4.0f ); HVec4 v( ZERO_INIT ); v = w; Test( v ); }
    { HVec2 xy( 1.0f, 2.0f ); HVec4 v( ZERO_INIT ); v = xy; Test( v ); }
    { HVec3 xyz( 1.0f, 2.0f, 3.0f ); HVec4 v( ZERO_INIT ); v = xyz; Test( v ); }
    { HVec4 xyzw( 1.0f, 2.0f, 3.0f, 4.0f ); HVec4 v( ZERO_INIT ); v = xyzw; Test( v ); }
    
    {
        HVec4 v( 1.0f, 2.0f, 3.0f, 4.0f );
        Test( v.X() );
        Test( v.Y() );
        Test( v.Z() );
        Test( v.W() );
        Test( v.XYZ() );
        Test( v.YZW() );
    }
    
    { HVec4 v( 1.0f, 2.0f, 3.0f, 4.0f ); HF32 x( 5.0f ); v.X( x ); Test( v ); }
    { HVec4 v( 1.0f, 2.0f, 3.0f, 4.0f ); HF32 y( 5.0f ); v.Y( y ); Test( v ); }
    { HVec4 v( 1.0f, 2.0f, 3.0f, 4.0f ); HF32 z( 5.0f ); v.Z( z ); Test( v ); }
    { HVec4 v( 1.0f, 2.0f, 3.0f, 4.0f ); HF32 w( 5.0f ); v.W( w ); Test( v ); }
    
    {
        HVec4 a( 0.0f, 1.0f, 2.0f, 3.0f );
        HVec4 b( 4.0f, 5.0f, 6.0f, 7.0f );
        Test( -a );
        Test( -b );
        Test( a + b );
        Test( b + a );
        Test( a - b );
        Test( b - a );
        Test( a * b );
        Test( b * a );
        Test( a / b );
        Test( b / a );
        HVec4 c( a ); c += b; Test( c );
        HVec4 d( a ); d -= c; Test( d );
        HF32  f0( 0.0f );
        HF32  f1( 1.0f );
        HF32  f2( 2.0f );
        Test( a * f0 );
        Test( a * f1 );
        Test( a * f2 );
        Test( a / f0 );
        Test( a / f1 );
        Test( a / f2 );
        HVec4 e( a ); e *= f0; Test( e );
        HVec4 f( a ); f *= f1; Test( f );
        HVec4 g( a ); g *= f2; Test( g );
        HVec4 h( a ); h /= f0; Test( h );
        HVec4 i( a ); i /= f1; Test( i );
        HVec4 j( a ); j /= f2; Test( j );
    }
    
    {
        HVec4 a( 0.0f, 1.0f, 2.0f, 3.0f );
        HVec4 b( 4.0f, 5.0f, 6.0f, 7.0f );
        Test( a.Min( b ) );
        Test( b.Min( a ) );
        Test( a.Max( b ) );
        Test( b.Max( a ) );
    }
    
    {
        HVec4 a( 0.0f, 0.0f, 0.0f, 0.0f );
        HVec4 b( 0.0f, 1.0f, 2.0f, 3.0f );
        HVec4 c( 4.0f, 5.0f, 6.0f, 7.0f );
        Test( a.Sum() );
        Test( b.Sum() );
        Test( c.Sum() );
        Test( a.Length() );
        Test( b.Length() );
        Test( c.Length() );
        Test( a.Length2() );
        Test( b.Length2() );
        Test( c.Length2() );
        Test( a.LengthSquared() );
        Test( b.LengthSquared() );
        Test( c.LengthSquared() );
        Test( a.RcpLength() );
        Test( b.RcpLength() );
        Test( c.RcpLength() );
        Test( a.ReciprocalLength() );
        Test( b.ReciprocalLength() );
        Test( c.ReciprocalLength() );
        Test( a.Normalised() );
        Test( b.Normalised() );
        Test( c.Normalised() );
        Test( a.SafeNormalised( a ) );
        Test( b.SafeNormalised( b ) );
        Test( c.SafeNormalised( c ) );
        HVec4 a2( a ); a2.Normalise(); Test( a2 );
        HVec4 b2( b ); b2.Normalise(); Test( b2 );
        HVec4 c2( c ); c2.Normalise(); Test( c2 );
        HVec4 a3( a ); a3.SafeNormalise( a ); Test( a3 );
        HVec4 b3( b ); b3.SafeNormalise( b ); Test( b3 );
        HVec4 c3( c ); c3.SafeNormalise( c ); Test( c3 );
        Test( a | a );
        Test( a | b );
        Test( a | c );
        Test( b | a );
        Test( b | b );
        Test( b | c );
        Test( c | a );
        Test( c | b );
        Test( c | c );
    }
    
    for ( F32 a = -1.0f; a <= 1.0f; a += 1.0f )
    {
        for ( F32 b = -1.0f; b <= 1.0f; b += 1.0f )
        {
            for ( F32 c = -1.0f; c <= 1.0f; c += 1.0f )
            {
                for ( F32 d = -1.0f; d <= 1.0f; d += 1.0f )
                {
                    HMat44 m( HVec4( a, b, c, d ),
                              HVec4( b, c, d, a ),
                              HVec4( c, d, a, b ),
                              HVec4( d, a, b, c ) );
                    HVec3 v( a, b, c );
                    Test( v * m );
                }
            }
        }
    }
    
    for ( F32 ax = -1.0f; ax <= 1.0f; ax += 1.0f )
    {
        for ( F32 ay = -1.0f; ay <= 1.0f; ay += 1.0f )
        {
            for ( F32 az = -1.0f; az <= 1.0f; az += 1.0f )
            {
                for ( F32 aw = -1.0f; aw <= 1.0f; aw += 1.0f )
                {
                    HVec4 a( ax, ay, az, aw );
                    for ( F32 bx = -1.0f; bx <= 1.0f; bx += 1.0f )
                    {
                        for ( F32 by = -1.0f; by <= 1.0f; by += 1.0f )
                        {
                            for ( F32 bz = -1.0f; bz <= 1.0f; bz += 1.0f )
                            {
                                for ( F32 bw = -1.0f; bw <= 1.0f; bw += 1.0f )
                                {
                                    HVec4 b( bx, by, bz, bw );
                                    Test( a == b );
                                    Test( a != b );
                                    Test( a <= b );
                                    Test( a <  b );
                                    Test( a >= b );
                                    Test( a >  b );
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    #endif // TEMPORARY
    
    OutputDebugString( "\nMaths::Test(): Finished\n" );
    
    CloseHandle( g_MathsTestHandle );
    g_MathsTestHandle = NULL;
}

#endif // #if defined( USER_JDENTON ) && defined( PLATFORM_PC )

}; // namespace Maths



