#pragma once

#include "core\maths\mathsutil.h"
#include "core\maths\utmaths\mvector.h"
#include "core\maths\utmaths\mmatrix.h"

namespace Maths
{

extern const XMVECTOR XMVECTOR_CONSTANT_X;
extern const XMVECTOR XMVECTOR_CONSTANT_Y;
extern const XMVECTOR XMVECTOR_CONSTANT_Z;
extern const XMVECTOR XMVECTOR_CONSTANT_NEGATIVE_X;
extern const XMVECTOR XMVECTOR_CONSTANT_NEGATIVE_Y;
extern const XMVECTOR XMVECTOR_CONSTANT_NEGATIVE_Z;
extern const XMVECTOR XMVECTOR_CONSTANT_XY;
extern const XMVECTOR XMVECTOR_CONSTANT_XZ;
extern const XMVECTOR XMVECTOR_CONSTANT_YZ;
extern const XMVECTOR XMVECTOR_CONSTANT_XYZ;
extern const XMVECTOR XMVECTOR_CONSTANT_MIN;
extern const XMVECTOR XMVECTOR_CONSTANT_MAX;
extern const XMVECTOR XMVECTOR_CONSTANT_NEGATIVE_MAX;
extern const XMVECTOR XMVECTOR_CONSTANT_SELECT_X;
extern const XMVECTOR XMVECTOR_CONSTANT_SELECT_Y;
extern const XMVECTOR XMVECTOR_CONSTANT_SELECT_Z;

}

__forceinline XMVECTOR XMVectorReplicateRef(const float& value)
{
    XMVECTOR V;

    V = __lvlx(&value, 0);
    V = __vspltw(V, 0);

    return V;
}

namespace UtMaths
{

/////////////////////////////////////////////////////////////////

// Type to return scalar results from XMVector_Wrapper, while remaining in vector form internally.
// Internal components are all equal.
__declspec(passinreg) struct XMFloat_Wrapper : private XMVECTOR
{
	__forceinline XMFloat_Wrapper() {}
	//__forceinline XMFloat_Wrapper(const XMFloat_Wrapper& other) {(XMVECTOR&) *this = other;}

	// initialise from a float - okay from memory but from a register will have to store and reload
	__forceinline XMFloat_Wrapper(const float& f) 
	{
		XMVECTOR v = __lvlx(&f, 0);
		*this = __vspltw(v, 0);
	}

	__forceinline explicit XMFloat_Wrapper(const XMVECTOR& v) {(XMVECTOR&) *this = v;}

	__forceinline operator const float& () const {return x;}
	__forceinline operator float& () {return x;}

	__forceinline XMFloat_Wrapper& operator = (const XMVECTOR& v) {(XMVECTOR&) *this = v; return *this;}
	__forceinline XMFloat_Wrapper& operator = (const XMFloat_Wrapper& v) {(XMVECTOR&) *this = (XMVECTOR&) v; return *this;}

	__forceinline XMFloat_Wrapper operator - () const {return XMFloat_Wrapper(-((XMVECTOR&) *this));}
	__forceinline XMFloat_Wrapper& operator += (const XMFloat_Wrapper& a) {(XMVECTOR&) *this += (XMVECTOR&)a; return *this;} 
    __forceinline XMFloat_Wrapper& operator -= (const XMFloat_Wrapper& a) {(XMVECTOR&) *this -= (XMVECTOR&)a; return *this;} 
	__forceinline XMFloat_Wrapper& operator *= (const XMFloat_Wrapper& a) {(XMVECTOR&) *this *= (XMVECTOR&)a; return *this;} 
    __forceinline XMFloat_Wrapper& operator /= (const XMFloat_Wrapper& a) {(XMVECTOR&) *this /= (XMVECTOR&)a; return *this;}

	__forceinline XMFloat_Wrapper& operator += (const float& a) {*this += XMFloat_Wrapper(a); return *this;} 
    __forceinline XMFloat_Wrapper& operator -= (const float& a) {*this -= XMFloat_Wrapper(a); return *this;} 
	__forceinline XMFloat_Wrapper& operator *= (const float& a) {*this *= XMFloat_Wrapper(a); return *this;} 
    __forceinline XMFloat_Wrapper& operator /= (const float& a) {*this /= XMFloat_Wrapper(a); return *this;} 
};

template <> struct ProductType<XMFloat_Wrapper, XMFloat_Wrapper> {typedef XMFloat_Wrapper Result;};
template <> struct QuotientType<XMFloat_Wrapper, XMFloat_Wrapper> {typedef XMFloat_Wrapper Result;};
template <> struct LinearType<XMFloat_Wrapper, XMFloat_Wrapper> {typedef XMFloat_Wrapper Result;};
template <> struct ProductType<XMFloat_Wrapper, float> {typedef XMFloat_Wrapper Result;};
template <> struct QuotientType<XMFloat_Wrapper, float> {typedef XMFloat_Wrapper Result;};
template <> struct LinearType<XMFloat_Wrapper, float> {typedef float Result;};
template <> struct ProductType<float, XMFloat_Wrapper> {typedef XMFloat_Wrapper Result;};
template <> struct QuotientType<float, XMFloat_Wrapper> {typedef XMFloat_Wrapper Result;};
template <> struct LinearType<float, XMFloat_Wrapper> {typedef float Result;};

// operators
__forceinline XMFloat_Wrapper operator + (const XMFloat_Wrapper& x1, const XMFloat_Wrapper& x2) {return XMFloat_Wrapper((XMVECTOR&) x1 + (XMVECTOR&) x2);}
__forceinline XMFloat_Wrapper operator - (const XMFloat_Wrapper& x1, const XMFloat_Wrapper& x2) {return XMFloat_Wrapper((XMVECTOR&) x1 - (XMVECTOR&) x2);}
__forceinline XMFloat_Wrapper operator * (const XMFloat_Wrapper& x1, const XMFloat_Wrapper& x2) {return XMFloat_Wrapper((XMVECTOR&) x1 * (XMVECTOR&) x2);}
__forceinline XMFloat_Wrapper operator / (const XMFloat_Wrapper& x1, const XMFloat_Wrapper& x2) {return XMFloat_Wrapper((XMVECTOR&) x1 / (XMVECTOR&) x2);}

// float operators - ideally using floats stored in memory - registers will be stored and reloaded as vectors
__forceinline XMFloat_Wrapper operator * (const XMFloat_Wrapper& x, const float& f) {return x * XMFloat_Wrapper(f);}
__forceinline XMFloat_Wrapper operator * (const float& f, const XMFloat_Wrapper& x) {return x * XMFloat_Wrapper(f);}
__forceinline XMFloat_Wrapper operator / (const XMFloat_Wrapper& x, const float& f) {return x / XMFloat_Wrapper(f);}
__forceinline XMFloat_Wrapper operator / (const float& f, const XMFloat_Wrapper& x) {return XMFloat_Wrapper(f) / x;}

__forceinline XMFloat_Wrapper operator + (const XMFloat_Wrapper& x, const float& f) {return x + XMFloat_Wrapper(f);}
__forceinline XMFloat_Wrapper operator + (const float& f, const XMFloat_Wrapper& x) {return x + XMFloat_Wrapper(f);}
__forceinline XMFloat_Wrapper operator - (const XMFloat_Wrapper& x, const float& f) {return x - XMFloat_Wrapper(f);}
__forceinline XMFloat_Wrapper operator - (const float& f, const XMFloat_Wrapper& x) {return XMFloat_Wrapper(f) - x;}

}

// Maths namespace functions for XMFloat_Wrapper
namespace Maths
{

__forceinline void Store(const UtMaths::XMFloat_Wrapper& s, float* f) {__stvewx((XMVECTOR&)s, f, 0);}

extern const XMVECTOR XMVECTOR_CONSTANT_EPSILON;
extern const XMVECTOR XMVECTOR_CONSTANT_HALF;
extern const XMVECTOR XMVECTOR_CONSTANT_TWO;
extern const XMVECTOR XMVECTOR_CONSTANT_THREE;
extern const XMVECTOR XMVECTOR_CONSTANT_FOUR;

template <> struct Constants<UtMaths::XMFloat_Wrapper> 
{
	static UtMaths::XMFloat_Wrapper Zero() {return UtMaths::XMFloat_Wrapper(XMVectorZero());}
	static UtMaths::XMFloat_Wrapper Epsilon() {return UtMaths::XMFloat_Wrapper(XMVECTOR_CONSTANT_EPSILON);}
	static UtMaths::XMFloat_Wrapper One() {return UtMaths::XMFloat_Wrapper(XMVectorSplatOne());}
	static UtMaths::XMFloat_Wrapper Half() {return UtMaths::XMFloat_Wrapper(XMVECTOR_CONSTANT_HALF);}
	static UtMaths::XMFloat_Wrapper Two() {return UtMaths::XMFloat_Wrapper(XMVECTOR_CONSTANT_TWO);}
	static UtMaths::XMFloat_Wrapper Three() {return UtMaths::XMFloat_Wrapper(XMVECTOR_CONSTANT_THREE);}
	static UtMaths::XMFloat_Wrapper Four() {return UtMaths::XMFloat_Wrapper(XMVECTOR_CONSTANT_FOUR);}
};

__forceinline UtMaths::XMFloat_Wrapper Sqrt(const UtMaths::XMFloat_Wrapper& a)
{
	return UtMaths::XMFloat_Wrapper(XMVectorSqrt((XMVECTOR&)a));
}

__forceinline UtMaths::XMFloat_Wrapper Abs(const UtMaths::XMFloat_Wrapper& a)
{
    return UtMaths::XMFloat_Wrapper(XMVectorMax((XMVECTOR&)a, -(XMVECTOR&)a));
}

__forceinline UtMaths::XMFloat_Wrapper Pow(const UtMaths::XMFloat_Wrapper& x, const UtMaths::XMFloat_Wrapper& y)
{
    return UtMaths::XMFloat_Wrapper(XMVectorPow((XMVECTOR&)x, (XMVECTOR&)y));
}

__forceinline UtMaths::XMFloat_Wrapper Sign(const UtMaths::XMFloat_Wrapper& a)
{
	return UtMaths::XMFloat_Wrapper(XMVectorSelect(XMVectorSelect(-XMVectorSplatOne(), XMVectorSplatOne(), XMVectorGreater((XMVECTOR&) a, XMVectorZero())), 
														XMVectorZero(), 
														XMVectorEqual((XMVECTOR&) a, XMVectorZero())));
}

__forceinline UtMaths::XMFloat_Wrapper Max(const UtMaths::XMFloat_Wrapper& a, const UtMaths::XMFloat_Wrapper& b)
{
	return UtMaths::XMFloat_Wrapper(XMVectorMax((XMVECTOR&)a, (XMVECTOR&)b));
}

__forceinline UtMaths::XMFloat_Wrapper Min(const UtMaths::XMFloat_Wrapper& a, const UtMaths::XMFloat_Wrapper& b)
{
	return UtMaths::XMFloat_Wrapper(XMVectorMin((XMVECTOR&)a, (XMVECTOR&)b));
}

__forceinline UtMaths::XMFloat_Wrapper Max(const UtMaths::XMFloat_Wrapper& a, const float& b)
{
	return UtMaths::XMFloat_Wrapper(XMVectorMax((XMVECTOR&)a, XMVectorReplicateRef(b)));
}

__forceinline UtMaths::XMFloat_Wrapper Min(const UtMaths::XMFloat_Wrapper& a, const float& b)
{
	return UtMaths::XMFloat_Wrapper(XMVectorMin((XMVECTOR&)a, XMVectorReplicateRef(b)));
}

__forceinline UtMaths::XMFloat_Wrapper Max(const float& a, const UtMaths::XMFloat_Wrapper& b)
{
	return UtMaths::XMFloat_Wrapper(XMVectorMax(XMVectorReplicateRef(a), (XMVECTOR&)b));
}

__forceinline UtMaths::XMFloat_Wrapper Min(const float& a, const UtMaths::XMFloat_Wrapper& b)
{
	return UtMaths::XMFloat_Wrapper(XMVectorMin(XMVectorReplicateRef(a), (XMVECTOR&)b));
}

__forceinline UtMaths::XMFloat_Wrapper Reciprocal(const UtMaths::XMFloat_Wrapper& x)
{
	return UtMaths::XMFloat_Wrapper(XMVectorReciprocal((XMVECTOR&)x));
}

__forceinline void SinCos(UtMaths::XMFloat_Wrapper* s, UtMaths::XMFloat_Wrapper* c, const UtMaths::XMFloat_Wrapper& a)
{
	XMVectorSinCos((XMVECTOR*) s, (XMVECTOR*) c, (XMVECTOR&) a);
}

}

///////////////////////////////////////////////////////////////

namespace UtMaths
{

// operators with float as a destination - ideally in memory as registers will be stored and reloaded
__forceinline float& operator += (float& f, const XMFloat_Wrapper& s) {Maths::Store(XMFloat_Wrapper(f) + s, &f); return f;}
__forceinline float& operator -= (float& f, const XMFloat_Wrapper& s) {Maths::Store(XMFloat_Wrapper(f) - s, &f); return f;}
__forceinline float& operator *= (float& f, const XMFloat_Wrapper& s) {Maths::Store(XMFloat_Wrapper(f) * s, &f); return f;}
__forceinline float& operator /= (float& f, const XMFloat_Wrapper& s) {Maths::Store(XMFloat_Wrapper(f) / s, &f); return f;}

// Type that acts like a bool but stays in vector form.
struct XMBoolean_Wrapper : private XMVECTOR
{
	XMBoolean_Wrapper() {}
	__forceinline XMBoolean_Wrapper(const XMVECTOR& v) {(XMVECTOR&) *this = v;}
	__forceinline XMBoolean_Wrapper(const XMBoolean_Wrapper& v) {(XMVECTOR&) *this = (XMVECTOR&) v;}
	__forceinline XMBoolean_Wrapper& operator = (const XMBoolean_Wrapper& v) {(XMVECTOR&) *this = (XMVECTOR&) v; return *this;}

	//__forceinline operator int () const {return XMVector4NotEqual(*this, XMVectorZero());}
	__forceinline operator bool () const {return XMVector4NotEqual(*this, XMVectorZero()) != 0;}

	__forceinline XMBoolean_Wrapper& operator &= (XMBoolean_Wrapper other) {(XMVECTOR&) *this = XMVectorAndInt(*this, other); return *this;}
	__forceinline XMBoolean_Wrapper& operator |= (XMBoolean_Wrapper other) {(XMVECTOR&) *this = XMVectorOrInt(*this, other); return *this;}
	__forceinline XMBoolean_Wrapper operator ! () {return XMVectorNorInt(*this, *this);}
};

__forceinline XMFloat_Wrapper Select(const XMBoolean_Wrapper& test, const XMFloat_Wrapper& resultIfTrue, const XMFloat_Wrapper& resultIfFalse)
{
	return XMFloat_Wrapper(XMVectorSelect((XMVECTOR&) resultIfFalse, (XMVECTOR&) resultIfTrue, (XMVECTOR&) test));
}

/////////////////

__forceinline XMBoolean_Wrapper operator > (const XMFloat_Wrapper& f1, const XMFloat_Wrapper& f2)
{
	return XMBoolean_Wrapper(XMVectorGreater((XMVECTOR&) f1, (XMVECTOR&) f2));
}

__forceinline XMBoolean_Wrapper operator < (const XMFloat_Wrapper& f1, const XMFloat_Wrapper& f2)
{
	return XMBoolean_Wrapper(XMVectorLess((XMVECTOR&) f1, (XMVECTOR&) f2));
}

__forceinline XMBoolean_Wrapper operator >= (const XMFloat_Wrapper& f1, const XMFloat_Wrapper& f2)
{
	return XMBoolean_Wrapper(XMVectorGreaterOrEqual((XMVECTOR&) f1, (XMVECTOR&) f2));
}

__forceinline XMBoolean_Wrapper operator <= (const XMFloat_Wrapper& f1, const XMFloat_Wrapper& f2)
{
	return XMBoolean_Wrapper(XMVectorLessOrEqual((XMVECTOR&) f1, (XMVECTOR&) f2));
}

__forceinline XMBoolean_Wrapper operator == (const XMFloat_Wrapper& f1, const XMFloat_Wrapper& f2)
{
	return XMBoolean_Wrapper(XMVectorEqual((XMVECTOR&) f1, (XMVECTOR&) f2));
}

__forceinline XMBoolean_Wrapper operator != (const XMFloat_Wrapper& f1, const XMFloat_Wrapper& f2)
{
	return XMBoolean_Wrapper(XMVectorNotEqual((XMVECTOR&) f1, (XMVECTOR&) f2));
}

//////////////

__forceinline XMBoolean_Wrapper operator > (const XMFloat_Wrapper& f1, const float& f2)
{
	return f1 > XMFloat_Wrapper(f2);
}

__forceinline XMBoolean_Wrapper operator < (const XMFloat_Wrapper& f1, const float& f2)
{
	return f1 < XMFloat_Wrapper(f2);
}

__forceinline XMBoolean_Wrapper operator >= (const XMFloat_Wrapper& f1, const float& f2)
{
	return f1 >= XMFloat_Wrapper(f2);
}

__forceinline XMBoolean_Wrapper operator <= (const XMFloat_Wrapper& f1, const float& f2)
{
	return f1 <= XMFloat_Wrapper(f2);
}

__forceinline XMBoolean_Wrapper operator == (const XMFloat_Wrapper& f1, const float& f2)
{
	return f1 == XMFloat_Wrapper(f2);
}

__forceinline XMBoolean_Wrapper operator != (const XMFloat_Wrapper& f1, const float& f2)
{
	return f1 != XMFloat_Wrapper(f2);
}

//////////////

__forceinline XMBoolean_Wrapper operator > (const float& f1, const XMFloat_Wrapper& f2)
{
	return XMFloat_Wrapper(f1) > f2;
}

__forceinline XMBoolean_Wrapper operator < (const float& f1, const XMFloat_Wrapper& f2)
{
	return XMFloat_Wrapper(f1) < f2;
}

__forceinline XMBoolean_Wrapper operator >= (const float& f1, const XMFloat_Wrapper& f2)
{
	return XMFloat_Wrapper(f1) >= f2;
}

__forceinline XMBoolean_Wrapper operator <= (const float& f1, const XMFloat_Wrapper& f2)
{
	return XMFloat_Wrapper(f1) <= f2;
}

__forceinline XMBoolean_Wrapper operator == (const float& f1, const XMFloat_Wrapper& f2)
{
	return XMFloat_Wrapper(f1) == f2;
}

__forceinline XMBoolean_Wrapper operator != (const float& f1, const XMFloat_Wrapper& f2)
{
	return XMFloat_Wrapper(f1) != f2;
}

/////////////

__forceinline XMBoolean_Wrapper operator & (const XMBoolean_Wrapper& l1, const XMBoolean_Wrapper& l2)
{
	return XMBoolean_Wrapper(XMVectorAndInt((XMVECTOR&) l1, (XMVECTOR&) l2));
}

__forceinline XMBoolean_Wrapper operator | (const XMBoolean_Wrapper& l1, const XMBoolean_Wrapper& l2)
{
	return XMBoolean_Wrapper(XMVectorOrInt((XMVECTOR&) l1, (XMVECTOR&) l2));
}

}

namespace Maths
{

template <> struct Constants<UtMaths::XMBoolean_Wrapper>
{
	static UtMaths::XMBoolean_Wrapper True() {return XMVectorTrueInt();}
	static UtMaths::XMBoolean_Wrapper False() {return XMVectorFalseInt();}
};

}

///////////////////////////////////////////////////////////////

namespace UtMaths
{

/// It can have N less than 4 to allow MVector to treat it as having fewer elements.
template <int N = 4>
class XMVector_Wrapper : private XMVECTOR
{
	typedef XMVector_Wrapper<N> This;

protected:
	// Constructors
    This() {} ///< @a

	__forceinline This(float f3, float f2, float f1) { x = f3; y = f2; z = f1; w = 0.f; BIZ_STATIC_ASSERT(N == 3);} 
    __forceinline This(float f3, float f2, float f1, float f0) { x = f3; y = f2; z = f1; w = f0; BIZ_STATIC_ASSERT(N == 4);} 

	__forceinline This(const XMVector_Wrapper& other) {(XMVECTOR&) *this = (XMVECTOR&) other;}
	__forceinline This(const MConstVector<This>& a) {(XMVECTOR&) *this = (XMVECTOR&) a;}
	__forceinline This(const MPoint<This>& a) {(XMVECTOR&) *this = (XMVECTOR&) a;}

	template<typename Evaluator> 
	__forceinline This(const Evaluator& e) {operator = (e);} ///< @a

public:
	using XMVECTOR::x;
	using XMVECTOR::y;
	using XMVECTOR::z;

	// access
	const XMFloat_Wrapper X() const {return (XMFloat_Wrapper&) XMVectorSplatX(*this);} ///< Access x component.
	const XMFloat_Wrapper Y() const {return (XMFloat_Wrapper&) XMVectorSplatY(*this);} ///< Access y component.
	const XMFloat_Wrapper Z() const {return (XMFloat_Wrapper&) XMVectorSplatZ(*this);} ///< Access z component.
	const XMFloat_Wrapper W() const {BIZ_STATIC_ASSERT(N == 4); return (XMFloat_Wrapper&) XMVectorSplatW(*this);} ///< Access w component.

	void X(const XMFloat_Wrapper& x) {((XMVECTOR&) *this) = XMVectorSelect(*this, (XMVECTOR&)x, XMVectorSelectControl(1, 0, 0, 0));}
	void Y(const XMFloat_Wrapper& y) {((XMVECTOR&) *this) = XMVectorSelect(*this, (XMVECTOR&)y, XMVectorSelectControl(0, 1, 0, 0));}
	void Z(const XMFloat_Wrapper& z) {((XMVECTOR&) *this) = XMVectorSelect(*this, (XMVECTOR&)z, XMVectorSelectControl(0, 0, 1, 0));}
	void W(const XMFloat_Wrapper& w) {BIZ_STATIC_ASSERT(N == 4); ((XMVECTOR&) *this) = XMVectorSelect(*this, (XMVECTOR&)w, XMVectorSelectControl(0, 0, 0, 1));}

	__forceinline This& operator = (const MConstVector<This>& a) {((XMVECTOR&) *this) = a; return *this;}
	__forceinline This& operator = (const MPoint<This>& a) {((XMVECTOR&) *this) = a; return *this;}
	__forceinline This& operator = (const XMVector_Wrapper& a) {((XMVECTOR&) *this) = a; return *this;}

	// general assignment
	template<typename Evaluator> 
	__forceinline void operator = (const Evaluator& e) {Recurse<SIZE>::Set(*this, e);}
	
	// access to elements
    template <int I>
    __forceinline const float& At() const
    {
        BIZ_STATIC_ASSERT((0 <= I) && (I <= SIZE));
        float *fp = (float*) this;
        return *(fp+I);
    }

    template <int I>
    __forceinline float& At()
    {
        BIZ_STATIC_ASSERT((0 <= I) && (I <= SIZE));
        float *fp = (float*) this;
        return *(fp+I);
    }

	__forceinline float& operator [] (int i) 
	{
		assert(i > -1 && i < SIZE); 
		float *fp = (float*) this;
        return *(fp+i);
	}

	__forceinline const float& operator [] (int i) const 
	{
		assert(i > -1 && i < SIZE); 
		float *fp = (float*) this;
        return *(fp+i);
	}

	typedef XMFloat_Wrapper DotProductType;
	typedef float DataType; ///< The type stored in the vector.
	const static int SIZE = N; ///< The number of elements in the vector.
	typedef This type1; ///< This type is used by MVector to form a constructor.
	typedef Ut::NullType type2;

	template <template <typename Vector> class MType1, template <typename Vector> class MType2>
	__forceinline static GenericVector<This> Sum(const MType1<This>& a, const MType2<This>& b) {return GenericVector<This>((This&)((XMVECTOR&) a + (XMVECTOR&) b));}

	template <template <typename Vector> class MType1, template <typename Vector> class MType2>
	__forceinline static GenericVector<This> Difference(const MType1<This>& a, const MType2<This>& b) {return GenericVector<This>((This&)((XMVECTOR&) a - (XMVECTOR&) b));} 

	__forceinline static GenericVector<This> Multiply(const MConstVector<This>& a, const XMFloat_Wrapper& b) {return GenericVector<This>((This&)((XMVECTOR&) a * (XMVECTOR&) b));} 
	__forceinline static GenericVector<This> Divide(const MConstVector<This>& a, const XMFloat_Wrapper& b) {return GenericVector<This>((This&)((XMVECTOR&) a / (XMVECTOR&) b));} 
	__forceinline static GenericVector<This> Multiply(const MConstVector<This>& a, float b) {return GenericVector<This>((This&)((XMVECTOR&) a * b));} 
	__forceinline static GenericVector<This> Divide(const MConstVector<This>& a, float b) {return GenericVector<This>((This&)((XMVECTOR&) a / b));} 
	
	__forceinline static XMFloat_Wrapper DotProduct(const MConstVector<This>& a, const MConstVector<This>& b) 
	{
		if (N == 3)
			return XMFloat_Wrapper(XMVector3Dot(a, b));
		else if (N == 4)
			return XMFloat_Wrapper(XMVector4Dot(a, b));
	} 
	
	__forceinline static GenericVector<This> CrossProduct(const MConstVector<This>& v1, const MConstVector<This>& v2) 
	{
		return GenericVector<This>((This&)XMVector3Cross(v1, v2));
	} 

	__forceinline static GenericVector<This> Negate(const MConstVector<This>& v1)
	{
		return GenericVector<This>((This&)XMVectorNegate(v1));
	}

protected:
	__forceinline This& operator += (const MConstVector<This>& a) {(XMVECTOR&) *this += (XMVECTOR&)a; return *this;} 
    __forceinline This& operator -= (const MConstVector<This>& a) {(XMVECTOR&) *this -= (XMVECTOR&)a; return *this;} 
	__forceinline This& operator *= (float f) {(XMVECTOR&) *this *= f; return *this;} 
    __forceinline This& operator /= (float f) {(XMVECTOR&) *this /= f; return *this;}
	__forceinline This& operator *= (const XMFloat_Wrapper& a) {(XMVECTOR&) *this *= (XMVECTOR&)a; return *this;} 
    __forceinline This& operator /= (const XMFloat_Wrapper& a) {(XMVECTOR&) *this /= (XMVECTOR&)a; return *this;}  

private:
	BIZ_STATIC_ASSERT(N <= 4);
};

// specialise the arithmetic types.
template<int N> struct VectorSumType<XMVector_Wrapper<N>, XMVector_Wrapper<N> > {typedef XMVector_Wrapper<N> Result;}; 
template<int N> struct VectorDifferenceType<XMVector_Wrapper<N>, XMVector_Wrapper<N> > {typedef XMVector_Wrapper<N> Result;}; 
template<int N> struct VectorMultiplyType<XMVector_Wrapper<N>, float> {typedef XMVector_Wrapper<N> Result;}; 
template<int N> struct VectorDivideType<XMVector_Wrapper<N>, float> {typedef XMVector_Wrapper<N> Result;};
template<int N> struct VectorMultiplyType<XMVector_Wrapper<N>, XMFloat_Wrapper> {typedef XMVector_Wrapper<N> Result;}; 
template<int N> struct VectorDivideType<XMVector_Wrapper<N>, XMFloat_Wrapper> {typedef XMVector_Wrapper<N> Result;}; 
template<int N> struct DotProductType<XMVector_Wrapper<N>, XMVector_Wrapper<N> > {typedef XMVector_Wrapper<N> Result;}; 
template<int N> struct CrossProductType<XMVector_Wrapper<N>, XMVector_Wrapper<N> > {typedef XMVector_Wrapper<N> Result;}; 
template<int N> struct VectorNegateType<XMVector_Wrapper<N> > {typedef XMVector_Wrapper<N> Result;};

// specialise the return type
template <int N>
struct TemporaryVector<XMVector_Wrapper<N>, float, N>
{
	typedef XMVector_Wrapper<N> Result;
};

/////////////////////////////////////////////////////////////////////////////////////

// initialise from XMVECTOR
__forceinline MVector<XMVector_Wrapper<3> >& MakeVector(XMVECTOR& vector)
{
	return (MVector<XMVector_Wrapper<3> >&) vector;
}

__forceinline const MVector<XMVector_Wrapper<3> >& MakeVector(const XMVECTOR& vector)
{
	return (const MVector<XMVector_Wrapper<3> >&) vector;
}

__forceinline MPoint<XMVector_Wrapper<3> >& MakePoint(XMVECTOR& vector)
{
	return (MPoint<XMVector_Wrapper<3> >&) vector;
}

__forceinline const MPoint<XMVector_Wrapper<3> >& MakePoint(const XMVECTOR& vector)
{
	return (const MPoint<XMVector_Wrapper<3> >&) vector;
}

__forceinline MNormal<XMVector_Wrapper<3> >& MakeNormal(XMVECTOR& vector)
{
	return (MNormal<XMVector_Wrapper<3> >&) vector;
}

__forceinline const MNormal<XMVector_Wrapper<3> >& MakeNormal(const XMVECTOR& vector)
{
	return (const MNormal<XMVector_Wrapper<3> >&) vector;
}

__forceinline XMVECTOR MakeVector(const XMVECTOR& x, const XMVECTOR& y, const XMVECTOR& z)
{
	XMVECTOR result = x;
	result = XMVectorInsert(result, (XMVECTOR&)y, 0, 0, 1, 0, 0);
    result = XMVectorInsert(result, (XMVECTOR&)z, 0, 0, 0, 1, 0);
	return result;
}

///////////////////////////////////////

__forceinline void Set(MVector<XMVector_Wrapper<3> >* out, const XMFloat_Wrapper& x, const XMFloat_Wrapper& y, const XMFloat_Wrapper& z)
{
	*out = MakeVector(MakeVector((const XMVECTOR&) x, (const XMVECTOR&) y, (const XMVECTOR&) z));
}

__forceinline void Set(MPoint<XMVector_Wrapper<3> >* out, const XMFloat_Wrapper& x, const XMFloat_Wrapper& y, const XMFloat_Wrapper& z)
{
	*out = MakePoint(MakeVector((XMVECTOR&) x, (XMVECTOR&) y, (XMVECTOR&) z));
}

// initialise from floats, ideally in memory, i.e. not local or temporary variables.
__forceinline void Set(MVector<XMVector_Wrapper<3> >* out, const float& x, const float& y, const float& z)
{
	Set(out, XMFloat_Wrapper(x),  XMFloat_Wrapper(y),  XMFloat_Wrapper(z));
}

__forceinline void Set(MPoint<XMVector_Wrapper<3> >* out, const float& x, const float& y, const float& z)
{
	Set(out, XMFloat_Wrapper(x),  XMFloat_Wrapper(y),  XMFloat_Wrapper(z));
}

__forceinline void Set(MVector<XMVector_Wrapper<4> >* v, MConstVector<XMVector_Wrapper<3> > xyz, XMFloat_Wrapper w)
{
	*v = xyz;
	v->W(w);
}

///////////////////////////////////////////////////////////////////////////////////////////

typedef MMatrix<BasicMatrix<MVector<XMVector_Wrapper<3> >, 3> > XMMatrixWrapper3;

__forceinline XMMATRIX XMMatrixTranspose3(const XMMATRIX& m)
{
	XMMATRIX P;
    XMMATRIX MT;

	P.r[0] = XMVectorMergeXY(m.r[0], m.r[2]); // m00m20m01m21
    P.r[1] = XMVectorMergeXY(m.r[1], m.r[1]); // m10m10m11m11
    P.r[2] = XMVectorMergeZW(m.r[0], m.r[2]); // m02m22m03m23
    P.r[3] = XMVectorMergeZW(m.r[1], m.r[1]); // m12m32m13m33

    MT.r[0] = XMVectorMergeXY(P.r[0], P.r[1]); // m00m10m20m30
    MT.r[1] = XMVectorMergeZW(P.r[0], P.r[1]); // m01m11m21m31
    MT.r[2] = XMVectorMergeXY(P.r[2], P.r[3]); // m02m12m22m32

    return MT;
}

/// Make orthonormal matrix from 'nearly' orthonormal matrix
inline void XMMatrixNormalise3(XMMATRIX& m) 
{
	XMVECTOR ax = m.r[0];
	XMVECTOR ay = m.r[1];
	XMVECTOR az = m.r[2];
	
	XMVECTOR one = XMVectorSplatOne();
	XMVECTOR half = Maths::XMVECTOR_CONSTANT_HALF;

	XMVECTOR xx = XMVectorSplatOne() - XMVector3LengthSq(ax);
	XMVECTOR yy = XMVectorSplatOne() - XMVector3LengthSq(ay);

	ax *= (one + xx * half);
	ay *= (one + yy * half);

	XMVECTOR xy = XMVector3Dot(ax, ay);

	// Want to replace one component of ay corresponding to the largest component of ax.
	// Code below is not perfect but quicker than doing the full logic.
	XMVECTOR ayn = -(xy - ax * ay) / ax;
	const XMVECTOR XMVECTOR_CONSTANT_INVROOT3 = {0.577350269, 0.577350269, 0.577350269, 0.577350269};
	ay = XMVectorSelect(ay, ayn, XMVectorGreater(XMVectorAbs(ax), XMVECTOR_CONSTANT_INVROOT3));

	az = XMVector3Cross(ax, ay);

	m.r[0] = ax;
	m.r[1] = ay;
	m.r[2] = az;
}

inline const XMMatrixWrapper3& MakeMatrix(const XMMATRIX& m)
{
	return (const XMMatrixWrapper3&) m;
}

/// Transpose the matrix.
inline XMMatrixWrapper3 operator ~ (const XMMatrixWrapper3& m)
{
	return MakeMatrix(XMMatrixTranspose3((const XMMATRIX&) m));
}

inline void Normalise(XMMatrixWrapper3* m)
{
	XMMatrixNormalise3((XMMATRIX&) *m);
}

////////////////////////
// matrix / vector

__forceinline XMVECTOR XMVectorMatrixMultiply3(const XMVECTOR& v, const XMMATRIX& m)
{
    XMVECTOR Z = XMVectorSplatZ(v);
    XMVECTOR Y = XMVectorSplatY(v);
    XMVECTOR X = XMVectorSplatX(v);

    XMVECTOR result = XMVectorMultiply(Z, m.r[2]);
    result = XMVectorMultiplyAdd(Y, m.r[1], result);
    result = XMVectorMultiplyAdd(X, m.r[0], result);

    return result;
}

__forceinline XMVECTOR XMMatrixVectorMultiply3(const XMMATRIX& m, const XMVECTOR& v)
{
    XMVECTOR result = XMVector3Dot(m.r[0], v);
	result = XMVectorInsert(result, XMVector3Dot(m.r[1], v), 0, 0, 1, 0, 0);
    result = XMVectorInsert(result, XMVector3Dot(m.r[2], v), 0, 0, 0, 1, 0);

    return result;
}

__forceinline MVector<XMVector_Wrapper<3> > operator * (const MConstVector<XMVector_Wrapper<3> >& v, const XMMatrixWrapper3& m)
{
	return MakeVector(XMVectorMatrixMultiply3((XMVECTOR&) v, (XMMATRIX&) m));
}

__forceinline MVector<XMVector_Wrapper<3> > operator * (const MVector<XMVector_Wrapper<3> >& v, const XMMatrixWrapper3& m)
{
	return MakeVector(XMVectorMatrixMultiply3((XMVECTOR&) v, (XMMATRIX&) m));
}

__forceinline MPoint<XMVector_Wrapper<3> > operator * (const MPoint<XMVector_Wrapper<3> >& v, const XMMatrixWrapper3& m)
{
	return MakePoint(XMVectorMatrixMultiply3((XMVECTOR&) v, (XMMATRIX&) m));
}

__forceinline MVector<XMVector_Wrapper<3> > operator * (const MNormal<XMVector_Wrapper<3> >& v, const XMMatrixWrapper3& m)
{
	return MakeVector(XMVectorMatrixMultiply3((XMVECTOR&) v, (XMMATRIX&) m));
}

__forceinline MVector<XMVector_Wrapper<3> > operator * (const XMMatrixWrapper3& m, const MConstVector<XMVector_Wrapper<3> >& v)
{
	return MakeVector(XMMatrixVectorMultiply3((XMMATRIX&) m, (XMVECTOR&) v));
}

__forceinline MVector<XMVector_Wrapper<3> > operator * (const XMMatrixWrapper3& m, const MVector<XMVector_Wrapper<3> >& v)
{
	return MakeVector(XMMatrixVectorMultiply3((XMMATRIX&) m, (XMVECTOR&) v));
}

__forceinline MPoint<XMVector_Wrapper<3> > operator * (const XMMatrixWrapper3& m, const MPoint<XMVector_Wrapper<3> >& v)
{
	return MakePoint(XMMatrixVectorMultiply3((XMMATRIX&) m, (XMVECTOR&) v));
}

__forceinline MVector<XMVector_Wrapper<3> > operator * (const XMMatrixWrapper3& m, const MNormal<XMVector_Wrapper<3> >& v)
{
	return MakeVector(XMMatrixVectorMultiply3((XMMATRIX&) m, (XMVECTOR&) v));
}

//////////////////////////////////////////////////

__forceinline XMMATRIX XMMatrixMatrixMultiply33(const XMMATRIX& m1, const XMMATRIX& m2)
{
    XMVECTOR X, Y, Z;
    XMMATRIX Result;

    X = XMVectorSplatX(m1.r[0]);
    Y = XMVectorSplatY(m1.r[0]);
    Z = XMVectorSplatZ(m1.r[0]);
 
    Result.r[0] = XMVectorMultiply(X, m2.r[0]);
    Result.r[0] = XMVectorMultiplyAdd(Y, m2.r[1], Result.r[0]);
    Result.r[0] = XMVectorMultiplyAdd(Z, m2.r[2], Result.r[0]);

    X = XMVectorSplatX(m1.r[1]);
    Y = XMVectorSplatY(m1.r[1]);
    Z = XMVectorSplatZ(m1.r[1]);

    Result.r[1] = XMVectorMultiply(X, m2.r[0]);
    Result.r[1] = XMVectorMultiplyAdd(Y, m2.r[1], Result.r[1]);
    Result.r[1] = XMVectorMultiplyAdd(Z, m2.r[2], Result.r[1]);
  
    X = XMVectorSplatX(m1.r[2]);
    Y = XMVectorSplatY(m1.r[2]);
    Z = XMVectorSplatZ(m1.r[2]);

    Result.r[2] = XMVectorMultiply(X, m2.r[0]);
    Result.r[2] = XMVectorMultiplyAdd(Y, m2.r[1], Result.r[2]);
    Result.r[2] = XMVectorMultiplyAdd(Z, m2.r[2], Result.r[2]);

    return Result;
}

__forceinline XMMatrixWrapper3 operator * (const XMMatrixWrapper3& m1, const XMMatrixWrapper3& m2)
{
	return MakeMatrix(XMMatrixMatrixMultiply33((XMMATRIX&) m1, (XMMATRIX&) m2));
}

/////////////////////////////////////////////////

__forceinline XMVector_Wrapper<3>::DotProductType Magnitude(const MVector<XMVector_Wrapper<3> >& v)
{
	return XMFloat_Wrapper(XMVectorSqrt((const XMVECTOR&) (v | v)));
}

__forceinline XMVector_Wrapper<3>::DotProductType Magnitude(const MConstVector<XMVector_Wrapper<3> >& v)
{
	return XMFloat_Wrapper(XMVectorSqrt((const XMVECTOR&) (v | v)));
}

__forceinline bool operator == (const MPoint<XMVector_Wrapper<3> >& p1, const MPoint<XMVector_Wrapper<3> >& p2)
{
	return XMVector3Equal((const XMVECTOR&) p1, (const XMVECTOR&) p2) != 0;
}

__forceinline MNormal<XMVector_Wrapper<3> > Normalised(const MVector<XMVector_Wrapper<3> >& v)
{
	return MakeNormal(XMVector3Normalize((const XMVECTOR&) v));
}

__forceinline MPoint<XMVector_Wrapper<3> > Min(const MPoint<XMVector_Wrapper<3> >& p1, const MPoint<XMVector_Wrapper<3> >& p2)
{
	return MakePoint(XMVectorMin((const XMVECTOR&) p1, (const XMVECTOR&) p2));
}

__forceinline MPoint<XMVector_Wrapper<3> > Max(const MPoint<XMVector_Wrapper<3> >& p1, const MPoint<XMVector_Wrapper<3> >& p2)
{
	return MakePoint(XMVectorMax((const XMVECTOR&) p1, (const XMVECTOR&) p2));
}

__forceinline MVector<XMVector_Wrapper<3> > ComponentMultiply(const MConstVector<XMVector_Wrapper<3> >& v1, const MConstVector<XMVector_Wrapper<3> >& v2)
{
	return MakeVector(XMVectorMultiply((const XMVECTOR&) v1, (const XMVECTOR&) v2));
}

__forceinline MVector<XMVector_Wrapper<3> > Abs(const MVector<XMVector_Wrapper<3> >& v)
{
	return MakeVector(XMVectorAbs((const XMVECTOR&) v));
}

////////////////////////////////////////////////////////////

}

namespace Maths
{

template<int N> struct Constants<UtMaths::MVector<UtMaths::XMVector_Wrapper<N> > >
{
	typedef UtMaths::MVector<UtMaths::XMVector_Wrapper<N> > VectorType;

	static const VectorType Zero() {return (VectorType&) XMVectorZero();}
	static const VectorType Null() {return (VectorType&) XMVectorZero();}
	static const VectorType& X() {return (VectorType&) XMVECTOR_CONSTANT_X;}
	static const VectorType& Y() {return (VectorType&) XMVECTOR_CONSTANT_Y;}
	static const VectorType& Z() {return (VectorType&) XMVECTOR_CONSTANT_Z;}
	static const VectorType& NegativeX() {return (VectorType&) XMVECTOR_CONSTANT_NEGATIVE_X;}
	static const VectorType& NegativeY() {return (VectorType&) XMVECTOR_CONSTANT_NEGATIVE_Y;}
	static const VectorType& NegativeZ() {return (VectorType&) XMVECTOR_CONSTANT_NEGATIVE_Z;}
	static const VectorType& XY() {return (VectorType&) XMVECTOR_CONSTANT_XY;}
	static const VectorType& XZ() {return (VectorType&) XMVECTOR_CONSTANT_XZ;}
	static const VectorType& YZ() {return (VectorType&) XMVECTOR_CONSTANT_YZ;}
	static const VectorType& XYZ() {return (VectorType&) XMVECTOR_CONSTANT_XYZ;}
};

template<int N> struct Constants<UtMaths::MNormal<UtMaths::XMVector_Wrapper<N> > >
{
	typedef UtMaths::MNormal<UtMaths::XMVector_Wrapper<N> > NormalType;

	static const NormalType& X() {return (NormalType&) XMVECTOR_CONSTANT_X;}
	static const NormalType& Y() {return (NormalType&) XMVECTOR_CONSTANT_Y;}
	static const NormalType& Z() {return (NormalType&) XMVECTOR_CONSTANT_Z;}
	static const NormalType& NegativeX() {return (NormalType&) XMVECTOR_CONSTANT_NEGATIVE_X;}
	static const NormalType& NegativeY() {return (NormalType&) XMVECTOR_CONSTANT_NEGATIVE_Y;}
	static const NormalType& NegativeZ() {return (NormalType&) XMVECTOR_CONSTANT_NEGATIVE_Z;}
};

template<int N> struct Constants<UtMaths::MPoint<UtMaths::XMVector_Wrapper<N> > >
{
	typedef UtMaths::MPoint<UtMaths::XMVector_Wrapper<N> > PointType;

	static const PointType& Origin() {return (PointType&) XMVectorZero();}
	static const PointType& Min() {return (PointType&) XMVECTOR_CONSTANT_MIN;}
	static const PointType& Max() {return (PointType&) XMVECTOR_CONSTANT_MAX;}
	static const PointType& NegativeMax() {return (PointType&) XMVECTOR_CONSTANT_NEGATIVE_MAX;}
};

}



