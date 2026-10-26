/////////////////////////////////////////////////////////////
// Template used to get constants of a given type.

#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_CONSTANTS_H
#define SOFT_MATHS_CONSTANTS_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_CONSTANTS_H
#define MATHS_CONSTANTS_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#include <limits.h>

#include <float.h>
#if defined( PLATFORM_PPU_SPU )
// JASOND: Redefining FLT_MAX & FLT_MIN for the PS3 since the standard SPU value for FLT_MAX is a NaN...
// FLT_MIN is now defined as the reciprocal of FLT_MAX
#undef FLT_MAX
#undef FLT_MIN
//#define FLT_MAX (1.0e37f)
//#define FLT_MIN (1.0e-37f)
#define FLT_MAX (8.5070591730234615865843651857942e+37f)
#define FLT_MIN (1.1754943508222875079687365372222e-38f)
#endif

namespace Maths //maths constants
{

extern const F32 PiBy8;
extern const F32 PiBy4;
extern const F32 PiBy2;
extern const F32 Pi;
extern const F32 Pi15;
extern const F32 Pi2;
extern const F32 Rcp2Pi;
extern const F64 F64_Pi;
extern const F32 F_Eps2;

template<typename DataType>
struct Constants {};

template <> struct Constants<int> 
{
	static int Zero()	{return 0;}
	static int One()	{return 1;}
	static int Two()	{return 2;}
	static int Min()	{return INT_MIN;}
	static int Max()	{return INT_MAX;}
};

template <> struct Constants<unsigned int> 
{
	static unsigned int Zero()	{return 0;}
	static unsigned int One()	{return 1;}
	static unsigned int Two()	{return 2;}
	static unsigned int Three()	{return 3;}
	static unsigned int Four()	{return 4;}
	static unsigned int Min()	{return 0;}
	static unsigned int Max()	{return 0xffffffff;}
};

template <> struct Constants<float> 
{
	static float Zero()     {return 0.f;}
	static float Epsilon()	{return FLT_EPSILON;}
	static float One()      {return 1.f;}
	static float Half()     {return 0.5f;}
	static float Two()      {return 2.f;}
	static float Three()    {return 3.f;}
	static float Four()		{return 4.f;}
	static float Pi()       {return 3.1415926535897932384626433832795f;}
	static float Min()		{return FLT_MIN;}
	static float Max()		{return FLT_MAX;}
	static float Epsilon2() { return F_Eps2; }
};

template <> struct Constants<double> 
{
	static double Zero()    {return 0.0;}
	static double Epsilon()	{return LDBL_EPSILON;}
	static double One()     {return 1.0;}
	static double Half()    {return 0.5;}
	static double Two()     {return 2.0;}
	static double Three()	{return 3.0;}
	static double Four()	{return 4.0;}
	static double Pi()      {return 3.1415926535897932384626433832795;}
	static double Min()		{return DBL_MIN;}
	static double Max()		{return DBL_MAX;}
	static double Epsilon2() { return F_Eps2; }
};

template <> struct Constants<bool>
{
	static bool True()	{return true;}
	static bool False()	{return false;}
};

/// Namespace for HVec maths code.

#if 1
enum _UNDEFINED         { UNDEFINED };
#else
class _UNDEFINED
{
public:
    _UNDEFINED() {}
    
    #if defined( COMPILER_VC )
    #pragma warning( push )
    #pragma warning( disable : 4700 )
    #endif
    
    operator float () const { float undefined = sqrt( -1.0f ); return undefined; }
    template < typename T_Anything > operator T_Anything () const { T_Anything undefined; memset( &undefined, 0xFF, sizeof( undefined ) ); return undefined; }
    
    #if defined( COMPILER_VC )
    #pragma warning( pop )
    #endif
};
static const _UNDEFINED UNDEFINED;
#endif

enum _ZERO_INIT         { ZERO_INIT };
enum _ZERO_MATRIX       { ZERO_MATRIX };        // This is used to initialise matrices to zero (they are initialised to the identity by default).
enum _IDENTITY_MATRIX   { IDENTITY_MATRIX };

enum E_Init
{
    E_Init_Undefined,
    E_Init_Zero,
    E_Init_Identity
};

}//end namespace Maths

#endif // #if defined( INCLUDE_THIS )
