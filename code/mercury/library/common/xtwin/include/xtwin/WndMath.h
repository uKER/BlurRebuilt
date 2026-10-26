#ifndef WNDMATH_H
#define WNDMATH_H

#ifdef _MSC_VER
#pragma once
#endif

class WinPoint : public tagPOINT
{
public:
	inline WinPoint(int x_ = 0, int y_ = 0)
	{
		x = x_;
		y = y_;
	}

	inline WinPoint(const POINT& pt)
	{
		x = pt.x;
		y = pt.y;
	}

public:
	inline WinPoint ScreenToClient(HWND hwnd) const
	{
		WinPoint newPt = *this;

		::ScreenToClient(hwnd, &newPt);

		return newPt;
	}

	inline WinPoint ClientToScreen(HWND hwnd) const
	{
		WinPoint newPt = *this;

		::ClientToScreen(hwnd, &newPt);

		return newPt;
	}

	static inline WinPoint FromLParam(LPARAM lParam)
	{
		return WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}

	static inline WinPoint FromLastMessage()
	{
		return FromLParam(GetMessagePos());
	}
};

inline bool operator==(const POINT& lhs, const POINT& rhs)
{
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

inline bool operator!=(const POINT& lhs, const POINT& rhs)
{
	return (lhs.x != rhs.x) || (lhs.y != rhs.y);
}

inline WinPoint operator+(const WinPoint& lhs, const WinPoint& rhs)
{
	return WinPoint(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline WinPoint operator-(const WinPoint& lhs, const WinPoint& rhs)
{
	return WinPoint(lhs.x - rhs.x, lhs.y - rhs.y);
}

class WinSize : public tagSIZE
{
public:
	inline WinSize(int cx_ = 0, int cy_ = 0)
	{
		cx = cx_;
		cy = cy_;
	}

	inline WinSize(const SIZE& sz)
	{
		cx = sz.cx;
		cy = sz.cy;
	}
};

class WinRect : public tagRECT
{
public:
	inline WinRect(int left_ = 0, int top_ = 0, int right_ = 0, int bottom_ = 0)
	{
		left	= left_;
		top		= top_;
		right	= right_;
		bottom	= bottom_;
	}

	inline WinRect(const WinPoint& pt, const WinSize& sz)
	{
		left	= pt.x;
		top		= pt.y;
		right	= pt.x + sz.cx;
		bottom	= pt.y + sz.cy;
	}

	inline WinRect(const RECT& rc)
	{
		left	= rc.left;
		top		= rc.top;
		right	= rc.right;
		bottom	= rc.bottom;
	}

	inline WinRect(HWND hwndClient)
	{
		GetClientRect(hwndClient, this);
	}

public:
	inline int Width() const
	{
		return right - left;
	}

	inline int Height() const
	{
		return bottom - top;
	}

	inline WinPoint Position() const
	{
		return WinPoint(left, top);
	}

	inline WinSize Size() const
	{
		return WinSize(Width(), Height());
	}

public:
	inline void Expand(int x, int y)
	{
		left	-= x;
		top		-= y;
		right	+= x;
		bottom	+= y;
	}

	inline void Expand(int v)
	{
		Expand(v, v);
	}

	inline void Shrink(int x, int y)
	{
		Expand(-x, -y);
	}

	inline void Shrink(const int v)
	{
		Expand(-v, -v);
	}
};

class WinTextMeasurer
{
public:
	WinTextMeasurer();
	~WinTextMeasurer();

public:
	void SetFont(HFONT font);
	WinSize Measure(const char* text, const WinSize& minSize);
	WinSize Measure(const wchar_t* text, const WinSize& minSize);

private:	
	HDC _ic;
};

// Xw structures

struct XwVector
{
	int x, y;

public:
	XwVector()							: x(0), y(0)				{ }
	XwVector(int xy)					: x(xy), y(xy)				{ }
	XwVector(int x_, int y_)			: x(x_), y(y_)				{ }
	XwVector(const WinPoint& pt)		: x(pt.x), y(pt.y)			{ }
	XwVector(const WinSize& sz)			: x(sz.cx), y(sz.cy)		{ }
	XwVector(const Gdiplus::Point& pt)	: x(pt.X), y(pt.Y)			{ }
	XwVector(const Gdiplus::Size& sz)	: x(sz.Width), y(sz.Height)	{ }
	XwVector(const XwVector& v)			: x(v.x), y(v.y)			{ }

public:
	XwVector& operator=(const XwVector& v) { x = v.x; y = v.y; return *this; }

public:
	operator WinPoint() const		{ return WinPoint(x, y);		}
	operator Gdiplus::Point() const	{ return Gdiplus::Point(x, y);	}
	operator Gdiplus::Size() const	{ return Gdiplus::Size(x, y);	}
};

// Xw operators

inline XwVector operator+(const XwVector& lhs, const XwVector& rhs)
{
	return XwVector(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline XwVector operator-(const XwVector& lhs, const XwVector& rhs)
{
	return XwVector(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline XwVector operator*(const XwVector& lhs, const XwVector& rhs)
{
	return XwVector(lhs.x * rhs.x, lhs.y * rhs.y);
}

inline XwVector operator/(const XwVector& lhs, const XwVector& rhs)
{
	return XwVector(lhs.x / rhs.x, lhs.y / rhs.y);
}

inline XwVector operator*(const XwVector& lhs, double rhs)
{
	return XwVector((int)(lhs.x * rhs), (int)(lhs.y * rhs));
}

inline XwVector operator/(const XwVector& lhs, double rhs)
{
	return XwVector((int)(lhs.x / rhs), (int)(lhs.y / rhs));
}

inline XwVector operator+=(XwVector& lhs, const XwVector& rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;

	return lhs;
}

inline XwVector& operator-=(XwVector& lhs, const XwVector& rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;

	return lhs;
}

inline XwVector& operator*=(XwVector& lhs, const XwVector& rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;

	return lhs;
}

inline XwVector& operator/=(XwVector& lhs, const XwVector& rhs)
{
	lhs.x /= rhs.x;
	lhs.y /= rhs.y;

	return lhs;
}

inline XwVector& operator*=(XwVector& lhs, double rhs)
{
	lhs.x = (int)(lhs.x * rhs);
	lhs.y = (int)(lhs.y * rhs);

	return lhs;
}

inline XwVector& operator/=(XwVector& lhs, double rhs)
{
	lhs.x = (int)(lhs.x / rhs);
	lhs.y = (int)(lhs.y / rhs);

	return lhs;
}

inline XwVector operator-(const XwVector& rhs)
{
	return XwVector(-rhs.x, -rhs.y);
}

inline int Sum(const XwVector& rhs)
{
	return rhs.x + rhs.y;
}

inline int Dot(const XwVector& lhs, const XwVector& rhs)
{
	return Sum(lhs * rhs);
}

inline int LengthSq(const XwVector& rhs)
{
	return Sum(rhs * rhs);
}

inline double Length(const XwVector& rhs)
{
	return sqrt((double)LengthSq(rhs));
}

inline double Rotation(const XwVector& rhs)
{
	return atan2((double)rhs.y, (double)rhs.x);
}

inline XwVector Rotation(double angle, double radius)
{
	return XwVector((int)(cos(angle) * radius), (int)(sin(angle) * radius));
}

inline XwVector Unit(const XwVector& v)
{
	return v / Length(v);
}

inline double Normalise(XwVector& v)
{
	double length = Length(v);

	v /= length;

	return length;
}

inline XwVector Perpendicular(const XwVector& v)
{
	return XwVector(v.y, -v.x);
}

inline int AreaOfPara(const XwVector& lhs, const XwVector& rhs)
{
	return lhs.x * rhs.y - rhs.x * lhs.y;
}

//
// XwRect
//

struct XwRect
{
	XwVector pt;
	XwVector sz;

public:
	XwRect()											: pt(), sz()												{ } 
	XwRect(const XwVector& pt_, const XwVector& sz_)	: pt(pt_), sz(sz_)											{ }
	XwRect(int x, int y, int width, int height)			: pt(x, y), sz(width, height)								{ }
	XwRect(const WinRect& rect)							: pt(rect.left, rect.top), sz(rect.Width(), rect.Height())	{ }

public:
	XwVector TopLeft() const		{ return pt;							}
	XwVector TopRight() const		{ return XwVector(pt.x + sz.x, pt.y);	}
	XwVector BottomLeft() const		{ return XwVector(pt.x, pt.y + sz.y);	}
	XwVector BottomRight() const	{ return pt + sz;						}

public:
	operator Gdiplus::Rect() const	{ return Gdiplus::Rect(pt.x, pt.y, sz.x, sz.y);									}
	operator Gdiplus::RectF() const	{ return Gdiplus::RectF((float)pt.x, (float)pt.y, (float)sz.x, (float)sz.y);	}
};

inline XwRect Translate(const XwRect& rect, const XwVector& offset)
{
	return XwRect(rect.pt + offset, rect.sz);
}

inline bool PtInRect(const XwRect& rect, const XwVector& pt)
{
	return (pt.x >= rect.pt.x) && (pt.y >= rect.pt.y) && (pt.x < (rect.pt.x + rect.sz.x)) && (pt.y < (rect.pt.y + rect.sz.y));
}

inline bool Intersects(const XwRect& r0, const XwRect& r1)
{
	XwVector s0 = r0.TopLeft();
	XwVector s1 = r1.TopLeft();

	XwVector e0 = r0.BottomRight();
	XwVector e1 = r1.BottomRight();

	if (e1.x < s0.x)
		return false;

	if (e0.x < s1.x)
		return false;

	if (e1.y < s0.y)
		return false;

	if (e0.y < s1.y)
		return false;

	return true;
}

inline XwRect Inflate(const XwRect& rect, const XwVector& v)
{
	return XwRect(rect.pt - v, rect.sz + (v * 2));
}

inline XwRect Inflate(const XwRect& rect, int x, int y)
{
	return Inflate(rect, XwVector(x, y));
}

inline XwRect Normalise(const XwRect& rect)
{
	XwRect newRect(rect);

	if (newRect.sz.x < 0)
	{
		newRect.pt.x += newRect.sz.x;
		newRect.sz.x = -newRect.sz.x;
	}

	if (newRect.sz.y < 0)
	{
		newRect.pt.y += newRect.sz.y;
		newRect.sz.y = -newRect.sz.y;
	}

	return newRect;
}

#endif // WNDMATH_H