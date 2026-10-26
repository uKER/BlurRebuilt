#ifndef XTWIN_NODEMATH_H
#define XTWIN_NODEMATH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin\WndMath.h"

//
// NgVector
//

struct NgVector
{
	float vx, vy;

public:
	inline NgVector(float xy = 0.0f) :
		vx(xy),
		vy(xy)
	{
	}

	inline NgVector(float x, float y) :
		vx(x),
		vy(y)
	{
	}

	inline NgVector(const WinPoint& pt) :
		vx((float)pt.x),
		vy((float)pt.y)
	{
	}

	explicit inline NgVector(const Gdiplus::PointF& pt) :
		vx(pt.X),
		vy(pt.Y)
	{
	}

public:
	inline NgVector(const NgVector& rhs) :
		vx(rhs.vx),
		vy(rhs.vy)
	{
	}

	NgVector& operator=(const NgVector& rhs)
	{
		vx = rhs.vx;
		vy = rhs.vy;

		return *this;
	}

public:
	inline operator Gdiplus::PointF() const
	{
		return Gdiplus::PointF(vx, vy);
	}

	inline operator Gdiplus::SizeF() const
	{
		return Gdiplus::SizeF(vx, vy);
	}
};

// NgVector operators

inline NgVector operator+(const NgVector& lhs, const NgVector& rhs)
{
	return NgVector(lhs.vx + rhs.vx, lhs.vy + rhs.vy);
}

inline NgVector operator-(const NgVector& lhs, const NgVector& rhs)
{
	return NgVector(lhs.vx - rhs.vx, lhs.vy - rhs.vy);
}

inline NgVector operator*(const NgVector& lhs, const NgVector& rhs)
{
	return NgVector(lhs.vx * rhs.vx, lhs.vy * rhs.vy);
}

inline NgVector operator/(const NgVector& lhs, const NgVector& rhs)
{
	return NgVector(lhs.vx / rhs.vx, lhs.vy / rhs.vy);
}

inline NgVector operator+=(NgVector& lhs, const NgVector& rhs)
{
	lhs.vx += rhs.vx;
	lhs.vy += rhs.vy;

	return lhs;
}

inline NgVector& operator-=(NgVector& lhs, const NgVector& rhs)
{
	lhs.vx -= rhs.vx;
	lhs.vy -= rhs.vy;

	return lhs;
}

inline NgVector& operator*=(NgVector& lhs, const NgVector& rhs)
{
	lhs.vx *= rhs.vx;
	lhs.vy *= rhs.vy;

	return lhs;
}

inline NgVector& operator/=(NgVector& lhs, const NgVector& rhs)
{
	lhs.vx /= rhs.vx;
	lhs.vy /= rhs.vy;

	return lhs;
}

inline NgVector operator-(const NgVector& rhs)
{
	return NgVector(-rhs.vx, -rhs.vy);
}

inline float Sum(const NgVector& rhs)
{
	return rhs.vx + rhs.vy;
}

inline float Dot(const NgVector& lhs, const NgVector& rhs)
{
	return Sum(lhs * rhs);
}

inline float LengthSq(const NgVector& rhs)
{
	return Sum(rhs * rhs);
}

inline float Length(const NgVector& rhs)
{
	return sqrtf(LengthSq(rhs));
}

inline float Rotation(const NgVector& rhs)
{
	return atan2f(rhs.vy, rhs.vx);
}

inline NgVector Rotation(float angle, float radius)
{
	return NgVector(cosf(angle) * radius, sinf(angle) * radius);
}

inline NgVector Unit(const NgVector& v)
{
	return v / Length(v);
}

inline float Normalise(NgVector& v)
{
	float length = Length(v);

	v /= length;

	return length;
}

inline NgVector Perpendicular(const NgVector& v)
{
	return NgVector(v.vy, -v.vx);
}

inline float AreaOfPara(const NgVector& lhs, const NgVector& rhs)
{
	return lhs.vx * rhs.vy - rhs.vx * lhs.vy;
}

//
// NgRect
//

struct NgRect
{
	NgVector pt;
	NgVector sz;

public:
	inline NgRect()
	{
	}

	inline NgRect(const NgVector& pt_, const NgVector& sz_) :
		pt(pt_),
		sz(sz_)
	{
	}

	inline NgRect(float x, float y, float width, float height) :
		pt(x, y),
		sz(width, height)
	{
	}

	inline NgRect(const WinRect& rect) :
		pt((float)rect.left, (float)rect.top),
		sz((float)rect.Width(), (float)rect.Height())
	{
	}

public:
	inline NgVector TopLeft() const
	{
		return pt;
	}

	inline NgVector TopRight() const
	{
		return NgVector(pt.vx + sz.vx, pt.vy);
	}

	inline NgVector BottomLeft() const
	{
		return NgVector(pt.vx, pt.vy + sz.vy);
	}

	inline NgVector BottomRight() const
	{
		return pt + sz;
	}

public:
	inline operator Gdiplus::RectF() const
	{
		return Gdiplus::RectF(pt.vx, pt.vy, sz.vx, sz.vy);
	}
};

inline NgRect Translate(const NgRect& rect, const NgVector& offset)
{
	return NgRect(rect.pt + offset, rect.sz);
}

inline bool PtInRect(const NgRect& rect, const NgVector& pt)
{
	return (pt.vx >= rect.pt.vx) && (pt.vy >= rect.pt.vy) && (pt.vx < (rect.pt.vx + rect.sz.vx)) && (pt.vy < (rect.pt.vy + rect.sz.vy));
}

inline bool Intersects(const NgRect& r0, const NgRect& r1)
{
	NgVector s0 = r0.TopLeft();
	NgVector s1 = r1.TopLeft();

	NgVector e0 = r0.BottomRight();
	NgVector e1 = r1.BottomRight();

	if (e1.vx < s0.vx)
		return false;

	if (e0.vx < s1.vx)
		return false;

	if (e1.vy < s0.vy)
		return false;

	if (e0.vy < s1.vy)
		return false;

	return true;
}

inline NgRect Inflate(const NgRect& rect, const NgVector& v)
{
	return NgRect(rect.pt - v, rect.sz + (v * 2.0f));
}

inline NgRect Inflate(const NgRect& rect, float x, float y)
{
	return Inflate(rect, NgVector(x, y));
}

inline NgRect Normalise(const NgRect& rect)
{
	NgRect newRect(rect);

	if (newRect.sz.vx < 0.0f)
	{
		newRect.pt.vx += newRect.sz.vx;
		newRect.sz.vx = -newRect.sz.vx;
	}

	if (newRect.sz.vy < 0.0f)
	{
		newRect.pt.vy += newRect.sz.vy;
		newRect.sz.vy = -newRect.sz.vy;
	}

	return newRect;
}

//
// NgColour
//

struct NgColour
{
	float r, g, b, a;

public:
	inline NgColour(float rgba = 1.0f) :
		r(rgba),
		g(rgba),
		b(rgba),
		a(rgba)
	{
	}

	inline NgColour(float rgb, float a_) :
		r(rgb),
		g(rgb),
		b(rgb),
		a(a_)
	{
	}

	inline NgColour(float r_, float g_, float b_, float a_ = 1.0f) :
		r(r_),
		g(g_),
		b(b_),
		a(a_)
	{
	}

	inline NgColour(const NgColour& rgb, float a_) :
		r(rgb.r),
		g(rgb.g),
		b(rgb.b),
		a(a_)
	{
	}

	inline NgColour(XtU32 colour) :
		r(((colour & 0x00FF0000) >> 16) / 256.0f),
		g(((colour & 0x0000FF00) >> 8) / 256.0f),
		b((colour & 0x000000FF) / 256.0f),
		a(((colour & 0xFF000000) >> 24) / 256.0f)
	{
	}

public:
	inline NgColour(const NgColour& rhs) :
		r(rhs.r),
		g(rhs.g),
		b(rhs.b),
		a(rhs.a)
	{
	}

	NgColour& operator=(const NgColour& rhs)
	{
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		a = rhs.a;

		return *this;
	}

public:
	inline operator Gdiplus::Color() const
	{
		return Gdiplus::Color(
				Clamp((int)(a * 256.0f), 0, 255),
				Clamp((int)(r * 256.0f), 0, 255),
				Clamp((int)(g * 256.0f), 0, 255),
				Clamp((int)(b * 256.0f), 0, 255)
			);
	}
};

inline NgColour operator+(const NgColour& lhs, const NgColour& rhs)
{
	return NgColour(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a);
}

inline NgColour operator-(const NgColour& lhs, const NgColour& rhs)
{
	return NgColour(lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a);
}

inline NgColour operator*(const NgColour& lhs, const NgColour& rhs)
{
	return NgColour(lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a);
}

inline NgColour operator/(const NgColour& lhs, const NgColour& rhs)
{
	return NgColour(lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b, lhs.a / rhs.a);
}

inline NgColour operator+=(NgColour& lhs, const NgColour& rhs)
{
	lhs.r += rhs.r;
	lhs.g += rhs.g;
	lhs.b += rhs.b;
	lhs.a += rhs.a;

	return lhs;
}

inline NgColour& operator-=(NgColour& lhs, const NgColour& rhs)
{
	lhs.r -= rhs.r;
	lhs.g -= rhs.g;
	lhs.b -= rhs.b;
	lhs.a -= rhs.a;

	return lhs;
}

inline NgColour& operator*=(NgColour& lhs, const NgColour& rhs)
{
	lhs.r *= rhs.r;
	lhs.g *= rhs.g;
	lhs.b *= rhs.b;
	lhs.a *= rhs.a;

	return lhs;
}

inline NgColour& operator/=(NgColour& lhs, const NgColour& rhs)
{
	lhs.r /= rhs.r;
	lhs.g /= rhs.g;
	lhs.b /= rhs.b;
	lhs.a /= rhs.a;

	return lhs;
}

inline NgColour operator-(const NgColour& rhs)
{
	return NgColour(-rhs.r, -rhs.g, -rhs.b, -rhs.a);
}

NgColour HsvToRgb(const NgColour& hsv);
NgColour RgbToHsv(const NgColour& rgb);

NgColour Lighten(const NgColour& colour, float v);
NgColour Lerp(const NgColour& colour0, const NgColour& colour1, float a);

#endif // XTWIN_NODEMATH_H