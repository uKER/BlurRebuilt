#ifndef __GNUC__
#pragma once
#endif
#ifndef _UTILITY_H2_
#define _UTILITY_H2_

#ifndef __GNUC__

// this warning appears when assigning a pointer to a bool, which is usually an error
#pragma warning(error : 4800) 

// deleting pointer to incomplete type
#pragma warning(error : 4150)

// could not inline
#pragma warning(disable : 4714)

// using argument-dependent lookup
#pragma warning (disable : 4675)

#endif

#include <core/utility/coredebug.h>

/////////////////////////////////////////////////////
// compile-time assert
/////////////////////////////////////////////////////

#if !defined( __GNUC__ )

// [Blur2PC] The original typedef-array trick is rejected by current MSVC when
// the same scope asserts twice ("redefinition; different subscripts").
#define BIZ_STATIC_ASSERT(exp) \
static_assert((exp), #exp)

#define BIZ_STATIC_ASSERT2(exp, err) \
static_assert((exp), #err)

#elif defined( PLATFORM_SPU )

template < bool x > struct GnuStaticAssertFailed;
template <> struct GnuStaticAssertFailed< true > { };
template < size_t x > struct GnuStaticAssertTest { };

#define BIZ_STATIC_ASSERT2( expr, line )    typedef GnuStaticAssertTest< sizeof( GnuStaticAssertFailed< !(bool)!( expr ) > ) > GnuStaticAssert##line;
#define static_assert3( expr, line )    BIZ_STATIC_ASSERT2( expr, line )
#define BIZ_STATIC_ASSERT( expr )          static_assert3( expr, __LINE__ )

#else

#define BIZ_STATIC_ASSERT(exp) ;
#define BIZ_STATIC_ASSERT2(exp, err) ;

#endif

#ifndef FORCE_INLINE
// use own FORCE_INLINE
#ifndef __GNUC__
#define UT_INLINE __forceinline
#define UT_FORCE_INLINE __forceinline
#else
#define UT_INLINE inline
#define UT_FORCE_INLINE inline
#endif
#else
// use FORCE_INLINE macro from SystemDefines.h
#define UT_INLINE FORCE_INLINE
#define UT_FORCE_INLINE FORCE_INLINE

#endif

//#include <core/allocators/allocator.h>

#ifndef MERCURY_NO_PLACEMENT_NEW
#include <new>
#endif // MERCURY_NO_PLACEMENT_NEW

/// @defgroup Containers
/// Main classes are:
/// @li @ref Ut::UtList
/// @li @ref Ut::UtVector
/// @li @ref Ut::UtSet
/// @li @ref Ut::UtMap
/// @li @ref Ut::MultiSet
/// @li @ref Ut::MultiMap
/// @li @ref Ut::OpenHashSet
/// @li @ref Ut::OpenHashMap
///
/// Key concepts:
/// @li Iterators, which are used to step through containers.
/// @li Sequences, which contain a pair of iterators.
/// @li Algorithms, which act on sequences.
/// @li Policies, which configure the behaviour of the containers.
/// 
/// This is an STL style container library. The naming conventions are the same
/// as far as possible, although function and class names are capitalised.
/// The containers are slightly lower level than STL and more configurable, 
/// though they also work as a direct replacement.
/// All error checking is done using assert, or with static checks where possible.

/// Namespace for utility code.
namespace Ut
{

/////////////////////////////////////////////////////
// Type manipulation (from Loki)
/////////////////////////////////////////////////////

/// @addtogroup Type_Manipulation
/// @{

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

#ifndef __GNUC__

/// Return true if T is the same type as U.
template <typename T, typename U>
struct SameType
{
private:
    template<typename>
    struct In 
    { enum { result = false }; };

    template<>
    struct In<T>
    { enum { result = true };  };

public:
    enum { result = In<U>::result }; ///< @a
};

#endif

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

#ifndef __GNUC__

/// Copy const-ness of second type onto first type.
template <typename T, typename U>
struct CopyConst
{
	typedef T Result; ///<
};

template <typename T, typename U>
struct CopyConst<typename T, typename const U>
{
	typedef const T Result;
};

#endif

/// The result member holds a pointer to a Destination object if Source can be converted to Destination with static_cast, otherwise zero.
/// Essentially, provides the same behaviour as dynamic_cast for non-polymorphic objects.
#ifndef __GNUC__
template <typename Source, typename Destination>
class StaticCast
{
public:
	/// @a
	StaticCast(Source* source)
	{
		GetResult<Ut::IsConvertible<Source, Destination>::exists>(source);
	};

	Destination* result; ///< @a

private:
	template <bool>
	void GetResult(Source* source)
	{
		result = static_cast<Destination*>(source);
	}

	template <>
	void GetResult<false>(Source* source)
	{
		result = 0;
	}
};
#endif

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

template <typename T>
struct Tchar {
    T t;
    char c;
};

/// Get alignment requirement for a type.
#define ALIGNMENT_OF(T)                            \
	((sizeof(Ut::Tchar<T>) > sizeof(T)) ?            \
     (sizeof(Ut::Tchar<T>)-sizeof(T)) : sizeof(T))

// Object the same size as T (though not necessarily with the same alignment)
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
#ifdef COMPILER_GNU
	__attribute__((aligned(16)))
#else
	__declspec(align(16))
#endif
	struct U128 {U32 x; U32 y; U32 z; U32 w;};

	U128 v;
};
template <> struct AlignedObject<128> 
{
#ifdef COMPILER_GNU
	__attribute__((aligned(128)))
#else
	__declspec(align(128))
#endif
	struct U128 {U32 x; U32 y; U32 z; U32 w;};

	U128 v;
};

// Class with the same size and alignment as T
template <typename T>
class SameSizeAndAlignment
{
	typedef AlignedObject<ALIGNMENT_OF(T)> U;
	U data[sizeof(T) / sizeof(U)];
};

/// @}

//////////////////////////////////////////////////////////////////

/// @addtogroup Misc
/// @{

/// An object containing two other objects.
template <typename First, typename Second>
class Pair
{
public:
	Pair() {}
	Pair(const First& f, const Second& s) : first(f), second(s) {} ///< @a

	First first; ///< @a
	Second second; ///< @a
};

/// Creates a Pair containing copies of the two function parameters.
template <typename First, typename Second>
Pair<First, Second> MakePair(const First& f, const Second& s)
{
	return Pair<First, Second>(f, s);
}

/// @a
template <typename First, typename Second>
inline bool operator < (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	if (p2.first < p1.first)
		return false;
	else if (p1.first < p2.first || p1.second < p2.second)
		return true;
	else 
		return false;
}

/// @a
template <typename First, typename Second>
inline bool operator > (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return p2 < p1;
}

/// @a
template <typename First, typename Second>
inline bool operator >= (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return !(p1 < p2);
}

/// @a
template <typename First, typename Second>
inline bool operator <= (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return !(p2 < p1);
}

/// @a
template <typename First, typename Second>
inline bool operator == (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return p1.first == p2.first && p1.second == p2.second;
}

/// @a
template <typename First, typename Second>
inline bool operator != (const Pair<First, Second>& p1, const Pair<First, Second>& p2)
{
	return p1.first != p2.first || p1.second != p2.second;
}

/// @}

////////////////////////////////////////////////////////////////////////////

// Move an object to a new location without making a copy.
template <typename T>
UT_INLINE void MoveRaw(T& dst, T& src)
{
	if (&dst != &src)
	{
		dst.~T(); // destroy the destination object
		memcpy(&dst, &src, sizeof(T)); // copy memory
		new (&src) T;  // clear the original - if it held any pointers they are now owned by p1
	}
}

template <typename T>
UT_INLINE void Clear(T& obj)
{
	obj.~T(); // destroy the object
	new (&obj) T; // create a new one over the top
}

// Move objects to a newly created array.
template <typename T>
void MoveArrayRaw(T* pDst, T* pSrc, int n)
{
	assert(n >= 0);

	// destroy destination
	for (int i = 0; i < n; i++)
		pDst[i].~T();

	memcpy(pDst, pSrc, sizeof(T) * n); // copy memory

	// clear the source objects
	for (int i = 0; i < n; i++)
		new(&pSrc[i]) T;
}

// exchange values stored at left and right
template<class T> inline
void SwapRaw(T& left, T& right)
{	
	T temp;

	// Use Move() rather than = so that we don't make unnecessary copies.
	MoveRaw(temp, left);
	MoveRaw(left, right);
	MoveRaw(right, temp);
}

/// @addtogroup Misc
/// @{

/// Exchange values stored at left and right.
template<class T> inline
void Swap(T& left, T& right)
{
	T temp = left; left = right; right = temp;
}

/// Rotate values stored at v0, v1, v2.
template<class T> inline
void Rotate(T& v0, T& v1, T& v2)
{
	T temp = v0; v0 = v1; v1 = v2; v2 = temp;
}

/// Rotate values stored at v0, v1, v2, v3.
template<class T> inline
void Rotate(T& v0, T& v1, T& v2, T& v3)
{
	T temp = v0; v0 = v1; v1 = v2; v2 = v3; v3 = temp;
}

/// @}
// get next aligned adress after the input adress
template<typename T, typename Y>
T*	AlignUp(Y* ptr, INTPTR alignment) 
{
	return (T*)
	(	((INTPTR) ptr)	+	(alignment-1)
	&	(~(alignment-1))
	);
};

// get a pointer aligned before the intput adress
template<typename T, typename Y>
T*	AlignDown(Y* ptr, INTPTR alignment) 
{
	return (T*)
	(	((INTPTR) ptr)
	&	(~(alignment-1))
	);
};
}

///////////////////////////////////////////////////////////////

namespace UtMaths
{

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


// todo, template like scoped ptr.
#define DECL_ALLOCA_ARRAY(TYPE,NAME,count)	TYPE*	NAME = (TYPE*) alloca(sizeof(TYPE)*count);
#define DECL_ALLOCA(TYPE,NAME)				TYPE*	NAME = (TYPE*) alloca(sizeof(TYPE));


}

#endif


