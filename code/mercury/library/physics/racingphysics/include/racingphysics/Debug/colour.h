#ifndef RP_COLOUR_H
#define RP_COLOUR_H
#ifdef _MSC_VER
#pragma once
#endif

namespace RacingPhysics
{

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
	bool operator !=(ColourARGB c) const {return c.packed != packed;} ///<.
	//operator unsigned long () const {return packed;} ///<.

	// modify
	inline ColourARGB& AddRGB(int ar, int ag, int ab); ///<.
	inline ColourARGB& AddRGB(const ColourARGB c); ///<.
	inline ColourARGB& SubtractRGB(int ar, int ag, int ab); ///<.
	inline ColourARGB& Invert(); ///<.
	inline ColourARGB& Scale(float s); ///<.
	UT_INLINE ColourARGB& ScaleDown(float s); ///<.
	inline ColourARGB& ScaleRGB(float s); ///<.
	inline ColourARGB& ScaleRGB(float rs, float gs, float bs); ///<.
	inline void operator +=(ColourARGB c); ///<.
	inline void operator -=(ColourARGB c); ///<.
	inline void operator *=(ColourARGB c); ///<.
	inline void operator *=(float f) {Scale(f);} ///<.
	inline void operator /=(ColourARGB c); ///<.
	inline ColourARGB& ScaleAlpha(float s); ///<.
	inline ColourARGB& SubtractAlpha(int ar); ///<.
	inline ColourARGB& Clear() {packed = 0; return *this;} ///<.
	inline ColourARGB& MaximizeBrightness(); ///<.
	inline ColourARGB& Fog(ColourARGB fogColour, float fogFactor); ///<.
	ColourARGB& Blend(ColourARGB c1, ColourARGB c2, float f)
	{
		SetARGB((int) (c1.A() * f + c2.A() * (1.f - f)), 
				(int) (c1.R() * f + c2.R() * (1.f - f)), 
				(int) (c1.G() * f + c2.G() * (1.f - f)), 
				(int) (c1.B() * f + c2.B() * (1.f - f)));
		return *this;
	} ///<.

	// colours
	const static ColourARGB Grey(int c) {RP_ASSERT(c >= 0 && c <= 255); return ColourARGB(c,c,c);} ///<.
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
// ColourRGBA
/////////////////////////////////////////////////////////

inline void ColourARGB::SetAlpha(int a)
{
	packed = (a << 24) | (packed & 0xffffff);
}

inline ColourARGB& ColourARGB::SubtractAlpha(int ar)
{
	int a = A() - ar; if (a < 0) a = 0;
	SetAlpha(a);
	return *this;
}

inline ColourARGB& ColourARGB::ScaleAlpha(float s)
{
	int a = (int)(A() * s); if (a > 255) a = 255;
	SetAlpha(a);
	return *this;
}

inline ColourARGB& ColourARGB::AddRGB(ColourARGB c)
{
	int r = R() + c.R(); if (r > 255) r = 255;
	int g = G() + c.G(); if (g > 255) g = 255;
	int b = B() + c.B(); if (b > 255) b = 255;
	int a = A();
	SetARGB(a,r,g,b);
	return *this;
}

inline ColourARGB& ColourARGB::AddRGB(int ar, int ag, int ab)
{
	int r = R() + ar; if (r > 255) r = 255;
	int g = G() + ag; if (g > 255) g = 255;
	int b = B() + ab; if (b > 255) b = 255;
	int a = A();
	SetARGB(a,r,g,b);
	return *this;
}

inline ColourARGB& ColourARGB::SubtractRGB(int ar, int ag, int ab)
{
	int r = R() - ar; if (r < 0) r = 0;
	int g = G() - ag; if (g < 0) g = 0;
	int b = B() - ab; if (b < 0) b = 0;
	int a = A();
	SetARGB(a,r,g,b);
	return *this;
}

inline ColourARGB& ColourARGB::Invert()
{
	int r = 255 - R();
	int g = 255 - G();
	int b = 255 - B();
	int a = 255 - A();
	SetARGB(a,r,g,b);
	return *this;
}

inline ColourARGB& ColourARGB::Scale(float s)
{
	if (s != 1.f)
	{
		RP_ASSERT(s >= 0.f);

		int scale = (int) (s * 255.f);

		int r = (R() * scale) / 255 ; if (r > 255) r = 255;
		int g = (G() * scale) / 255 ; if (g > 255) g = 255;
		int b = (B() * scale) / 255 ; if (b > 255) b = 255;
		int a = (A() * scale) / 255 ; if (a > 255) a = 255;
		SetARGB(a,r,g,b);
	}
	return *this;
}

inline ColourARGB& ColourARGB::ScaleDown(float s)
{
	int scale = (int) (s * 255.f);

	int r = (R() * scale) / 255;
	int g = (G() * scale) / 255; 
	int b = (B() * scale) / 255; 
	int a = (A() * scale) / 255;
	SetARGB(a,r,g,b);
	return *this;
}

inline ColourARGB& ColourARGB::ScaleRGB(float s)
{
	if (s != 1.f)
	{
		RP_ASSERT(s >= 0.f);

		int scale = (int) (s * 255.f);

		int r = (R() * scale) / 255 ; if (r > 255) r = 255;
		int g = (G() * scale) / 255 ; if (g > 255) g = 255;
		int b = (B() * scale) / 255 ; if (b > 255) b = 255;
		int a = A();
		SetARGB(a,r,g,b);
	}
	return *this;
}

inline ColourARGB& ColourARGB::ScaleRGB(float rs, float gs, float bs)
{
	int rscale = (int) (rs * 255.f);
	int gscale = (int) (gs * 255.f);
	int bscale = (int) (bs * 255.f);

	int r = (R() * rscale) / 255 ; if (r > 255) r = 255;
	int g = (G() * gscale) / 255 ; if (g > 255) g = 255;
	int b = (B() * bscale) / 255 ; if (b > 255) b = 255;
	int a = A();
	SetARGB(a,r,g,b);
	return *this;
}

inline ColourARGB& ColourARGB::MaximizeBrightness()
{
	int	max	= R();
	int	t	= G(); if (t > max) max = t;

	t = B(); if (t > max) max = t;

	if (max > 0)
		Scale(255.f / (float) max);
	return *this;
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

inline ColourARGB& ColourARGB::Fog(ColourARGB fogColour, float fogFactor) 
{
	RP_ASSERT(fogFactor >= 0.f && fogFactor <= 1.f);

	fogColour.ScaleRGB(1.f - fogFactor);
	ScaleRGB(fogFactor);
	AddRGB(fogColour);
	return *this;
}

inline ColourARGB ColourARGB::Max(const ColourARGB& c1, const ColourARGB& c2)
{
	int r = c1.R(); if (r < c2.R()) r = c2.R();
	int g = c1.G(); if (g < c2.G()) g = c2.G();
	int b = c1.B(); if (b < c2.B()) b = c2.B();
	int a = c1.A(); if (a < c2.A()) a = c2.A();

	return ColourARGB(a, r, g, b);
}

}

#endif
