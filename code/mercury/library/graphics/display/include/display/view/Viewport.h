#ifndef DISPLAY_VIEW_VIEWPORT_H
#define DISPLAY_VIEW_VIEWPORT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

//! Viewport.

class C_Viewport
{
public:

	C_Viewport();
	~C_Viewport();
	
	const U32 GetX() const;
	const U32 GetY() const;
	const U32 GetW() const;
	const U32 GetH() const;
	const F32 GetMinZ() const;
	const F32 GetMaxZ() const;

public:

	U32 m_X;
    U32 m_Y;
    U32 m_W;
    U32 m_H;
	F32 m_MinZ;
	F32 m_MaxZ;
};

}; // namespace Display

#include "Display/View/Viewport.inl"

#endif // DISPLAY_VIEW_VIEWPORT_H
