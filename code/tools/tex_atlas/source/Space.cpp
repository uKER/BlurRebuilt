#include "stdafx.h"
#include <tex_atlas/Space.h>

using namespace TexAtlas;

Space::Space()
{
	m_topLeft.Zero();
	m_bottomRight.Zero();
}

const Vec2S32& Space::TopLeftCorner() const
{
	return m_topLeft;
}

Vec2S32 Space::TopRightCorner() const
{
	return Vec2S32(m_bottomRight.X(), m_topLeft.Y());
}

Vec2S32 Space::BottomLeftCorner() const
{
	return Vec2S32(m_topLeft.X(), m_bottomRight.Y());
}

const Vec2S32& Space::BottomRightCorner() const
{
	return m_bottomRight;
}

S32 Space::Area() const
{
	Vec2S32 delta(m_bottomRight - m_topLeft);
	return delta.X() * delta.Y();
}

U32 Space::X() const
{
	return m_topLeft.X();
}

U32 Space::Y() const
{
	return m_topLeft.Y();
}

bool Space::Intersects(const Vec2S32& p) const
{
	if (
		(p.X() >= m_topLeft.X()) && (p.X() <= m_bottomRight.X())
		&&
		(p.Y() >= m_topLeft.Y()) && (p.Y() <= m_bottomRight.Y())
		)
	{
		//Top left corner lies within this space.
		return true;
	}
	return false;
}

bool Space::Intersects(const Space& s) const
{
	if (
		(s.TopLeftCorner().X() >= m_topLeft.X()) && (s.TopLeftCorner().X() <= m_bottomRight.X())
		&&
		(s.TopLeftCorner().Y() >= m_topLeft.Y()) && (s.TopLeftCorner().Y() <= m_bottomRight.Y())
		)
	{
		//Top left corner lies within this space.
		return true;
	}

	if (
		(s.TopRightCorner().X() >= m_topLeft.X()) && (s.TopRightCorner().X() <= m_bottomRight.X())
		&&
		(s.TopRightCorner().Y() >= m_topLeft.Y()) && (s.TopRightCorner().Y() <= m_bottomRight.Y())
		)
	{
		//Top right corner lies within this space.
		return true;
	}

	if (
		(s.BottomLeftCorner().X() >= m_topLeft.X()) && (s.BottomLeftCorner().X() <= m_bottomRight.X())
		&&
		(s.BottomLeftCorner().Y() >= m_topLeft.Y()) && (s.BottomLeftCorner().Y() <= m_bottomRight.Y())
		)
	{
		//Bottom left corner lies within this space.
		return true;
	}

	if (
		(s.BottomRightCorner().X() >= m_topLeft.X()) && (s.BottomRightCorner().X() <= m_bottomRight.X())
		&&
		(s.BottomRightCorner().Y() >= m_topLeft.Y()) && (s.BottomRightCorner().Y() <= m_bottomRight.Y())
		)
	{
		//Bottom right corner lies within this space.
		return true;
	}

	return false;
}

bool Space::CanContain(U32 dimensionX, U32 dimensionY) const
{
	return CanContain(Vec2S32(dimensionX, dimensionY));
}

bool Space::CanContain(const Vec2S32& dimensions) const
{
	Vec2S32 ourDimensions = Dimensions();
	if (
		(dimensions.X() <= ourDimensions.X()) 
		&&
		(dimensions.Y() <= ourDimensions.Y())
		)
	{
		return true;
	}
	return false;
}

bool Space::operator < (const Space& rhs) const
{
	//< then if < area and closest to 1.f aspect ratio.
	S32 lhsArea = Area();
	S32 rhsArea = rhs.Area();

	if (lhsArea < rhsArea) 
		return true;

	F32 lhsDistance = Aspect();
	F32 rhsDistance = rhs.Aspect();
	if (lhsDistance < 1)
		lhsDistance = 1/lhsDistance;
	if (rhsDistance < 1)
		rhsDistance = 1/rhsDistance;
	if ((lhsArea <= rhsArea) &&
		(lhsDistance < rhsDistance))
		return true;

	return false;
}

Vec2S32 Space::Dimensions() const
{
	return Vec2S32(m_bottomRight.X() - m_topLeft.X(), m_bottomRight.Y() - m_topLeft.Y());
}

U32 Space::Width() const
{
	return m_bottomRight.X() - m_topLeft.X();
}

U32 Space::Height() const
{
	return m_bottomRight.Y() - m_topLeft.Y();
}

F32 Space::Aspect() const
{
	Vec2S32 d(Dimensions());
	if (d.Y() == 0)
		return 0;
	return (F32)d.X() / (F32)d.Y();
}

void Space::Set(
	U32 topLeftX, 
	U32 topLeftY,
	U32 width, 
	U32 height)
{
	m_topLeft.X(topLeftX);
	m_topLeft.Y(topLeftY);
	m_bottomRight.X(topLeftX + width);
	m_bottomRight.Y(topLeftY + height);
}

