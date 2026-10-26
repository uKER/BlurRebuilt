
#include "stdafx.h"

#include "ui/game/Gadgets/UIAdobeBox.h"

#include "ui\Illustrator\UIAdobeShape.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UIAdobeBox, "UIAdobeBox" )
	XT_BASE( UI::BaseUIShape )
	XT_FIELD( m_CornerSize, "border_size" )
XT_END_TYPE


namespace UI
{

void UIAdobeBox::Init()
{
	BaseUIShape::Init();

	m_CornerSize = 10.0f;
}


void UIAdobeBox::Setup( lua_State* pState )
{
	BaseUIShape::Setup( pState );

	FindPieces();
}


void UIAdobeBox::FrameUpdate()
{
	BaseUIShape::FrameUpdate();

#ifndef CFG_GOLD
	if ( m_pSceneHandle && m_pSceneHandle->IsDirty() )
	{
		FindPieces();
	}
#endif // CFG_GOLD
}


void UIAdobeBox::FindPieces()
{
	const char* names[E_Corner_Count] =
	{
		"top_left",
		"top_centre",
		"top_right",
		"middle_left",
		"middle_centre",
		"middle_right",
		"bottom_left",
		"bottom_centre",
		"bottom_right"
	};

	for ( S32 i = 0; i < E_Corner_Count; i++ )
		m_Shapes[i] = FindShape( names[i] );
}

void UIAdobeBox::Render()
{
	if( !m_Activated || !m_pSceneHandle )
		return;

	C_UIAutoTransform transform( this );
	BaseUIShape::Render();

	const F32 width = m_Size.x - ( m_CornerSize * 2.0f );
	const F32 height = m_Size.y - ( m_CornerSize * 2.0f );

	Maths::HVec3 drawPos = m_Position + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size );

	F32 x = drawPos.X();
	F32 y = drawPos.Y();

	F32 sizeX = 0.0f;
	F32 sizeY = 0.0f;

	const TransformIndex colourMatrixIndex = gSimple2D.PopColourTransformMatrix();

	// Render each piece of the frame.
	for ( S32 shape = 0; shape < E_Corner_Count; shape++ )
	{
		const C_AdobeShape* pShape = m_Shapes[ shape ];

		if( !pShape )
			continue;

		switch ( shape )
		{
			case E_Corner_TopLeft :
			{
				x = drawPos.X();
				y = drawPos.Y();

				sizeX = m_CornerSize;
				sizeY = m_CornerSize;
				break;
			}
			case E_Corner_TopCentre :
			{
				x = drawPos.X() + m_CornerSize;
				y = drawPos.Y();

				sizeX = width;
				sizeY = m_CornerSize;
				break;
			}
			case E_Corner_TopRight :
			{
				x = drawPos.X() + m_CornerSize + width;
				y = drawPos.Y();

				sizeX = m_CornerSize;
				sizeY = m_CornerSize;
				break;
			}
			case E_Corner_MiddleLeft :
			{
				x = drawPos.X();
				y = drawPos.Y() + m_CornerSize * Up();

				sizeX = m_CornerSize;
				sizeY = height;
				break;
			}
			case E_Corner_MiddleCentre :
			{
				x = drawPos.X() + m_CornerSize;
				y = drawPos.Y() + m_CornerSize * Up();

				sizeX = width;
				sizeY = height;
				break;
			}
			case E_Corner_MiddleRight :
			{
				x = drawPos.X() + m_Size.x - ( m_CornerSize * pShape->BoundingBox().Max().X() );
				y = drawPos.Y() + m_CornerSize * Up();

				sizeX = m_CornerSize;
				sizeY = height;
				break;
			}
			case E_Corner_BottomLeft :
			{
				x = drawPos.X();
				y = drawPos.Y() + ( m_CornerSize + height ) * Up();

				sizeX = m_CornerSize;
				sizeY = m_CornerSize;
				break;
			}
			case E_Corner_BottomCentre :
			{
				x = drawPos.X() + m_CornerSize;
				y = drawPos.Y() + ( m_Size.y - ( m_CornerSize * pShape->BoundingBox().Max().Y() ) ) * Up();

				sizeX = width;
				sizeY = m_CornerSize;
				break;
			}
			case E_Corner_BottomRight :
			{
				x = drawPos.X() + m_CornerSize + width;
				y = drawPos.Y() + ( m_CornerSize + height ) * Up();

				sizeX = m_CornerSize;
				sizeY = m_CornerSize;
				break;
			}
		}
		RenderPiece( pShape, x, y, drawPos.Z(), sizeX, sizeY * Up(), colourMatrixIndex );
	}

	return;

	// **************************** //
/*
	const TransformIndex flat_matrix_index = gSimple2D.PopColourTransformMatrix();
	
	TransformIndex ti;
	CalcTransformHierarchy( ti );

	const bool isTransformed = ( ti != 0 );
	const F32 adobeScale = 1.0f / ILLUSTRATOR_SCALE;
	const F32 drawSizeX = (m_Size.x * adobeScale);

	// Flip the data if its in 3D
	const F32 up = gSimple2D.IsPanel3D( m_Panel ) ? -1.0f : 1.0f;
	const F32 drawSizeY = m_Size.y * adobeScale * up;

	// Draw position
	Maths::HVec3 position = m_Position;

	Maths::HMat43 col_transform;

	if(isTransformed)
	{
		// For transforming colour matrix
		col_transform.SetIdentity();
		col_transform.Translation( (gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size) );
		col_transform = col_transform * gSimple2D.GetPostTransformMatrix( ti );

		// Clear 'basic' translation
		position = Maths::HVec3( 0.0f, 0.0f, 0.0f );
	}
	// justifications
	position += (gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size);

	if(!isTransformed)
	{
		col_transform.SetIdentity();
		col_transform.Translation( position );
	}

	// Calculate the sizes of each piece.
	const F32 length = m_pSceneHandle->SizeX();
	const F32 height = m_pSceneHandle->SizeY() * up;

	const F32 middleSizeX = ( length * drawSizeX ) - ( m_BorderSize * 2.0f );
	const F32 middleSizeY = ( height * drawSizeY ) - ( m_BorderSize * 2.0f );

	const F32 thicknessTop = m_Shapes[E_Corner_TopLeft] ? ( m_BorderSize / ( m_Shapes[E_Corner_TopLeft]->SizeY() * up ) ) : 0.0f;
	const F32 thicknessBottom = m_Shapes[E_Corner_BottomLeft] ? ( m_BorderSize / ( m_Shapes[E_Corner_BottomLeft]->SizeY() * up ) ) : 0.0f;
	const F32 thicknessLeft = m_Shapes[E_Corner_TopLeft] ? ( m_BorderSize / m_Shapes[E_Corner_TopLeft]->SizeX() ) : 0.0f;
	const F32 thicknessRight = m_Shapes[E_Corner_BottomLeft] ? ( m_BorderSize / m_Shapes[E_Corner_BottomLeft]->SizeX() ) : 0.0f;

	F32 x = position.X();
	F32 y = position.Y();
	F32 z = position.Z();

	F32 sizeX = drawSizeX;
	F32 sizeY = drawSizeY;

	// Render each piece of the frame.
	for ( S32 shape = 0; shape < E_Corner_Count; shape++ )
	{
		C_AdobeShape* pShape = m_Shapes[ shape ];

		if( !pShape )
			continue;

		switch ( shape )
		{
			case E_Corner_TopLeft :
			{
				sizeX = m_BorderSize / pShape->SizeX();
				sizeY = m_BorderSize / ( pShape->SizeY() * up );

				x = position.X() - ( pShape->GetTopLeft().X() * sizeX );
				y = position.Y();
				break;
			}
			case E_Corner_TopCentre :
			{
				sizeX = middleSizeX / pShape->SizeX();
				sizeY = thicknessTop;

				x = position.X() - ( pShape->GetTopLeft().X() * sizeX ) + m_BorderSize;
				y = position.Y();
				break;
			}
			case E_Corner_TopRight :
			{
				sizeX = m_BorderSize / pShape->SizeX();
				sizeY = m_BorderSize / ( pShape->SizeY() * up );

				x = position.X() - ( pShape->GetTopLeft().X() * sizeX ) + m_BorderSize + middleSizeX; 
				y = position.Y();
				break;
			}
			case E_Corner_MiddleLeft :
			{
				sizeX = thicknessLeft;
				sizeY = middleSizeY / ( pShape->SizeY() * up );

				x = position.X() - ( pShape->GetTopLeft().X() * sizeX ); 
				y = position.Y() - ( pShape->GetTopLeft().Y() * sizeY ) + ( m_BorderSize * up );
				break;
			}
			case E_Corner_MiddleCentre :
			{
				sizeX = middleSizeX / pShape->SizeX();
				sizeY = middleSizeY / ( pShape->SizeY() * up );

				x = position.X() - ( pShape->GetTopLeft().X() * sizeX ) + m_BorderSize;
				y = position.Y() - ( pShape->GetTopLeft().Y() * sizeY ) + ( m_BorderSize * up );
				break;
			}
			case E_Corner_MiddleRight :
			{
				sizeX = thicknessRight;
				sizeY = middleSizeY / ( pShape->SizeY() * up );

				x = position.X() - ( ( pShape->GetTopLeft().X() + pShape->SizeX() ) * sizeX ) + ( length * drawSizeX ); 
				y = position.Y() - ( pShape->GetTopLeft().Y() * sizeY ) + ( m_BorderSize * up );
				break;
			}
			case E_Corner_BottomLeft :
			{
				sizeX = m_BorderSize / pShape->SizeX();
				sizeY = m_BorderSize / ( pShape->SizeY() * up );

				x = position.X() - ( pShape->GetTopLeft().X() * sizeX ); 
				y = position.Y() - ( pShape->GetTopLeft().Y() * sizeY ) + ( ( m_BorderSize +  middleSizeY ) * up );

				break;
			}
			case E_Corner_BottomCentre :
			{
				sizeX = middleSizeX / pShape->SizeX();
				sizeY = thicknessBottom;

				x = position.X() - ( pShape->GetTopLeft().X() * sizeX ) + m_BorderSize;
				y = position.Y() - ( ( pShape->GetTopLeft().Y() + pShape->SizeY() ) * sizeY ) + ( ( height * drawSizeY ) * up );
				break;
			}
			case E_Corner_BottomRight :
			{
				sizeX = m_BorderSize / pShape->SizeX();
				sizeY = m_BorderSize / ( pShape->SizeY() * up );

				x = position.X() - ( pShape->GetTopLeft().X() * sizeX ) + m_BorderSize + middleSizeX; 
				y = position.Y() - ( pShape->GetTopLeft().Y() * sizeY ) + ( ( m_BorderSize +  middleSizeY ) * up );
				break;
			}
		}
		// Need to store this offset.
		Maths::HVec3 offset = position - Maths::HVec3( x, y, 0.0f );

		for ( S32 i = 0; i < pShape->m_FillData.Size(); i++ )
			pShape->m_FillData[i]->m_StaticShapeOffset = offset;

		for ( S32 i = 0; i < pShape->m_OutlineData.Size(); i++ )
			pShape->m_OutlineData[i]->m_StaticShapeOffset = offset;

		RenderPiece( pShape, x, y, z, sizeX, sizeY, ti, flat_matrix_index, col_transform );
	}
	*/
}

void UIAdobeBox::RenderPiece( const C_AdobeShape* pShape, const F32 fX, const F32 fY, const F32 fZ, const F32 fSizeX, const F32 fSizeY, const TransformIndex uColourMatrixIndex  )
{
	SetBlendMode( pShape );

	if ( pShape->HasFill() )
	{
		RenderFill( pShape, Maths::HVec3( fX, fY, fZ ), fSizeX, fSizeY, uColourMatrixIndex );
	}
	if ( pShape->HasStroke() )
	{				
		RenderStroke( pShape, Maths::HVec3( fX, fY, fZ ), fSizeX, fSizeY, uColourMatrixIndex );
	}
}

}; // namespace UI
