#include "StdAfx.h"


#include <Core/Maths.h>
#include <Core/Maths/HVec/New/IncludeComplete.h>
#include <Core/Maths/HVec/New/IncludeComplete.inl>
#include <Core/Threads/Thread.h>
#include "Render/Render/NewMathsRender.h"
#include "Render/Render/GridRender.h"
#include "TestSections/TestSectionMaths.h"
#include <Framework/Application/UserControls.h>
#include <Framework/Application/ApplicationBase.h>
#include <camera2.h>
#include "System/Misc/Random.h"

static Debug::Logger& g_Log = Debug::GetLogger( "mercury.maths" );

namespace Framework
{


#if 0

F64 RcpRef( F64 x )         { return 1.0 / x; }
F64 DivRef( F64 x, F64 y )  { return x / y; }
F64 RcpSqrtRef( F64 x )     { return 1.0 / sqrt( x ); }

void sincos( F64 x, F64* pSin, F64* pCos ) { *pSin = sin( x ); *pCos = cos( x ); }

void NewMathsTest()
{
    using namespace Maths;
    
    IdleThread thread1( 1 );
    
    //SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
    
    const F64 PI( 3.1415926535897932384626433832795 );
    
    for ( F32 x=1.0f; x!=0.0f; x*=0.5f )
    {
        const F32 rcp_x = GetF32( Rcp( MakeVF32( x ) ) );
        const F32 rcp_sqrt_x = GetF32( RcpSqrt( MakeVF32( x ) ) );
        if ( !_finite( rcp_x ) )
            __debugbreak();
        if ( !_finite( rcp_sqrt_x ) )
            __debugbreak();
    }
    
    for ( F32 x=1.0f; _finite(x); x*=2.0f )
    {
        const F32 rcp_x = GetF32( Rcp( MakeVF32( x ) ) );
        const F32 rcp_sqrt_x = GetF32( RcpSqrt( MakeVF32( x ) ) );
        if ( !_finite( rcp_x ) )
            __debugbreak();
        if ( !_finite( rcp_sqrt_x ) )
            __debugbreak();
    }
    
    Analyse( "Rcp", &Rcp, &RcpRef, Interval( -10.0, 10.0 ), 100000 );
    
    Analyse( "Div", &Div, &DivRef, Interval( -10.0, 10.0 ), 100, Interval( -10.0, 10.0 ), 1000 );
    
    Analyse( "RcpSqrt", &RcpSqrt, &RcpSqrtRef, Interval( -10.0, 10.0 ), 100000 );
    
    Analyse( "Sqrt", &Sqrt, &sqrt, Interval( -10.0, 10.0 ), 100000 );
    
    Analyse( "Sin", &Sin, &sin, Interval( -PI * 2.0, PI * 2.0 ), 100000 );
    Analyse( "Cos", &Cos, &cos, Interval( -PI * 2.0, PI * 2.0 ), 100000 );
    Analyse( "Tan", &Tan, &tan, Interval( -PI, PI ), 100000 );
    Analyse( "SinCos", &SinCos, &sincos, Interval( -PI * 2.0, PI * 2.0 ), 100000 );
    
    Analyse( "ArcSin", &ArcSin, &asin, Interval( -1.0, 1.0 ), 100000 );
    Analyse( "ArcCos", &ArcCos, &acos, Interval( -1.0, 1.0 ), 100000 );
    Analyse( "ArcCos", &ArcCos, &acos, Interval( -1.0, 1.0 ), 100 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10.0, 10.0 ), 100000 );
    Analyse( "ArcTan", &ArcTan, &atan2, Interval( -10.0, 10.0 ), 100, Interval( -10.0, 10.0 ), 1000 );
    
    //Analyse( "ArcTan", &ArcTan, &atan2, Interval( -1.0, 1.0 ), 10, Interval( -1.0, 1.0 ), 10, true );
    
    Analyse( "ArcTan", &ArcTan, &atan, Interval( 0.0, 0.05 ), 100000 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -1.0, 1.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10.0, 10.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -100.0, 100.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -1000.0, 1000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10000.0, 10000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -100000.0, 100000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -1000000.0, 1000000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10000000.0, 10000000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -100000000.0, 100000000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -1000000000.0, 1000000000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10000000000.0, 10000000000.0 ), 1 );
    
    Analyse( "Log", &Log, &log, Interval( 0.0, 1.0 ), 100000 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 10.0 ), 100000 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 100.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 1000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 10000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 100000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 1000000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 10000000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 100000000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 1000000000.0 ), 1 /*, true */ );
    
    Analyse( "Exp", &Exp, &exp, Interval( -0.1, 0.1 ), 100000 /*, true */ );
    Analyse( "Exp", &Exp, &exp, Interval( -1.0, 1.0 ), 100000 /*, true */ );
    Analyse( "Exp", &Exp, &exp, Interval( -10.0, 10.0 ), 100000 /*, true */ );
    Analyse( "Exp", &Exp, &exp, Interval( -88.0, 88.0 ), 100000 /*, true */ );
    
    Analyse( "Pow", &Pow, &pow, Interval( -2.0, 2.0 ), 100, Interval( -2.0, 2.0 ), 1000 );
    
    Profile( "Log", &Log, Interval( 0.0, 1.0 ), 100000 );
    
    Profile( "Cos", &Cos, Interval( -PI * 2.0, PI * 2.0 ), 100000 );
    
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
    
    #if defined( COMPILER_VC )
    __debugbreak();
    #endif
}

#endif







#if 0

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( disable : 4503 )
#endif

void TestGP()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const Point p0( MakePoint( MakePoint3( zero, zero, zero ) ) );
    const Point p1( MakePoint( MakePoint3( one,  zero, zero ) ) );
    const Point p2( MakePoint( MakePoint3( zero, one,  zero ) ) );
    const Point p3( MakePoint( MakePoint3( zero, zero, one  ) ) );
    
    printf( "\nGeometric Product:\n" );
    printf( "\n" );
    printf( "p0 = " ); Print( p0 );
    printf( "p1 = " ); Print( p1 );
    printf( "p2 = " ); Print( p2 );
    printf( "p3 = " ); Print( p3 );
    printf( "\n" );
    printf( "p0 * p0 = " ); Print( p0 * p0 );
    printf( "p0 * p1 = " ); Print( p0 * p1 );
    printf( "p0 * p2 = " ); Print( p0 * p2 );
    printf( "p0 * p3 = " ); Print( p0 * p3 );
    printf( "\n" );
    printf( "p1 * p0 = " ); Print( p1 * p0 );
    printf( "p1 * p1 = " ); Print( p1 * p1 );
    printf( "p1 * p2 = " ); Print( p1 * p2 );
    printf( "p1 * p3 = " ); Print( p1 * p3 );
    printf( "\n" );
    printf( "p2 * p0 = " ); Print( p2 * p0 );
    printf( "p2 * p1 = " ); Print( p2 * p1 );
    printf( "p2 * p2 = " ); Print( p2 * p2 );
    printf( "p2 * p3 = " ); Print( p2 * p3 );
    printf( "\n" );
    printf( "p3 * p0 = " ); Print( p3 * p0 );
    printf( "p3 * p1 = " ); Print( p3 * p1 );
    printf( "p3 * p2 = " ); Print( p3 * p2 );
    printf( "p3 * p3 = " ); Print( p3 * p3 );
    printf( "\n" );
}

void TestOP()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const Point p0( MakePoint( MakePoint3( zero, zero, zero ) ) );
    const Point p1( MakePoint( MakePoint3( one,  zero, zero ) ) );
    const Point p2( MakePoint( MakePoint3( zero, one,  zero ) ) );
    const Point p3( MakePoint( MakePoint3( zero, zero, one  ) ) );
    
    printf( "\nOuter Product:\n\n" );
    
    printf( "p0 ^ p0: " ); Print( p0 ^ p0 );
    printf( "p0 ^ p1: " ); Print( p0 ^ p1 );
    printf( "p0 ^ p2: " ); Print( p0 ^ p2 );
    printf( "p0 ^ p3: " ); Print( p0 ^ p3 );
                                      
    printf( "p1 ^ p0: " ); Print( p1 ^ p0 );
    printf( "p1 ^ p1: " ); Print( p1 ^ p1 );
    printf( "p1 ^ p2: " ); Print( p1 ^ p2 );
    printf( "p1 ^ p3: " ); Print( p1 ^ p3 );
                                      
    printf( "p2 ^ p0: " ); Print( p2 ^ p0 );
    printf( "p2 ^ p1: " ); Print( p2 ^ p1 );
    printf( "p2 ^ p2: " ); Print( p2 ^ p2 );
    printf( "p2 ^ p3: " ); Print( p2 ^ p3 );
                                      
    printf( "p3 ^ p0: " ); Print( p3 ^ p0 );
    printf( "p3 ^ p1: " ); Print( p3 ^ p1 );
    printf( "p3 ^ p2: " ); Print( p3 ^ p2 );
    printf( "p3 ^ p3: " ); Print( p3 ^ p3 );
}

void TestIP()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const Point p0( MakePoint( MakePoint3( zero, zero, zero ) ) );
    const Point p1( MakePoint( MakePoint3( one,  zero, zero ) ) );
    const Point p2( MakePoint( MakePoint3( zero, one,  zero ) ) );
    const Point p3( MakePoint( MakePoint3( zero, zero, one  ) ) );
    
    printf( "\nInner Product:\n\n" );
    
    printf( "p0 | p0: " ); Print( p0 | p0 );
    printf( "p0 | p1: " ); Print( p0 | p1 );
    printf( "p0 | p2: " ); Print( p0 | p2 );
    printf( "p0 | p3: " ); Print( p0 | p3 );
                                      
    printf( "p1 | p0: " ); Print( p1 | p0 );
    printf( "p1 | p1: " ); Print( p1 | p1 );
    printf( "p1 | p2: " ); Print( p1 | p2 );
    printf( "p1 | p3: " ); Print( p1 | p3 );
                                      
    printf( "p2 | p0: " ); Print( p2 | p0 );
    printf( "p2 | p1: " ); Print( p2 | p1 );
    printf( "p2 | p2: " ); Print( p2 | p2 );
    printf( "p2 | p3: " ); Print( p2 | p3 );
                                      
    printf( "p3 | p0: " ); Print( p3 | p0 );
    printf( "p3 | p1: " ); Print( p3 | p1 );
    printf( "p3 | p2: " ); Print( p3 | p2 );
    printf( "p3 | p3: " ); Print( p3 | p3 );
}

void TestSP()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const Point p0( MakePoint( MakePoint3( zero, zero, zero ) ) );
    const Point p1( MakePoint( MakePoint3( one,  zero, zero ) ) );
    const Point p2( MakePoint( MakePoint3( zero, one,  zero ) ) );
    const Point p3( MakePoint( MakePoint3( zero, zero, one  ) ) );
    
    printf( "\nScalar Product:\n\n" );
    
    printf( "p0 %% p0: " ); Print( p0 % p0 );
    printf( "p0 %% p1: " ); Print( p0 % p1 );
    printf( "p0 %% p2: " ); Print( p0 % p2 );
    printf( "p0 %% p3: " ); Print( p0 % p3 );
                                   
    printf( "p1 %% p0: " ); Print( p1 % p0 );
    printf( "p1 %% p1: " ); Print( p1 % p1 );
    printf( "p1 %% p2: " ); Print( p1 % p2 );
    printf( "p1 %% p3: " ); Print( p1 % p3 );
                                   
    printf( "p2 %% p0: " ); Print( p2 % p0 );
    printf( "p2 %% p1: " ); Print( p2 % p1 );
    printf( "p2 %% p2: " ); Print( p2 % p2 );
    printf( "p2 %% p3: " ); Print( p2 % p3 );
                                   
    printf( "p3 %% p0: " ); Print( p3 % p0 );
    printf( "p3 %% p1: " ); Print( p3 % p1 );
    printf( "p3 %% p2: " ); Print( p3 % p2 );
    printf( "p3 %% p3: " ); Print( p3 % p3 );
}

void TestLC()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const Point p0( MakePoint( MakePoint3( zero, zero, zero ) ) );
    const Point p1( MakePoint( MakePoint3( one,  zero, zero ) ) );
    const Point p2( MakePoint( MakePoint3( zero, one,  zero ) ) );
    const Point p3( MakePoint( MakePoint3( zero, zero, one  ) ) );
    
    printf( "\nLeft Contraction:\n\n" );
    
    printf( "p0 << p0: " ); Print( p0 << p0 );
    printf( "p0 << p1: " ); Print( p0 << p1 );
    printf( "p0 << p2: " ); Print( p0 << p2 );
    printf( "p0 << p3: " ); Print( p0 << p3 );
                                 
    printf( "p1 << p0: " ); Print( p1 << p0 );
    printf( "p1 << p1: " ); Print( p1 << p1 );
    printf( "p1 << p2: " ); Print( p1 << p2 );
    printf( "p1 << p3: " ); Print( p1 << p3 );
                                 
    printf( "p2 << p0: " ); Print( p2 << p0 );
    printf( "p2 << p1: " ); Print( p2 << p1 );
    printf( "p2 << p2: " ); Print( p2 << p2 );
    printf( "p2 << p3: " ); Print( p2 << p3 );
                                 
    printf( "p3 << p0: " ); Print( p3 << p0 );
    printf( "p3 << p1: " ); Print( p3 << p1 );
    printf( "p3 << p2: " ); Print( p3 << p2 );
    printf( "p3 << p3: " ); Print( p3 << p3 );
}

void TestRC()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const Point p0( MakePoint( MakePoint3( zero, zero, zero ) ) );
    const Point p1( MakePoint( MakePoint3( one,  zero, zero ) ) );
    const Point p2( MakePoint( MakePoint3( zero, one,  zero ) ) );
    const Point p3( MakePoint( MakePoint3( zero, zero, one  ) ) );
    
    printf( "\nRight Contraction:\n\n" );
    
    printf( "p0 >> p0: " ); Print( p0 >> p0 );
    printf( "p0 >> p1: " ); Print( p0 >> p1 );
    printf( "p0 >> p2: " ); Print( p0 >> p2 );
    printf( "p0 >> p3: " ); Print( p0 >> p3 );
                                 
    printf( "p1 >> p0: " ); Print( p1 >> p0 );
    printf( "p1 >> p1: " ); Print( p1 >> p1 );
    printf( "p1 >> p2: " ); Print( p1 >> p2 );
    printf( "p1 >> p3: " ); Print( p1 >> p3 );
                                 
    printf( "p2 >> p0: " ); Print( p2 >> p0 );
    printf( "p2 >> p1: " ); Print( p2 >> p1 );
    printf( "p2 >> p2: " ); Print( p2 >> p2 );
    printf( "p2 >> p3: " ); Print( p2 >> p3 );
                                 
    printf( "p3 >> p0: " ); Print( p3 >> p0 );
    printf( "p3 >> p1: " ); Print( p3 >> p1 );
    printf( "p3 >> p2: " ); Print( p3 >> p2 );
    printf( "p3 >> p3: " ); Print( p3 >> p3 );
}

void TestDistance()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const Point p0( MakePoint( MakePoint3( zero, zero, zero ) ) );
    const Point p1( MakePoint( MakePoint3( one,  zero, zero ) ) );
    const Point p2( MakePoint( MakePoint3( zero, one,  zero ) ) );
    const Point p3( MakePoint( MakePoint3( zero, zero, one  ) ) );
    
    printf( "\nPoint-Point Distance:\n\n" );
    
    printf( "Distance( p0, p0 ): %f\n", GetF32( Distance( p0, p0 ) ) );
    printf( "Distance( p0, p1 ): %f\n", GetF32( Distance( p0, p1 ) ) );
    printf( "Distance( p0, p2 ): %f\n", GetF32( Distance( p0, p2 ) ) );
    printf( "Distance( p0, p3 ): %f\n", GetF32( Distance( p0, p3 ) ) );
                                                                
    printf( "Distance( p1, p0 ): %f\n", GetF32( Distance( p1, p0 ) ) );
    printf( "Distance( p1, p1 ): %f\n", GetF32( Distance( p1, p1 ) ) );
    printf( "Distance( p1, p2 ): %f\n", GetF32( Distance( p1, p2 ) ) );
    printf( "Distance( p1, p3 ): %f\n", GetF32( Distance( p1, p3 ) ) );
                                                                
    printf( "Distance( p2, p0 ): %f\n", GetF32( Distance( p2, p0 ) ) );
    printf( "Distance( p2, p1 ): %f\n", GetF32( Distance( p2, p1 ) ) );
    printf( "Distance( p2, p2 ): %f\n", GetF32( Distance( p2, p2 ) ) );
    printf( "Distance( p2, p3 ): %f\n", GetF32( Distance( p2, p3 ) ) );
                                                                
    printf( "Distance( p3, p0 ): %f\n", GetF32( Distance( p3, p0 ) ) );
    printf( "Distance( p3, p1 ): %f\n", GetF32( Distance( p3, p1 ) ) );
    printf( "Distance( p3, p2 ): %f\n", GetF32( Distance( p3, p2 ) ) );
    printf( "Distance( p3, p3 ): %f\n", GetF32( Distance( p3, p3 ) ) );
    
    const Scalar two( Const<VF32>::Two() );
    const Scalar three( Const<VF32>::Three() );
    
    printf( "Distance( p0 * 2, p0 * 3 ): %f\n", GetF32( Distance( p0 * two, p0 * three ) ) );
    printf( "Distance( p0 * 2, p1 * 3 ): %f\n", GetF32( Distance( p0 * two, p1 * three ) ) );
    printf( "Distance( p0 * 2, p2 * 3 ): %f\n", GetF32( Distance( p0 * two, p2 * three ) ) );
    printf( "Distance( p0 * 2, p3 * 3 ): %f\n", GetF32( Distance( p0 * two, p3 * three ) ) );
                                                                                            
    printf( "Distance( p1 * 2, p0 * 3 ): %f\n", GetF32( Distance( p1 * two, p0 * three ) ) );
    printf( "Distance( p1 * 2, p1 * 3 ): %f\n", GetF32( Distance( p1 * two, p1 * three ) ) );
    printf( "Distance( p1 * 2, p2 * 3 ): %f\n", GetF32( Distance( p1 * two, p2 * three ) ) );
    printf( "Distance( p1 * 2, p3 * 3 ): %f\n", GetF32( Distance( p1 * two, p3 * three ) ) );
                                                                                            
    printf( "Distance( p2 * 2, p0 * 3 ): %f\n", GetF32( Distance( p2 * two, p0 * three ) ) );
    printf( "Distance( p2 * 2, p1 * 3 ): %f\n", GetF32( Distance( p2 * two, p1 * three ) ) );
    printf( "Distance( p2 * 2, p2 * 3 ): %f\n", GetF32( Distance( p2 * two, p2 * three ) ) );
    printf( "Distance( p2 * 2, p3 * 3 ): %f\n", GetF32( Distance( p2 * two, p3 * three ) ) );
                                                                                            
    printf( "Distance( p3 * 2, p0 * 3 ): %f\n", GetF32( Distance( p3 * two, p0 * three ) ) );
    printf( "Distance( p3 * 2, p1 * 3 ): %f\n", GetF32( Distance( p3 * two, p1 * three ) ) );
    printf( "Distance( p3 * 2, p2 * 3 ): %f\n", GetF32( Distance( p3 * two, p2 * three ) ) );
    printf( "Distance( p3 * 2, p3 * 3 ): %f\n", GetF32( Distance( p3 * two, p3 * three ) ) );
}

void TestDualPlane()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 half( Const<VF32>::Half() );
    const VF32 one( Const<VF32>::One() );
    
    const Point p0( MakePoint( MakePoint3( zero, zero, zero ) ) );
    const Point p1( MakePoint( MakePoint3( one,  zero, zero ) ) );
    const Point p2( MakePoint( MakePoint3( zero, one,  zero ) ) );
    const Point p3( MakePoint( MakePoint3( zero, zero, one  ) ) );
    
    const Normal3 n1( Const<Normal3>::AxisX() );
    const Normal3 n2( Const<Normal3>::AxisY() );
    const Normal3 n3( Const<Normal3>::AxisZ() );
    
    const DualPlane dp1( MakeDualPlane( n1, half ) );
    const DualPlane dp2( MakeDualPlane( n2, half ) );
    const DualPlane dp3( MakeDualPlane( n3, half ) );
    
    printf( "\nDualPlane-Point Distance:\n\n" );
    
    printf( "dp1 = " ); Print( dp1 );
    printf( "dp2 = " ); Print( dp2 );
    printf( "dp3 = " ); Print( dp3 );
    printf( "\n" );
    
    printf( "Distance( dp1, p0 ): %f\n", GetF32( Distance( dp1, p0 ) ) );
    printf( "Distance( dp1, p1 ): %f\n", GetF32( Distance( dp1, p1 ) ) );
    printf( "Distance( dp1, p2 ): %f\n", GetF32( Distance( dp1, p2 ) ) );
    printf( "Distance( dp1, p3 ): %f\n", GetF32( Distance( dp1, p3 ) ) );
                                                              
    printf( "Distance( dp2, p0 ): %f\n", GetF32( Distance( dp2, p0 ) ) );
    printf( "Distance( dp2, p1 ): %f\n", GetF32( Distance( dp2, p1 ) ) );
    printf( "Distance( dp2, p2 ): %f\n", GetF32( Distance( dp2, p2 ) ) );
    printf( "Distance( dp2, p3 ): %f\n", GetF32( Distance( dp2, p3 ) ) );
                                                              
    printf( "Distance( dp3, p0 ): %f\n", GetF32( Distance( dp3, p0 ) ) );
    printf( "Distance( dp3, p1 ): %f\n", GetF32( Distance( dp3, p1 ) ) );
    printf( "Distance( dp3, p2 ): %f\n", GetF32( Distance( dp3, p2 ) ) );
    printf( "Distance( dp3, p3 ): %f\n", GetF32( Distance( dp3, p3 ) ) );
    
    printf( "\n" );
    
    const Scalar two( Const<VF32>::Two() );
    const Scalar three( Const<VF32>::Three() );
    
    printf( "Distance( dp1 * 3, p0 * 2 ): %f\n", GetF32( Distance( dp1 * three, p0 * two ) ) );
    printf( "Distance( dp1 * 3, p1 * 2 ): %f\n", GetF32( Distance( dp1 * three, p1 * two ) ) );
    printf( "Distance( dp1 * 3, p2 * 2 ): %f\n", GetF32( Distance( dp1 * three, p2 * two ) ) );
    printf( "Distance( dp1 * 3, p3 * 2 ): %f\n", GetF32( Distance( dp1 * three, p3 * two ) ) );
    printf( "\n" );
                                                                      
    printf( "Distance( dp2 * 3, p0 * 2 ): %f\n", GetF32( Distance( dp2 * three, p0 * two ) ) );
    printf( "Distance( dp2 * 3, p1 * 2 ): %f\n", GetF32( Distance( dp2 * three, p1 * two ) ) );
    printf( "Distance( dp2 * 3, p2 * 2 ): %f\n", GetF32( Distance( dp2 * three, p2 * two ) ) );
    printf( "Distance( dp2 * 3, p3 * 2 ): %f\n", GetF32( Distance( dp2 * three, p3 * two ) ) );
    printf( "\n" );
                                                                      
    printf( "Distance( dp3 * 3, p0 * 2 ): %f\n", GetF32( Distance( dp3 * three, p0 * two ) ) );
    printf( "Distance( dp3 * 3, p1 * 2 ): %f\n", GetF32( Distance( dp3 * three, p1 * two ) ) );
    printf( "Distance( dp3 * 3, p2 * 2 ): %f\n", GetF32( Distance( dp3 * three, p2 * two ) ) );
    printf( "Distance( dp3 * 3, p3 * 2 ): %f\n", GetF32( Distance( dp3 * three, p3 * two ) ) );
    printf( "\n" );
    
    printf( "dp1 = " ); Print( dp1 );
    printf( "dp2 = " ); Print( dp2 );
    printf( "dp3 = " ); Print( dp3 );
    printf( "\n" );
    
    printf( "Direct( dp1 ) = " ); Print( Direct( dp1 ) );
    printf( "Direct( dp2 ) = " ); Print( Direct( dp2 ) );
    printf( "Direct( dp3 ) = " ); Print( Direct( dp3 ) );
    printf( "\n" );
    
    printf( "Dual( Direct( dp1 ) ) = " ); Print( Dual( Direct( dp1 ) ) );
    printf( "Dual( Direct( dp2 ) ) = " ); Print( Dual( Direct( dp2 ) ) );
    printf( "Dual( Direct( dp3 ) ) = " ); Print( Dual( Direct( dp3 ) ) );
    printf( "\n" );
    
    #if defined( COMPILER_VC )
    __debugbreak();
    #endif
}

void TestDualSphere1()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const VF32 ten( Const<VF32>::Ten() );
    
    const Point p0( MakePoint( zero, zero, zero ) );
    const Point p1( MakePoint( ten,  zero, zero ) );
    const Point p2( MakePoint( zero, ten,  zero ) );
    const Point p3( MakePoint( zero, zero, ten  ) );
    
    const DualSphere ds0( MakeDualSphere( p0, one ) );
    const DualSphere ds1( MakeDualSphere( p1, one ) );
    const DualSphere ds2( MakeDualSphere( p2, one ) );
    const DualSphere ds3( MakeDualSphere( p3, one ) );
    
    printf( "\nDualSphere-Point Distance (1):\n\n" );
    
    printf( "ds0 = " ); Print( ds0 );
    printf( "ds1 = " ); Print( ds1 );
    printf( "ds2 = " ); Print( ds2 );
    printf( "ds3 = " ); Print( ds3 );
    printf( "\n" );
    
    printf( "Distance( ds0, p0 ): %f\n", GetF32( Distance( ds0, p0 ) ) );
    printf( "Distance( ds0, p1 ): %f\n", GetF32( Distance( ds0, p1 ) ) );
    printf( "Distance( ds0, p2 ): %f\n", GetF32( Distance( ds0, p2 ) ) );
    printf( "Distance( ds0, p3 ): %f\n", GetF32( Distance( ds0, p3 ) ) );
                                                              
    printf( "Distance( ds1, p0 ): %f\n", GetF32( Distance( ds1, p0 ) ) );
    printf( "Distance( ds1, p1 ): %f\n", GetF32( Distance( ds1, p1 ) ) );
    printf( "Distance( ds1, p2 ): %f\n", GetF32( Distance( ds1, p2 ) ) );
    printf( "Distance( ds1, p3 ): %f\n", GetF32( Distance( ds1, p3 ) ) );
                                                              
    printf( "Distance( ds2, p0 ): %f\n", GetF32( Distance( ds2, p0 ) ) );
    printf( "Distance( ds2, p1 ): %f\n", GetF32( Distance( ds2, p1 ) ) );
    printf( "Distance( ds2, p2 ): %f\n", GetF32( Distance( ds2, p2 ) ) );
    printf( "Distance( ds2, p3 ): %f\n", GetF32( Distance( ds2, p3 ) ) );
                                                              
    printf( "Distance( ds3, p0 ): %f\n", GetF32( Distance( ds3, p0 ) ) );
    printf( "Distance( ds3, p1 ): %f\n", GetF32( Distance( ds3, p1 ) ) );
    printf( "Distance( ds3, p2 ): %f\n", GetF32( Distance( ds3, p2 ) ) );
    printf( "Distance( ds3, p3 ): %f\n", GetF32( Distance( ds3, p3 ) ) );
}

void TestDualSphere2()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const VF32 ten( Const<VF32>::Ten() );
    
    const Point p0( MakePoint( zero, zero, zero ) );
    const Point p1( MakePoint( ten,  zero, zero ) );
    const Point p2( MakePoint( zero, ten,  zero ) );
    const Point p3( MakePoint( zero, zero, ten  ) );
    
    const DualSphere ds0( MakeDualSphere( p0, one ) );
    const DualSphere ds1( MakeDualSphere( p1, one ) );
    const DualSphere ds2( MakeDualSphere( p2, one ) );
    const DualSphere ds3( MakeDualSphere( p3, one ) );
    
    printf( "\nDualSphere-Point Distance (2):\n\n" );
    
    const Scalar two( Const<VF32>::Two() );
    const Scalar three( Const<VF32>::Three() );
    
    printf( "Distance( ds0 * 3, p0 * 2 ): %f\n", GetF32( Distance( ds0 * three, p0 * two ) ) );
    printf( "Distance( ds0 * 3, p1 * 2 ): %f\n", GetF32( Distance( ds0 * three, p1 * two ) ) );
    printf( "Distance( ds0 * 3, p2 * 2 ): %f\n", GetF32( Distance( ds0 * three, p2 * two ) ) );
    printf( "Distance( ds0 * 3, p3 * 2 ): %f\n", GetF32( Distance( ds0 * three, p3 * two ) ) );
                                                                          
    printf( "Distance( ds1 * 3, p0 * 2 ): %f\n", GetF32( Distance( ds1 * three, p0 * two ) ) );
    printf( "Distance( ds1 * 3, p1 * 2 ): %f\n", GetF32( Distance( ds1 * three, p1 * two ) ) );
    printf( "Distance( ds1 * 3, p2 * 2 ): %f\n", GetF32( Distance( ds1 * three, p2 * two ) ) );
    printf( "Distance( ds1 * 3, p3 * 2 ): %f\n", GetF32( Distance( ds1 * three, p3 * two ) ) );
                                                                          
    printf( "Distance( ds2 * 3, p0 * 2 ): %f\n", GetF32( Distance( ds2 * three, p0 * two ) ) );
    printf( "Distance( ds2 * 3, p1 * 2 ): %f\n", GetF32( Distance( ds2 * three, p1 * two ) ) );
    printf( "Distance( ds2 * 3, p2 * 2 ): %f\n", GetF32( Distance( ds2 * three, p2 * two ) ) );
    printf( "Distance( ds2 * 3, p3 * 2 ): %f\n", GetF32( Distance( ds2 * three, p3 * two ) ) );
                                                                          
    printf( "Distance( ds3 * 3, p0 * 2 ): %f\n", GetF32( Distance( ds3 * three, p0 * two ) ) );
    printf( "Distance( ds3 * 3, p1 * 2 ): %f\n", GetF32( Distance( ds3 * three, p1 * two ) ) );
    printf( "Distance( ds3 * 3, p2 * 2 ): %f\n", GetF32( Distance( ds3 * three, p2 * two ) ) );
    printf( "Distance( ds3 * 3, p3 * 2 ): %f\n", GetF32( Distance( ds3 * three, p3 * two ) ) );
}

void TestDualSphere3()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const VF32 ten( Const<VF32>::Ten() );
    
    const Point p0( MakePoint( zero, zero, zero ) );
    const Point p1( MakePoint( ten,  zero, zero ) );
    const Point p2( MakePoint( zero, ten,  zero ) );
    const Point p3( MakePoint( zero, zero, ten  ) );
    
    const DualSphere ds0( MakeDualSphere( p0, Const<VF32>::Two() ) );
    const DualSphere ds1( MakeDualSphere( p1, Const<VF32>::Two() ) );
    const DualSphere ds2( MakeDualSphere( p2, Const<VF32>::Two() ) );
    const DualSphere ds3( MakeDualSphere( p3, Const<VF32>::Two() ) );
    
    printf( "\nDualSphere-DualSphere Distance:\n\n" );
    
    printf( "ds0 = " ); Print( ds0 );
    printf( "ds1 = " ); Print( ds1 );
    printf( "ds2 = " ); Print( ds2 );
    printf( "ds3 = " ); Print( ds3 );
    printf( "\n" );
    
    const Scalar two( Const<VF32>::Two() );
    const Scalar three( Const<VF32>::Three() );
    
    printf( "Distance( ds0 * 3, ds0 * 2 ): %f\n", GetF32( Distance( ds0 * three, ds0 * two ) ) );
    printf( "Distance( ds0 * 3, ds1 * 2 ): %f\n", GetF32( Distance( ds0 * three, ds1 * two ) ) );
    printf( "Distance( ds0 * 3, ds2 * 2 ): %f\n", GetF32( Distance( ds0 * three, ds2 * two ) ) );
    printf( "Distance( ds0 * 3, ds3 * 2 ): %f\n", GetF32( Distance( ds0 * three, ds3 * two ) ) );
                                                                                   
    printf( "Distance( ds1 * 3, ds0 * 2 ): %f\n", GetF32( Distance( ds1 * three, ds0 * two ) ) );
    printf( "Distance( ds1 * 3, ds1 * 2 ): %f\n", GetF32( Distance( ds1 * three, ds1 * two ) ) );
    printf( "Distance( ds1 * 3, ds2 * 2 ): %f\n", GetF32( Distance( ds1 * three, ds2 * two ) ) );
    printf( "Distance( ds1 * 3, ds3 * 2 ): %f\n", GetF32( Distance( ds1 * three, ds3 * two ) ) );
                                                                                   
    printf( "Distance( ds2 * 3, ds0 * 2 ): %f\n", GetF32( Distance( ds2 * three, ds0 * two ) ) );
    printf( "Distance( ds2 * 3, ds1 * 2 ): %f\n", GetF32( Distance( ds2 * three, ds1 * two ) ) );
    printf( "Distance( ds2 * 3, ds2 * 2 ): %f\n", GetF32( Distance( ds2 * three, ds2 * two ) ) );
    printf( "Distance( ds2 * 3, ds3 * 2 ): %f\n", GetF32( Distance( ds2 * three, ds3 * two ) ) );
                                                                                   
    printf( "Distance( ds3 * 3, ds0 * 2 ): %f\n", GetF32( Distance( ds3 * three, ds0 * two ) ) );
    printf( "Distance( ds3 * 3, ds1 * 2 ): %f\n", GetF32( Distance( ds3 * three, ds1 * two ) ) );
    printf( "Distance( ds3 * 3, ds2 * 2 ): %f\n", GetF32( Distance( ds3 * three, ds2 * two ) ) );
    printf( "Distance( ds3 * 3, ds3 * 2 ): %f\n", GetF32( Distance( ds3 * three, ds3 * two ) ) );
}

void TestTranslator()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    printf( "\nTranslators:\n\n" );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const Vector vx( one, zero, zero );
    const Vector vy( zero, one, zero );
    const Vector vz( zero, zero, one );
    const Translator tx( MakeTranslator( vx ) );
    const Translator ty( MakeTranslator( vy ) );
    const Translator tz( MakeTranslator( vz ) );
    const Translator itx( Inverse( tx ) );
    const Translator ity( Inverse( ty ) );
    const Translator itz( Inverse( tz ) );
    
    printf( "origin %% infinity = " ); Print( Origin( one ) % Infinity( one ) );
    printf( "infinity %% origin = " ); Print( Infinity( one ) % Origin( one ) );
    printf( "\n" );
    
    printf( "origin * infinity = " ); Print( Origin( one ) * Infinity( one ) );
    printf( "infinity * origin = " ); Print( Infinity( one ) * Origin( one ) );
    printf( "\n" );
    
    printf( "tx = " ); Print( tx );
    printf( "ty = " ); Print( ty );
    printf( "tz = " ); Print( tz );
    printf( "\n" );
    
    printf( "Inverse( tx ) = " ); Print( itx );
    printf( "Inverse( ty ) = " ); Print( ity );
    printf( "Inverse( tz ) = " ); Print( itz );
    printf( "\n" );
    
    const Point p0( MakePoint( zero, zero, zero ) );
    const Point p1( MakePoint( one, zero, zero ) );
    const Point p2( MakePoint( zero, one, zero ) );
    const Point p3( MakePoint( zero, zero, one ) );
    
    printf( "tx * p0 = " ); Print( tx * p0 );
    printf( "ty * p0 = " ); Print( ty * p0 );
    printf( "tz * p0 = " ); Print( tz * p0 );
    printf( "\n" );
    
    printf( "origin = " ); Print( Origin( one ) );
    printf( "tx * origin = " ); Print( tx * Origin( one ) );
    printf( "ty * origin = " ); Print( ty * Origin( one ) );
    printf( "tz * origin = " ); Print( tz * Origin( one ) );
    printf( "\n" );
    
    printf( "tx * origin * itx = " ); Print( ( tx * Origin( one ) ) * itx );
    printf( "ty * origin * ity = " ); Print( ( ty * Origin( one ) ) * ity );
    printf( "tz * origin * itz = " ); Print( ( tz * Origin( one ) ) * itz );
    printf( "\n" );
    
    printf( "p0 = " ); Print( p0 );
    printf( "tx * p0 * itx = " ); Print( tx * p0 * itx );
    printf( "ty * p0 * ity = " ); Print( ty * p0 * ity );
    printf( "tz * p0 * itz = " ); Print( tz * p0 * itz );
    printf( "\n" );
    
    printf( "p1 = " ); Print( p1 );
    printf( "tx * p1 * itx = " ); Print( tx * p1 * itx );
    printf( "ty * p1 * ity = " ); Print( ty * p1 * ity );
    printf( "tz * p1 * itz = " ); Print( tz * p1 * itz );
    printf( "\n" );
    
    printf( "p2 = " ); Print( p2 );
    printf( "tx * p2 * itx = " ); Print( tx * p2 * itx );
    printf( "ty * p2 * ity = " ); Print( ty * p2 * ity );
    printf( "tz * p2 * itz = " ); Print( tz * p2 * itz );
    printf( "\n" );
    
    printf( "p3 = " ); Print( p3 );
    printf( "tx * p3 * itx = " ); Print( tx * p3 * itx );
    printf( "ty * p3 * ity = " ); Print( ty * p3 * ity );
    printf( "tz * p3 * itz = " ); Print( tz * p3 * itz );
    printf( "\n" );
    
    //#if defined( COMPILER_VC )
    //__debugbreak();
    //#endif
}

void TestRotor()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    printf( "\nRotors:\n\n" );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const Vector vx( one, zero, zero );
    const Vector vy( zero, one, zero );
    const Vector vz( zero, zero, one );
    const Rotor rx( MakeRotor( vy, vz ) );
    //const Rotor ry( MakeRotor( vz, vx ) );
    const Rotor ry( MakeRotor( vx, vz ) );
    const Rotor rz( MakeRotor( vx, vy ) );
    const Rotor irx( Inverse( rx ) );
    const Rotor iry( Inverse( ry ) );
    const Rotor irz( Inverse( rz ) );
    
    printf( "rx = " ); Print( rx );
    printf( "ry = " ); Print( ry );
    printf( "rz = " ); Print( rz );
    printf( "\n" );
    
    printf( "Inverse( rx ) = " ); Print( irx );
    printf( "Inverse( ry ) = " ); Print( iry );
    printf( "Inverse( rz ) = " ); Print( irz );
    printf( "\n" );
    
    const Point p0( MakePoint( zero, zero, zero ) );
    const Point p1( MakePoint( one, zero, zero ) );
    const Point p2( MakePoint( zero, one, zero ) );
    const Point p3( MakePoint( zero, zero, one ) );
    
    printf( "p0 = " ); Print( p0 );
    printf( "p1 = " ); Print( p1 );
    printf( "p2 = " ); Print( p2 );
    printf( "p3 = " ); Print( p3 );
    printf( "\n" );
    
    printf( "rx * p0 * irx = " ); Print( rx * p0 * irx );
    printf( "ry * p0 * iry = " ); Print( ry * p0 * iry );
    printf( "rz * p0 * irz = " ); Print( rz * p0 * irz );
    printf( "\n" );
    
    printf( "rx * p1 * irx = " ); Print( rx * p1 * irx );
    printf( "ry * p1 * iry = " ); Print( ry * p1 * iry );
    printf( "rz * p1 * irz = " ); Print( rz * p1 * irz );
    printf( "\n" );
    
    printf( "rx * p2 * irx = " ); Print( rx * p2 * irx );
    printf( "ry * p2 * iry = " ); Print( ry * p2 * iry );
    printf( "rz * p2 * irz = " ); Print( rz * p2 * irz );
    printf( "\n" );
    
    printf( "rx * p3 * irx = " ); Print( rx * p3 * irx );
    printf( "ry * p3 * iry = " ); Print( ry * p3 * iry );
    printf( "rz * p3 * irz = " ); Print( rz * p3 * irz );
    printf( "\n" );
    
    const Point p0_rx( rx * p0 * irx );
    const Point p0_ry( ry * p0 * iry );
    const Point p0_rz( rz * p0 * irz );
    
    const Point p1_rx( rx * p1 * irx );
    const Point p1_ry( ry * p1 * iry );
    const Point p1_rz( rz * p1 * irz );
                             
    const Point p2_rx( rx * p2 * irx );
    const Point p2_ry( ry * p2 * iry );
    const Point p2_rz( rz * p2 * irz );
                             
    const Point p3_rx( rx * p3 * irx );
    const Point p3_ry( ry * p3 * iry );
    const Point p3_rz( rz * p3 * irz );
    
    Print( p0_rx );
    Print( p0_ry );
    Print( p0_rz );
    printf( "\n" );
    
    Print( p1_rx );
    Print( p1_ry );
    Print( p1_rz );
    printf( "\n" );
         
    Print( p2_rx );
    Print( p2_ry );
    Print( p2_rz );
    printf( "\n" );
         
    Print( p3_rx );
    Print( p3_ry );
    Print( p3_rz );
    printf( "\n" );
    
    //#if defined( COMPILER_VC )
    //__debugbreak();
    //#endif
}

void TestFlats()
{
    using namespace Maths;
    using namespace Maths::GA;
    
    printf( "\nFlats:\n\n" );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    
    const Origin    eo( one );
    const Vector_e1 e1( one );
    const Vector_e2 e2( one );
    const Vector_e3 e3( one );
    const Infinity  ei( one );
    
    const PseudoScalar   I( Const<PseudoScalar>::One() );
    const PseudoScalarE3 I3( Const<PseudoScalarE3>::One() );
    const PseudoScalar   InverseI( Const<PseudoScalar>::Inverse() );
    const PseudoScalarE3 InverseI3( Const<PseudoScalarE3>::Inverse() );
    
    const Vector vx( one, zero, zero );
    const Vector vy( zero, one, zero );
    const Vector vz( zero, zero, one );
    
    const Point p0( MakePoint( zero, zero, zero ) );
    const Point p1( MakePoint( one, zero, zero ) );
    const Point p2( MakePoint( zero, one, zero ) );
    const Point p3( MakePoint( zero, zero, one ) );
    
    printf( "eo = " ); Print( eo );
    printf( "ei = " ); Print( ei );
    printf( "eo ^ eo = " ); Print( eo ^ eo );
    printf( "eo ^ ei = " ); Print( eo ^ ei );
    printf( "ei ^ eo = " ); Print( ei ^ eo );
    printf( "ei ^ ei = " ); Print( ei ^ ei );
    printf( "(eo^ei)eo = " ); Print( ( eo ^ ei ) * eo );
    printf( "(eo^ei)ei = " ); Print( ( eo ^ ei ) * ei );
    printf( "(eo^ei)(eo^ei) = " ); Print( ( eo ^ ei ) * ( eo ^ ei ) );
    
    printf( "p0 = " ); Print( p0 );
    printf( "p1 = " ); Print( p1 );
    printf( "p2 = " ); Print( p2 );
    printf( "p3 = " ); Print( p3 );
    printf( "\n" );
    
    printf( "Flat points:\n" );
    printf( "p0 ^ ei = " ); Print( p0 ^ ei );
    printf( "p1 ^ ei = " ); Print( p1 ^ ei );
    printf( "p2 ^ ei = " ); Print( p2 ^ ei );
    printf( "p3 ^ ei = " ); Print( p3 ^ ei );
    printf( "\n" );
    
    GA::Display( p0 ^ ei );
    
    printf( "PseudoScalar = " ); Print( eo ^ e1 ^ e2 ^ e3 ^ ei );
    printf( "PseudoScalar = " ); Print( I );
    printf( "Inverse( PseudoScalar ) = " ); Print( eo ^ e3 ^ e2 ^ e1 ^ ei );
    printf( "Inverse( PseudoScalar ) = " ); Print( Inverse( I ) );
    printf( "Inverse( PseudoScalar ) = " ); Print( InverseI );
    printf( "Reverse( PseudoScalar ) = " ); Print( Reverse( I ) );
    printf( "I3 = " ); Print( I3 );
    printf( "Inverse( I3 ) = " ); Print( Inverse( I3 ) );
    printf( "Inverse( I3 ) = " ); Print( InverseI3 );
    printf( "eo ^ Inverse( I3 ) ^ ei = " ); Print( eo ^ Inverse( I3 ) ^ ei );
    printf( "eo ^ Inverse( I3 ) ^ ei = " ); Print( eo ^ InverseI3 ^ ei );
    
    #if defined( COMPILER_VC )
    __debugbreak();
    #endif
}

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

#endif







//#if defined( USER_JDENTON )
//#pragma optimize( "", off )
//#endif

F32 g_zero = 0.0f;

C_TestSectionMaths::C_TestSectionMaths( const Util::C_String& name ) : C_TestSection( name )
{
    using namespace Maths;

    //IdleThread idleThread1( 1 );
    //IdleThread idleThread2( 2 );
    //IdleThread idleThread3( 3 );
    //IdleThread idleThread4( 4 );
    //IdleThread idleThread5( 5 );
    
	// Set the debug feed to fly camera:
	C_View& view = C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	view.DebugFeed().SetDebugType(Cam2::E_FlyCamera);
    
    
    //const VF32 two( Const<VF32>::Two() );
    //const Vector3 vector3_zero( Const<Vector3>::Zero() );
    
    Vector3 translation( Const<Vector3>::Zero() );
    Vector3 offset_x( Const<Vector3>::UnitX() * Const<VF32>::Two() );
    //Vector3 offset_y( Const<Vector3>::UnitY() * Const<VF32>::Two() );
    Vector3 offset_z( Const<Vector3>::UnitZ() * Const<VF32>::Two() );
    
    m_Time = Const<VF32>::Zero();
    
    m_Matrix[0] = MakeMat43( Const<Mat33>::Identity(), translation );        translation += offset_x;
    m_Matrix[1] = MakeMat43( Const<Mat33>::RotateX90CW(), translation );     translation += offset_x;
    m_Matrix[2] = MakeMat43( Const<Mat33>::RotateY90CW(), translation );     translation += offset_x;
    m_Matrix[3] = MakeMat43( Const<Mat33>::RotateZ90CW(), translation );     translation += offset_x;
    m_Matrix[4] = MakeMat43( Const<Mat33>::RotateX90AntiCW(), translation ); translation += offset_x;
    m_Matrix[5] = MakeMat43( Const<Mat33>::RotateY90AntiCW(), translation ); translation += offset_x;
    m_Matrix[6] = MakeMat43( Const<Mat33>::RotateZ90AntiCW(), translation ); translation += offset_x;
    m_Matrix[7] = MakeMat43( Const<Mat33>::RotateX180(), translation );      translation += offset_x;
    m_Matrix[8] = MakeMat43( Const<Mat33>::RotateY180(), translation );      translation += offset_x;
    m_Matrix[9] = MakeMat43( Const<Mat33>::RotateZ180(), translation );      translation += offset_x;
    
    translation += offset_z - ( offset_x * Const<VF32>::Int<10>() );
    
    m_Matrix[10] = MakeMat43( Const<Mat33>::Identity(), translation );        translation += offset_x;
    m_Matrix[11] = MakeMat43( Const<Mat33>::RotateX90CW(), translation );     translation += offset_x;
    m_Matrix[12] = MakeMat43( Const<Mat33>::RotateY90CW(), translation );     translation += offset_x;
    m_Matrix[13] = MakeMat43( Const<Mat33>::RotateZ90CW(), translation );     translation += offset_x;
    m_Matrix[14] = MakeMat43( Const<Mat33>::RotateX90AntiCW(), translation ); translation += offset_x;
    m_Matrix[15] = MakeMat43( Const<Mat33>::RotateY90AntiCW(), translation ); translation += offset_x;
    m_Matrix[16] = MakeMat43( Const<Mat33>::RotateZ90AntiCW(), translation ); translation += offset_x;
    m_Matrix[17] = MakeMat43( Const<Mat33>::RotateX180(), translation );      translation += offset_x;
    m_Matrix[18] = MakeMat43( Const<Mat33>::RotateY180(), translation );      translation += offset_x;
    m_Matrix[19] = MakeMat43( Const<Mat33>::RotateZ180(), translation );      translation += offset_x;
    
    translation += offset_z - ( offset_x * Const<VF32>::Int<10>() );
    
    m_Matrix[20] = MakeMat43( Const<Mat33>::Identity(), translation );        translation += offset_x;
    m_Matrix[21] = MakeMat43( Const<Mat33>::RotateX90CW(), translation );     translation += offset_x;
    m_Matrix[22] = MakeMat43( Const<Mat33>::RotateY90CW(), translation );     translation += offset_x;
    m_Matrix[23] = MakeMat43( Const<Mat33>::RotateZ90CW(), translation );     translation += offset_x;
    m_Matrix[24] = MakeMat43( Const<Mat33>::RotateX90AntiCW(), translation ); translation += offset_x;
    m_Matrix[25] = MakeMat43( Const<Mat33>::RotateY90AntiCW(), translation ); translation += offset_x;
    m_Matrix[26] = MakeMat43( Const<Mat33>::RotateZ90AntiCW(), translation ); translation += offset_x;
    m_Matrix[27] = MakeMat43( Const<Mat33>::RotateX180(), translation );      translation += offset_x;
    m_Matrix[28] = MakeMat43( Const<Mat33>::RotateY180(), translation );      translation += offset_x;
    m_Matrix[29] = MakeMat43( Const<Mat33>::RotateZ180(), translation );      translation += offset_x;
    
    translation += offset_z - ( offset_x * Const<VF32>::Int<10>() );
    
    m_Matrix[30] = MakeMat43( Const<Mat33>::Identity(), translation );        translation += offset_x;
    m_Matrix[31] = MakeMat43( Const<Mat33>::RotateX90CW(), translation );     translation += offset_x;
    m_Matrix[32] = MakeMat43( Const<Mat33>::RotateY90CW(), translation );     translation += offset_x;
    m_Matrix[33] = MakeMat43( Const<Mat33>::RotateZ90CW(), translation );     translation += offset_x;
    m_Matrix[34] = MakeMat43( Const<Mat33>::RotateX90AntiCW(), translation ); translation += offset_x;
    m_Matrix[35] = MakeMat43( Const<Mat33>::RotateY90AntiCW(), translation ); translation += offset_x;
    m_Matrix[36] = MakeMat43( Const<Mat33>::RotateZ90AntiCW(), translation ); translation += offset_x;
    m_Matrix[37] = MakeMat43( Const<Mat33>::RotateX180(), translation );      translation += offset_x;
    m_Matrix[38] = MakeMat43( Const<Mat33>::RotateY180(), translation );      translation += offset_x;
    m_Matrix[39] = MakeMat43( Const<Mat33>::RotateZ180(), translation );      translation += offset_x;
    
    translation += offset_z - ( offset_x * Const<VF32>::Int<10>() );
    
    m_Quaternion[0] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Zero() );
    m_Quaternion[1] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Frac<1,4>() );
    m_Quaternion[2] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Half() );
    m_Quaternion[3] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Frac<3,4>() );
    m_Quaternion[4] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::One() );
    m_Quaternion[5] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Frac<5,4>() );
    m_Quaternion[6] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Frac<6,4>() );
    m_Quaternion[7] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Frac<7,4>() );
    m_Quaternion[8] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Two() );
    m_Quaternion[9] = MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::Pi() * Const<VF32>::Frac<9,4>() );
    
    m_Quaternion[10] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Zero() );
    m_Quaternion[11] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Frac<1,4>() );
    m_Quaternion[12] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Half() );
    m_Quaternion[13] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Frac<3,4>() );
    m_Quaternion[14] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::One() );
    m_Quaternion[15] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Frac<5,4>() );
    m_Quaternion[16] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Frac<6,4>() );
    m_Quaternion[17] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Frac<7,4>() );
    m_Quaternion[18] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Two() );
    m_Quaternion[19] = MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::Pi() * Const<VF32>::Frac<9,4>() );
    
    m_Quaternion[20] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Zero() );
    m_Quaternion[21] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Frac<1,4>() );
    m_Quaternion[22] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Half() );
    m_Quaternion[23] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Frac<3,4>() );
    m_Quaternion[24] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::One() );
    m_Quaternion[25] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Frac<5,4>() );
    m_Quaternion[26] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Frac<6,4>() );
    m_Quaternion[27] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Frac<7,4>() );
    m_Quaternion[28] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Two() );
    m_Quaternion[29] = MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::Pi() * Const<VF32>::Frac<9,4>() );
    
    translation = Const<Vector3>::Zero() - ( offset_z * Const<VF32>::Int<9>() );
    
    m_QuatTrans[0] = MakeQuatTrans( Const<Quaternion>::Identity(), translation );
    m_QuatTrans[1] = MakeQuatTrans( Const<Quaternion>::Identity(), MakeVector3( 0.0f, 0.0f, 3.0f ) );
    m_QuatTrans[2] = MakeQuatTrans( Const<Quaternion>::Identity(), MakeVector3( 0.0f, 0.0f, 2.0f ) );
    m_QuatTrans[3] = MakeQuatTrans( Const<Quaternion>::Identity(), MakeVector3( 0.0f, 0.0f, 1.0f ) );
    
    m_DualQuaternion[0] = MakeDualQuaternion( Const<Quaternion>::Identity(), translation - offset_z * Const<VF32>::Frac<1,10>() );
    m_DualQuaternion[1] = MakeDualQuaternion( m_QuatTrans[1] );
    m_DualQuaternion[2] = MakeDualQuaternion( m_QuatTrans[2] );
    m_DualQuaternion[3] = MakeDualQuaternion( m_QuatTrans[3] );
    
    translation -= offset_z * Const<VF32>::Int<6>();
    
    m_Point[0] = MakePoint3( 0.0f, 0.0f, 0.0f );
    m_Point[1] = MakePoint3( 0.0f, 0.0f, 0.0f );
    m_Point[2] = MakePoint3( 0.0f, 0.0f, 0.0f );
    m_Point[3] = MakePoint3( 0.0f, 0.0f, 0.0f );
    m_Point[4] = MakePoint3( 0.0f, 0.0f, 0.0f );
    m_Point[5] = MakePoint3( 0.0f, 0.0f, 0.0f );
    m_Point[6] = MakePoint3( 0.0f, 0.0f, 0.0f );
    m_Point[7] = MakePoint3( 0.0f, 0.0f, 0.0f );
    m_Point[8] = MakePoint3( 0.0f, 0.0f, 0.0f );
    
    m_Line[0] = MakeLine( m_Point[0], m_Point[1] );
    m_Line[1] = MakeLine( m_Point[1], m_Point[2] );
    m_Line[2] = MakeLine( m_Point[2], m_Point[0] );
    m_Line[3] = MakeLine( m_Point[3], m_Point[4] );
    
    m_Plane[0] = MakePlane( Const<Point3>::Zero(), Const<Normal3>::AxisX() );
    m_Plane[1] = MakePlane( Const<Point3>::Zero(), Const<Normal3>::AxisY() );
    
    m_Sphere[0] = MakeSphere( Const<Point3>::Zero(), Const<VF32>::Zero() );
    m_Sphere[1] = MakeSphere( Const<Point3>::Zero(), Const<VF32>::Zero() );
    
    //m_Triangle[0] = MakeTriangle( MakePoint3(  1.0f, 0.0f,  0.0f ),
    //                              MakePoint3( -1.0f, 0.0f, -1.0f ),
    //                              MakePoint3( -1.0f, 0.0f,  1.0f ) );
    //
    //m_Triangle[1] = MakeTriangle( MakePoint3(  0.0f, 0.0f,  1.0f ),
    //                              MakePoint3(  1.0f, 0.0f, -1.0f ),
    //                              MakePoint3( -1.0f, 0.0f, -1.0f ) );
    
    m_Triangle[0] = MakeTriangle( MakePoint3(  5.0f, 0.0f,  0.0f ),
                                  MakePoint3( -5.0f, 0.0f, -1.0f ),
                                  MakePoint3( -5.0f, 0.0f,  1.0f ) );
    
    m_Triangle[1] = MakeTriangle( MakePoint3(  0.0f, 0.0f,  1.0f ),
                                  MakePoint3(  5.0f, 0.0f, -1.0f ),
                                  MakePoint3( -5.0f, 0.0f, -1.0f ) );
    
    SetInvalidExtent( &m_AxisBox[0] );
    SetInvalidExtent( &m_AxisBox[1] );
    
    if ( 0 )
    {
        Util::C_Random rand;
        
        ALIGN16 F32 data[2048];
        ALIGN16 F32 buffer0[2048];
        ALIGN16 F32 buffer1[2048];
        
        const F32 twoPi( Maths::Pi2 );
        
        for ( S32 i=0; i<1024; i++ )
        {
            const F32 fi( (F32)i * twoPi );
            const F32 ri( rand.FRand( twoPi ) );
            data[i] = sinf( fi / 1024.0f ) + cosf( fi / 1024.0f ) +
                      sinf( fi / 512.0f )  + cosf( fi / 512.0f ) +
                      sinf( fi / 256.0f )  + cosf( fi / 256.0f ) +
                      sinf( fi / 128.0f )  + cosf( fi / 128.0f ) +
                      sinf( fi / 64.0f )   + cosf( fi / 64.0f ) +
                      sinf( fi / 32.0f )   + cosf( fi / 32.0f ) +
                      sinf( fi / 16.0f )   + cosf( fi / 16.0f ) +
                      sinf( fi / 8.0f )    + cosf( fi / 8.0f ) +
                      sinf( fi / 4.0f )    + cosf( fi / 4.0f ) +
                      sinf( ri ) * 0.1f    + cosf( ri ) * 0.1f;
        }
        
        
        for ( S32 j=0; j<3; j++ )
        {
            for ( S32 i=0; i<1024; i++ )
            {
                const F32 fi( (F32)i * twoPi );
                const F32 ri( rand.FRand( twoPi ) );
                buffer0[i] = buffer1[i] = data[i];
            }
            
            LOGINFO( g_Log, "\nRealFFT_Split0" );
            
            
            #if defined( COMPILER_VC )
            #pragma warning( push )
            #pragma warning( disable: 4996 ) // warning C4996: 'Maths::RealFFT_Split0': was declared deprecated
            #endif
            
            RealFFT_Split0( buffer0, 1024 );
            
            #if defined( COMPILER_VC )
            #pragma warning( pop )
            #endif
            
            
            LOGINFO( g_Log, "\nRealFFT_Split" );
            
            
            RealFFT_Split( buffer1, 1024 );
            
            
            LOGINFO( g_Log, "\n" );
            for ( S32 i=0; i<1024; i++ )
            {
                const F32 data0( buffer0[i] );
                const F32 data1( buffer1[i] );
                const F32 delta( fabsf( data1 - data0 ) );
                if ( delta >= 0.001f )
                {
                    LOGINFO( g_Log, "buffer0[%4d] = %+8.3f, buffer1[%4d] = %+8.3f, delta = %+8.3f\n", i, data0, i, data1, delta );
                }
            }
            
            LOGINFO( g_Log, "\nIRealFFT_Split0" );
            
            
            #if defined( COMPILER_VC )
            #pragma warning( push )
            #pragma warning( disable: 4996 ) // warning C4996: 'Maths::IRealFFT_Split0': was declared deprecated
            #endif
            
            IRealFFT_Split0( buffer0, 1024 );
            
            #if defined( COMPILER_VC )
            #pragma warning( pop )
            #endif
            
            
            LOGINFO( g_Log, "\nIRealFFT_Split" );
            
            
            IRealFFT_Split( buffer1, 1024 );
            
            
            LOGINFO( g_Log, "\n" );
            for ( S32 i=0; i<1024; i++ )
            {
                const F32 data0( buffer0[i] );
                const F32 data1( buffer1[i] );
                const F32 delta( fabsf( data1 - data0 ) );
                if ( delta >= 0.001f )
                {
                    LOGINFO( g_Log, "buffer0[%4d] = %+8.3f, buffer1[%4d] = %+8.3f, delta = %+8.3f\n", i, data0, i, data1, delta );
                }
            }
        }
        
        DebugBreak();
    }
}

C_TestSectionMaths::~C_TestSectionMaths()
{
}

void C_TestSectionMaths::OnUpdate()
{
    using namespace Maths;
    //using namespace Maths::GA;
    
    //const VF32 zero( Const<VF32>::Zero() );
    //const VF32 half( Const<VF32>::Half() );
    const VF32 one( Const<VF32>::One() );
    //const VF32 two( Const<VF32>::Two() );
    //const VF32 three( Const<VF32>::Three() );
    //const VF32 four( Const<VF32>::Four() );
    //const VF32 five( Const<VF32>::Five() );
    
    #if 0 && defined( USER_JDENTON )
    
    static GA::Scalar       s;
    static GA::VectorC3     v;
    static GA::BiVectorC3   b;
    static GA::TriVectorC3  t;
    static GA::QuadVectorC3 q;
    static GA::PseudoScalar p;
    static GA::MultiVector  m;
    
    #if 0
    
    LOGINFO( g_Log, "Scalar:          sizeof = % 3d, elements = %d\n", sizeof( GA::Scalar ), GA::Scalar::m_Elements );
    LOGINFO( g_Log, "BladeVector:     sizeof = % 3d, elements = %d\n", sizeof( GA::VectorC3 ), GA::VectorC3::m_Elements );
    LOGINFO( g_Log, "BladeBiVector:   sizeof = % 3d, elements = %d\n", sizeof( GA::BiVectorC3 ), GA::BiVectorC3::m_Elements );
    LOGINFO( g_Log, "BladeTriVector:  sizeof = % 3d, elements = %d\n", sizeof( GA::TriVectorC3 ), GA::TriVectorC3::m_Elements );
    LOGINFO( g_Log, "BladeQuadVector: sizeof = % 3d, elements = %d\n", sizeof( GA::QuadVectorC3 ), GA::QuadVectorC3::m_Elements );
    LOGINFO( g_Log, "PseudoScalar:    sizeof = % 3d, elements = %d\n", sizeof( GA::PseudoScalar ), GA::PseudoScalar::m_Elements );
    LOGINFO( g_Log, "MultiVector:     sizeof = % 3d, elements = %d\n", sizeof( GA::MultiVector ), GA::MultiVector::m_Elements );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Scalar\n" );
    GA::Display( s );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Vector\n" );
    GA::Display( v );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BiVector\n" );
    GA::Display( b );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "TriVector\n" );
    GA::Display( t );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "QuadVector\n" );
    GA::Display( q );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "PseudoScalar\n" );
    GA::Display( p );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "MultiVector\n" );
    GA::Display( m );
    LOGINFO( g_Log, "\n" );
    
    
    LOGINFO( g_Log, "Scalar: index s = %d\n", T_BasisIndex< GA::Scalar::m_ElementBits, BasisBits_s >::m_Index );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Vector: index e0 = %d\n", T_BasisIndex< GA::BladeVector::m_ElementBits, BasisBits_e0 >::m_Index );
    LOGINFO( g_Log, "Vector: index e1 = %d\n", T_BasisIndex< GA::BladeVector::m_ElementBits, BasisBits_e1 >::m_Index );
    LOGINFO( g_Log, "Vector: index e2 = %d\n", T_BasisIndex< GA::BladeVector::m_ElementBits, BasisBits_e2 >::m_Index );
    LOGINFO( g_Log, "Vector: index e3 = %d\n", T_BasisIndex< GA::BladeVector::m_ElementBits, BasisBits_e3 >::m_Index );
    LOGINFO( g_Log, "Vector: index e4 = %d\n", T_BasisIndex< GA::BladeVector::m_ElementBits, BasisBits_e4 >::m_Index );
    LOGINFO( g_Log, "\n" );
    LOGINFO( g_Log, "Vector: index s   = %d\n", T_BasisIndex< GA::BladeVector::m_ElementBits, BasisBits_s >::m_Index );
    LOGINFO( g_Log, "Vector: index e01 = %d\n", T_BasisIndex< GA::BladeVector::m_ElementBits, BasisBits_e01 >::m_Index );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BiVector: index e01 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e01 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e02 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e02 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e03 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e03 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e04 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e04 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e12 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e12 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e13 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e13 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e14 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e14 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e23 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e23 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e24 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e24 >::m_Index );
    LOGINFO( g_Log, "BiVector: index e34 = %d\n", T_BasisIndex< GA::BladeBiVector::m_ElementBits, BasisBits_e34 >::m_Index );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "TriVector: index e012 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e012 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e013 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e013 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e014 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e014 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e023 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e023 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e024 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e024 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e034 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e034 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e123 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e123 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e124 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e124 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e134 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e134 >::m_Index );
    LOGINFO( g_Log, "TriVector: index e234 = %d\n", T_BasisIndex< GA::BladeTriVector::m_ElementBits, BasisBits_e234 >::m_Index );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "QuadVector: index e0123 = %d\n", T_BasisIndex< GA::BladeQuadVector::m_ElementBits, BasisBits_e0123 >::m_Index );
    LOGINFO( g_Log, "QuadVector: index e0124 = %d\n", T_BasisIndex< GA::BladeQuadVector::m_ElementBits, BasisBits_e0124 >::m_Index );
    LOGINFO( g_Log, "QuadVector: index e0134 = %d\n", T_BasisIndex< GA::BladeQuadVector::m_ElementBits, BasisBits_e0134 >::m_Index );
    LOGINFO( g_Log, "QuadVector: index e0234 = %d\n", T_BasisIndex< GA::BladeQuadVector::m_ElementBits, BasisBits_e0234 >::m_Index );
    LOGINFO( g_Log, "QuadVector: index e1234 = %d\n", T_BasisIndex< GA::BladeQuadVector::m_ElementBits, BasisBits_e1234 >::m_Index );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "PseudoScalar: index e01234 = %d\n", T_BasisIndex< GA::PseudoScalar::m_ElementBits, BasisBits_e01234 >::m_Index );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "MultiVector: index s      = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_s >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e0     = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e0 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e1     = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e1 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e2     = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e2 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e3     = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e3 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e4     = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e4 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e01    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e01 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e02    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e02 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e03    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e03 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e04    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e04 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e12    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e12 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e13    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e13 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e14    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e14 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e23    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e23 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e24    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e24 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e34    = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e34 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e012   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e012 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e013   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e013 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e014   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e014 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e023   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e023 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e024   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e024 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e034   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e034 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e123   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e123 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e124   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e124 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e134   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e134 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e234   = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e234 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e0123  = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e0123 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e0124  = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e0124 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e0134  = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e0134 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e0234  = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e0234 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e1234  = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e1234 >::m_Index );
    LOGINFO( g_Log, "MultiVector: index e01234 = %d\n", T_BasisIndex< GA::MultiVector::m_ElementBits, BasisBits_e01234 >::m_Index );
    LOGINFO( g_Log, "\n" );
    
    #endif
    
    #if 1
    
    //static GA::Scalar          s0( one );
    //static GA::PseudoScalar    p0( one );
    //static GA::BladeVector     v0( zero, zero, zero, zero, zero );
    //static GA::BladeBiVector   b0;
    //static GA::BladeTriVector  t0;
    //static GA::BladeQuadVector q0( zero, zero, zero, zero, zero );
    //static GA::MultiVector     m0;
    //static GA::Origin          o0( one );
    //static GA::Infinity        i0( one );
    //
    //GA::Scalar          s1( one ), s2( one );
    //GA::PseudoScalar    p1( one ), p2( one );
    //GA::BladeVector     v1, v2( zero, zero, zero, zero, zero );
    //GA::BladeBiVector   b1, b2;
    //GA::BladeTriVector  t1, t2;
    //GA::BladeQuadVector q1, q2( zero, zero, zero, zero, zero );
    //GA::MultiVector     m1, m2;
    //GA::Origin          o1( one ), o2( one );
    //GA::Infinity        i1( one ), i2( one );
    
    //printf( "s0 grade bits = %02X\n", s0.m_GradeBits );
    //printf( "v0 grade bits = %02X\n", v0.m_GradeBits );
    //printf( "b0 grade bits = %02X\n", b0.m_GradeBits );
    //printf( "t0 grade bits = %02X\n", t0.m_GradeBits );
    //printf( "q0 grade bits = %02X\n", q0.m_GradeBits );
    //printf( "p0 grade bits = %02X\n", p0.m_GradeBits );
    //printf( "m0 grade bits = %02X\n", m0.m_GradeBits );
    //printf( "o0 grade bits = %02X\n", o0.m_GradeBits );
    //printf( "i0 grade bits = %02X\n", i0.m_GradeBits );
    
    //s0 % s0;
    //p0 % p0;
    //v0 % v0;
    //b0 % b0;
    //t0 % t0;
    //q0 % q0;
    ////m0 % m0;
    //o0 % o0;
    //i0 % i0;
    //i0 % o0;
    
    #if 0
    
    LOGINFO( g_Log, "Scalar * Scalar\n" );
    s0 = s1 * s2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Scalar * BladeVector\n" );
    v0 = s1 * v2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Scalar * BladeBiVector\n" );
    b0 = s1 * b2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Scalar * BladeTriVector\n" );
    t0 = s1 * t2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Scalar * BladeQuadVector\n" );
    q0 = s1 * q2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Scalar * PseudoScalar\n" );
    p0 = s1 * p2;
    LOGINFO( g_Log, "\n" );
    
    
    LOGINFO( g_Log, "BladeVector * Scalar\n" );
    v0 = v1 * s2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeVector * BladeVector\n" );
    T_Vector< 5, Conformal3D, (ElementBits)( ElementBits_s | ElementBits_BladeBiVector ) > sbv0 = v1 * v2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeVector * BladeBiVector\n" );
    v1 * b2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeVector * BladeTriVector\n" );
    v1 * t2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeVector * BladeQuadVector\n" );
    v1 * q2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeVector * PseudoScalar\n" );
    v1 * p2;
    LOGINFO( g_Log, "\n" );
    
    
    LOGINFO( g_Log, "BladeBiVector * Scalar\n" );
    b1 * s2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeBiVector * BladeVector\n" );
    b1 * v2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeBiVector * BladeBiVector\n" );
    b1 * b2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeBiVector * BladeTriVector\n" );
    b1 * t2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeBiVector * BladeQuadVector\n" );
    b1 * q2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeBiVector * PseudoScalar\n" );
    b1 * p2;
    LOGINFO( g_Log, "\n" );
    
    
    LOGINFO( g_Log, "BladeTriVector * Scalar\n" );
    t1 * s2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeTriVector * BladeVector\n" );
    t1 * v2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeTriVector * BladeBiVector\n" );
    t1 * b2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeTriVector * BladeTriVector\n" );
    t1 * t2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeTriVector * BladeQuadVector\n" );
    t1 * q2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeTriVector * PseudoScalar\n" );
    t1 * p2;
    LOGINFO( g_Log, "\n" );
    
    
    LOGINFO( g_Log, "BladeQuadVector * Scalar\n" );
    q1 * s2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeQuadVector * BladeVector\n" );
    q1 * v2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeQuadVector * BladeBiVector\n" );
    q1 * b2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeQuadVector * BladeTriVector\n" );
    q1 * t2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeQuadVector * BladeQuadVector\n" );
    q1 * q2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "BladeQuadVector * PseudoScalar\n" );
    q1 * p2;
    LOGINFO( g_Log, "\n" );
    
    
    LOGINFO( g_Log, "PseudoScalar * Scalar\n" );
    p1 * s2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "PseudoScalar * BladeVector\n" );
    p1 * v2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "PseudoScalar * BladeBiVector\n" );
    p1 * b2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "PseudoScalar * BladeTriVector\n" );
    p1 * t2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "PseudoScalar * BladeQuadVector\n" );
    p1 * q2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "PseudoScalar * PseudoScalar\n" );
    p1 * p2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "MultiVector * MultiVector\n" );
    m1 * m2;
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Origin * Origin\n" );
    Print( o1 * o2 );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Infinity * Infinity\n" );
    Print( i1 * i2 );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Origin * Infinity\n" );
    Print( o1 * i2 );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "Infinity * Origin\n" );
    Print( i1 * o2 );
    LOGINFO( g_Log, "\n" );
    
    #endif
    
    
    //typedef T_Vector< ElementBits_e01, 5, Conformal3D > Vector_e01;
    //typedef T_Vector< ElementBits_e34, 5, Conformal3D > Vector_e34;
    //
    //Vector_e01 e01( one );
    //Vector_e34 e34( one );
    //
    //LOGINFO( g_Log, "e01 * e34\n" );
    //Print( e01 * e34 );
    //LOGINFO( g_Log, "\n" );
    
    #endif
    
    #if 0
    
    const GA::Origin origin( one );
    printf( "origin: " ); Print( origin );
    
    const GA::Infinity infinity( one );
    printf( "infinity: " ); Print( infinity );
    
    const GA::Infinity minusInfinity( -infinity );
    printf( "minus infinity: " ); Print( minusInfinity );
    
    #endif
    
    
    #if 0
    
    TestGP();
    TestOP();
    TestIP();
    TestSP();
    TestLC();
    TestRC();
    
    TestDistance();
    TestDualPlane();
    TestDualSphere1();
    TestDualSphere2();
    TestDualSphere3();
    
    TestTranslator();
    TestRotor();
    
    TestFlats();
    
    #if defined( COMPILER_VC )
    __debugbreak();
    #endif
    
    #endif // #if 0
    
    
    #endif
    
    
    #if 0
    {
        //const Mat33 A( MakeMat33( MakeVector3( 1.0f, 0.0f, 0.0f ),
        //                          MakeVector3( 0.0f, 1.0f, 0.0f ),
        //                          MakeVector3( 0.0f, 0.0f, 1.0f ) ) );
        //const Mat33 A( MakeMat33( MakeVector3(  0.0f, 0.0f, 1.0f ),
        //                          MakeVector3( -1.0f, 0.0f, 0.0f ),
        //                          MakeVector3(  0.0f, 1.0f, 0.0f ) ) );
        //const Mat33 A( MakeMat33( MakeVector3( 1.0f, 0.0f, 0.0f ),
        //                          MakeVector3( 0.0f, 1.0f, 0.0f ),
        //                          MakeVector3( 0.0f, 0.0f, 2.0f ) ) );
        //const Mat33 A( MakeMat33( MakeVector3( 0.0f, 0.0f, -1.0f ),
        //                          MakeVector3( 1.0f, 0.0f,  0.0f ),
        //                          MakeVector3( 0.0f, 2.0f,  0.0f ) ) );
        const Mat33 A( MakeMat33( MakeVector3( 1.0f, 0.0f, 0.0f ),
                                  MakeVector3( 0.0f, 2.0f, 0.0f ),
                                  MakeVector3( 0.0f, 0.0f, 3.0f ) ) );
        //const Mat33 A( MakeMat33( MakeVector3(  0.0f, 0.0f, 1.0f ),
        //                          MakeVector3( -2.0f, 0.0f, 0.0f ),
        //                          MakeVector3(  0.0f, 3.0f, 0.0f ) ) );
        //const Mat33 A( MakeMat33( MakeVector3( 1.0f, 2.0f, 3.0f ),
        //                          MakeVector3( 2.0f, 3.0f, 4.0f ),
        //                          MakeVector3( 3.0f, 4.0f, 5.0f ) ) );
        //const Mat33 A( MakeMat33( MakeVector3( 1.0f, 3.0f, 1.0f ),
        //                          MakeVector3( 3.0f, 4.0f, 2.0f ),
        //                          MakeVector3( 1.0f, 2.0f, 1.0f ) ) );
        
        const Mat33 At( Transpose( A ) );
        const Mat33 A_At( A * At );
        const Mat33 At_A( At * A );
        
        Mat33     P;
        ScaleMat3 D;
        
        if ( SymmetricEigenDecomposition( A, &P, &D ) )
        {
            LOGINFO( g_Log, "Eigen Decomposition succeeded!\n" );
            
            const Mat33 A2( P * D * Inverse( P ) );
            
            LOGINFO( g_Log, "A = ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowX( A ) ) ), GetF32( Y( RowX( A ) ) ), GetF32( Z( RowX( A ) ) ) );
            LOGINFO( g_Log, "    ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowY( A ) ) ), GetF32( Y( RowY( A ) ) ), GetF32( Z( RowY( A ) ) ) );
            LOGINFO( g_Log, "    ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowZ( A ) ) ), GetF32( Y( RowZ( A ) ) ), GetF32( Z( RowZ( A ) ) ) );
            
            LOGINFO( g_Log, "P = ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowX( P ) ) ), GetF32( Y( RowX( P ) ) ), GetF32( Z( RowX( P ) ) ) );
            LOGINFO( g_Log, "    ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowY( P ) ) ), GetF32( Y( RowY( P ) ) ), GetF32( Z( RowY( P ) ) ) );
            LOGINFO( g_Log, "    ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowZ( P ) ) ), GetF32( Y( RowZ( P ) ) ), GetF32( Z( RowZ( P ) ) ) );
            
            LOGINFO( g_Log, "D = ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( D ) ), 0.0f, 0.0f );
            LOGINFO( g_Log, "    ( %+6.3f, %+6.3f, %+6.3f )\n", 0.0f, GetF32( Y( D ) ), 0.0f );
            LOGINFO( g_Log, "    ( %+6.3f, %+6.3f, %+6.3f )\n", 0.0f, 0.0f, GetF32( Z( D ) ) );
            
            LOGINFO( g_Log, "A2 = ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowX( A2 ) ) ), GetF32( Y( RowX( A2 ) ) ), GetF32( Z( RowX( A2 ) ) ) );
            LOGINFO( g_Log, "     ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowY( A2 ) ) ), GetF32( Y( RowY( A2 ) ) ), GetF32( Z( RowY( A2 ) ) ) );
            LOGINFO( g_Log, "     ( %+6.3f, %+6.3f, %+6.3f )\n", GetF32( X( RowZ( A2 ) ) ), GetF32( Y( RowZ( A2 ) ) ), GetF32( Z( RowZ( A2 ) ) ) );
        }
        else
            LOGINFO( g_Log, "Eigen Decomposition failed!\n" );
        
        Mat33 sqrtA;
        if ( SymmetricSqrt( A, &sqrtA ) )
        {
            const Mat33 A2( sqrtA * sqrtA );
            LOGINFO( g_Log, "Matrix Sqrt succeeded!\n" );
        }
        else
            LOGINFO( g_Log, "Matrix Sqrt failed!\n" );
        
        #if defined( USER_JDENTON )
        #if defined( COMPILER_VC )
        __debugbreak();
        #else
        DebugBreak();
        #endif
        #endif
    }
    #endif
    
    
    #if 0
    {
        const Point3 triPointA( MakePoint3( 0.0f, 0.0f, 0.0f ) );
        const Point3 triPointB( MakePoint3( 0.0f, 0.0f, 1.0f ) );
        const Point3 triPointC( MakePoint3( 1.0f, 0.0f, 0.0f ) );
        const Triangle triangle( MakeTriangle( triPointA, triPointB, triPointC ) );
        
        const Line line0( MakeLine( MakePoint3( 0.25f,  1.0f, 0.25f ), MakePoint3( 0.25f,  3.0f, 0.25f ) ) ); // false
        const Line line1( MakeLine( MakePoint3( 0.25f,  0.0f, 0.25f ), MakePoint3( 0.25f,  2.0f, 0.25f ) ) ); // true
        const Line line2( MakeLine( MakePoint3( 0.25f, -1.0f, 0.25f ), MakePoint3( 0.25f,  1.0f, 0.25f ) ) ); // true
        const Line line3( MakeLine( MakePoint3( 0.25f, -2.0f, 0.25f ), MakePoint3( 0.25f,  0.0f, 0.25f ) ) ); // true
        const Line line4( MakeLine( MakePoint3( 0.25f, -3.0f, 0.25f ), MakePoint3( 0.25f, -1.0f, 0.25f ) ) ); // false
        
        const Line line5( MakeLine( MakePoint3( 0.01f,  -1.0f, 0.01f ),  MakePoint3( 0.01f,  1.0f, 0.01f ) ) );  // true
        const Line line6( MakeLine( MakePoint3( 0.001f, -1.0f, 0.99f ),  MakePoint3( 0.001f, 1.0f, 0.99f ) ) );  // true
        const Line line7( MakeLine( MakePoint3( 0.99f,  -1.0f, 0.001f ), MakePoint3( 0.99f,  1.0f, 0.001f ) ) ); // true
        
        const Line line8( MakeLine( MakePoint3( 1.0f, -1.0f, 1.0f ), MakePoint3( 1.0f, 1.0f, 1.0f ) ) ); // false
        
        const Line line9( MakeLine( MakePoint3( 0.25f, 1.0f, 0.25f ), MakePoint3( 0.25f, -1.0f, 0.25f ) ) ); // cull-none:true, cull-ccw:false
        
        Point3  hitPoint;
        VF32    hitT;
        Vector2 hitUV;
        
        if ( Intersect( triangle, line0, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line0 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line0 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line0 intersection: false\n" );
        
        if ( Intersect( triangle, line1, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line1 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line1 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line1 intersection: false\n" );
        
        if ( Intersect( triangle, line2, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line2 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line2 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line2 intersection: false\n" );
        
        if ( Intersect( triangle, line3, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line3 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line3 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line3 intersection: false\n" );
        
        if ( Intersect( triangle, line4, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line4 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line4 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line4 intersection: false\n" );
        
        if ( Intersect( triangle, line5, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line5 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line5 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line5 intersection: false\n" );
        
        if ( Intersect( triangle, line6, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line6 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line6 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line6 intersection: false\n" );
        
        if ( Intersect( triangle, line7, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line7 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line7 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line7 intersection: false\n" );
        
        if ( Intersect( triangle, line8, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line8 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line8 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line8 intersection: false\n" );
        
        if ( Intersect( triangle, line9, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line9 intersection: true\n" );
            LOGINFO( g_Log, "triangle-line9 intersection: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line9 intersection: false\n" );
        
        if ( IntersectCullCCW( triangle, line9, &hitPoint, &hitT, &hitUV ) )
        {
            LOGINFO( g_Log, "triangle-line9 intersection cull-ccw: true\n" );
            LOGINFO( g_Log, "triangle-line9 intersection cull-ccw: hitPoint = ( %+6.3f, %+6.3f, %+6.3f ), hitT = %+6.3f, hitUV = ( %+6.3f, %+6.3f )\n",
                GetF32( X( hitPoint ) ), GetF32( Y( hitPoint ) ), GetF32( Z( hitPoint ) ), GetF32( hitT ), GetF32( X( hitUV ) ), GetF32( Y( hitUV ) ) );
        }
        else
            LOGINFO( g_Log, "triangle-line9 intersection cull-ccw: false\n" );
        
        #if defined( USER_JDENTON )
        #if defined( COMPILER_VC )
        __debugbreak();
        #else
        DebugBreak();
        #endif
        #endif
    }
    #endif
    
    
    #if 0 && defined( USER_JDENTON )
    {
        StorageVec4F32_U8   storage_u8;
        StorageVec4F32_U8N  storage_u8n;
        StorageVec4F32_S16  storage_s16;
        StorageVec4F32_S16N storage_s16n;
        StorageVec4F32_F16  storage_f16_a;
        StorageVec4F32_F16  storage_f16_b;
        StorageVec4F32_F16  storage_f16_c;
        StorageVec4F32_F16  storage_f16_d;
        StorageVec4F32_F16  storage_f16_e;
        
        
        const Vec4F32 a( MakeVec4F32( -32767.0f, -255.0f, -1.0f, 0.0f ) );
        const Vec4F32 b( MakeVec4F32( 1.0f, 255.0f, 32767.0f, 65535.0f ) );
        const Vec4F32 c( MakeVec4F32( 0.001f, 0.0001f, 0.00001f, 0.000000f ) );
        const Vec4F32 d( MakeVec4F32( 32767.0f, 65535.0f, 131072.0f, 262144.0f ) );
        const Vec4F32 e( MakeVec4F32( 1.0f / g_zero, g_zero / g_zero, sqrt( -1.0f ), sqrt( -1.0f ) / g_zero ) );
        
        #if 0
        
        Store( a, &storage_u8 ); const Vec4F32 test_a0( Load( &storage_u8 ) );
        Store( b, &storage_u8 ); const Vec4F32 test_b0( Load( &storage_u8 ) );
        
        Store( a, &storage_u8n ); const Vec4F32 test_a1( Load( &storage_u8n ) );
        Store( b, &storage_u8n ); const Vec4F32 test_b1( Load( &storage_u8n ) );
        
        Store( a, &storage_s16 ); const Vec4F32 test_a2( Load( &storage_s16 ) );
        Store( b, &storage_s16 ); const Vec4F32 test_b2( Load( &storage_s16 ) );
        
        Store( a, &storage_s16n ); const Vec4F32 test_a3( Load( &storage_s16n ) );
        Store( b, &storage_s16n ); const Vec4F32 test_b3( Load( &storage_s16n ) );
        
        #endif
        
        Store( a, &storage_f16_a ); const Vec4F32 test_a4( Load( &storage_f16_a ) );
        LOGINFO( g_Log, "test_a4 ( %+15.6f, %+15.6f, %+15.6f, %+15.6f )\n", GetF32(X(test_a4)), GetF32(Y(test_a4)), GetF32(Z(test_a4)), GetF32(W(test_a4)) );
                                                                                                                                                      
        Store( b, &storage_f16_b ); const Vec4F32 test_b4( Load( &storage_f16_b ) );                                                                  
        LOGINFO( g_Log, "test_b4 ( %+15.6f, %+15.6f, %+15.6f, %+15.6f )\n", GetF32(X(test_b4)), GetF32(Y(test_b4)), GetF32(Z(test_b4)), GetF32(W(test_b4)) );
                                                                                                                                                      
        Store( c, &storage_f16_c ); const Vec4F32 test_c4( Load( &storage_f16_c ) );                                                                  
        LOGINFO( g_Log, "test_c4 ( %+15.6f, %+15.6f, %+15.6f, %+15.6f )\n", GetF32(X(test_c4)), GetF32(Y(test_c4)), GetF32(Z(test_c4)), GetF32(W(test_c4)) );
                                                                                                                                                      
        Store( d, &storage_f16_d ); const Vec4F32 test_d4( Load( &storage_f16_d ) );                                                                  
        LOGINFO( g_Log, "test_d4 ( %+15.6f, %+15.6f, %+15.6f, %+15.6f )\n", GetF32(X(test_d4)), GetF32(Y(test_d4)), GetF32(Z(test_d4)), GetF32(W(test_d4)) );
                                                                                                                                                      
        Store( e, &storage_f16_e ); const Vec4F32 test_e4( Load( &storage_f16_e ) );                                                                  
        LOGINFO( g_Log, "test_e4 ( %+15.6f, %+15.6f, %+15.6f, %+15.6f )\n", GetF32(X(test_e4)), GetF32(Y(test_e4)), GetF32(Z(test_e4)), GetF32(W(test_e4)) );
        
        #if 0
        
        const VF32 test_rcp_0( Rcp( Const<VF32>::One() ) );
        const VF32 test_rcp_1( Rcp24( Const<VF32>::One() ) );
        
        const VF32 test_rcp_sqrt_0( RcpSqrt( Const<VF32>::One() ) );
        const VF32 test_rcp_sqrt_1( RcpSqrt24( Const<VF32>::One() ) );
        
        const VF32 test_sqrt_0( Sqrt( Const<VF32>::One() ) );
        const VF32 test_sqrt_1( Sqrt24( Const<VF32>::One() ) );
        
        const VF32 test_div_0( Div( Const<VF32>::One(), Const<VF32>::One() ) );
        const VF32 test_div_1( Div24( Const<VF32>::One(), Const<VF32>::One() ) );
        
        #endif
        
        #if defined( USER_JDENTON )
        #if defined( COMPILER_VC )
        __debugbreak();
        #else
        DebugBreak();
        #endif
        #endif
    }
    #endif
    
    
    
    #if 0 && defined( USER_JDENTON )
    
    const Complex z0( MakeComplex( -1.0f, -1.0f ) );
    const Complex z1( MakeComplex( -1.0f,  0.0f ) );
    const Complex z2( MakeComplex( -1.0f,  1.0f ) );
    const Complex z3( MakeComplex(  0.0f, -1.0f ) );
    const Complex z4( MakeComplex(  0.0f,  0.0f ) );
    const Complex z5( MakeComplex(  0.0f,  1.0f ) );
    const Complex z6( MakeComplex(  1.0f, -1.0f ) );
    const Complex z7( MakeComplex(  1.0f,  0.0f ) );
    const Complex z8( MakeComplex(  1.0f,  1.0f ) );
    
    const Complex sqrt_z0( Sqrt( z0 ) );
    const Complex sqrt_z1( Sqrt( z1 ) );
    const Complex sqrt_z2( Sqrt( z2 ) );
    const Complex sqrt_z3( Sqrt( z3 ) );
    const Complex sqrt_z4( Sqrt( z4 ) );
    const Complex sqrt_z5( Sqrt( z5 ) );
    const Complex sqrt_z6( Sqrt( z6 ) );
    const Complex sqrt_z7( Sqrt( z7 ) );
    const Complex sqrt_z8( Sqrt( z8 ) );
    
    const Complex test1_z0( sqrt_z0 * sqrt_z0 );
    const Complex test1_z1( sqrt_z1 * sqrt_z1 );
    const Complex test1_z2( sqrt_z2 * sqrt_z2 );
    const Complex test1_z3( sqrt_z3 * sqrt_z3 );
    const Complex test1_z4( sqrt_z4 * sqrt_z4 );
    const Complex test1_z5( sqrt_z5 * sqrt_z5 );
    const Complex test1_z6( sqrt_z6 * sqrt_z6 );
    const Complex test1_z7( sqrt_z7 * sqrt_z7 );
    const Complex test1_z8( sqrt_z8 * sqrt_z8 );
    
    const Complex inv_z0( Inverse( z0 ) );
    const Complex inv_z1( Inverse( z1 ) );
    const Complex inv_z2( Inverse( z2 ) );
    const Complex inv_z3( Inverse( z3 ) );
    const Complex inv_z4( Inverse( z4 ) );
    const Complex inv_z5( Inverse( z5 ) );
    const Complex inv_z6( Inverse( z6 ) );
    const Complex inv_z7( Inverse( z7 ) );
    const Complex inv_z8( Inverse( z8 ) );
    
    const Complex test2_z0( Inverse( inv_z0 ) );
    const Complex test2_z1( Inverse( inv_z1 ) );
    const Complex test2_z2( Inverse( inv_z2 ) );
    const Complex test2_z3( Inverse( inv_z3 ) );
    const Complex test2_z4( Inverse( inv_z4 ) );
    const Complex test2_z5( Inverse( inv_z5 ) );
    const Complex test2_z6( Inverse( inv_z6 ) );
    const Complex test2_z7( Inverse( inv_z7 ) );
    const Complex test2_z8( Inverse( inv_z8 ) );
    
    const Complex test3_z0( z0 * inv_z0 );
    const Complex test3_z1( z1 * inv_z1 );
    const Complex test3_z2( z2 * inv_z2 );
    const Complex test3_z3( z3 * inv_z3 );
    const Complex test3_z4( z4 * inv_z4 );
    const Complex test3_z5( z5 * inv_z5 );
    const Complex test3_z6( z6 * inv_z6 );
    const Complex test3_z7( z7 * inv_z7 );
    const Complex test3_z8( z8 * inv_z8 );
    
    const Complex test_pow0( Pow( MakeComplex( 2.0f, 0.0f ), MakeVF32( 3.0f ) ) );
    const Complex test_pow1( Pow( MakeComplex( 2.0f, 0.0f ), MakeVF32( -3.0f ) ) );
    
    const VF32 cube_root_test0( CubeRoot( MakeVF32( 0.0f ) ) );
    const VF32 cube_root_test1( CubeRoot( MakeVF32( 0.5f ) ) );
    const VF32 cube_root_test2( CubeRoot( MakeVF32( 1.0f ) ) );
    const VF32 cube_root_test3( CubeRoot( MakeVF32( 8.0f ) ) );
    const VF32 cube_root_test4( CubeRoot( MakeVF32( 10.0f ) ) );
    const VF32 cube_root_test5( CubeRoot( MakeVF32( 100.0f ) ) );
    const VF32 cube_root_test6( CubeRoot( MakeVF32( -0.5f ) ) );
    const VF32 cube_root_test7( CubeRoot( MakeVF32( -1.0f ) ) );
    const VF32 cube_root_test8( CubeRoot( MakeVF32( -8.0f ) ) );
    const VF32 cube_root_test9( CubeRoot( MakeVF32( -10.0f ) ) );
    const VF32 cube_root_test10( CubeRoot( MakeVF32( 1000.0f ) ) );
    const VF32 cube_root_test11( CubeRoot( MakeVF32( 10000.0f ) ) );
    const VF32 cube_root_test12( CubeRoot( MakeVF32( 100000.0f ) ) );
    const VF32 cube_root_test13( CubeRoot( MakeVF32( 1000000.0f ) ) );
    const VF32 cube_root_test14( CubeRoot( MakeVF32( 10000000.0f ) ) );
    const VF32 cube_root_test15( CubeRoot( MakeVF32( 100000000.0f ) ) );
    const VF32 cube_root_test16( CubeRoot( MakeVF32( 1000000000.0f ) ) );
    const VF32 cube_root_test17( CubeRoot( MakeVF32( 10000000000.0f ) ) );
    const VF32 cube_root_test18( CubeRoot( MakeVF32( 100000000000.0f ) ) );
    const VF32 cube_root_test19( CubeRoot( MakeVF32( 1000000000000.0f ) ) );
    
    #if defined( USER_JDENTON )
    #if defined( COMPILER_VC )
    __debugbreak();
    #else
    DebugBreak();
    #endif
    #endif
    
    #endif
    
    
    
    #if 0 && defined( USER_JDENTON )
    
    {
        Mat44 test;
        SetRowX( &test, MakeVector4( zero, zero, zero, one ) );
        SetRowY( &test, MakeVector4( zero, zero, one, zero ) );
        SetRowZ( &test, MakeVector4( zero, one, zero, zero ) );
        SetRowW( &test, MakeVector4( one, zero, zero, zero ) );
        const Mat44 inverseTest( Inverse( test ) );
        
        const Mat44 I1( test * inverseTest );
        const Mat44 I2( inverseTest * test );
        
        #if defined( USER_JDENTON )
        #if defined( COMPILER_VC )
        __debugbreak();
        #else
        DebugBreak();
        #endif
        #endif
    }
    
    {
        Mat44 test;
        SetRowX( &test, MakeVector4( zero, one, two, three ) );
        SetRowY( &test, MakeVector4( one, two, three, zero ) );
        SetRowZ( &test, MakeVector4( two, three, zero, one ) );
        SetRowW( &test, MakeVector4( three, zero, one, two ) );
        const Mat44 inverseTest( Inverse( test ) );
        
        const Mat44 I1( test * inverseTest );
        const Mat44 I2( inverseTest * test );
        
        #if defined( USER_JDENTON )
        #if defined( COMPILER_VC )
        __debugbreak();
        #else
        DebugBreak();
        #endif
        #endif
    }
    
    {
        Mat66 test;
        SetRow<0>( &test, MakeVector6( zero, zero, zero, zero, zero, two  ) );
        SetRow<1>( &test, MakeVector6( zero, zero, zero, zero, two , zero ) );
        SetRow<2>( &test, MakeVector6( zero, zero, zero, two , zero, zero ) );
        SetRow<3>( &test, MakeVector6( zero, zero, two , zero, zero, zero ) );
        SetRow<4>( &test, MakeVector6( zero, two , zero, zero, zero, zero ) );
        SetRow<5>( &test, MakeVector6( two , zero, zero, zero, zero, zero ) );
        const Mat66 inverseTest( Inverse( test ) );
        
        const Mat66 I1( test * inverseTest );
        const Mat66 I2( inverseTest * test );
        
        #if defined( USER_JDENTON )
        #if defined( COMPILER_VC )
        __debugbreak();
        #else
        DebugBreak();
        #endif
        #endif
    }
    
    {
        Mat66 test;
        SetRow<0>( &test, MakeVector6( zero, one, two, three, four, five ) );
        SetRow<1>( &test, MakeVector6( one, two, three, four, five, zero ) );
        SetRow<2>( &test, MakeVector6( two, three, four, five, zero, one ) );
        SetRow<3>( &test, MakeVector6( three, four, five, zero, one, two ) );
        SetRow<4>( &test, MakeVector6( four, five, zero, one, two, three ) );
        SetRow<5>( &test, MakeVector6( five, zero, one, two, three, four ) );
        const Mat66 inverseTest( Inverse( test ) );
        
        const Mat66 I1( test * inverseTest );
        const Mat66 I2( inverseTest * test );
        
        #if defined( USER_JDENTON )
        #if defined( COMPILER_VC )
        __debugbreak();
        #else
        DebugBreak();
        #endif
        #endif
    }
    
    #endif
    
    
    
    
    #if 0 && defined( USER_JDENTON )
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 pi( Const<VF32>::Pi() );
    const VF32 piDiv5( pi * Const<VF32>::Frac<1,5>() );
    const VF32 limit( Const<VF32>::Frac<1,100>() );
    
    for ( VF32 x=zero; x<=pi; x+=piDiv5 )
    {
        for ( VF32 y=zero; y<=pi; y+=piDiv5 )
        {
            for ( VF32 z=zero; z<=pi; z+=piDiv5 )
            {
                const EulerXYZ xyz1( MakeEulerXYZ( x, y, z ) );
                const EulerXZY xzy1( MakeEulerXZY( x, y, z ) );
                const EulerYXZ yxz1( MakeEulerYXZ( x, y, z ) );
                const EulerYZX yzx1( MakeEulerYZX( x, y, z ) );
                const EulerZXY zxy1( MakeEulerZXY( x, y, z ) );
                const EulerZYX zyx1( MakeEulerZYX( x, y, z ) );
                
                const EulerXYZ xyz2( MakeEulerXYZ( MakeMat33( xyz1 ) ) );
                const EulerXZY xzy2( MakeEulerXZY( MakeMat33( xzy1 ) ) );
                const EulerYXZ yxz2( MakeEulerYXZ( MakeMat33( yxz1 ) ) );
                const EulerYZX yzx2( MakeEulerYZX( MakeMat33( yzx1 ) ) );
                const EulerZXY zxy2( MakeEulerZXY( MakeMat33( zxy1 ) ) );
                const EulerZYX zyx2( MakeEulerZYX( MakeMat33( zyx1 ) ) );
                
                const Mat33 xyz_m1( MakeMat33( xyz1 ) );
                const Mat33 xzy_m1( MakeMat33( xzy1 ) );
                const Mat33 yxz_m1( MakeMat33( yxz1 ) );
                const Mat33 yzx_m1( MakeMat33( yzx1 ) );
                const Mat33 zxy_m1( MakeMat33( zxy1 ) );
                const Mat33 zyx_m1( MakeMat33( zyx1 ) );
                                                     
                const Mat33 xyz_m2( MakeMat33( xyz2 ) );
                const Mat33 xzy_m2( MakeMat33( xzy2 ) );
                const Mat33 yxz_m2( MakeMat33( yxz2 ) );
                const Mat33 yzx_m2( MakeMat33( yzx2 ) );
                const Mat33 zxy_m2( MakeMat33( zxy2 ) );
                const Mat33 zyx_m2( MakeMat33( zyx2 ) );
                
                if ( !Equal( xyz_m1, xyz_m2 ) )
                    __debugbreak();
                
                if ( !Equal( xzy_m1, xzy_m2 ) )
                    __debugbreak();
                
                if ( !Equal( yxz_m1, yxz_m2 ) )
                    __debugbreak();
                
                if ( !Equal( yzx_m1, yzx_m2 ) )
                    __debugbreak();
                
                if ( !Equal( zxy_m1, zxy_m2 ) )
                    __debugbreak();
                
                if ( !Equal( zyx_m1, zyx_m2 ) )
                    __debugbreak();
                
            }
        }
    }
    
    #endif
    
    #if 0
    
    // scale * shear * rotate
    const ScaleMat3 scale( MakeScaleMat3( MakeVF32( 2.0f ),
                                          MakeVF32( -3.0f ),
                                          MakeVF32( 4.0f ) ) );
    
    const ShearMat3 shear( MakeShearMat3( MakeVF32( 5.0f ),
                                          MakeVF32( 6.0f ),
                                          MakeVF32( 7.0f ) ) );
    
    const Mat33 rotate( Const<Mat33>::RotateY180() );
    
    const Mat33 ss( scale * shear );
    const Mat33 ssr( ss * rotate );
    
    ScaleMat3 scale2;
    ShearMat3 shear2;
    Mat33     rotate2;
    
    if ( !DecomposeScaleShearRotate( ssr, &scale2, &shear2, &rotate2 ) )
    {
        __debugbreak();
    }
    
    const Mat33 ss2( scale2 * shear2 );
    const Mat33 ssr2( ss2 * rotate2 );
    
    __debugbreak();
    
    #endif
    
    
	C_View& view = C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	view.DebugFeed().SetInput( GetFlyCameraControls(0) );
    
    const Mat33 rotation_x( MakeMat33_RotX( Const<VF32>::TwoPi() * Const<VF32>::Frac<1,1000>() ) );
    const Mat33 rotation_y( MakeMat33_RotY( Const<VF32>::TwoPi() * Const<VF32>::Frac<1,1000>() ) );
    const Mat33 rotation_z( MakeMat33_RotZ( Const<VF32>::TwoPi() * Const<VF32>::Frac<1,1000>() ) );
    
    m_Time += Const<VF32>::One();
    
    for ( S32 i=10; i<20; i++ )
    {
        m_Matrix[i] = OrthoNormalise( MakeMat43( MakeMat33( m_Matrix[i] ) * rotation_x, RowW( m_Matrix[i] ) ) );
    }
    
    for ( S32 i=20; i<30; i++ )
    {
        m_Matrix[i] = OrthoNormalise( MakeMat43( MakeMat33( m_Matrix[i] ) * rotation_y, RowW( m_Matrix[i] ) ) );
    }
                                                                                
    for ( S32 i=30; i<40; i++ )
    {
        m_Matrix[i] = OrthoNormalise( MakeMat43( MakeMat33( m_Matrix[i] ) * rotation_z, RowW( m_Matrix[i] ) ) );
    }
    
    {
        const Quaternion rotator( MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::TwoPi() * Const<VF32>::Frac<1,1000>() ) );
        
        for ( S32 i=0; i<30; i++ )
        {
            m_Quaternion[i] = Normalise( !rotator * m_Quaternion[i] * rotator );
        }
    }
    
    {
        const Quaternion rotator[4] =
        {
            MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::TwoPi() * Const<VF32>::Frac<1,1000>() ),
            MakeUnitQuaternion( Const<Vector3>::UnitY(), Const<VF32>::TwoPi() * Const<VF32>::Frac<1,1000>() ),
            MakeUnitQuaternion( Const<Vector3>::UnitZ(), Const<VF32>::TwoPi() * Const<VF32>::Frac<1,1000>() ),
            MakeUnitQuaternion( Const<Vector3>::UnitX(), Const<VF32>::TwoPi() * Const<VF32>::Frac<1,1000>() ),
        };
        
        //PMCState pmcstate;
        //
        //StartProfiling( PMC_SETUP_OVERVIEW_PB0T0 );
        //for ( S32 j=0; j<1000000; j++ )
        //{
            for ( S32 i=1; i<4; i++ )
            {
                m_QuatTrans[i] = m_QuatTrans[i] * MakeQuatTrans( rotator[i], Const<Vector3>::Zero() );
            }
        //}
        //const S64 t0 = EndProfiling( &pmcstate, true );
        //
        //StartProfiling( PMC_SETUP_OVERVIEW_PB0T0 );
        //for ( S32 j=0; j<1000000; j++ )
        //{
            for ( S32 i=1; i<4; i++ )
            {
                m_DualQuaternion[i] = m_DualQuaternion[i] * MakeDualQuaternion( rotator[i] );
            }
        //}
        //const S64 t1 = EndProfiling( &pmcstate, true );
        //
        //DebugBreak();
        
        const VF32 time_scale( MakeVF32( 1.0f / 5000.0f ) );
        const VF32 point_scale( MakeVF32( 2.0f ) );
        //const VF32 point_scale2( MakeVF32( 3.0f ) );
        const VF32 sphere_p_scale( MakeVF32( 2.0f ) );
        const VF32 sphere_r_scale( MakeVF32( 1.0f ) );
        
        // first 25 primes: 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
        const VF32 t0( MakeVF32( 2.0f ) );
        const VF32 t1( MakeVF32( 3.0f ) );
        const VF32 t2( MakeVF32( 5.0f ) );
        const VF32 t3( MakeVF32( 7.0f ) );
        const VF32 t4( MakeVF32( 11.0f ) );
        const VF32 t5( MakeVF32( 13.0f ) );
        const VF32 t6( MakeVF32( 17.0f ) );
        const VF32 t7( MakeVF32( 19.0f ) );
        const VF32 t8( MakeVF32( 23.0f ) );
        const VF32 t9( MakeVF32( 29.0f ) );
        const VF32 t10( MakeVF32( 31.0f ) );
        const VF32 t11( MakeVF32( 37.0f ) );
        const VF32 t12( MakeVF32( 41.0f ) );
        const VF32 t13( MakeVF32( 43.0f ) );
        const VF32 t14( MakeVF32( 47.0f ) );
        const VF32 t15( MakeVF32( 53.0f ) );
        const VF32 t16( MakeVF32( 59.0f ) );
        const VF32 t17( MakeVF32( 61.0f ) );
        const VF32 t18( MakeVF32( 67.0f ) );
        const VF32 t19( MakeVF32( 71.0f ) );
        const VF32 t20( MakeVF32( 73.0f ) );
        const VF32 t21( MakeVF32( 79.0f ) );
        const VF32 t22( MakeVF32( 83.0f ) );
        const VF32 t23( MakeVF32( 89.0f ) );
        const VF32 t24( MakeVF32( 97.0f ) );
        const VF32 t25( MakeVF32( 101.0f ) );
        const VF32 t26( MakeVF32( 103.0f ) );
        
        m_Point[0] = MakePoint3( Sin( m_Time * t0 * time_scale ),
                                 Sin( m_Time * t1 * time_scale ),
                                 Sin( m_Time * t2 * time_scale ) ) * point_scale;
        
        m_Point[1] = MakePoint3( Sin( m_Time * t3 * time_scale ),
                                 Sin( m_Time * t4 * time_scale ),
                                 Sin( m_Time * t5 * time_scale ) ) * point_scale;
        
        m_Point[2] = MakePoint3( Sin( m_Time * t6 * time_scale ),
                                 Sin( m_Time * t7 * time_scale ),
                                 Sin( m_Time * t8 * time_scale ) ) * point_scale;
        
        m_Point[3] = MakePoint3( Sin( m_Time * t9  * time_scale ),
                                 Sin( m_Time * t10 * time_scale ),
                                 Sin( m_Time * t11 * time_scale ) ) * point_scale;
        
        m_Point[4] = MakePoint3( Sin( m_Time * t12 * time_scale ),
                                 Sin( m_Time * t13 * time_scale ),
                                 Sin( m_Time * t14 * time_scale ) ) * point_scale;
        
        m_Point[5] = MakePoint3( Sin( m_Time * t15 * time_scale ),
                                 Sin( m_Time * t16 * time_scale ),
                                 Sin( m_Time * t17 * time_scale ) ) * point_scale;
        
        m_Point[6] = MakePoint3( Sin( m_Time * t18 * time_scale ),
                                 Sin( m_Time * t19 * time_scale ),
                                 Sin( m_Time * t20 * time_scale ) ) * point_scale;
        
        m_Point[7] = MakePoint3( Sin( m_Time * t21 * time_scale ),
                                 Sin( m_Time * t22 * time_scale ),
                                 Sin( m_Time * t23 * time_scale ) ) * point_scale;
        
        m_Point[8] = MakePoint3( Sin( m_Time * t24 * time_scale ),
                                 Sin( m_Time * t25 * time_scale ),
                                 Sin( m_Time * t26 * time_scale ) ) * point_scale;
        
        m_Line[0] = MakeLine( m_Point[0], m_Point[1] );
        m_Line[1] = MakeLine( m_Point[1], m_Point[2] );
        m_Line[2] = MakeLine( m_Point[2], m_Point[0] );
        m_Line[3] = MakeLine( m_Point[3], m_Point[4] );
        
        m_Plane[0] = m_Plane[0] * MakeMat33( rotator[2] );
        m_Plane[1] = m_Plane[1] * MakeMat33( rotator[3] );
        
        m_Sphere[0] = MakeSphere( MakePoint3( Sin( m_Time * t0 * time_scale ),
                                              Sin( m_Time * t1 * time_scale ),
                                              Sin( m_Time * t2 * time_scale ) ) * sphere_p_scale,
                                  ( Sin( m_Time * t3 * time_scale ) + one ) * sphere_r_scale );
        
        m_Sphere[1] = MakeSphere( MakePoint3( Sin( m_Time * t4 * time_scale ),
                                              Sin( m_Time * t5 * time_scale ),
                                              Sin( m_Time * t6 * time_scale ) ) * sphere_p_scale,
                                  ( Sin( m_Time * t7 * time_scale ) + one ) * sphere_r_scale );
        
        m_Triangle[0] = m_Triangle[0] * MakeMat33( rotator[2] );
        m_Triangle[1] = m_Triangle[1] * MakeMat33( rotator[3] );
        
        SetInvalidExtent( &m_AxisBox[0] );
        SetInvalidExtent( &m_AxisBox[1] );
        
        UpdateExtent( &m_AxisBox[0], m_Point[0] );
        UpdateExtent( &m_AxisBox[0], m_Point[1] );
        UpdateExtent( &m_AxisBox[0], m_Point[2] );
        UpdateExtent( &m_AxisBox[1], m_Point[3] );
        UpdateExtent( &m_AxisBox[1], m_Point[4] );
        UpdateExtent( &m_AxisBox[1], m_Point[5] );
    }
}

//#if defined( USER_JDENTON )
//#pragma optimize( "", on )
//#endif


static Debug::Bool MathsTestRenderMatrix    ( "MathsTest.Render.Matrix",     false, Debug::Persist() );
static Debug::Bool MathsTestRenderQuaternion( "MathsTest.Render.Quaternion", false, Debug::Persist() );
static Debug::Bool MathsTestRenderQuatTrans ( "MathsTest.Render.QuatTrans",  false, Debug::Persist() );

static Debug::Bool MathsTestRenderDualQuat     ( "MathsTest.Render.DualQuat",      false, Debug::Persist() );
static Debug::Bool MathsTestRenderDualQuatLerp ( "MathsTest.Render.DualQuatLerp",  false, Debug::Persist() );
static Debug::Bool MathsTestRenderDualQuatSlerp( "MathsTest.Render.DualQuatSlerp", false, Debug::Persist() );
static Debug::Bool MathsTestRenderDualQuatDLB  ( "MathsTest.Render.DualQuatDLB",   false, Debug::Persist() );
static Debug::Bool MathsTestRenderDualQuatDIB  ( "MathsTest.Render.DualQuatDIB",   false, Debug::Persist() );

static Debug::Bool MathsTestRenderIntersectLinePlane       ( "MathsTest.Render.Intersect.LinePlane",        false, Debug::Persist() );
static Debug::Bool MathsTestRenderIntersectLineSphere      ( "MathsTest.Render.Intersect.LineSphere",       false, Debug::Persist() );
static Debug::Bool MathsTestRenderIntersectSphereSphere    ( "MathsTest.Render.Intersect.SphereSphere",     false, Debug::Persist() );
static Debug::Bool MathsTestRenderIntersectAxisBoxAxisBox  ( "MathsTest.Render.Intersect.AxisBoxAxisBox",   false, Debug::Persist() );
static Debug::Bool MathsTestRenderIntersectLineLine        ( "MathsTest.Render.Intersect.LineLine",         false, Debug::Persist() );
static Debug::Bool MathsTestRenderIntersectPlanePlane      ( "MathsTest.Render.Intersect.PlanePlane",       false, Debug::Persist() );
static Debug::Bool MathsTestRenderIntersectLineTriangle    ( "MathsTest.Render.Intersect.LineTriangle",     false, Debug::Persist() );
static Debug::Bool MathsTestRenderIntersectTriangleTriangle( "MathsTest.Render.Intersect.TriangleTriangle", false, Debug::Persist() );

static Debug::Bool MathsTestRenderVolumePointSphere     ( "MathsTest.Render.Volume.PointSphere",      false, Debug::Persist() );
static Debug::Bool MathsTestRenderVolumePointAxisBox    ( "MathsTest.Render.Volume.PointAxisBox",     false, Debug::Persist() );
static Debug::Bool MathsTestRenderVolumePointOrientedBox( "MathsTest.Render.Volume.PointOrientedBox", false, Debug::Persist() );

static Debug::Bool MathsTestRenderClosestPointPlane       ( "MathsTest.Render.Closest.PointPlane", false, Debug::Persist() );
static Debug::Bool MathsTestRenderClosestPointSphere      ( "MathsTest.Render.Closest.PointSphere", false, Debug::Persist() );
static Debug::Bool MathsTestRenderClosestPointInfiniteLine( "MathsTest.Render.Closest.PointInfiniteLine", false, Debug::Persist() );
static Debug::Bool MathsTestRenderClosestPointLineSegment ( "MathsTest.Render.Closest.PointLineSegment", false, Debug::Persist() );
static Debug::Bool MathsTestRenderClosestPointTriangle    ( "MathsTest.Render.Closest.PointTriangle", false, Debug::Persist() );

static Debug::Float MathsTestRenderInterpolate( "MathsTest.Render.Interpolate", 0.5f, Debug::Range(0.0f, 1.0f) );

void C_TestSectionMaths::OnRender3DViewWithAA( const S32 viewIndex )
{
    using namespace Maths;
    
    PERF_EVENT( "RenderSectionMathsTest" );
    
    const RGBA red( 0xFF, 0x00, 0x00 );
    const RGBA green( 0x00, 0xFF, 0x00 );
    const RGBA blue( 0x00, 0x00, 0xFF );
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    //const VF32 two( Const<VF32>::Two() );
    const VF32 half( Const<VF32>::Half() );
    
    //Display::Device()->Clear( Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225) );
    Display::Device()->Clear( Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(96, 96, 96, 255) );
    
    Render::DrawGrid();
    
    Util::C_String str;
    str.Printf( " Section Maths Test" );
    Display::FontRender()->Draw( str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255) );
    
    #if 0 // Temp quaternion slerp test
    
    Quaternion qa0, qa1, qa2, qa3, qa4, qa5, qa6, qa7, qa8, qa9;
    Quaternion qb0, qb1, qb2, qb3, qb4, qb5, qb6, qb7, qb8, qb9;
    
    for ( S32 i=0; i<30; i++ )
    {
        const Quaternion qa( m_Quaternion[i] );
        
        qa0 = Slerp( qa, qa, Const<VF32>::Frac<0,4>() );
        qa1 = Slerp( qa, qa, Const<VF32>::Frac<1,4>() );
        qa2 = Slerp( qa, qa, Const<VF32>::Frac<2,4>() );
        qa3 = Slerp( qa, qa, Const<VF32>::Frac<3,4>() );
        qa4 = Slerp( qa, qa, Const<VF32>::Frac<4,4>() );
        
        qa5 = Slerp( qa, -qa, Const<VF32>::Frac<0,4>() );
        qa6 = Slerp( qa, -qa, Const<VF32>::Frac<1,4>() );
        qa7 = Slerp( qa, -qa, Const<VF32>::Frac<2,4>() );
        qa8 = Slerp( qa, -qa, Const<VF32>::Frac<3,4>() );
        qa9 = Slerp( qa, -qa, Const<VF32>::Frac<4,4>() );
        
        const C_Quaternion qb( m_Quaternion[i] );
        
        qb0 = qb.Slerp( qb, HF32( 0.0f  ) );
        qb1 = qb.Slerp( qb, HF32( 0.25f ) );
        qb2 = qb.Slerp( qb, HF32( 0.5f  ) );
        qb3 = qb.Slerp( qb, HF32( 0.75f ) );
        qb4 = qb.Slerp( qb, HF32( 1.0f  ) );
        
        qb5 = qb.Slerp( -qb, HF32( 0.0f  ) );
        qb6 = qb.Slerp( -qb, HF32( 0.25f ) );
        qb7 = qb.Slerp( -qb, HF32( 0.5f  ) );
        qb8 = qb.Slerp( -qb, HF32( 0.75f ) );
        qb9 = qb.Slerp( -qb, HF32( 1.0f  ) );
    }
    
    #endif
    
    if ( MathsTestRenderMatrix )
    {
        for ( S32 i=0; i<40; i++ )
        {
            Draw( m_Matrix[i] );
        }
    }
    
    if ( MathsTestRenderQuaternion )
    {
        const Vector3 vx( Const<Vector3>::UnitX() * half );
        const Vector3 vy( Const<Vector3>::UnitY() * half );
        const Vector3 vz( Const<Vector3>::UnitZ() * half );
        
        for ( S32 i=0; i<10; i++ )
        {
            const S32 qi( i );
            const Point3 p( MakePoint3( (F32)i*2, 0.0f, -4.0f ) );
            Draw( m_Quaternion[qi], p );
            Draw( MakeMat33( m_Quaternion[qi] ), p );
            DrawArrow( p, vx * m_Quaternion[qi], red );
            DrawArrow( p, vy * m_Quaternion[qi], green );
            DrawArrow( p, vz * m_Quaternion[qi], blue );
        }
        
        for ( S32 i=0; i<10; i++ )
        {
            const S32 qi( i + 10 );
            const Point3 p( MakePoint3( (F32)i*2, 0.0f, -6.0f ) );
            Draw( m_Quaternion[qi], p );
            Draw( MakeMat33( m_Quaternion[qi] ), p );
            DrawArrow( p, vx * m_Quaternion[qi], red );
            DrawArrow( p, vy * m_Quaternion[qi], green );
            DrawArrow( p, vz * m_Quaternion[qi], blue );
        }
        
        for ( S32 i=0; i<10; i++ )
        {
            const S32 qi( i + 20 );
            const Point3 p( MakePoint3( (F32)i*2, 0.0f, -8.0f ) );
            Draw( m_Quaternion[qi], p );
            Draw( MakeMat33( m_Quaternion[qi] ), p );
            DrawArrow( p, vx * m_Quaternion[qi], red );
            DrawArrow( p, vy * m_Quaternion[qi], green );
            DrawArrow( p, vz * m_Quaternion[qi], blue );
        }
    }
    
    if ( MathsTestRenderQuatTrans )
    {
        //PMCState pmcstate;
        //StartProfiling( PMC_SETUP_OVERVIEW_PB0T0 );
        //
        //for ( S32 j=0; j<10000; j++ )
        //{
            QuatTrans qt = Const<QuatTrans>::Identity();
            for ( S32 i=0; i<4; i++ )
            {
                qt = m_QuatTrans[i] * qt;
                //qt = Normalise( m_QuatTrans[i] * qt );
                
                //Draw( Quat( qt ), Trans( qt ) );
                Draw( qt );
            }
        //}
        //
        //const S64 t = EndProfiling( &pmcstate, true );
    }
    
    if ( MathsTestRenderDualQuat )
    {
        //PMCState pmcstate;
        //StartProfiling( PMC_SETUP_OVERVIEW_PB0T0 );
        //
        //for ( S32 j=0; j<10000; j++ )
        //{
            const VF32 t( MakeVF32( (F32)MathsTestRenderInterpolate ) );
            const DualQuaternion id( Const<DualQuaternion>::Identity() );
            DualQuaternion dq = id;
            for ( S32 i=0; i<4; i++ )
            {
                dq = m_DualQuaternion[i] * dq;
                
                if ( MathsTestRenderDualQuatLerp )
                {
                    Draw( Lerp( id, Nearest( id, dq ), t ) );
                }
                
                if ( MathsTestRenderDualQuatSlerp )
                {
                    Draw( Slerp( id, Nearest( id, dq ), t ) );
                }
                
                DualQuaternion dqArray[2];
                dqArray[0] = id;
                dqArray[1] = Nearest( id, dq );
                VF32 tArray[2];
                tArray[0] = one - t;
                tArray[1] = t;
                
                if ( MathsTestRenderDualQuatDLB )
                {
                    Draw( DLB( dqArray, tArray, 2 ) );
                }
                
                if ( MathsTestRenderDualQuatDIB )
                {
                    Draw( DIB( dqArray, tArray, 2, Const<VF32>::Frac<1,10>() ) );
                }
            }
        //}
        //
        //const S64 t = EndProfiling( &pmcstate, true );
    }
    
    
    if ( 1 )
    {
        const VF32 zero( Const<VF32>::Zero() );
        const VF32 one( Const<VF32>::One() );
        
        const Vector4 vec4( MakeVector4( one, zero, zero, zero ) );
        const Vector6 vec6( MakeVector6( one, zero, zero, zero, zero, zero ) );
        
        const Mat64 test1( MakeMat64( MakeVector4(  0.0f,  1.0f,  2.0f,  3.0f ),
                                      MakeVector4(  4.0f,  5.0f,  6.0f,  7.0f ),
                                      MakeVector4(  8.0f,  9.0f, 10.0f, 11.0f ),
                                      MakeVector4( 12.0f, 13.0f, 14.0f, 15.0f ),
                                      MakeVector4( 16.0f, 17.0f, 18.0f, 19.0f ),
                                      MakeVector4( 20.0f, 21.0f, 22.0f, 23.0f ) ) );
        
        const Mat46 test2( Transpose( test1 ) );
        const Mat64 test3( Transpose( test2 ) );
        
        const Vector6 test4( test1 * vec4 );
        //const Vector4 test5( test2 * vec6 );
        
        const Vector6 test6( DiagonalMultiply( test1, test2 ) );
        
        DebugBreak();
    }
    
    
    
    if ( MathsTestRenderIntersectLinePlane )
    {
        Draw( m_Line[0], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Line[1], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Line[2], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Plane[0], RGBA( 0x00, 0x00, 0xFF ) );
        
        Point3 p;
        
        if ( Intersect( m_Line[0], m_Plane[0], &p ) )
            Draw( p, RGBA( 0x00, 0xFF, 0x00 ) );
        else
            Draw( p, RGBA( 0xFF, 0x00, 0x00 ) );
        
        if ( Intersect( m_Line[1], m_Plane[0], &p ) )
            Draw( p, RGBA( 0x00, 0xFF, 0x00 ) );
        else
            Draw( p, RGBA( 0xFF, 0x00, 0x00 ) );
        
        if ( Intersect( m_Line[2], m_Plane[0], &p ) )
            Draw( p, RGBA( 0x00, 0xFF, 0x00 ) );
        else
            Draw( p, RGBA( 0xFF, 0x00, 0x00 ) );
    }
    
    if ( MathsTestRenderIntersectLineSphere )
    {
        Draw( m_Line[0], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Line[1], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Line[2], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Sphere[0], RGBA( 0x00, 0x00, 0xFF ) );
        
        QuadraticResult q;
        
        if ( Intersect( m_Line[0], m_Sphere[0], &q ) )
        {
            if ( ( q.m_X1 >= zero ) & ( q.m_X1 <= one ) )
                Draw( Point( m_Line[0], q.m_X1 ), RGBA( 0x00, 0xFF, 0x00 ) );
            else
                Draw( Point( m_Line[0], q.m_X1 ), RGBA( 0xFF, 0x00, 0x00 ) );
            
            if ( ( q.m_X2 >= zero ) & ( q.m_X2 <= one ) )
                Draw( Point( m_Line[0], q.m_X2 ), RGBA( 0x00, 0xFF, 0x00 ) );
            else
                Draw( Point( m_Line[0], q.m_X2 ), RGBA( 0xFF, 0x00, 0x00 ) );
        }
        
        if ( Intersect( m_Line[1], m_Sphere[0], &q ) )
        {
            if ( ( q.m_X1 >= zero ) & ( q.m_X1 <= one ) )
                Draw( Point( m_Line[1], q.m_X1 ), RGBA( 0x00, 0xFF, 0x00 ) );
            else
                Draw( Point( m_Line[1], q.m_X1 ), RGBA( 0xFF, 0x00, 0x00 ) );
            
            if ( ( q.m_X2 >= zero ) & ( q.m_X2 <= one ) )
                Draw( Point( m_Line[1], q.m_X2 ), RGBA( 0x00, 0xFF, 0x00 ) );
            else
                Draw( Point( m_Line[1], q.m_X2 ), RGBA( 0xFF, 0x00, 0x00 ) );
        }
        
        if ( Intersect( m_Line[2], m_Sphere[0], &q ) )
        {
            if ( ( q.m_X1 >= zero ) & ( q.m_X1 <= one ) )
                Draw( Point( m_Line[2], q.m_X1 ), RGBA( 0x00, 0xFF, 0x00 ) );
            else
                Draw( Point( m_Line[2], q.m_X1 ), RGBA( 0xFF, 0x00, 0x00 ) );
            
            if ( ( q.m_X2 >= zero ) & ( q.m_X2 <= one ) )
                Draw( Point( m_Line[2], q.m_X2 ), RGBA( 0x00, 0xFF, 0x00 ) );
            else
                Draw( Point( m_Line[2], q.m_X2 ), RGBA( 0xFF, 0x00, 0x00 ) );
        }
    }
    
    if ( MathsTestRenderIntersectSphereSphere )
    {
        const Point3  c0( Centre( m_Sphere[0] ) );
        const Point3  c1( Centre( m_Sphere[1] ) );
        const VF32    r0( Radius( m_Sphere[0] ) );
        const VF32    r1( Radius( m_Sphere[1] ) );
        const Vector3 v( c1 - c0 );
        const Normal3 n( Normalise( v ) );
        DrawArrow( c0,  n, r0, RGBA( 0x00, 0x00, 0xFF ) );
        DrawArrow( c1, -n, r1, RGBA( 0x00, 0x00, 0xFF ) );
        
        if ( Intersect( m_Sphere[0], m_Sphere[1] ) )
        {
            Draw( m_Sphere[0], RGBA( 0x00, 0xFF, 0x00 ) );
            Draw( m_Sphere[1], RGBA( 0x00, 0xFF, 0x00 ) );
        }
        else
        {
            Draw( m_Sphere[0], RGBA( 0xFF, 0x00, 0x00 ) );
            Draw( m_Sphere[1], RGBA( 0xFF, 0x00, 0x00 ) );
        }
    }
    
    if ( MathsTestRenderIntersectAxisBoxAxisBox )
    {
        Draw( m_Point[0], RGBA( 0xFF, 0x00, 0xFF ) );
        Draw( m_Point[1], RGBA( 0xFF, 0x00, 0xFF ) );
        Draw( m_Point[2], RGBA( 0xFF, 0x00, 0xFF ) );
        Draw( m_Point[3], RGBA( 0xFF, 0x00, 0xFF ) );
        Draw( m_Point[4], RGBA( 0x00, 0xFF, 0xFF ) );
        Draw( m_Point[5], RGBA( 0x00, 0xFF, 0xFF ) );
        Draw( m_Point[6], RGBA( 0x00, 0xFF, 0xFF ) );
        Draw( m_Point[7], RGBA( 0x00, 0xFF, 0xFF ) );
        
        Draw( m_AxisBox[0], RGBA( 0x00, 0x00, 0xFF ) );
        Draw( m_AxisBox[1], RGBA( 0x00, 0x00, 0xFF ) );
        
        AxisBox intersectBox;
        
        //if ( Intersect( m_AxisBox[0], m_AxisBox[1], &intersectBox ) )
        //{
        //    Draw( intersectBox, RGBA( 0x00, 0xFF, 0x00 ) );
        //}
        Intersect( m_AxisBox[0], m_AxisBox[1], &intersectBox );
        if ( Valid( intersectBox ) )
        {
            Draw( intersectBox, RGBA( 0x00, 0xFF, 0x00 ) );
        }
    }
    
    if ( MathsTestRenderIntersectLineLine )
    {
        Draw( m_Line[3], RGBA( 0x00, 0x00, 0xFF ) );
        
        VF32 t1, t2;
        
        if ( Intersect( m_Line[0], m_Line[3], &t1, &t2, Const<VF32>::Frac<1,10>() ) )
        {
            Draw( m_Line[0], RGBA( 0x00, 0xFF, 0x00 ) );
            //Draw( Point( m_Line[0], t1 ), RGBA( 0x00, 0xFF, 0x00 ) );
        }
        else
        {
            Draw( m_Line[0], RGBA( 0xFF, 0xFF, 0x00 ) );
            //Draw( Point( m_Line[0], t1 ), RGBA( 0xFF, 0xFF, 0x00 ) );
            //Draw( Point( m_Line[3], t2 ), RGBA( 0xFF, 0xFF, 0x00 ) );
        }
        Draw( Point( m_Line[0], t1 ), RGBA( 0xFF, 0xFF, 0x00 ) );
        Draw( Point( m_Line[3], t2 ), RGBA( 0xFF, 0xFF, 0x00 ) );
        
        if ( Intersect( m_Line[1], m_Line[3], &t1, &t2, Const<VF32>::Frac<1,10>() ) )
        {
            Draw( m_Line[1], RGBA( 0x00, 0xFF, 0x00 ) );
            //Draw( Point( m_Line[1], t1 ), RGBA( 0x00, 0xFF, 0x00 ) );
        }
        else
        {
            Draw( m_Line[1], RGBA( 0xFF, 0x00, 0xFF ) );
            //Draw( Point( m_Line[1], t1 ), RGBA( 0xFF, 0x00, 0xFF ) );
            //Draw( Point( m_Line[3], t2 ), RGBA( 0xFF, 0x00, 0xFF ) );
        }
        Draw( Point( m_Line[1], t1 ), RGBA( 0xFF, 0x00, 0xFF ) );
        Draw( Point( m_Line[3], t2 ), RGBA( 0xFF, 0x00, 0xFF ) );
        
        if ( Intersect( m_Line[2], m_Line[3], &t1, &t2, Const<VF32>::Frac<1,10>() ) )
        {
            Draw( m_Line[2], RGBA( 0x00, 0xFF, 0x00 ) );
            //Draw( Point( m_Line[2], t1 ), RGBA( 0x00, 0xFF, 0x00 ) );
        }
        else
        {
            Draw( m_Line[2], RGBA( 0x00, 0xFF, 0xFF ) );
            //Draw( Point( m_Line[2], t1 ), RGBA( 0x00, 0xFF, 0xFF ) );
            //Draw( Point( m_Line[3], t2 ), RGBA( 0x00, 0xFF, 0xFF ) );
        }
        Draw( Point( m_Line[2], t1 ), RGBA( 0x00, 0xFF, 0xFF ) );
        Draw( Point( m_Line[3], t2 ), RGBA( 0x00, 0xFF, 0xFF ) );
    }
    
    if ( MathsTestRenderIntersectPlanePlane )
    {
        Draw( m_Plane[0], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Plane[1], RGBA( 0x00, 0x00, 0xFF ) );
        
        Line line;
        if ( Intersect( m_Plane[0], m_Plane[1], &line ) )
        {
            Draw( line, RGBA( 0xFF, 0x00, 0x00 ) );
        }
    }
    
    if ( MathsTestRenderIntersectLineTriangle )
    {
        Point3  p1, p2;
        VF32    t1, t2;
        Vector2 uv1, uv2;
        const VBool hit1( IntersectCullCCW( m_Line[0], m_Triangle[0], &p1, &t1, &uv1 ) );
        const VBool hit2( Intersect( m_Line[3], m_Triangle[0], &p2, &t2, &uv2 ) );
        
        if ( hit1 )
        {
            DrawArrow( m_Line[0], RGBA( 0x00, 0xFF, 0x00 ) ); 
            Draw( p1, RGBA( 0x00, 0x00, 0xFF ) );
        }
        else
        {
            DrawArrow( m_Line[0], RGBA( 0xFF, 0x00, 0x00 ) ); 
            Draw( p1, RGBA( 0xFF, 0xFF, 0x00 ) );
        }
        
        if ( hit2 )
        {
            Draw( m_Line[3], RGBA( 0x00, 0xFF, 0x00 ) ); 
            Draw( p2, RGBA( 0x00, 0x00, 0xFF ) );
        }
        else
        {
            Draw( m_Line[3], RGBA( 0xFF, 0x00, 0x00 ) ); 
            Draw( p2, RGBA( 0xFF, 0xFF, 0x00 ) );
        }
        
        if ( hit1 | hit2 )
            Draw( m_Triangle[0], RGBA( 0x00, 0xFF, 0x00 ) );
        else
            Draw( m_Triangle[0], RGBA( 0xFF, 0x00, 0x00 ) );
    }
    
    if ( MathsTestRenderIntersectTriangleTriangle )
    {
        Draw( m_Triangle[0], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Triangle[1], RGBA( 0x00, 0x00, 0xFF ) );
        
        //PMCState pmcstate;
        //StartProfiling( PMC_SETUP_OVERVIEW_PB0T0 );
        
        Point3 P1( Const<Point3>::Zero() ),
               P2( Const<Point3>::Zero() );
        //for ( S32 j=0; j<1000000; j++ )
        //{
            if ( Intersect( m_Triangle[0], m_Triangle[1], &P1, &P2 ) )
            {
                const Line line( MakeLine( P1, P2 ) );
                Draw( line, RGBA( 0xFF, 0x00, 0x00 ) );
            }
        //}
        //const Line line( MakeLine( P1, P2 ) );
        //Draw( line, RGBA( 0xFF, 0x00, 0x00 ) );
        //const S64 t = EndProfiling( &pmcstate, true );
    }
    
    
    if ( MathsTestRenderVolumePointSphere )
    {
        Draw( m_Sphere[0], RGBA( 0xFF, 0xFF, 0x00 ) );
        Draw( m_Sphere[1], RGBA( 0x00, 0xFF, 0xFF ) );
        
        for ( S32 i=0; i<9; i++ )
        {
            if ( Inside( m_Point[i], m_Sphere[0] ) |
                 Inside( m_Point[i], m_Sphere[1] ) )
            {
                Draw( m_Point[i], RGBA( 0x00, 0xFF, 0x00 ) );
            }
            else
            {
                Draw( m_Point[i], RGBA( 0xFF, 0x00, 0x00 ) );
            }
        }
    }
    
    if ( MathsTestRenderVolumePointAxisBox )
    {
        Draw( m_AxisBox[0], RGBA( 0xFF, 0xFF, 0x00 ) );
        Draw( m_AxisBox[1], RGBA( 0x00, 0xFF, 0xFF ) );
        
        for ( S32 i=0; i<9; i++ )
        {
            if ( Inside( m_Point[i], m_AxisBox[0] ) |
                 Inside( m_Point[i], m_AxisBox[1] ) )
            {
                Draw( m_Point[i], RGBA( 0x00, 0xFF, 0x00 ) );
            }
            else
            {
                Draw( m_Point[i], RGBA( 0xFF, 0x00, 0x00 ) );
            }
        }
    }
    
    if ( MathsTestRenderVolumePointOrientedBox )
    {
        const Mat33 m33( MakeMat33_AxisZY( m_Point[1] - m_Point[0], m_Point[2] - m_Point[0] ) );
        const Mat43 m43( MakeMat43( m33, m_Point[0] ) );
        const OrientedBox obb( MakeOrientedBox( m43, Abs( m_Point[3] - m_Point[0] ) ) );
        //Draw( m_Point[0], RGBA( 0xFF, 0x00, 0xFF ) );
        //Draw( m_Point[1], RGBA( 0x00, 0x00, 0xFF ) );
        //Draw( m_Point[2], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( obb, RGBA( 0x00, 0x00, 0xFF ) );
    }
    
    
    if ( MathsTestRenderClosestPointPlane )
    {
        Draw( m_Point[8], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Plane[0], RGBA( 0x00, 0x00, 0xFF ) );
        const Point3 p( ClosestPointOnPlane( m_Point[8], m_Plane[0] ) );
        Draw( p, RGBA( 0xFF, 0x00, 0x00 ) );
        DrawArrow( m_Point[8], p, RGBA( 0x00, 0xFF, 0xFF ) );
        DrawArrow( Point( m_Plane[0] ), p, RGBA( 0xFF, 0xFF, 0x00 ) );
    }
    
    if ( MathsTestRenderClosestPointSphere )
    {
        Draw( m_Point[8], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Sphere[0], RGBA( 0x00, 0x00, 0xFF ) );
        const Point3 p( ClosestPointOnSphere( m_Point[8], m_Sphere[0] ) );
        Draw( p, RGBA( 0xFF, 0x00, 0x00 ) );
        DrawArrow( m_Point[8], p, RGBA( 0x00, 0xFF, 0xFF ) );
        DrawArrow( Centre( m_Sphere[0] ), p, RGBA( 0xFF, 0xFF, 0x00 ) );
    }
    
    if ( MathsTestRenderClosestPointInfiniteLine )
    {
        Draw( m_Point[8], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Line[0], RGBA( 0x00, 0x00, 0xFF ) );
        const Point3 p( ClosestPointOnInfiniteLine( m_Point[8], m_Line[0] ) );
        Draw( p, RGBA( 0xFF, 0x00, 0x00 ) );
        DrawArrow( m_Point[8], p, RGBA( 0x00, 0xFF, 0xFF ) );
    }
    
    if ( MathsTestRenderClosestPointLineSegment )
    {
        Draw( m_Point[8], RGBA( 0x00, 0xFF, 0x00 ) );
        Draw( m_Line[0], RGBA( 0x00, 0x00, 0xFF ) );
        const Point3 p( ClosestPointOnLineSegment( m_Point[8], m_Line[0] ) );
        Draw( p, RGBA( 0xFF, 0x00, 0x00 ) );
        DrawArrow( m_Point[8], p, RGBA( 0x00, 0xFF, 0xFF ) );
    }
    
    if ( MathsTestRenderClosestPointTriangle )
    {
        const Point3 closest0( ClosestPointOnTriangle( m_Point[8], PointA( m_Triangle[0] ), PointB( m_Triangle[0] ), PointC( m_Triangle[0] ) ) );
        const Point3 closest1( ClosestPointOnTriangle( m_Point[8], PointA( m_Triangle[1] ), PointB( m_Triangle[1] ), PointC( m_Triangle[1] ) ) );
        
        DrawPoint( m_Point[8], RGBA( 0xFF, 0x00, 0xFF ) );
        
        Draw( m_Triangle[0], RGBA( 0x00, 0x00, 0xFF ) );
        Draw( m_Triangle[1], RGBA( 0x00, 0x00, 0xFF ) );
        
        DrawPoint( closest0, RGBA( 0xFF, 0x00, 0x00 ) );
        DrawPoint( closest1, RGBA( 0xFF, 0x00, 0x00 ) );
        
        DrawArrow( m_Point[8], closest0, RGBA( 0x00, 0xFF, 0xFF ) );
        DrawArrow( m_Point[8], closest1, RGBA( 0x00, 0xFF, 0xFF ) );
    }
    
    //DebugBreak();
}

}; // namespace Samples








