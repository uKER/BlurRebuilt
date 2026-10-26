#include "Stdafx.h"

#include "ui/Illustrator/UIAdobePoint.h"

namespace UI
{

C_AdobePoint::C_AdobePoint()
:	m_DistFromStart( 0.0f ),
	m_Time( 0.0f ),
	m_Length( 0.0f ),
	m_Corner( false )
{

}

C_AdobePoint::C_AdobePoint( const Maths::HVec2& vPos )
:	m_Pos( vPos ),
	m_DistFromStart( 0.0f ),
	m_Time( 0.0f ),
	m_Length( 0.0f ),
	m_Corner( false )
{
}

C_AdobePoint::C_AdobePoint( const F32 fX, const F32 fY )
:	m_Pos( fX, fY ),
	m_DistFromStart( 0.0f ),
	m_Time( 0.0f ),
	m_Length( 0.0f ),
	m_Corner( false )
{
}


C_AdobePoint::C_AdobePoint( const Maths::HVec2& vPos, const Maths::HVec2& vIn, const Maths::HVec2& vOut, const bool bCorner )
:	m_Pos( vPos ),
	m_In( vIn ),
	m_Out( vOut ),
	m_Corner( bCorner )
{
}


C_AdobePoint::C_AdobePoint( const Maths::HVec2& vPos, Maths::HVec2& vUV )
:	m_Pos( vPos ),
	m_UV( vUV )
{
}



C_AdobePoint::~C_AdobePoint()
{
}


bool C_AdobePoint::operator == ( const C_AdobePoint& o )
{
	return ( ( m_Pos.X() == o.m_Pos.X() ) && ( m_Pos.Y() == o.m_Pos.Y() ) );
}

bool C_AdobePoint::operator != ( const C_AdobePoint& o )
{
	return ( ( m_Pos.X() != o.m_Pos.X() ) || ( m_Pos.Y() != o.m_Pos.Y() ) );
}

}// namespace UI
