#ifndef GEOMETRY_2D_INL
#define GEOMETRY_2D_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

//
// ---------------------------------------------
// |-- C_Rect : Axis Aligned Rectangle class --|
// ---------------------------------------------
//

/// Default Constructor: Sets invalid extents
FORCE_INLINE
C_Rect::C_Rect() :
    m_Min( HF32( +FLT_MAX ) ),
    m_Max( HF32( -FLT_MAX ) )
{
}

/// Copy Constructor
FORCE_INLINE
C_Rect::C_Rect( const C_Rect& copy ) :
    m_Min( copy.m_Min ),
    m_Max( copy.m_Max )
{
}

/// HVec2 Min & Max Constructor
FORCE_INLINE
C_Rect::C_Rect( const HVec2 PCREF min, const HVec2 PCREF max ) :
	m_Min( min ),
	m_Max( max )
{
}

/// SetDimensions sets up C_Rect from min x,y position and with/height
FORCE_INLINE
void C_Rect::SetDimensions(const F32 x, const F32 y, const F32 width, const F32 height)
{
	m_Min = HVec2(x,y);
	m_Max = m_Min + HVec2(width, height);
}

/// SetBounds sets up C_Rect from min and max extents
FORCE_INLINE
void C_Rect::SetBounds(const F32 xmin, const F32 ymin, const F32 xmax, const F32 ymax)
{
	m_Min = HVec2(xmin,ymin);
	m_Max = HVec2(xmax,ymax);
}

/// SetDimensions sets up C_Rect from min x,y position and with/height
FORCE_INLINE
void C_Rect::SetDimensions(const Maths::HVec2 PCREF min, const Maths::HVec2 PCREF size)
{
	m_Min = min;
	m_Max = m_Min + size;
}

/// SetBounds sets up C_Rect from min and max extents
FORCE_INLINE
void C_Rect::SetBounds(const Maths::HVec2 PCREF min, const Maths::HVec2 PCREF max)
{
	m_Min = min;
	m_Max = max;
}

/// Accessor: Get Min
FORCE_INLINE
const HVec2 C_Rect::Min() const
{
	return m_Min;
}

/// Accessor: Get Max
FORCE_INLINE
const HVec2 C_Rect::Max() const
{
	return m_Max;
}

/// Accessor: Set Min
FORCE_INLINE
void C_Rect::Min( const HVec2 PCREF min )
{
	m_Min = min;
}

/// Accessor: Set Max
FORCE_INLINE
void C_Rect::Max( const HVec2 PCREF max )
{
	m_Max = max;
}

/// Get Centre of box
FORCE_INLINE
const HVec2 C_Rect::Centre() const
{
	return (m_Min + m_Max) * 0.5f;
}

/// Get Diagonal of box ( min -> max )
FORCE_INLINE
const HVec2 C_Rect::Size() const
{
	return (m_Max - m_Min);
}

/// Set Zero extents
FORCE_INLINE
void C_Rect::SetZeroExtents()
{
	m_Min = HVec2(0.0f, 0.0f);
	m_Max = HVec2(0.0f, 0.0f);
}

/// Set invalid extents
FORCE_INLINE
void C_Rect::SetInvalidExtents()
{
	m_Min = HVec2( HF32( +FLT_MAX ) );
	m_Max = HVec2( HF32( -FLT_MAX ) );
}

template< typename Archive > 
void C_Rect::Deserialise( Archive& ar )
{
	ar >> m_Min;
	ar >> m_Max;
}

template< typename Archive > 
void C_Rect::Serialise( Archive& ar )
{
	ar << m_Min;
	ar << m_Max;
}

FORCE_INLINE
const Maths::Scalar_Bool Intersect(const C_Rect& r1, const C_Rect& r2)
{
	return !(r2.Min().X() > r1.Max().X() | r2.Max().X() < r1.Min().X() | r2.Max().Y() < r1.Min().Y() | r2.Min().Y() > r1.Max().Y());
}

//
// ---------------------------------------------
// |-- C_Transform2d : Axis Aligned Rectangle class --|
// ---------------------------------------------
//

FORCE_INLINE
C_Transform2d::C_Transform2d()
{
	Identity();
}

FORCE_INLINE
C_Transform2d::C_Transform2d(const Maths::C_Rect& frame) :
	m_Scale( frame.Size() ),
	m_Offset( frame.Min() )
{
}

FORCE_INLINE
void C_Transform2d::Identity()
{
    m_Scale = HVec2(1.0f, 1.0f);
	m_Offset = HVec2(0.0f, 0.0f);
}

FORCE_INLINE
const Maths::HVec2& C_Transform2d::Scale() const
{
	return m_Scale;
}

FORCE_INLINE
void C_Transform2d::Scale( const HVec2 PCREF scale )
{
	m_Scale = scale;
}

FORCE_INLINE
const Maths::HVec2& C_Transform2d::Offset() const
{
	return m_Offset;
}

FORCE_INLINE
void C_Transform2d::Offset( const HVec2 PCREF offset)
{
	m_Offset = offset;
}


}; // namespace Maths


#endif
