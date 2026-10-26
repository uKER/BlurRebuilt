#ifndef DISPLAY_UTILS_RECT_H
#define DISPLAY_UTILS_RECT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

class C_Rect
{
public:

	C_Rect();
	C_Rect(const F32 x1, const F32 y1, const F32 x2, const F32 y2);
	~C_Rect();

public:

	F32 x1;
	F32 y1;
	F32 x2;
	F32 y2;
};

}; // namespace Display

#endif // DISPLAY_UTILS_RECT_H
