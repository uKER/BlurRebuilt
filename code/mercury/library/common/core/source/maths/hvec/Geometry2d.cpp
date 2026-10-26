#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{


///// Update extents with vertex
//void C_Rect::UpdateExtents( const Maths::HVec2& v )
//{
//}
//
///// Update extents with another rectangle
//void C_Rect::UpdateExtents( const C_Rect& box )
//{
//}
//    
///// Determine if rect inside another rect
//NOALIAS const U32 C_Rect::Inside( const C_Rect& other ) const __restrict
//{
//}
//
///// Determine if point inside rect
//const U32 C_Rect::Inside( const Maths::HVec2& v ) const
//{
//}
//
///// Calculate the area of the rect
//const HF32 C_Rect::Area() const
//{
//}

//
// ---------------------------------------------
// |-- C_Transform2d : Axis Aligned Rectangle class --|
// ---------------------------------------------
//

C_Transform2d C_Transform2d::operator*(const C_Transform2d& input)
{
	C_Transform2d result;

	result.m_Offset = (input.m_Offset * m_Scale) + m_Offset;
	result.m_Scale = input.m_Scale * m_Scale;

	return(result);
}

void C_Transform2d::Invert()
{
	m_Scale.X( 1.0f / m_Scale.X() );
	m_Scale.Y( 1.0f / m_Scale.Y() );

	m_Offset.X( (-m_Offset.X() * m_Scale.X()));
	m_Offset.Y( (-m_Offset.Y() * m_Scale.Y()));
}

const HVec2 C_Transform2d::Apply( const HVec2 PCREF src ) const
{
	HVec2 result;

	result = src * m_Scale + m_Offset;

	return result;
}

const HVec2 C_Transform2d::ApplyInverse( const HVec2 PCREF src ) const
{
	HVec2 result;

	result = (src - m_Offset) / m_Scale;

	return result;
}


}; // namespace Maths



