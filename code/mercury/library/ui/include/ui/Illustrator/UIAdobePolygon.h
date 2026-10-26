
#ifndef UI_UIADOBEPOLYGON_H
#define UI_UIADOBEPOLYGON_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

// User Headers
#include "ui/Illustrator/UIAdobeElement.h"
#include "ui/Illustrator/UIAdobeColour.h"
#include "ui/Illustrator/UIAdobePoint.h"
#include "ui/Graphics/Polygons/UIPolygonClasses.h"

namespace UI
{

/// @addtogroup Illustrator
/// @{

class C_AdobeVertex
{
public:
	C_AdobeVertex( const Maths::HVec2& vPos = Maths::HVec2(), const Maths::HVec4& vNormal = Maths::HVec4(), const Maths::HVec2& vUV = Maths::HVec2() )
	:	m_Pos( vPos ),
		m_Normal( vNormal ),
		m_UV( vUV )
	{};

public:
	Maths::HVec2		m_Pos;
	Maths::HVec4		m_Normal;
	Maths::HVec2		m_UV;
};

typedef Btl::Vector< C_AdobeVertex >	TVerticies;
typedef Btl::Vector< U16 >				TIndicies;

typedef Btl::Vector< TVerticies >		TVertexList;
typedef Btl::Vector< TIndicies >		TIndexList;

//-------------------------------------------------------------------
//	Class : C_AdobePolygon
//-------------------------------------------------------------------
///	The actual raw data that is contained in the ipf files.
/// @remarks - Compound shapes and paths will be built from the data.
class C_AdobePolygon : public C_AdobeElement
{
public:
	/// @a
	C_AdobePolygon();

	C_AdobeElement*			Clone() { return new C_AdobePolygon( *this ); }

public:
	C_AdobeFillStyle		m_FillStyle;			///< @a
	C_AdobeStrokeStyle		m_StrokeStyle;			///< @a
	C_AdobeBlendStyle		m_BlendStyle;			///< @a

	Maths::C_AxisBox		m_BoundingBox;			/// < Axis aligned bounding box.
	Maths::HVec2			m_ScalarOffset;
	Maths::HVec2			m_ScalarValue;

	TVertexList				m_FillVertexList;
	TIndexList				m_FillIndexList;

	TVertexList				m_StrokeVertexList;
};

///@}

} // namespace UI

#endif // UI_UIADOBEPOLYGON_H
