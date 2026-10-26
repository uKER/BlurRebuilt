#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_UTIL_H
#define SOFT_MATHS_UTIL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_UTIL_H
#define MATHS_UTIL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#if defined (PLATFORM_PS3)
#include <ppu_intrinsics.h>
#endif

#include <core/maths/Constants.h>

namespace Maths
{
//class HF32;

#if defined(PLATFORM_PC) || (defined( WORD_SIZE ) && ( WORD_SIZE == 64 ))
template < typename T > NOALIAS __forceinline const U64 PointerToInt( T* ptr ) __attribute__(( const, always_inline ));
template < typename T > NOALIAS __forceinline const U64 PointerToInt( T* ptr )
{
    return (U64)ptr;
}
#else
template < typename T > NOALIAS __forceinline const U32 PointerToInt( T* ptr ) __attribute__(( const, always_inline ));
template < typename T > NOALIAS __forceinline const U32 PointerToInt( T* ptr )
{
    return (U32)ptr;
}
#endif

inline bool CheckBuggeredFloat( F32 f )
{
    //int i = *(U32*)&f;
    typedef union { F32 f; U32 i; } F2I;
    F2I convert;
    convert.f = f;
    const U32 i = convert.i;
    return ( i & 0x7f000000 ) == 0x7f000000;
}

template < int Size, int Offset >
NOALIAS __forceinline void _Prefetch( void* ptr )
{
    #if defined( _VMX128_ )
    //if ( Size > 0 ) // This causes an internal compiler error...
    //{
    //    __dcbt( Offset, ptr );
    //    _Prefetch< Size - 0x80, Offset + 0x80 >( ptr );
    //}
    if ( Size > 0x000 ) __dcbt( Offset + 0x000, ptr );
    if ( Size > 0x080 ) __dcbt( Offset + 0x080, ptr );
    if ( Size > 0x100 ) __dcbt( Offset + 0x100, ptr );
    if ( Size > 0x180 ) __dcbt( Offset + 0x180, ptr );
    if ( Size > 0x200 ) __dcbt( Offset + 0x200, ptr );
    if ( Size > 0x280 ) __dcbt( Offset + 0x280, ptr );
    if ( Size > 0x300 ) __dcbt( Offset + 0x300, ptr );
    if ( Size > 0x380 ) __dcbt( Offset + 0x380, ptr );
    if ( Size > 0x400 ) __dcbt( Offset + 0x400, ptr );
    if ( Size > 0x480 ) __dcbt( Offset + 0x480, ptr );
    if ( Size > 0x500 ) __dcbt( Offset + 0x500, ptr );
    if ( Size > 0x580 ) __dcbt( Offset + 0x580, ptr );
    if ( Size > 0x600 ) __dcbt( Offset + 0x600, ptr );
    if ( Size > 0x680 ) __dcbt( Offset + 0x680, ptr );
    if ( Size > 0x700 ) __dcbt( Offset + 0x700, ptr );
    if ( Size > 0x780 ) __dcbt( Offset + 0x780, ptr );
	#endif
	#if defined( PLATFORM_PS3 )
    if ( Size > 0x000 ) __dcbt( Offset + 0x000 + (U8*)ptr );
    if ( Size > 0x080 ) __dcbt( Offset + 0x080 + (U8*)ptr );
    if ( Size > 0x100 ) __dcbt( Offset + 0x100 + (U8*)ptr );
    if ( Size > 0x180 ) __dcbt( Offset + 0x180 + (U8*)ptr );
    if ( Size > 0x200 ) __dcbt( Offset + 0x200 + (U8*)ptr );
    if ( Size > 0x280 ) __dcbt( Offset + 0x280 + (U8*)ptr );
    if ( Size > 0x300 ) __dcbt( Offset + 0x300 + (U8*)ptr );
    if ( Size > 0x380 ) __dcbt( Offset + 0x380 + (U8*)ptr );
    if ( Size > 0x400 ) __dcbt( Offset + 0x400 + (U8*)ptr );
    if ( Size > 0x480 ) __dcbt( Offset + 0x480 + (U8*)ptr );
    if ( Size > 0x500 ) __dcbt( Offset + 0x500 + (U8*)ptr );
    if ( Size > 0x580 ) __dcbt( Offset + 0x580 + (U8*)ptr );
    if ( Size > 0x600 ) __dcbt( Offset + 0x600 + (U8*)ptr );
    if ( Size > 0x680 ) __dcbt( Offset + 0x680 + (U8*)ptr );
    if ( Size > 0x700 ) __dcbt( Offset + 0x700 + (U8*)ptr );
    if ( Size > 0x780 ) __dcbt( Offset + 0x780 + (U8*)ptr );
	#endif
}

template < typename T >
NOALIAS __forceinline void Prefetch( T const * const __restrict ptr )
{
    // Prefetch cache lines containing aligned structure
    _Prefetch< sizeof( T ), 0 >( (void*)ptr );
}

template < typename T >
NOALIAS __forceinline void PrefetchUnaligned( T const * const __restrict ptr )
{
    // Prefetch cache lines containing aligned structure
    _Prefetch< sizeof( T ), 0 >( (void*)ptr );
    
    if ( sizeof( T ) > 1 )
    {
        // Prefetch cache line containing last byte
        _Prefetch< 1, sizeof( T ) - 1 >( (void*)ptr );
    }
}

template < typename T >
NOALIAS __forceinline void PrefetchNext( T const * const __restrict ptr )
{
    // Prefetch cache lines containing aligned structure
    _Prefetch< sizeof( T ), sizeof( T ) >( (void*)ptr );
}

template < typename T >
NOALIAS __forceinline void PrefetchNextUnaligned( T const * const __restrict ptr )
{
    // Prefetch cache lines containing aligned structure
    _Prefetch< sizeof( T ), sizeof( T ) >( (void*)ptr );
    
    if ( sizeof( T ) > 1 )
    {
        // Prefetch cache line containing last byte
        _Prefetch< 1, sizeof( T ) + sizeof( T ) - 1 >( (void*)ptr );
    }
}



/// Templated min
template < class T >
NOALIAS __forceinline const T Min( const T a, const T b )
{
    return a < b ? a : b;
}

/// Templated max
template < class T >
NOALIAS __forceinline const T Max( const T a, const T b )
{
    return a > b ? a : b;
}

// [TODO - test, verify codegen... ]
NOALIAS __forceinline bool	BitToBool(const S32 flags, const int shift) {
	const char tmp = (char)((flags>>shift)&1);
	return	(bool&) tmp;
};
NOALIAS __forceinline bool	BitToBool(const U32 flags, const int shift) {
	const char tmp = (char)((((S32)flags)>>shift)&1);
	return	(bool&) tmp;
};

/// IntToBool Returns: ( src == 0 ? false [0x00] : true [0x01] )
NOALIAS __forceinline const bool IntToBool( const S32 src )
{
    const char result( (char)( (U32)( src | -src ) >> 31 ) );
    return (bool&)result;
}

/// IntToMask [S32] Returns: ( src == 0 ? 0x00000000 : 0xFFFFFFFF )
NOALIAS __forceinline const S32 IntToMask( const S32 src )
{
    return ( src | -src ) >> 31;
}
NOALIAS __forceinline const U32 IntToMask( const U32 src )
{
    return (U32)(( (S32)src | -(S32)src ) >> 31);
}

/// IntToMask [S64] Returns: ( src == 0 ? 0x0000000000000000 : 0xFFFFFFFFFFFFFFFF )
NOALIAS __forceinline const S64 IntToMask( const S64 src )
{
    return ( src | -src ) >> 63;
}

/// Bool to S32 mask
NOALIAS __forceinline const S32 BoolToMaskS32( const bool src )
{
    //const S32 temp( (S32&)src );
    const S32 temp( (S32)(char)src );
    //const S32 mask( ( temp | -temp ) >> 31 );
    const S32 mask( -temp ); // false(0) -> 0x00000000, true(1) -> 0xFFFFFFFF
    return mask;
}

/// Bool to U32 mask
NOALIAS __forceinline const U32 BoolToMaskU32( const bool src )
{
    //const S32 temp( (S32&)src );
    const S32 temp( (S32)(char)src );
    //const U32 mask( (U32)( ( temp | -temp ) >> 31 ) );
    const U32 mask( (U32)-temp ); // false(0) -> 0x00000000, true(1) -> 0xFFFFFFFF
    return mask;
}

template<int BIT_INDEX>
NOALIAS __forceinline const U32 BoolToBit(const bool src)
{
	return	((U32)src)<<BIT_INDEX;
}

NOALIAS __forceinline int BoolToInt( const bool x )
{
	const char c =((const char&)x);
	return	(int) c;
}

// TODO - #IFNDEF PLATFORM_XENON these can be done directly.
__forceinline F32	BoolToFloat(const int boolInInt) {	// see also BoolToFSel for fsel-able version
	// ASSERT ==0 or 1
	static const F32	returnValues[2]={0.f,1.f};
	return	returnValues[boolInInt];
}
__forceinline F32	BoolToFloat(bool x) {
	return	BoolToFloat( BoolToInt(x) );
};

__forceinline F32	BoolToFSel(const int boolInInt) {
	// ASSERT ==0 or 1
	const F32	returnValues[2]={-1.f,1.f};
	return	returnValues[boolInInt];
}
__forceinline F32	BoolToFSel(const bool x) {
	return	BoolToFSel( BoolToInt(x) );
};
// Load Sign From Bit: 
__forceinline F32
LoadFloatSignFromBit(	const	U32 v, const int shiftIndex	)
{
	return	BoolToFSel( (int)(((v>>shiftIndex)&1)^1) );
}


//	Select: xenon opt to go via int load.
__forceinline F32	Select(bool	selector, F32 x, F32 y) 
{
#if defined(PLATFORM_PS3) && !defined(PLATFORM_SPU)
	static const F32 fsel_values[2]= { -1.f, 1.f };
	return	(F32) __fsel(  fsel_values[ BoolToInt(selector) ], x, y);
#else
    return selector ? x : y;
#endif
}

__forceinline F32	fsel(bool	selector, F32 x, F32 y)  { return Select(selector, x,y); }


class	CBool 
{
	S32	m_Data;	// Class treats this as 0 or 1.
	CBool(const S32 value, int)				{ m_Data = value; }	// hack to enable direct construction,
																// distinct from CBool(S32) which converts.
																// must be a better way..
	#define CONSTRUCT_BOOL(DATA) CBool(DATA,0)
public:
	friend CBool operator && (const CBool& a, const CBool & b);
	friend CBool operator || (const CBool& a, const CBool & b);
	friend CBool operator & (const CBool& a, const CBool & b);
	friend CBool operator | (const CBool& a, const CBool & b);
	friend CBool operator ^ (const CBool& a, const CBool & b);
	__forceinline CBool(const bool	src)	{ m_Data = (const char&) src;	}

	// assigning to bool is returns performance warning error, so we should construct ?
	explicit CBool(int src)					{ m_Data = (S8) IntToBool(src); }
	CBool(const CBool& src)					{ m_Data = src.m_Data; }

	// TODO: Verify codegen. I have seen ref-casting break register assignment.
	// need to check there's no better way to get from an int to a bool.
	// does it rather want to be the shifting malarchy.
	
	#if !defined( PLATFORM_PS3 )
	operator bool() const						{ const char temp = (char)m_Data; return (bool&)temp; }
	#else
	operator bool() const						{ return (bool)(char)m_Data; }
	#endif
	
	operator int()	const						{ return m_Data;			}
	operator unsigned int()	const				{ return m_Data;			}

	CBool	operator!() const { return	CONSTRUCT_BOOL(m_Data^1);	}
};

__forceinline CBool operator&& (const CBool& a, const CBool & b) {
	return	CONSTRUCT_BOOL(a.m_Data & b.m_Data);
}
__forceinline CBool operator|| (const CBool& a, const CBool & b) {
	return	CONSTRUCT_BOOL(a.m_Data | b.m_Data);
}
__forceinline CBool operator& (const CBool& a, const CBool & b) {
	return	CONSTRUCT_BOOL(a.m_Data & b.m_Data);
}
__forceinline CBool operator| (const CBool& a, const CBool & b) {
	return	CONSTRUCT_BOOL(a.m_Data | b.m_Data);
}
__forceinline CBool operator^ (const CBool& a, const CBool & b) {
	return	CONSTRUCT_BOOL(a.m_Data ^ b.m_Data);
}


__forceinline bool	MakeBool(bool x) { return x;}
__forceinline bool	MakeBool(S8	src) { return IntToBool(src);}

// Test if value is in a certain range; return 0xF..:0
// saves on combining the sub-expressions...
__forceinline S32	IsOutOfRangeMask(const S32 value, const S32 min, const S32 max) {
	const S32 out = (value-min)|(max - value);
	return	out>>31;
}
__forceinline S32	IsOutOfRange(const S32 value, const S32 min, const S32 max) {
	return	IsOutOfRangeMask(value,min,max) &1;
}
__forceinline S32	IsInRange(const S32 value, const S32 min, const S32 max) {
	return	(~IsOutOfRangeMask(value,min,max)) &1;
}

// TODO - version returning FSEL / VSEL masks.
__forceinline bool	IsInRange(const F32 value, const F32 min, const F32 max) {
	return	(value>=min) && (value<=max);
}


/// SplatBit [U32/S32] Returns: ( (src & (1<<bitIndex)== 0) ? 0x0000000000000000 : 0xFFFFFFFFFFFFFFFF )
template<int bitIndex>
U32	SplatBit(const U32 bits) {	
	const int shift = 31-bitIndex;
	return (U32)(((S32)bits) << shift) >> shift;
}
template<int bitIndex>
S32	SplatBit(const S32 bits) {	
	const int shift = 31-bitIndex;
	return (((S32)bits)<<shift) >> shift;
}

__forceinline U32 PackU32(U16 a, U16 b) { return ((U32)(a))|(((U32)b)<<16); }

// (value==wrap-1?0:value+1)
template<typename INT>
__forceinline INT IncWrap(INT value, INT wrap) {
	enum { NumBits = sizeof(INT)*8 };
	value++;
	INT wrapMinusOne = wrap-1;
	INT overstepMask = (wrapMinusOne-value)>>(NumBits-1);
	return value & (~overstepMask);
}
// (value>0?value-1:wrap-1)
template<typename INT>
__forceinline INT DecWrap(INT value, INT wrap) 
{
	enum { NumBits = sizeof(INT)*8 };
	value--;
	int underflowMask = value >> (NumBits-1);
	return value | ((wrap-1) & underflowMask);
}

// TODO - optimize this
template<typename T, typename Y>
inline S32		EqAny(const T value, const Y	a, const Y b) {
	return	BoolToInt(value==a)|BoolToInt(value==b);
}
template<typename T, typename Y>
inline S32		EqAny(const T value, const Y a, const Y b, const Y c) {
	return	BoolToInt(value==a)|BoolToInt(value==b)|BoolToInt(value==c);
}
template<typename T,typename Y>
inline S32		EqAny(const T value, const Y a, const Y b, const Y c,const Y d) {
	return	BoolToInt(value==a)|BoolToInt(value==b)|BoolToInt(value==c)|BoolToInt(value==d);
}
// mask flips true if a < b
inline S32	MaskLT(const S32 a, const S32 b) {
	return	(a-b)>>31;
}
inline S32	MaskLE(const S32 a, const S32 b) {
	return	((a-b)-1)>>31;
}

inline S32	LT(const S32 a, const S32 b) {
	return	((U32)(a-b))>>31;
}
inline S32	LE(const S32 a, const S32 b) {
	return	((U32)(a-b-1))>>31;
}
inline S32	GE(const S32 a, const S32 b) { return 1^(LT(a,b)); }
inline S32	MaskGE(const S32 a, const S32 b) { return ~(LT(a,b)); }







template < class T >
NOALIAS __forceinline T Clamped( const T value, const T lo, const T hi )
{
    #if defined( _DEBUG )
    assert(hi >= lo);
    #endif
    
    return Max( lo, Min( hi, value ) );
}

#ifndef PLATFORM_LINUX

#if defined(PLATFORM_PC) || defined(PLATFORM_SPU)
INLINE F32 __fsel(F32 cmp, F32 a, F32 b) { return (cmp>=0)?a:b; }
#endif

// WTL - synonymous, Like HLSL.
template <class T>
__forceinline T Clamp(const T value, const T lo, const T hi) { return Clamped(value,lo,hi); }
template <class T>
__forceinline T ClampSigned(const T value, const T maxAbsValue) { return Clamped(value,-maxAbsValue,maxAbsValue); }
// WTL - adding 'Set****' for assignment forms.
template <class T>
__forceinline void SetClamp(T& value, const T lo, const T hi) { value = Clamped(value,lo,hi); }

__forceinline float  Clamp01(const float value) { return Clamped(value,0.f, 1.f); }
__forceinline void SetClamp01(float& value) { value = Clamped(value,0.f, 1.f); }
// [WTL -Wrap round. Postfix 'wrap' - wrap the value from 0 to limit
// [TODO - assert a>=-limit && a<limit*2]
__forceinline float Wrap(const float a, const float limit)	{
	float asub = a-limit;
	float tmp = (float)__fsel(asub, asub, a);
	tmp = (float)__fsel(tmp, tmp, tmp+limit);
	return tmp;
}
// [ TODO - ASSERT inc<limit, ASSERT a>=0 && a<limit]
__forceinline float AddWrap(const float a, const float inc, const float limit) {
	float tmp=a+inc;
	float asub = tmp-limit;
	return	(float)__fsel(asub, asub, tmp);
}
// [ TODO - ASSERT inc<limit, ASSERT a>=0 && a<limit]
__forceinline float SubWrap(const float a, const float dec, const float limit) 
{
	float tmp = a-dec;
	return	(float)__fsel(tmp, tmp, tmp+limit);
}
__forceinline float Wrap01(const float a)	{
	return	Wrap(a,1.0f);
}
__forceinline void SetWrap01(float& a)	{
	a = Wrap(a,1.0f);
}

#endif



// Linear interpolation from f0 to f1 based on t.
NOALIAS __forceinline float Lerp(float f0, float f1, float t)
{
    return f0 + t * (f1 - f0);
}

// Add two values and subtract the product.
NOALIAS __forceinline float AddSmooth(float a, float b)
{
    assert(a >= 0.f && a <= 1.f);
    assert(b >= 0.f && b <= 1.f);
    return a + b - a * b;
}

/// If val < -max, returns -max, and if val > max, returns max, otherwise returns val.
template<class T> 
NOALIAS __forceinline T ClampedAbs( const T val, const T max )
{
    #if defined( _DEBUG )
    assert(max >= Maths::Constants<T>::Zero());
    #endif
    
    return Min( Max( val, -max ), max );
}

template < class T >
__forceinline T Sign( const T x )
{
    return ( x < Maths::Constants<T>::Zero() ? -Maths::Constants<T>::One() : 
           ( x > Maths::Constants<T>::Zero() ? Maths::Constants<T>::One() : Maths::Constants<T>::Zero() ) );
}



/// Templated abs
template < typename T >
NOALIAS __forceinline const T Abs( const T a )
{
    return ::abs( a );
}

#if !defined(PLATFORM_PS3)
//#if !defined( PLATFORM_XENON ) // xenon optimised versions further below

template <>
NOALIAS __forceinline const float Abs( const float a )
{
    return ::fabsf( a );
}

template <>
NOALIAS __forceinline const double Abs( const double a )
{
   return ::fabs( a );
}

#if defined( COMPILER_VC )

template <>
NOALIAS __forceinline const S64 Abs( const S64 a )
{
    return ::_abs64( a );
}

#endif

#endif


/// Templated square
template < typename T >
NOALIAS __forceinline const T Square( const T x )
{
    return x * x;
}

/// Templated sqrt
template < typename T >
NOALIAS __forceinline const T Sqrt( const T src )
{
    const T zero( 0.0 );
    return ( src >= zero ) ? ::sqrt( src ) : zero;
}

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( disable: 4723 )
#endif

/// Templated reciprocal
template < typename T >
NOALIAS __forceinline const T Reciprocal( const T src )
{
    const T zero( 0.0 );
	//return ( Abs(src) > (1.0f / FLT_MAX) ) ? Maths::Constants<T>::One() / src : zero;
	return ( Abs( src ) > FLT_MIN ) ? Maths::Constants<T>::One() / src : zero; // New: FLT_MIN = 1/FLT_MAX
}

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

/// Templated reciprocal sqrt
template < typename T >
NOALIAS __forceinline const T ReciprocalSqrt( const T src )
{
    const T zero( 0.0 );
    return ( src > zero ) ? Maths::Constants<T>::One() / ::sqrt( src ) : zero;
}

template < typename T >
NOALIAS __forceinline const T Sin( const T a )
{
    return ::sin( a );
}

template < typename T >
NOALIAS __forceinline const T Cos( const T a )
{
    return ::cos( a );
}

template < typename T >
NOALIAS __forceinline void SinCos(T* s, T* c, const T a)
{
    *s = ::sin( a );
    *c = ::cos( a );
}

template < typename T >
NOALIAS __forceinline T SmoothStep( const T a )
{
	const T a2 = a * a;
	const T a3 = a2 * a;
	return 3.0f * a2 - 2.0f * a3;
}

/// Templated Select if test >= 0
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result Select_GE_Zero( const T_Compare test, const T_Result result_if_test_GE_zero, const T_Result result_if_test_LT_zero )
{
    return ( test >= Maths::Constants<T_Compare>::Zero() ) ? result_if_test_GE_zero : result_if_test_LT_zero;
}

/// Templated Select if test >= 0 ( for non-const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline T_Result* Select_GE_Zero( const T_Compare test, T_Result* result_if_test_GE_zero, T_Result* result_if_test_LT_zero )
{
    return ( test >= Maths::Constants<T_Compare>::Zero() ) ? result_if_test_GE_zero : result_if_test_LT_zero;
}

/// Templated Select if test >= 0 ( for const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result* Select_GE_Zero( const T_Compare test, const T_Result* result_if_test_GE_zero, const T_Result* result_if_test_LT_zero )
{
    return ( test >= Maths::Constants<T_Compare>::Zero() ) ? result_if_test_GE_zero : result_if_test_LT_zero;
}



/// Templated Select if Equal
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result Select_EQ( const T_Compare a, const T_Compare b, const T_Result result_if_true, const T_Result result_if_false )
{
    return ( a == b ) ? result_if_true : result_if_false;
}

/// Templated Select if Not-Equal
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result Select_NE( const T_Compare a, const T_Compare b, const T_Result result_if_true, const T_Result result_if_false )
{
    return ( a != b ) ? result_if_true : result_if_false;
}

/// Templated Select if Greater-Than
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result Select_GT( const T_Compare a, const T_Compare b, const T_Result result_if_true, const T_Result result_if_false )
{
    return ( a > b ) ? result_if_true : result_if_false;
}

/// Templated Select if Less-Than
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result Select_LT( const T_Compare a, const T_Compare b, const T_Result result_if_true, const T_Result result_if_false )
{
    return ( a < b ) ? result_if_true : result_if_false;
}

/// Templated Select if Greater-Equal
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result Select_GE( const T_Compare a, const T_Compare b, const T_Result result_if_true, const T_Result result_if_false )
{
    return ( a >= b ) ? result_if_true : result_if_false;
}

/// Templated Select if Less-Equal
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result Select_LE( const T_Compare a, const T_Compare b, const T_Result result_if_true, const T_Result result_if_false )
{
    return ( a <= b ) ? result_if_true : result_if_false;
}



/// Templated Select if Equal ( for non-const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline T_Result* Select_EQ( const T_Compare a, const T_Compare b, T_Result* result_if_true, T_Result* result_if_false )
{
    return ( a == b ) ? result_if_true : result_if_false;
}

/// Templated Select if Not-Equal ( for non-const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline T_Result* Select_NE( const T_Compare a, const T_Compare b, T_Result* result_if_true, T_Result* result_if_false )
{
    return ( a != b ) ? result_if_true : result_if_false;
}

/// Templated Select if Greater-Than ( for non-const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline T_Result* Select_GT( const T_Compare a, const T_Compare b, T_Result* result_if_true, T_Result* result_if_false )
{
    return ( a > b ) ? result_if_true : result_if_false;
}

/// Templated Select if Less-Than ( for non-const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline T_Result* Select_LT( const T_Compare a, const T_Compare b, T_Result* result_if_true, T_Result* result_if_false )
{
    return ( a < b ) ? result_if_true : result_if_false;
}

/// Templated Select if Greater-Equal ( for non-const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline T_Result* Select_GE( const T_Compare a, const T_Compare b, T_Result* result_if_true, T_Result* result_if_false )
{
    return ( a >= b ) ? result_if_true : result_if_false;
}

/// Templated Select if Less-Equal ( for non-const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline T_Result* Select_LE( const T_Compare a, const T_Compare b, T_Result* result_if_true, T_Result* result_if_false )
{
    return ( a <= b ) ? result_if_true : result_if_false;
}



/// Templated Select if Equal ( for const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result* Select_EQ( const T_Compare a, const T_Compare b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    return ( a == b ) ? result_if_true : result_if_false;
}

/// Templated Select if Not-Equal ( for const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result* Select_NE( const T_Compare a, const T_Compare b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    return ( a != b ) ? result_if_true : result_if_false;
}

/// Templated Select if Greater-Than ( for const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result* Select_GT( const T_Compare a, const T_Compare b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    return ( a > b ) ? result_if_true : result_if_false;
}

/// Templated Select if Less-Than ( for const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result* Select_LT( const T_Compare a, const T_Compare b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    return ( a < b ) ? result_if_true : result_if_false;
}

/// Templated Select if Greater-Equal ( for const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result* Select_GE( const T_Compare a, const T_Compare b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    return ( a >= b ) ? result_if_true : result_if_false;
}

/// Templated Select if Less-Equal ( for const pointers to T_Result )
template < typename T_Compare, typename T_Result >
NOALIAS __forceinline const T_Result* Select_LE( const T_Compare a, const T_Compare b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    return ( a <= b ) ? result_if_true : result_if_false;
}




/*
NOALIAS __forceinline const S32 Select( const bool test, const S32 result_if_true, const S32 result_if_false )
{
    return test ? result_if_true : result_if_false;
}
*/

NOALIAS __forceinline U32 Select( const bool test, const U32 result_if_true, const U32 result_if_false )
{
#if defined(PLATFORM_PS3) && !defined(PLATFORM_SPU)

	// [TODO -codegen, was this ok without it ?]
	U32	mask = BoolToMaskU32(test);
	return	(mask & result_if_true) | ((~mask)&result_if_false);
#else
    return test ? result_if_true : result_if_false;
#endif
	
}
NOALIAS __forceinline S32 Select( const bool test, const S32 result_if_true, const S32 result_if_false )
{
#if defined(PLATFORM_PS3) && !defined(PLATFORM_SPU)

	// [TODO -codegen, was this ok without it ?]
	S32	mask = BoolToMaskS32(test);
	return	(mask & result_if_true) | ((~mask)&result_if_false);
#else
	// spu is BRILLIANT!!! it can ACTUALLY HANDLE CONDITIONALS!!!
    return test ? result_if_true : result_if_false;
#endif
	
}
NOALIAS __forceinline S32 Select( const S32 boolInInt_Test, const S32 result_if_true, const S32 result_if_false )
{
#if defined(PLATFORM_PS3) && !defined(PLATFORM_SPU)

	// [TODO -codegen, was this ok without it ?]
	S32	mask = IntToMask(boolInInt_Test);
	return	(mask & result_if_true) | ((~mask)&result_if_false);
#else
	// spu is BRILLIANT.. it can actually compile conditionals !
    return boolInInt_Test ? result_if_true : result_if_false;
#endif
	
}



/// Templated Equal with epsilon
template < typename T >
NOALIAS __forceinline bool Equal( const T a, const T b, const T epsilon )
{
    return Abs( a - b ) < epsilon;
}

/// Templated F32 Degrees to Radians
template < typename T >
NOALIAS __forceinline T DegToRad( const T d )
{
    return d * ( Maths::Constants<T>::Pi() / ((T)(180.0)) );
}

/// Templated F32 Radians to Degrees
template < typename T >
NOALIAS __forceinline T RadToDeg( const T r )
{
    return r * ( ((T)180.0) / Maths::Constants<T>::Pi() );
}





NOALIAS __forceinline const F32 Floor( const F32 f ) __attribute__(( const, always_inline ));
NOALIAS __forceinline const F32 Floor( const F32 f )
{
    return floorf( f );
}

NOALIAS __forceinline const F32 Ceil( const F32 f ) __attribute__(( const, always_inline ));
NOALIAS __forceinline const F32 Ceil( const F32 f )
{
    return ceilf( f );
}


























#if defined(PLATFORM_PS3)
//#if defined( PLATFORM_XENON )

//
// ******************************************
// ***** XENON SPECIFIC IMPLEMENTATIONS *****
// ******************************************
//

/// Xenon float & double template specialisations for min, max, abs & sqrt

template <>
NOALIAS __forceinline const U32 Min( const U32 a, const U32 b )
{
    const U32 mask( (U32)( ( S64(b) - S64(a) ) >> 63 ) );
    return a + ( ( b - a ) & mask );
}

template <>
NOALIAS __forceinline const S32 Min( const S32 a, const S32 b )
{
    const U32 mask( (U32)( ( S64(b) - S64(a) ) >> 63 ) );
    return a + ( ( b - a ) & mask );
}

template <>
NOALIAS __forceinline const U32 Max( const U32 a, const U32 b )
{
    const U32 mask( (U32)( ( S64(a) - S64(b) ) >> 63 ) );
    return a + ( ( b - a ) & mask );
}

template <>
NOALIAS __forceinline const S32 Max( const S32 a, const S32 b )
{
    const U32 mask( (U32)( ( S64(a) - S64(b) ) >> 63 ) );
    return a + ( ( b - a ) & mask );
}

//template <>
//NOALIAS __forceinline const U64 Min( const U64 a, const U64 b )
//{
//    const U64 mask( (S64)( (S64)b - (S64)a ) >> 63 );
//    return a + ( ( b - a ) & mask );
//}

//template <>
//NOALIAS __forceinline const S64 Min( const S64 a, const S64 b )
//{
//    const S64 mask( ( b - a ) >> 63 );
//    return a + ( ( b - a ) & mask );
//}

//template <>
//NOALIAS __forceinline const U64 Max( const U64 a, const U64 b )
//{
//    const U64 mask( (S64)( (S64)a - (S64)b ) >> 63 );
//    return a + ( ( b - a ) & mask );
//}

//template <>
//NOALIAS __forceinline const S64 Max( const S64 a, const S64 b )
//{
//    const S64 mask( ( a - b ) >> 63 );
//    return a + ( ( b - a ) & mask );
//}

template <>
NOALIAS __forceinline const float Min( const float a, const float b )
{
#if defined( PLATFORM_PS3 )
	return __fsels( a - b, b, a );
#endif
}

template <>
NOALIAS __forceinline const double Min( const double a, const double b )
{
    return __fsel( a - b, b, a );
}

template <>
NOALIAS __forceinline const float Max( const float a, const float b )
{
#if defined( PLATFORM_PS3 )
	return __fsels( a - b, a, b );
#endif
}

template <>
NOALIAS __forceinline const double Max( const double a, const double b )
{
    return __fsel( a - b, a, b );
}



template <>
NOALIAS __forceinline float Sign( const float x )
{
#if defined( PLATFORM_PS3 )
	return __fsels( x, __fsels( -x, 0.0f, 1.0f ), -1.0f );
#endif
}
    
template <>
NOALIAS __forceinline double Sign( const double x )
{
    return float( __fsel( x, __fsel( -x, 0.0, 1.0 ), -1.0 ) );
}
    
template <>
NOALIAS __forceinline const float Abs( const float a )
{
#if defined( PLATFORM_PS3 )
	return __fabsf( a );
#endif
}

template <>
NOALIAS __forceinline const double Abs( const double a )
{
    return __fabs( a );
}

template <>
NOALIAS __forceinline const float Reciprocal( const float src )
{
#if defined( PLATFORM_PS3 )
    const float one( 1.0f );
    const float B( src );
    const float y0( __fres( src ) );                      // Estimate: 8 bit accuracy
    const float y1( y0 + ( y0 * ( one - ( B * y0 ) ) ) ); // NR iteration 1: 16 bit accuracy
    const float y2( y1 + ( y1 * ( one - ( B * y1 ) ) ) ); // NR iteration 2: 24 bit accuracy
    return __fsels( __fabsf( y2 ), y2, 0.0f );            // Returns 0.0 if y2 is a NAN
#endif
}

template <>
NOALIAS __forceinline const double Reciprocal( const double src )
{
    const double one( 1.0 );
    const double B( src );
    const double y0( __fres( src ) );                      // Estimate: 8 bit accuracy
    const double y1( y0 + ( y0 * ( one - ( B * y0 ) ) ) ); // NR iteration 1: 16 bit accuracy
    const double y2( y1 + ( y1 * ( one - ( B * y1 ) ) ) ); // NR iteration 2: 32 bit accuracy
    const double y3( y2 + ( y2 * ( one - ( B * y2 ) ) ) ); // NR iteration 3: 53 bit accuracy
    //return __fsel( y3, y3, __fsel( -y3, y3, 0.0 ) ); // Returns 0.0 if y3 is a NAN
    return __fsel( __fabs( y3 ), y3, 0.0 ); // Returns 0.0 if y3 is a NAN
}

template <>
NOALIAS __forceinline const float ReciprocalSqrt( const float src )
{
#if defined( PLATFORM_PS3 )
    const float half( 0.5f );
    const float one( 1.0f );
    const float B( src );
    const float y0( __frsp( __frsqrte( B ) ) );                       // Estimate: 8 bit accuracy
    const float y1( y0 + ( half * y0 * ( one - ( B * y0 * y0 ) ) ) ); // NR iteration 1: 16 bit accuracy
    const float y2( y1 + ( half * y1 * ( one - ( B * y1 * y1 ) ) ) ); // NR iteration 2: 24 bit accuracy
    return __fsels( y2, y2, 0.0f );                                   // Returns 0.0 if y2 is a NAN
#endif
}

template <>
NOALIAS __forceinline const double ReciprocalSqrt( const double src )
{
    const double half( 0.5 );
    const double one( 1.0 );
    const double B( src );
    const double y0( __frsqrte( B ) );                                 // Estimate: 8 bit accuracy
    const double y1( y0 + ( half * y0 * ( one - ( B * y0 * y0 ) ) ) ); // NR iteration 1: 16 bit accuracy
    const double y2( y1 + ( half * y1 * ( one - ( B * y1 * y1 ) ) ) ); // NR iteration 2: 32 bit accuracy
    const double y3( y2 + ( half * y2 * ( one - ( B * y2 * y2 ) ) ) ); // NR iteration 3: 53 bit accuracy
    return __fsel( y3, y3, 0.0 ); // Returns 0.0 if y3 is a NAN
}

template <>
NOALIAS __forceinline const float Sqrt( const float a )
{
#if defined( PLATFORM_PS3 )
    return __fsels( -a, 0.0f, a * ReciprocalSqrt( a ) ); // Returns 0.0 if a <= 0.0
#endif
}

template <>
NOALIAS __forceinline const double Sqrt( const double a )
{
    return __fsel( -a, 0.0, a * ReciprocalSqrt( a ) ); // Returns 0.0 if a <= 0.0
}






/// Templated Select if test >= 0
template <>
NOALIAS __forceinline const F32 Select_GE_Zero( const F32 test, const F32 result_if_test_GE_zero, const F32 result_if_test_LT_zero )
{
#if defined( PLATFORM_PS3 )
    return __fsels( test, result_if_test_GE_zero, result_if_test_LT_zero );
#endif
}




/// Templated Select if a == b
template <>
NOALIAS __forceinline const F32 Select_EQ( const F32 a, const F32 b, const F32 result_if_true, const F32 result_if_false )
{
#if defined( PLATFORM_PS3 )
    return __fsels( -Abs( a - b ) /* >= 0.0f */, result_if_true, result_if_false );
#endif
}

/// Templated Select if a != b
template <>
NOALIAS __forceinline const F32 Select_NE( const F32 a, const F32 b, const F32 result_if_true, const F32 result_if_false )
{
#if defined( PLATFORM_PS3 )
    return __fsels( -Abs( a - b ) /* >= 0.0f */, result_if_false, result_if_true );
#endif
}

/// Templated Select if a >= b
template <>
NOALIAS __forceinline const F32 Select_GE( const F32 a, const F32 b, const F32 result_if_true, const F32 result_if_false )
{
#if defined( PLATFORM_PS3 )
    return __fsels( a - b /* >= 0.0f */, result_if_true, result_if_false );
#endif
}

/// Templated Select if a < b
template <>
NOALIAS __forceinline const F32 Select_LT( const F32 a, const F32 b, const F32 result_if_true, const F32 result_if_false )
{
#if defined( PLATFORM_PS3 )
    return __fsels( a - b /* < 0.0f */, result_if_false, result_if_true );
#endif
}

/// Templated Select if a <= b
template <>
NOALIAS __forceinline const F32 Select_LE( const F32 a, const F32 b, const F32 result_if_true, const F32 result_if_false )
{
#if defined( PLATFORM_PS3 )
    return __fsels( b - a /* >= 0.0f */, result_if_true, result_if_false );
#endif
}

/// Templated Select if a > b
template <>
NOALIAS __forceinline const F32 Select_GT( const F32 a, const F32 b, const F32 result_if_true, const F32 result_if_false )
{
#if defined( PLATFORM_PS3 )
    return __fsels( b - a /* < 0.0f */, result_if_false, result_if_true );
#endif
}





/// Templated Select if test >= 0
template <>
NOALIAS __forceinline const F64 Select_GE_Zero( const F64 test, const F64 result_if_test_GE_zero, const F64 result_if_test_LT_zero )
{
    return __fsel( test, result_if_test_GE_zero, result_if_test_LT_zero );
}

/// Templated Select if test >= 0
template <>
NOALIAS __forceinline const S32 Select_GE_Zero( const S32 test, const S32 result_if_test_GE_zero, const S32 result_if_test_LT_zero )
{
    //const S32 mask = ( test >= 0 ) ? 0x00000000 : 0xFFFFFFFF;
    const S32 mask( test >> 31 );
    return result_if_test_GE_zero + ( ( result_if_test_LT_zero - result_if_test_GE_zero ) & mask );
}

/// Templated Select if test >= 0
template <>
NOALIAS __forceinline const U32 Select_GE_Zero( const S32 test, const U32 result_if_test_GE_zero, const U32 result_if_test_LT_zero )
{
    //const S32 mask = ( test >= 0 ) ? 0x00000000 : 0xFFFFFFFF;
    const S32 mask( test >> 31 );
    return result_if_test_GE_zero + ( ( result_if_test_LT_zero - result_if_test_GE_zero ) & mask );
}

/// Templated Select if test >= 0 ( for pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline T_Result* Select_GE_Zero( const S32 test, T_Result* result_if_test_GE_zero, T_Result* result_if_test_LT_zero )
{
    //const S32 mask = ( test >= 0 ) ? 0x00000000 : 0xFFFFFFFF;
    const S32 mask( test >> 31 );
    return (T_Result*)( (S32)result_if_test_GE_zero + ( ( (S32)result_if_test_LT_zero - (S32)result_if_test_GE_zero ) & mask ) );
}

/// Templated Select if test >= 0 ( for const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline const T_Result* Select_GE_Zero( const S32 test, const T_Result* result_if_test_GE_zero, const T_Result* result_if_test_LT_zero )
{
    //const S32 mask = ( test >= 0 ) ? 0x00000000 : 0xFFFFFFFF;
    const S32 mask( test >> 31 );
    return (const T_Result*)( (S32)result_if_test_GE_zero + ( ( (S32)result_if_test_LT_zero - (S32)result_if_test_GE_zero ) & mask ) );
}




/// Templated Select if Equal
template <>
NOALIAS __forceinline const S32 Select_EQ( const S32 a, const S32 b, const S32 result_if_true, const S32 result_if_false )
{
    const S32 mask( IntToMask( a - b ) );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Not-Equal
template <>
NOALIAS __forceinline const S32 Select_NE( const S32 a, const S32 b, const S32 result_if_true, const S32 result_if_false )
{
    const S32 mask( ~IntToMask( a - b ) );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Greater-Than
template <>
NOALIAS __forceinline const S32 Select_GT( const S32 a, const S32 b, const S32 result_if_true, const S32 result_if_false )
{
    const S32 mask( ( a - b - 1 ) >> 31 );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Less-Than
template <>
NOALIAS __forceinline const S32 Select_LT( const S32 a, const S32 b, const S32 result_if_true, const S32 result_if_false )
{
    const S32 mask( ( b - a - 1 ) >> 31 );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Greater-Equal
template <>
NOALIAS __forceinline const S32 Select_GE( const S32 a, const S32 b, const S32 result_if_true, const S32 result_if_false )
{
    const S32 mask( ( a - b ) >> 31 );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Less-Equal
template <>
NOALIAS __forceinline const S32 Select_LE( const S32 a, const S32 b, const S32 result_if_true, const S32 result_if_false )
{
    const S32 mask( ( b - a ) >> 31 );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}



/// Templated Select if Equal
template <>
NOALIAS __forceinline const U32 Select_EQ( const S32 a, const S32 b, const U32 result_if_true, const U32 result_if_false )
{
    const S32 mask( IntToMask( a - b ) );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Not-Equal
template <>
NOALIAS __forceinline const U32 Select_NE( const S32 a, const S32 b, const U32 result_if_true, const U32 result_if_false )
{
    const S32 mask( ~IntToMask( a - b ) );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Greater-Than
template <>
NOALIAS __forceinline const U32 Select_GT( const S32 a, const S32 b, const U32 result_if_true, const U32 result_if_false )
{
    const S32 mask( ( a - b - 1 ) >> 31 );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Less-Than
template <>
NOALIAS __forceinline const U32 Select_LT( const S32 a, const S32 b, const U32 result_if_true, const U32 result_if_false )
{
    const S32 mask( ( b - a - 1 ) >> 31 );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Greater-Equal
template <>
NOALIAS __forceinline const U32 Select_GE( const S32 a, const S32 b, const U32 result_if_true, const U32 result_if_false )
{
    const S32 mask( ( a - b ) >> 31 );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}

/// Templated Select if Less-Equal
template <>
NOALIAS __forceinline const U32 Select_LE( const S32 a, const S32 b, const U32 result_if_true, const U32 result_if_false )
{
    const S32 mask( ( b - a ) >> 31 );
    return result_if_true + ( ( result_if_false - result_if_true ) & mask );
}



/// Templated Select if Equal ( for non-const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline T_Result* Select_EQ( const S32 a, const S32 b, T_Result* result_if_true, T_Result* result_if_false )
{
    const S32 mask( IntToMask( a - b ) );
    return (T_Result*)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Not-Equal ( for non-const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline T_Result* Select_NE( const S32 a, const S32 b, T_Result* result_if_true, T_Result* result_if_false )
{
    const S32 mask( ~IntToMask( a - b ) );
    return (T_Result*)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Greater-Than ( for non-const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline T_Result* Select_GT( const S32 a, const S32 b, T_Result* result_if_true, T_Result* result_if_false )
{
    const S32 mask( ( a - b - 1 ) >> 31 );
    return (T_Result*)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Less-Than ( for non-const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline T_Result* Select_LT( const S32 a, const S32 b, T_Result* result_if_true, T_Result* result_if_false )
{
    const S32 mask( ( b - a - 1 ) >> 31 );
    return (T_Result*)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Greater-Equal ( for non-const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline T_Result* Select_GE( const S32 a, const S32 b, T_Result* result_if_true, T_Result* result_if_false )
{
    const S32 mask( ( a - b ) >> 31 );
    return (T_Result*)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Less-Equal ( for non-const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline T_Result* Select_LE( const S32 a, const S32 b, T_Result* result_if_true, T_Result* result_if_false )
{
    const S32 mask( ( b - a ) >> 31 );
    return (T_Result*)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}




/// Templated Select if Equal ( for const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline const T_Result* Select_EQ( const S32 a, const S32 b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    const S32 mask( IntToMask( a - b ) );
    return (T_Result const *)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Not-Equal ( for const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline const T_Result* Select_NE( const S32 a, const S32 b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    const S32 mask( ~IntToMask( a - b ) );
    return (T_Result const *)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Greater-Than ( for const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline const T_Result* Select_GT( const S32 a, const S32 b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    const S32 mask( ( a - b - 1 ) >> 31 );
    return (T_Result const *)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Less-Than ( for const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline const T_Result* Select_LT( const S32 a, const S32 b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    const S32 mask( ( b - a - 1 ) >> 31 );
    return (T_Result const *)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Greater-Equal ( for const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline const T_Result* Select_GE( const S32 a, const S32 b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    const S32 mask( ( a - b ) >> 31 );
    return (T_Result const *)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}

/// Templated Select if Less-Equal ( for const pointers to T_Result )
template < typename T_Result >
NOALIAS __forceinline const T_Result* Select_LE( const S32 a, const S32 b, const T_Result* result_if_true, const T_Result* result_if_false )
{
    const S32 mask( ( b - a ) >> 31 );
    return (T_Result const *)( (S32)result_if_true + ( ( (S32)result_if_false - (S32)result_if_true ) & mask ) );
}





//#if defined ( PLATFORM_XENON )

// TODO - for PC this might be better another way.
template < typename T >
NOALIAS __forceinline T IncWrapped( const T value, const T max ) 
{
    const T incValue = value + 1;
    return Select_LT( incValue, max, incValue, 0 );
}
template < typename T >
NOALIAS __forceinline T DecWrapped( const T value, const T max ) 
{
    // TODO might a Select_GEZ be applicable here.
    const T decValue = value - 1;
    return Select_GE( decValue, 0, decValue, max-1 );
}
//#endif

#endif	// PLATFORM XENON

// See Wiki / writing branchless code for docs..
// Common helpers that stop you needing to think it's a Min when enstoping to a Max and vica versa.
// [WTL - TODO - rename ... is 'lim' a more obscure name than 'clamp' or 'saturate'. 'clamp' is one-way.]
#define SetAddClamp	SetAddLim
#define SetSubClamp	SetSubClamp
template<typename T>	__forceinline void	SetMax(T& dst, const T src) { dst = Max(dst,src);	}
template<typename T>	__forceinline void	SetMin(T& dst, const T src) { dst = Min(dst,src);	}
template<typename T>	__forceinline void	SetAddLim(T& dst, const T inc, const T limit) { dst = Min(dst+inc,limit);	}
template<typename T>	__forceinline void	SetSubLim(T& dst, const T inc, const T limit) { dst = Max(dst-=inc,limit);	}
template<typename T>	__forceinline T	AddLim(const T src, const T inc, const T limit) { return	Min(src+inc,limit);		}
template<typename T>	__forceinline T	SubLim(const T src, const T inc, const T limit) { return	Max(src-inc,limit);	}
template<typename T>	__forceinline void	SetMinMax(T& dstMin, T& dstMax, const T src) { dstMin = Min(dstMin,src);dstMax = Max(dstMax,src);	}
// "SetMinMax" == "UpdateExtents" precisely named.
template<typename T>	__forceinline void	SetMinMin(T& min,T& max, const T src) { SetMin(min,src); SetMax(max,src);	}

template<>
__forceinline float	SubLim(const float value, const float decrement, const float endstop) {
	return	Max(value-decrement, endstop);
}
template<>
__forceinline float	AddLim(const float value, const float increment, const float endstop) {
	return	Min(value+increment, endstop);
}
template<>
__forceinline void	SetSubLim(float& dstValue, const float decrement, const float endstop) {
	dstValue = SubLim(dstValue, decrement, endstop);
}
template<>
__forceinline void	SetAddLim(float& dstValue, const float increment, const float endstop) {
	dstValue = AddLim(dstValue, increment, endstop);
}

template<>
__forceinline void	SetMin(float& dst, const float src) {
	dst = Min(dst,src);
}
template<>
__forceinline void	SetMax(float& dst, const float src) {
	dst = Max(dst,src);
}

template<>
__forceinline void	SetMinMax(float& minValueOut, float& maxValueOut, const float valueIn) {
	SetMax(maxValueOut,valueIn);
	SetMin(minValueOut,valueIn);
}


// TODO - ARCH_PPC DEFINE
#if defined(PLATFORM_PS3)
	inline float FSel(const float cmp, const float a, const float b) { return __fsels(cmp,a,b); }
#else
	inline float FSel(const float cmp,const float a, const float b) { return (cmp>=0)?a:b; }
#endif

__forceinline float ZeroIf_LT(const float value, const float deadzone) {
	return	Select_GE(value, deadzone, value, 0.f);
}
__forceinline void SetZeroIf_LT(float& value, const float deadzone) {
	value = ZeroIf_LT(value,deadzone);
}
// TODO - Deadzone - controller type case.





NOALIAS __forceinline const F32 Div( const F32 A, const F32 B )
{
    
    return A / B;
    
}


__forceinline float InvLerp(float f0, float f1, float t) {
	return	Div( (t-f0), f1-f0 );
}

// [WTL - wrapper encourages optimization of clamping before divide, versus Clamp(InvLerp(..))]
__forceinline float InvLerpClamp(float f0, float f1, float t) {
	float inv = Reciprocal(f1-f0);
	t-=f0;
	t=FSel(t, t, 0.f);
	t=Min(f1,t);
	return	t * inv;
}

///////////////////////////////////////////////////////////////
//
// Multiple Selects

// Assign to LValues conditionally.
// caution on using this r.e. side-effects..
template<typename T, typename BOOL_T>
__forceinline void	AssignSelectLValue(T& dstTrue, T& dstFalse, BOOL_T condition, const T& rvalue)
{
	dstTrue		= Select(condition, rvalue, dstTrue);
	dstFalse	= Select(condition, rvalue, dstFalse);
};
__forceinline  void	AssignFSelLValue(F32& dst0, F32& dst1, const F32 fsel, const F32 rvalue)
{
	dst0 = (F32)__fsel(fsel, rvalue, dst0);
	dst1 = (F32)__fsel(fsel, dst1, rvalue);
}
// todo - this would want another version for STORE.
template<>
__forceinline void	AssignSelectLValue(F32& dstTrue, F32& dstFalse, bool cond, const F32& rvalue)
{
	AssignFSelLValue(dstTrue, dstFalse, BoolToFSel(cond), rvalue);
}
template<>
__forceinline void	AssignSelectLValue(int& dstTrue, int& dstFalse, bool cond, const int& src)
{
#ifdef PLATFORM_SPU
	dstTrue		= (cond)?src:dstTrue;	// cond==true: dstTrue gets src;		cond==false: dstTrue unchanged.
	dstFalse	= (cond)?dstFalse:src;	// cond==true: dstFalse gets dstFalse;	cond==false: dstFalse gets src;
#else
	// TODO- Doesn't Intel have conditional-move.
	int	maskTrue	=	BoolToMaskS32(cond);
	int	maskFalse	=	!maskTrue;
	dstTrue			= (dstTrue & maskFalse) | (src & maskTrue);
	dstFalse		= (dstFalse & maskTrue) | (src & maskFalse);
#endif
};

// Single-line wrappers for conditional assignment.

// todo - resolve naming issue. 'If_Set' sounds too much like 'if something is set'. i'd prefer to post-fix it really.
template<typename T,typename Y>
__forceinline T If_Set(const Y condition, T& dstValue, const T srcValue ) {
	return dstValue = (T) Select(condition, srcValue, dstValue);
}
#define	If_Then_Set If_Set
template<typename T,typename Y>
__forceinline T If_SetAdd(const Y condition, T& dstValue, const T srcValue ) {
	return dstValue = (T) Select(condition, dstValue+srcValue, dstValue);
}
template<typename T,typename Y>
__forceinline T If_SetSub(const Y condition, T& dstValue, const T srcValue ) {
	return dstValue = (T) Select(condition, dstValue+srcValue, dstValue);
}
template<typename T,typename Y>
__forceinline T IfGT_SetAdd(const Y cmpA, const Y cmpB,	T& dstValue, const T srcValue ) {
	return dstValue = (T) Select_GT(cmpA, cmpB, dstValue+srcValue, dstValue);
}
template<typename T,typename Y>
__forceinline T IfLT_SetSub(const Y cmpA, const Y cmpB,	T& dstValue, const T srcValue ) {
	return dstValue = (T) Select_LT(cmpA, cmpB, dstValue+srcValue, dstValue);
}
// lisp-esque 'cond' control block :(
template<typename B, typename T>
__forceinline T	Cond(B b0, T v0, B b1, T v1, T defaultValue=0) {
	T	ret = defaultValue;
	ret = (T)Select(b0, ret, v0);
	ret = (T)Select(b1, ret, v1);
	return	(T)ret;
}

//! Returns p aligned to the next multiple of a.  A must be a power of 2.
template <typename T>
inline T Align(T p, size_t a) { return (T)(((size_t)(p)+((a)-1))&(~((a)-1))); }

}; //end namespace Maths

#endif // #if defined( INCLUDE_THIS )


