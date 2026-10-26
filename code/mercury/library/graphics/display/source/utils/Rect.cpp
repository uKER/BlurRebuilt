#include "stdafx.h"
#include "Display/Utils/Rect.h"

namespace Display
{

C_Rect::C_Rect() :
	x1(0.0f),
	y1(0.0f),
	x2(0.0f),
	y2(0.0f)
{
}

C_Rect::C_Rect(const F32 x1, const F32 y1, const F32 x2, const F32 y2) :
	x1(x1),
	y1(y1),
	x2(x2),
	y2(y2)
{
}

C_Rect::~C_Rect()
{
}

}; // namespace Display
