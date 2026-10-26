
#include "Stdafx.h"

#include "ui/illustrator/UIAdobeColour.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

// C_AdobeStrokeStyle
C_AdobeStrokeStyle::C_AdobeStrokeStyle( ) : 
	m_Width( 0.0f )
{

}

C_AdobeStrokeStyle::C_AdobeStrokeStyle( const C_AdobeColour& colour, const F32 width )
{
	m_Width = width;
	m_SolidColour = colour;
}

C_AdobeGradient::C_AdobeGradient( const C_AdobeGradient& copy ) :
	m_pStops( NULL )
{
	this->operator=( copy );
}

// C_AdobeGradient
C_AdobeGradient::C_AdobeGradient( ) :
	m_pStops( NULL ),
	m_StopCount( 0 )
{

}

C_AdobeGradient::~C_AdobeGradient( )
{
	SAFE_DELETE_ARY( m_pStops );
	m_StopCount = 0;
}

// C_AdobeFillStyle
C_AdobeFillStyle::C_AdobeFillStyle( ) :
	m_Kind( E_FillStyle_Kind_None )
{

}

C_AdobeFillStyle::C_AdobeFillStyle( const C_AdobeColour& colour ) :
	m_Kind( E_FillStyle_Kind_Solid )
{
	m_SolidColour = colour;
}

C_AdobeFillStyle::C_AdobeFillStyle( const C_AdobeGradientStyle& gradient ) : 
	m_Kind( E_FillStyle_Kind_Gradient )
{
	m_GradientStyle = gradient;
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
