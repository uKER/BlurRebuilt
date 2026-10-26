#ifndef BTL_TYPES_H
#define BTL_TYPES_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////////
// Type manipulation
//////////////////////////////////////////////////////////

/// Bizarre Template Library
namespace Btl
{

#ifndef BIZ_STATIC_ASSERT
#ifndef __GNUC__
// [Blur2PC] see core/utility/utility.h
#define BIZ_STATIC_ASSERT(exp) \
static_assert((exp), #exp)
#define BIZ_STATIC_ASSERT2(exp, err) \
static_assert((exp), #err)
#else
#define BIZ_STATIC_ASSERT(exp) ;
#define BIZ_STATIC_ASSERT2(exp, err) ;
#endif
#endif

#ifndef BTL_ASSERT
#define BTL_ASSERT(expr)				BIZ_ASSERT(expr)
#define BTL_ASSERT_MSG(expr, msg, ...)	BIZ_ASSERT_MSG(expr, msg, ## __VA_ARGS__)
#define BTL_ASSERT_FAIL(msg, ...)		BIZ_ASSERT_FAIL(msg, ## __VA_ARGS__)
#endif

/////////////////////////////////////////////////////
// Type manipulation (mostly from Loki)
/////////////////////////////////////////////////////

/// @addtogroup Type_Manipulation
/// @{

///////////////////////////////////////////////////
/// Select T if flag is true, F otherwise
template <bool flag, typename T, typename F>
struct Select
{
	typedef T Result; ///< @a 
};

template <typename T, typename F>
struct Select<false, T, F>
{
	typedef F Result;
};

/////////////////////////////////////////////////

namespace Types
{
	typedef char Small;
	class Big {char dummy[2];};
}

/// Does T convert to U?
template <class T, class U>
class IsConvertible
{
public:
	static Types::Big   Test(...);
	static Types::Small Test(U*);
	static T* MakeT();

	BIZ_STATIC_ASSERT(sizeof(T) >= 0);
	BIZ_STATIC_ASSERT(sizeof(U) >= 0);

	enum {exists = sizeof(Test(MakeT())) == sizeof(Types::Small)}; ///< @a
};

/// Return true if T is the same type as U.
template <typename T, typename U>
struct SameType
{
    enum { result = false };
};

template <typename T>
struct SameType<T, T>
{
    enum { result = true };
};

/// If the type is not a reference, return the type. Otherwise return the type referred to.
template <typename T>
struct StripReference
{
	typedef T Result; ///< @a 
};

template <typename T>
struct StripReference<T&>
{
	typedef T Result;
};

/// If the type is not const, return the type. Otherwise return the type without const.
template <typename T>
struct StripConst
{
	typedef T Result; ///< @a 
};

template <typename T>
struct StripConst<const T>
{
	typedef T Result;
};

/// Copy const-ness of second type onto first type.
template <typename T, typename U>
struct CopyConst
{
	typedef typename StripConst<T>::Result Result; ///<
};

template <typename T, typename U>
struct CopyConst<T, const U>
{
	typedef const T Result;
};

/// The result member holds a pointer to a Destination object if Source can be converted to Destination with static_cast, otherwise zero.
/// Essentially, provides the same behaviour as dynamic_cast for non-polymorphic objects.
template <typename Source, typename Destination>
class StaticCast
{
public:
	/// @a 
	StaticCast(Source* source)
	{
		if (IsConvertible<Source, Destination>::exists)
			result = (Destination*) source;
		else
			result = 0;
	};

	operator Destination* () {return result;}

	Destination* result; ///< @a
};

/// Template for a static integer.
template <int N>
struct StaticInteger
{
    const static int value = N; ///< @a
};

/// Object that holds a type for template function argument matching.
template <typename T>
struct TypeHolder
{
	TypeHolder() {}
	typedef T Type;
};
    
/// A type that does nothing. Useful as a template argument.
class NullType
{
private:
	NullType() {}
};

/// @}

///////////////////////////////////////////////////////////////////
// Alignment

/// @addtogroup Misc
/// @{

template <typename T>
struct Tchar {
    T t;
    char c;
};

/// Get alignment requirement for a type.
#define BTL_ALIGNMENT_OF(T)								\
	((sizeof(Btl::Tchar<T>) > sizeof(T)) ?			\
     (sizeof(Btl::Tchar<T>)-sizeof(T)) : sizeof(T))

/// Object the same size as T (though not necessarily with the same alignment)
template <typename T>
class SameSizeObject
{
	U8 data[sizeof(T)];
};

template <int i> struct AlignedObject;
template <> struct AlignedObject<1> {U8 x;};
template <> struct AlignedObject<2> {U16 x;};
template <> struct AlignedObject<4> {U32 x;};
template <> struct AlignedObject<8> {U64 x;};
template <> struct AlignedObject<16> 
{
	struct 
#ifdef COMPILER_GNU
	__attribute__((aligned(16)))
#else
	__declspec(align(16))
#endif
	U128 {U32 x; U32 y; U32 z; U32 w;};

	U128 v;
};

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( disable: 4324 ) // warning C4324: 'Btl::AlignedObject<128>::U128' : structure was padded due to __declspec(align())
#endif

template <> struct AlignedObject<128> 
{
	struct 
#ifdef COMPILER_GNU
	__attribute__((aligned(128)))
#else
	__declspec(align(128))
#endif
	U128 {U32 x; U32 y; U32 z; U32 w;};

	U128 v;
};

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

/// Class with the same size and alignment as T
template <typename T, int N = 1>
class SameSizeAndAlignment
{
	typedef AlignedObject<BTL_ALIGNMENT_OF(T)> U;
	U data[sizeof(T) * N / sizeof(U)];
};

/// Get next aligned address after the input address.
template<typename T>
T*	AlignUp(T* ptr, INTPTR alignment) 
{
	return (T*)
	(	((INTPTR) ptr)	+	(alignment-1)
	&	(~(alignment-1))
	);
};

/// Get a pointer aligned before the input adress
template<typename T>
T*	AlignDown(T* ptr, INTPTR alignment) 
{
	return (T*)
	(	((INTPTR) ptr)
	&	(~(alignment-1))
	);
};

inline U16 AlignUp(U16 offset, U16 alignment)
{
	return (U16)( (offset + (alignment - 1)) & ~(alignment - 1) );
}

inline U16 AlignDown(U16 offset, U16 alignment)
{
	return (U16)( offset & ~(alignment - 1) );
}

inline U32 AlignUp(U32 offset, U32 alignment)
{
	return (offset + (alignment - 1)) & ~(alignment - 1);
}

inline U32 AlignDown(U32 offset, U32 alignment)
{
	return offset & ~(alignment - 1);
}

inline U64 AlignUp(U64 offset, U64 alignment)
{
	return (offset + (alignment - 1)) & ~(alignment - 1);
}

inline U64 AlignDown(U64 offset, U64 alignment)
{
	return offset & ~(alignment - 1);
}

/// @}

////////////////////////////////////////////////////////////////////////////
// These are deliberately undocumented due to being unsafe on some types.

template <typename T>
class MoveByMemcopy
{
public:
	static void Move(void* dst, T* src) 
	{
		memcpy(dst, src, sizeof(T));
	}

	static void MoveArray(void* pDst, T* pSrc, int n) 
	{
		memcpy(pDst, pSrc, sizeof(T) * n);
	}
};

template <typename T>
class MoveByConstructor
{
public:
	static void Move(void* pDst, T* pSrc) 
	{
		new (pDst) T(*pSrc);
		pSrc->~T();
	}

	static void MoveArray(void* pDst, T* pSrc, int n) 
	{
		for (int i = 0; i < n; i++)
		{
			new (((T*)pDst) + i) T(pSrc[i]);
			pSrc[i].~T();
		}
	}
};

template <typename T>
void Move(void* pDst, T* pSrc) 
{
	MoveByConstructor<T>::Move(pDst, pSrc);
}

template <typename T>
void MoveArray(void* pDst, T* pSrc, int n) 
{
	MoveByConstructor<T>::MoveArray(pDst, pSrc, n);
}

template<class T> inline
void SwapByMemcopy(T& left, T& right)
{	
	if (&left != &right)
	{
		T temp; 
		temp.~T();
		memcpy(&temp, &left, sizeof(T));
		memcpy(&left, &right, sizeof(T));
		memcpy(&right, &temp, sizeof(T));
		new (&temp) T;
	}
}

///////////////////////////////////////////////////////////////////

/// @addtogroup Misc
/// @{

/// Exchange values stored at left and right.
template<class T> inline
void Swap(T& left, T& right)
{
	T temp = left; left = right; right = temp;
}

///////////////////////////////////////////////////////////////

template <int N1, int N2>
struct StaticMin
{
	const static int RESULT = (N1 < N2) ? N1 : N2;
};

template <int N1, int N2>
struct StaticMax
{
	const static int RESULT = (N1 > N2) ? N1 : N2;
};

/// @}

}

///////////////////////////////////////////////////////////////

#endif

