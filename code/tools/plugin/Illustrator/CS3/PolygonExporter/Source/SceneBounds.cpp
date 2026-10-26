
#include "SceneBounds.h"

namespace Ipf
{

void C_SceneBounds::Clear()
{
	m_BoundingBox = Maths::C_AxisBox();
}


void C_SceneBounds::AddLayerBounds( AIArtHandle pLayerHandle )
{
	// A layer is a group.
	AddGroupBounds( pLayerHandle );
}


void C_SceneBounds::AddGroupBounds( AIArtHandle pGroupHandle )
{
	AIArtHandle pArtHandle;
	short artType;

	// Get the group name and the first child.
	sAIArt->GetArtFirstChild( pGroupHandle, &pArtHandle );

	// While children are still present.
	while ( pArtHandle )
	{
		sAIArt->GetArtType( pArtHandle, &artType );

		// Traverse the group hierarchy
		if ( artType == kGroupArt )
		{
			AddGroupBounds( pArtHandle );
		}
		else
		{
			AddArtBounds( pArtHandle );
		}
		// Get next child.
		sAIArt->GetArtSibling( pArtHandle, &pArtHandle );
	}
}


void C_SceneBounds::AddArtBounds( AIArtHandle pArtHandle )
{
	AIRealRect rect;
	sAIArt->GetArtBounds( pArtHandle, &rect );

	// Take off the outline dimensions as we dont care about it
	AIPathStyle	style;
	sAIPathStyle->GetPathStyle( pArtHandle, &style );

	F32 halfOutline = ( style.strokePaint ) ? style.stroke.width * 0.5f : 0.0f;

	m_BoundingBox.UpdateExtents( Maths::C_AxisBox( Maths::HVec3( rect.left + halfOutline, rect.bottom + halfOutline, 0.0f ), 
												   Maths::HVec3( rect.right - halfOutline, rect.top - halfOutline, 0.0f ) ) );
}

};