

#include "Stdafx.h"

#include "ui/Illustrator/UIAdobeShape.h"
#include "ui/Graphics/Simple2d.h"

namespace UI
{

C_AdobeShape::C_AdobeShape() 
:	C_AdobeElement( Ipf::E_ElementType_Shape )
{

}

C_AdobeShape::~C_AdobeShape()
{
	for ( S32 i = 0; i < m_Fill.Size(); i++ )
	{
		SAFE_DELETE_PTR( m_Fill[i] );
	}
	m_Fill.Clear();

	for ( S32 i = 0; i < m_Stroke.Size(); i++ )
	{
		SAFE_DELETE_PTR( m_Stroke[i] );
	}
	m_Stroke.Clear();
}

void C_AdobeShape::Deserialise( Ipf::IpfDeserialiser& stream )
{
	stream	>> m_Level 
			>> m_Name
			>> m_FillStyle
			>> m_StrokeStyle
			>> m_BlendStyle
			>> m_BoundingBox
			>> m_ScalarOffset
			>> m_ScalarValue;
}


} // namespace UI
