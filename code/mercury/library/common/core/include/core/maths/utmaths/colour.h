//////////////////////////////////////////////////////////////
//  Colour
//////////////////////////////////////////////////////////////

#ifndef _UTILITY_COLOUR_H_
#define _UTILITY_COLOUR_H_

#pragma once

#include "maths.h"

namespace UtMaths
{

/// 4 component colour vector with named elements.
template <typename T>
class VectorRGBA : public BaseVector<T, 4>
{
public:
	/// Named elements for convenience.
	T r, g, b, a;

	// constructors
	VectorRGBA() {} ///< Default constructor.
	VectorRGBA(T pr, T pg, T pb, T pa) : r(pr), g(pg), b(pb), a(pa) {}  ///< Three element constructor.

	template <typename Evaluator>
	VectorRGBA(const Evaluator& e) {BaseVector<T, 4>::operator = (e);}

	// access
	T R() const {return r;} ///< Access r component.
	T G() const {return g;} ///< Access g component.
	T B() const {return b;} ///< Access b component.
	T A() const {return a;} ///< Access a component.
};

}

class ColourARGB;

/// Colour stored as four floating point components.
class ColourVectorRGBA
{
public:
	float r, g, b, a;

	ColourVectorRGBA() {} ///<.
	ColourVectorRGBA(float ir, float ig, float ib, float ia) {r = ir; g = ig; b = ib; a = ia;} ///<.
	ColourVectorRGBA(float ir, float ig, float ib) {r = ir; g = ig; b = ib; a = 1.f;} ///<.
	explicit ColourVectorRGBA(const ColourARGB& c); ///<.
	ColourVectorRGBA(const ColourVectorRGBA& c) : r(c.r), g(c.g), b(c.b), a(c.a) {} ///<.

	inline void SetRGBA(float ir, float ig, float ib, float ia) {r = ir; g = ig; b = ib; a = ia;} ///<.

	inline void operator +=(const ColourVectorRGBA& c); ///<.
	inline void operator -=(const ColourVectorRGBA& c); ///<.
	inline void operator *=(const ColourVectorRGBA& c); ///<.
	inline void operator /=(const ColourVectorRGBA& c); ///<.
	inline void operator *=(float f); ///<.

	inline void MultiplyUC(float f); ///<.
	inline void AddUC(const ColourVectorRGBA& c); ///<.
	inline void Blend(const ColourVectorRGBA& c1, const ColourVectorRGBA& c2, float f); ///<.
	inline void MaximizeBrightness(); ///<.
	inline void GetBrightnessInAlpha() {a = (r + g + b) / 3.f;} ///<.
	inline void AddRGB(float ir, float ig, float ib) {r += ir; g += ig; b += ib;} ///<.
	inline void AddRGB(const ColourVectorRGBA& c) {r += c.r; g += c.g; b += c.b;} ///<.
	inline void ScaleRGB(const ColourVectorRGBA& c) {r *= c.r; g *= c.g; b *= c.b;} ///<.
	inline void ScaleRGB(float f) {r *= f; g *= f; b *= f;} ///<.
	inline void Fog(const ColourVectorRGBA& fogColour, float fogFactor); ///<.
	ColourVectorRGBA GreyScale() {float f = Brightness(); return ColourVectorRGBA(f, f, f, a);} ///<.

	ColourVectorRGBA operator +(const ColourVectorRGBA& c) const {ColourVectorRGBA c1(*this); c1 += c; return c1;} ///<.
	ColourVectorRGBA operator *(const ColourVectorRGBA& c) const {ColourVectorRGBA c1(*this); c1 *= c; return c1;} ///<.
	ColourVectorRGBA operator *(float f) const {ColourVectorRGBA c1(*this); c1 *= f; return c1;} ///<.
	ColourVectorRGBA operator /(const ColourVectorRGBA& c) const {ColourVectorRGBA c1(*this); c1 /= c; return c1;} ///<.
	ColourVectorRGBA operator -(const ColourVectorRGBA& c) const {ColourVectorRGBA c1(*this); c1 -= c; return c1;} ///<.

	float Brightness() const {return (r + g + b) / 3.f;} ///<.

	// Colours
	const static ColourVectorRGBA White; ///<.
	const static ColourVectorRGBA Black; ///<.
};

/// Colour stored in four bytes.
class ColourARGB
{
public:
	// constructors
	ColourARGB() {packed = 0;} ///<.
	ColourARGB(int r, int g, int b) {packed = (255 << 24) + (r << 16) + (g << 8) + (b << 0);} ///<.
	ColourARGB(int a, int r, int g, int b) {packed = (a << 24) + (r << 16) + (g << 8) + (b << 0);} ///<.
	ColourARGB(float a, int r, int g, int b) {packed = ((int)a << 24) + (r << 16) + (g << 8) + (b << 0);} ///<.
	ColourARGB(const ColourARGB& c) {packed = c.packed;} ///<.
	explicit ColourARGB(unsigned long p) {packed = p;} ///<.
	explicit ColourARGB(const ColourVectorRGBA& c); ///<.

	// initialisation
	void Set(long ic) {packed=ic;} ///<.
	void SetRGB(int r, int g, int b) {packed = (r << 16) + (g << 8) + (b << 0);} ///<.
	inline void SetAlpha(int a); ///<.
	void SetARGB(int a, int r, int g, int b) {packed = (a << 24) + (r << 16) + (g << 8) + (b << 0);} ///<.

	// return information
	UT_INLINE int R() const {return (packed >> 16) & 0xff;} ///<.
	UT_INLINE int G() const {return (packed >> 8) & 0xff;} ///<.
	UT_INLINE int B() const {return (packed >> 0) & 0xff;} ///<.
	UT_INLINE int A() const {return (packed >> 24) & 0xff;} ///<.
	UT_INLINE int Alpha() const {return (packed >> 24) & 0xff;} ///<.
	ColourARGB Inverted() const {ColourARGB c(*this); c.Invert(); return c;} ///<.
	ColourARGB Scaled(float s) const {ColourARGB c(*this); c.Scale(s); return c;} ///<.
	ColourARGB operator +(ColourARGB c) const {ColourARGB c1(*this); c1 += c; return c1;} ///<.
	ColourARGB operator *(ColourARGB c) const {ColourARGB c1(*this); c1 *= c; return c1;} ///<.
	ColourARGB operator /(ColourARGB c) const {ColourARGB c1(*this); c1 /= c; return c1;} ///<.
	ColourARGB operator -(ColourARGB c) const {ColourARGB c1(*this); c1 -= c; return c1;} ///<.
	float Brightness() const {return (float) (R() + G() + B()) / (255.f * 3.f);} ///<.
	long Packed() const {return packed;} ///<.
	ColourVectorRGBA Vector() const 
	{
		return ColourVectorRGBA(
			R() * (1.f / 255.f),
			G() * (1.f / 255.f),
			B() * (1.f / 255.f),
			A() * (1.f / 255.f));
	} ///<
	inline ColourVectorRGBA ColourARGB::AmbientAndScalingVector(ColourARGB ambience) const; ///<.
	bool operator !=(ColourARGB c) const {return c.packed != packed;} ///<.
	operator unsigned long () const {return packed;} ///<.

	// modify
	inline void AddRGB(int ar, int ag, int ab); ///<.
	inline void AddRGB(const ColourARGB c); ///<.
	inline void SubtractRGB(int ar, int ag, int ab); ///<.
	inline void Invert(); ///<.
	inline void Scale(float s); ///<.
	UT_INLINE void ScaleDown(float s); ///<.
	inline void ScaleRGB(float s); ///<.
	inline void ScaleRGB(float rs, float gs, float bs); ///<.
	inline void operator +=(ColourARGB c); ///<.
	inline void operator -=(ColourARGB c); ///<.
	inline void operator *=(ColourARGB c); ///<.
	inline void operator *=(float f) {Scale(f);} ///<.
	inline void operator /=(ColourARGB c); ///<.
	inline void ScaleAlpha(float s); ///<.
	inline void SubtractAlpha(int ar); ///<.
	inline void Clear() {packed = 0;} ///<.
	inline void MaximizeBrightness(); ///<.
	inline void Fog(ColourARGB fogColour, float fogFactor); ///<.
	void Blend(ColourARGB c1, ColourARGB c2, float f)
	{
		SetARGB((int) (c1.A() * f + c2.A() * (1.f - f)), 
				(int) (c1.R() * f + c2.R() * (1.f - f)), 
				(int) (c1.G() * f + c2.G() * (1.f - f)), 
				(int) (c1.B() * f + c2.B() * (1.f - f)));
	} ///<.

	// colours
	const static ColourARGB Grey(int c) {assert(c >= 0 && c <= 255); return ColourARGB(c,c,c);} ///<.
	const static ColourARGB Black; ///<.
	const static ColourARGB White; ///<.
	const static ColourARGB Red; ///<.
	const static ColourARGB Orange; ///<.
	const static ColourARGB Green; ///<.
	const static ColourARGB Blue; ///<.
	const static ColourARGB Yellow; ///<.
	const static ColourARGB Magenta; ///<.
	const static ColourARGB Cyan; ///<.

	inline static ColourARGB Max(const ColourARGB& c1, const ColourARGB& c2); ///<.
	//static ColourARGB Red() {return ColourARGB(255,0,0);}

private:
	union
	{
		unsigned long packed;

		struct
		{
#ifdef _PPC_
			U8 bAlpha;
			U8 bRed;
			U8 bGreen;
			U8 bBlue;
#else
			U8 bBlue;
			U8 bGreen;
			U8 bRed;
			U8 bAlpha;
#endif
		} byte;
	};
};

/////////////////////////////////////////////////////////
// ColourVectorARGB
/////////////////////////////////////////////////////////

inline ColourVectorRGBA::ColourVectorRGBA(const ColourARGB& c)
{
	r = ((float) c.R()) / 255.f;
	g = ((float) c.G()) / 255.f;
	b = ((float) c.B()) / 255.f;
	a = ((float) c.A()) / 255.f;
}

/////////////////////////////////////////////////////////
// ColourRGBA
/////////////////////////////////////////////////////////

inline ColourARGB::ColourARGB(const ColourVectorRGBA& c)
{
	int r = (int) (c.r * 255.f); if (r > 255) r = 255; if (r < 0) r = 0;
	int g = (int) (c.g * 255.f); if (g > 255) g = 255; if (g < 0) g = 0;
	int b = (int) (c.b * 255.f); if (b > 255) b = 255; if (b < 0) b = 0;
	int a = (int) (c.a * 255.f); if (a > 255) a = 255; if (a < 0) a = 0;

	packed = (a << 24) + (r << 16) + (g << 8) + (b << 0);
}

inline void ColourARGB::SetAlpha(int a)
{
	packed = (a << 24) | (packed & 0xffffff);
}

inline void ColourARGB::SubtractAlpha(int ar)
{
	int a = A() - ar; if (a < 0) a = 0;
	SetAlpha(a);
}

inline void ColourARGB::ScaleAlpha(float s)
{
	int a = (int)(A() * s); if (a > 255) a = 255;
	SetAlpha(a);
}

inline void ColourARGB::AddRGB(ColourARGB c)
{
	int r = R() + c.R(); if (r > 255) r = 255;
	int g = G() + c.G(); if (g > 255) g = 255;
	int b = B() + c.B(); if (b > 255) b = 255;
	int a = A();
	SetARGB(a,r,g,b);
}

inline void ColourARGB::AddRGB(int ar, int ag, int ab)
{
	int r = R() + ar; if (r > 255) r = 255;
	int g = G() + ag; if (g > 255) g = 255;
	int b = B() + ab; if (b > 255) b = 255;
	int a = A();
	SetARGB(a,r,g,b);
}

inline void ColourARGB::SubtractRGB(int ar, int ag, int ab)
{
	int r = R() - ar; if (r < 0) r = 0;
	int g = G() - ag; if (g < 0) g = 0;
	int b = B() - ab; if (b < 0) b = 0;
	int a = A();
	SetARGB(a,r,g,b);
}

inline void ColourARGB::Invert()
{
	int r = 255 - R();
	int g = 255 - G();
	int b = 255 - B();
	int a = 255 - A();
	SetARGB(a,r,g,b);
}

inline void ColourARGB::Scale(float s)
{
	if (s != 1.f)
	{
		assert(s >= 0.f);

		int scale = (int) (s * 255.f);

		int r = (R() * scale) / 255 ; if (r > 255) r = 255;
		int g = (G() * scale) / 255 ; if (g > 255) g = 255;
		int b = (B() * scale) / 255 ; if (b > 255) b = 255;
		int a = (A() * scale) / 255 ; if (a > 255) a = 255;
		SetARGB(a,r,g,b);
	}
}

inline void ColourARGB::ScaleDown(float s)
{
	int scale = (int) (s * 255.f);

	int r = (R() * scale) / 255;
	int g = (G() * scale) / 255; 
	int b = (B() * scale) / 255; 
	int a = (A() * scale) / 255;
	SetARGB(a,r,g,b);
}

inline void ColourARGB::ScaleRGB(float s)
{
	if (s != 1.f)
	{
		assert(s >= 0.f);

		int scale = (int) (s * 255.f);

		int r = (R() * scale) / 255 ; if (r > 255) r = 255;
		int g = (G() * scale) / 255 ; if (g > 255) g = 255;
		int b = (B() * scale) / 255 ; if (b > 255) b = 255;
		int a = A();
		SetARGB(a,r,g,b);
	}
}

inline void ColourARGB::ScaleRGB(float rs, float gs, float bs)
{
	int rscale = (int) (rs * 255.f);
	int gscale = (int) (gs * 255.f);
	int bscale = (int) (bs * 255.f);

	int r = (R() * rscale) / 255 ; if (r > 255) r = 255;
	int g = (G() * gscale) / 255 ; if (g > 255) g = 255;
	int b = (B() * bscale) / 255 ; if (b > 255) b = 255;
	int a = A();
	SetARGB(a,r,g,b);
}

inline void ColourARGB::MaximizeBrightness()
{
	int	max	= R();
	int	t	= G(); if (t > max) max = t;

	t = B(); if (t > max) max = t;

	if (max > 0)
		Scale(255.f / (float) max);
}

inline void ColourARGB::operator +=(ColourARGB c)
{
	int r = R() + c.R();		if (r > 255) r = 255;
	int g = G() + c.G();	if (g > 255) g = 255;
	int b = B() + c.B();	if (b > 255) b = 255;
	int a = A() + c.A();	if (a > 255) a = 255;

	SetARGB(a,r,g,b);
}

inline void ColourARGB::operator *=(ColourARGB c)
{
	int r = R() * c.R() / (255);
	int g = G() * c.G() / (255);
	int b = B() * c.B() / (255);
	int a = A() * c.A() / (255);

	SetARGB(a,r,g,b);
}

inline void ColourARGB::operator /=(ColourARGB c)
{
	int r; 
	if (c.R() > 0) 
		r = 255 * R() / c.R();
	else
		r = 255;

	int g; 
	if (c.G() > 0) 
		g = 255 * G() / c.G();
	else
		g = 255;

	int b; 
	if (c.B() > 0) 
		b = 255 * B() / c.B();
	else
		b = 255;

	int a; 
	if (c.A() > 0) 
		a = 255 * A() / c.A();
	else
		a = 255;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	if (a > 255) a = 255;

	SetARGB(a,r,g,b);
}

inline void ColourARGB::operator -=(ColourARGB c) 
{
	int r = R() - c.R(); if (r < 0) r = 0;
	int g = G() - c.G(); if (g < 0) g = 0;
	int b = B() - c.B(); if (b < 0) b = 0;
	int a = A() - c.A(); if (a < 0) a = 0;
	SetARGB(a,r,g,b);
}

inline void ColourARGB::Fog(ColourARGB fogColour, float fogFactor) 
{
	assert(fogFactor >= 0.f && fogFactor <= 1.f);

	fogColour.ScaleRGB(1.f - fogFactor);
	ScaleRGB(fogFactor);
	AddRGB(fogColour);
}

inline ColourVectorRGBA ColourARGB::AmbientAndScalingVector(const ColourARGB ambience) const
{
	ColourVectorRGBA c(ambience);

	// get the directional part
	ColourARGB d(*this - ambience);

	c.a = 0.f;
	float b = d.R() / 255.f; if (b > c.a) c.a = b;
	b = d.G() / 255.f; if (b > c.a) c.a = b;
	b = d.B() / 255.f; if (b > c.a) c.a = b;

	return c;
}

inline ColourARGB ColourARGB::Max(const ColourARGB& c1, const ColourARGB& c2)
{
	int r = c1.R(); if (r < c2.R()) r = c2.R();
	int g = c1.G(); if (g < c2.G()) g = c2.G();
	int b = c1.B(); if (b < c2.B()) b = c2.B();
	int a = c1.A(); if (a < c2.A()) a = c2.A();

	return ColourARGB(a, r, g, b);
}

////////////////////////////////////////////////////////////

inline void ColourVectorRGBA::operator *=(const ColourVectorRGBA& c)
{
	r *= c.r;
	g *= c.g;
	b *= c.b;
	a *= c.a;
}

inline void ColourVectorRGBA::operator *=(float f)
{
	r *= f;
	g *= f;
	b *= f;
	a *= f;
}

inline void ColourVectorRGBA::operator /=(const ColourVectorRGBA& c)
{
	r /= c.r;
	g /= c.g;
	b /= c.b;
	a /= c.a;
}

inline void ColourVectorRGBA::operator +=(const ColourVectorRGBA& c)
{
	r += c.r;
	g += c.g;
	b += c.b;
	a += c.a;
}

inline void ColourVectorRGBA::operator -=(const ColourVectorRGBA& c)
{
	r -= c.r;
	g -= c.g;
	b -= c.b;
	a -= c.a;
}

// unclamped methods
inline void ColourVectorRGBA::MultiplyUC(float f)
{
	r *= f;
	g *= f;
	b *= f;
	a *= f;
}

inline void ColourVectorRGBA::AddUC(const ColourVectorRGBA& c)
{
	r += c.r;
	g += c.g;
	b += c.b;
	a += c.a;
}

inline void ColourVectorRGBA::Blend(const ColourVectorRGBA& c1, const ColourVectorRGBA& c2, float f)
{
	float f1 = 1.f - f;
	r = c1.r * f + c2.r * f1;
	g = c1.g * f + c2.g * f1;
	b = c1.b * f + c2.b * f1;
	a = c1.a * f + c2.a * f1;
}

inline void ColourVectorRGBA::MaximizeBrightness()
{
	float max = r;
	float t = g; if (t > max) max = t;
	t = b; if (t > max) max = t;

	if (max > 0)
		MultiplyUC(1.f / max);
}

inline void ColourVectorRGBA::Fog(const ColourVectorRGBA& fogColour, float fogFactor) 
{
	assert(fogFactor >= 0.f && fogFactor <= 1.f);
	Blend(*this, fogColour, fogFactor);
}





#endif
