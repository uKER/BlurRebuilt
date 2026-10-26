
#include "stdafx.h"
#include "ShapeBuilder.h"
#include "Utility.h"

#include "ui/Illustrator/UIAdobeScene.h"
#include "ui/Illustrator/UIAdobePath.h"

#include "ui/Graphics/Polygons/UITriangulator.h"
#include "ui/Graphics/Polygons/UIHoleResolver.h"
#include "ui/Graphics/Polygons/UIPolygonClasses.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

namespace Ipf
{

C_ShapeBuilder C_ShapeBuilder::m_sInstance;

const F32 kTrimOutlineThreshold = 30.0f;
const F32 kStrightLineTolerence = 0.98f;


void ConvertTo( const UI::TVertexList& vIn, UI::UIPolygonList& vOut )
{
	vOut.Clear();
	for ( S32 i = 0; i < vIn.Size(); i++ )
	{
		vOut.PushBack( UI::C_UIResultPolygon() );

		for ( S32 j = 0; j < vIn[i].Size(); j++ )
			vOut[i].AddPoint( vIn[i][j].m_Pos, vIn[i][j].m_Normal );
	}
}


void ConvertTo( UI::UIPolygonList& vIn, UI::TVertexList& vOut )
{
	vOut.Clear();
	for ( S32 i = 0; i < vIn.Size(); i++ )
	{
		vOut.PushBack( UI::TVerticies() );

		for ( U32 j = 0; j < vIn[i].NumPoints(); j++ )
			vOut[i].PushBack( UI::C_AdobeVertex( vIn[i][j].GetPoint(), vIn[i][j].GetVector4() ) );
	}
}


void ConvertTo( UI::UIPolygonIndexList& vIn, UI::TIndexList& vOut )
{
	vOut.Clear();
	for ( S32 i = 0; i < vIn.Size(); i++ )
	{
		vOut.PushBack( UI::TIndicies() );

		for ( S32 j = 0; j < vIn[i].NumIndices(); j++ )
			vOut[i].PushBack( vIn[i][j] );
	}
}

const bool C_ShapeBuilder::BuildPath( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon, const char* pName, const Maths::C_AxisBox& bBounds )
{
	Util::C_String name = pName;
	S32 sCurveStepOverride = GetCurveStepOverride( name );
	F32 fOldCurveStep = m_CurveStep;

	if ( sCurveStepOverride != -1 )
		m_CurveStep = (F32)sCurveStepOverride;

	pPolygon->SetName( pName );

	SetBlendStyle( pPath, pPolygon );

	// Get the path segments making them local positioning.
	UI::AdobePointList segments;
	GetSegments( pPath, segments, bBounds, true );

	AIBoolean closed = false;
	sAIPath->GetPathClosed( pPath, &closed );

	// Build the fill data
	if ( SetFillStyle( pPath, pPolygon ) )
	{
		UI::TVerticies verticies;
		EvaluateFillSegments( segments, ( closed ) ? true : false, verticies );

		UI::UIPolygonList results;
		UI::UIPolygonIndexList indexList;

		// copy from ipf format - meh
		results.PushBack( UI::C_UIResultPolygon() );
		UI::C_UIResultPolygon& p = results.Back();

		for ( S32 i = 0; i < verticies.Size(); i++ )
			p.AddPoint( verticies[ i ].m_Pos );

		Triangulate( results, indexList );

		ConvertTo( results, pPolygon->m_FillVertexList );
		ConvertTo( indexList, pPolygon->m_FillIndexList );

		for ( S32 i = 0; i < pPolygon->m_FillVertexList.Size(); i++ )
		{
			NormaliseVerticies( pPolygon->m_FillVertexList[i], bBounds );
			CalculateBoundingBox( pPolygon->m_FillVertexList[i], pPolygon->m_BoundingBox, false );
		}
	}

	// Build the stroke data
	if ( SetStrokeStyle( pPath, pPolygon ) )
	{
		UI::TVerticies verticies;

		if ( m_FixedSpacedStrokes )
		{
			UI::TVerticies fillVerticies;
			EvaluateFixedSpaceStrokeSegments( segments, m_CurveStep, fillVerticies );

			EvaluateStrokeSegments( fillVerticies, ( closed ) ? true : false, pPolygon->m_StrokeStyle.Width() * 0.5f, verticies );
		}
		else
		{
			UI::TVerticies fillVerticies;
			EvaluateFillSegments( segments, ( closed ) ? true : false, fillVerticies );

			EvaluateStrokeSegments( fillVerticies, ( closed ) ? true : false, pPolygon->m_StrokeStyle.Width() * 0.5f, verticies );
		}

		// Normalise and update the bounding box based on the stroke data.
		NormaliseVerticies( verticies, bBounds );
		CalculateBoundingBox( verticies, pPolygon->m_BoundingBox, false );

		pPolygon->m_StrokeVertexList.PushBack( verticies );
	}

	CalculateScalarAmounts( pPath, pPolygon );

	m_CurveStep = fOldCurveStep;

	return true;
}

const bool C_ShapeBuilder::BuildCompoundPath( const AIArtHandle pGroup, UI::C_AdobePolygon* pPolygon, const char* pName, const Maths::C_AxisBox& bBounds )
{
	Util::C_String name = pName;
	S32 sCurveStepOverride = GetCurveStepOverride( name );
	F32 fOldCurveStep = m_CurveStep;

	if ( sCurveStepOverride != -1 )
		m_CurveStep = (F32)sCurveStepOverride;

	pPolygon->SetName( name );

	SetBlendStyle( pGroup, pPolygon );

	AIArtHandle pathChild;
	sAIArt->GetArtFirstChild( pGroup, &pathChild );

	UI::TVertexList vertexList;

	// Find all the items in the group
	while( pathChild )
	{
		short artType;	
		sAIArt->GetArtType( pathChild, &artType );

		if ( artType == kPathArt )
		{
			AIBoolean closed = false;
			sAIPath->GetPathClosed( pathChild, &closed );

			// Get the path segments making them local positioning.
			UI::AdobePointList segments;
			GetSegments( pathChild, segments, bBounds );

			UI::TVerticies verticies;
			EvaluateFillSegments( segments, ( closed ) ? true : false, verticies );
			vertexList.PushBack( verticies );

			/*
			if ( SetStrokeStyle( pathChild, pPolygon ) )
			{
				UI::TVerticies verticies;
				EvaluateStrokeSegments( segments, ( closed ) ? true : false, pPolygon->m_StrokeStyle.Width(), verticies );

				pPolygon->m_StrokeVertexList.PushBack( verticies );
			}
			*/
		}
		else
		{
			return false;
		}
		sAIArt->GetArtSibling( pathChild, &pathChild );
	}

	// Convert all vertex lists into result polygons so that they can go through the hole resolver
	UI::UIPolygonList compoundList;
	ConvertTo( vertexList, compoundList );

	// Resolve the holes
	UI::UIPolygonList results;
	UI::FindAndResolveHoles( compoundList, results );

	// Build the fill data
	if ( SetFillStyle( pGroup, pPolygon ) )
	{
		UI::UIPolygonIndexList indexList;
		UI::Triangulate( results, indexList, true );

		// do the finning
		DoSoftwareAA_Shape( compoundList, results, indexList );

		// Copy it back to ipf format... sigh
		ConvertTo( results, pPolygon->m_FillVertexList );
		ConvertTo( indexList, pPolygon->m_FillIndexList );

		for ( S32 i = 0; i < pPolygon->m_FillVertexList.Size(); i++ )
		{
			NormaliseVerticies( pPolygon->m_FillVertexList[i], bBounds );
			CalculateBoundingBox( pPolygon->m_FillVertexList[i], pPolygon->m_BoundingBox, false );
		}
	}

	m_CurveStep = fOldCurveStep;

	return true;
}


void C_ShapeBuilder::CalculateBoundingBox( UI::TVerticies& vVerticies, Maths::C_AxisBox& sBounds, const bool bClear )
{
	if ( bClear )
		sBounds = Maths::C_AxisBox();

	for ( S32 i = 0; i < vVerticies.Size(); i++ )
	{
		sBounds.UpdateExtents( Maths::C_Vertex( vVerticies[i].m_Pos.X(), vVerticies[i].m_Pos.Y(), 0.0f ) );
	}
}


void C_ShapeBuilder::CalculateScalarAmounts( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon )
{
	// Store scalar values
	AIRealRect rect;
	sAIArt->GetArtBounds( pPath, &rect );

	const F32 pathWidth = rect.right - rect.left;
	const F32 pathHeight = rect.top - rect.bottom;

	const F32 width = m_SceneBounds.m_Max.X() - m_SceneBounds.m_Min.X();
	const F32 height = m_SceneBounds.m_Max.Y() - m_SceneBounds.m_Min.Y();

	F32 x = ( rect.left - m_SceneBounds.m_Min.X() ) / width;
	F32 y = ( rect.top - m_SceneBounds.m_Min.Y() ) / height;

	pPolygon->m_ScalarOffset = Maths::HVec2( x, 1.0f - y );
	pPolygon->m_ScalarValue = Maths::HVec2( pathWidth / width, pathHeight / height );
}


void C_ShapeBuilder::NormaliseVerticies( UI::TVerticies& vVerticies, const Maths::C_AxisBox& sBounds )
{
	const F32 width = sBounds.m_Max.X() - sBounds.m_Min.X();
	const F32 height = sBounds.m_Max.Y() - sBounds.m_Min.Y();

	const F32 mul = ( width > height ) ? width : height;

	for ( S32 i = 0; i < vVerticies.Size(); i++ )
	{
		F32 x = ( vVerticies[i].m_Pos.X() + width ) - sBounds.m_Max.X();
		F32 y = ( vVerticies[i].m_Pos.Y() + height ) - sBounds.m_Max.Y();

		// Convert to 0-1 then multiply magic scale
		vVerticies[i].m_Pos.X( ( x / mul ) * ILLUSTRATOR_SCALE );
		vVerticies[i].m_Pos.Y( ( y / mul ) * ILLUSTRATOR_SCALE );
	}
}


void C_ShapeBuilder::GetSegments( AIArtHandle pPath, UI::AdobePointList& vSegments, const Maths::C_AxisBox& bBounds, const bool bOffsetOutline )
{
	AIPathSegment	segments[MAX_SEGMENTS];
	AIPathStyle		style;
	short			count = 0;
	AIBoolean		closed = false;

	sAIPath->GetPathClosed( pPath, &closed);
	sAIPath->GetPathSegmentCount( pPath, &count);
	sAIPath->GetPathSegments( pPath, 0, count, segments);

	sAIPathStyle->GetPathStyle( pPath, &style );

	F32 halfOutline = 0.0f; // ( bOffsetOutline ) ? ( ( style.strokePaint ) ? style.stroke.width * 0.5f : 0.0f ) : 0.0f;
	F32 height = ( bBounds.Max().Y() - bBounds.Min().Y() ) - halfOutline;

	// Add each of the points to the polygon.
	AIPathSegment *p = segments;
	for ( short i = 0; i < count; i++, p++ )
	{
		// Offset the verticies so that they are in local space and flip y so it matches the UI
		F32 pX = p->p.h;
		F32 pY = p->p.v;
		Maths::HVec2 pos = Maths::HVec2( pX - halfOutline, (height - (pY - bBounds.Min().Y())) + bBounds.Min().Y() );

		// Offset the verticies so that they are in local space and flip y so it matches the UI
		F32 inX = p->in.h;
		F32 inY = p->in.v;
		Maths::HVec2 in = Maths::HVec2( inX - halfOutline, (height - (inY - bBounds.Min().Y())) + bBounds.Min().Y()  );

		// Offset the verticies so that they are in local space and flip y so it matches the UI
		F32 outX = p->out.h;
		F32 outY = p->out.v;
		Maths::HVec2 out = Maths::HVec2( outX - halfOutline, (height - (outY - bBounds.Min().Y())) + bBounds.Min().Y()  );

		// Add the point to the shape
		vSegments.PushBack( UI::C_AdobePoint( pos, in, out, ( p->corner > 0 ) ? true : false ) );
	}
}


void C_ShapeBuilder::SetBlendStyle( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon )
{
	// Set blend
	const AIReal opacity = sAIBlendStyle->GetOpacity( pPath );
	const AIBlendingMode bm = sAIBlendStyle->GetBlendingMode( pPath );

	if( opacity != 1.0f || bm != kAINormalBlendingMode)
	{
		UI::C_AdobeBlendStyle::E_BlendMode blendMode;

		switch( bm )
		{
		// Supported modes
		case kAINormalBlendingMode :
			blendMode = UI::C_AdobeBlendStyle::E_BlendMode_Normal;
			break;
		case kAIMultiplyBlendingMode :
			blendMode = UI::C_AdobeBlendStyle::E_BlendMode_Multiply;
			break;
		// Not currently supported
		case kAIScreenBlendingMode :
		case kAIOverlayBlendingMode :
		case kAISoftLightBlendingMode :
		case kAIHardLightBlendingMode :
		case kAIColorDodgeBlendingMode :
		case kAIColorBurnBlendingMode :
		case kAIDarkenBlendingMode :
		case kAILightenBlendingMode :
		case kAIDifferenceBlendingMode :
		case kAIExclusionBlendingMode :
		case kAIHueBlendingMode :
		case kAISaturationBlendingMode :
		case kAIColorBlendingMode :
		case kAILuminosityBlendingMode :
		default:
			// Just export as 'normal'
			blendMode = UI::C_AdobeBlendStyle::E_BlendMode_Normal;
			break;
		};
		pPolygon->m_BlendStyle = UI::C_AdobeBlendStyle( blendMode, opacity );
	}
}


const bool C_ShapeBuilder::SetFillStyle( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon )
{
	AIPathStyle	style;
	sAIPathStyle->GetPathStyle( pPath, &style );

	// Shape has fill style
	if( !style.fillPaint )
		return false;

	UI::C_AdobeColour colour;
	UI::C_AdobeFillStyle::E_FillStyle_Kind kind = UI::C_AdobeFillStyle::E_FillStyle_Kind_None;

	switch( style.fill.color.kind )
	{
	case kCustomColor:
	case kThreeColor:
	case kFourColor:
	case kGrayColor:
		kind = UI::C_AdobeFillStyle::E_FillStyle_Kind_Solid;
		break;
	case kGradient:
		kind = UI::C_AdobeFillStyle::E_FillStyle_Kind_Gradient;
		break;
	}

	if( kind == UI::C_AdobeFillStyle::E_FillStyle_Kind_Solid )
	{
		pPolygon->m_FillStyle = UI::C_AdobeFillStyle( ConvertColour( style.fill.color ) );
	}
	else
	if( kind == UI::C_AdobeFillStyle::E_FillStyle_Kind_Gradient )
	{
		/*
		// Check if this already exists
		S32 index = -1; 
		for(S32 i = 0; i < gUsedGradientsList.Size(); i++)
			if( style.fill.color.c.b.gradient == gUsedGradientsList[i] )
			{
				index = i;
				break;
			}

		if(index == - 1)
		{
			// Push dictionary index
			index = gUsedGradientsList.Size();
			gUsedGradientsList.PushBack( style.fill.color.c.b.gradient );
		}

		// Get the rotation angle from the gradient matrix
		const Maths::C_MatrixRotateRPH rph(Maths::HMat33( 
			Maths::HVec3( style.fill.color.c.b.matrix.a,	style.fill.color.c.b.matrix.b,	0.0f ),
			Maths::HVec3( style.fill.color.c.b.matrix.c,	style.fill.color.c.b.matrix.d,	0.0f ),
			Maths::HVec3( 0.0f,								0.0f,							1.0f )).GetRotationRPH() );
		F32 angle = (Maths::HF32)rph.Roll();

		// Build our gradient object
		UI::C_AdobeGradientStyle gradient;

		gradient.m_DictionaryIndex	= index;
		gradient.m_Angle			= style.fill.color.c.b.gradientAngle + Maths::RadToDeg( angle );
		gradient.m_Px				= style.fill.color.c.b.gradientOrigin.h;
		gradient.m_Py				= style.fill.color.c.b.gradientOrigin.v;
		gradient.m_Length			= style.fill.color.c.b.gradientLength;

		// Asign to the shape's fill style
		pPolygon->m_FillStyle = UI::C_AdobeFillStyle( gradient );
		*/
	}
	return true;
}


const bool C_ShapeBuilder::SetStrokeStyle( const AIArtHandle pPath, UI::C_AdobePolygon* pPolygon )
{
	AIPathStyle	style;
	sAIPathStyle->GetPathStyle( pPath, &style );

	if( !style.strokePaint )
		return false;

//	F32 height = top - bottom;
//	F32 width = right - left;

//	F32 strokeWidth = ( width > height ) ? ( style.stroke.width / width ) : ( style.stroke.width / height );

	pPolygon->m_StrokeStyle = UI::C_AdobeStrokeStyle( ConvertColour( style.stroke.color ), style.stroke.width );
	return true;
}


const bool C_ShapeBuilder::EvaluateFillSegments( const UI::AdobePointList& vSegments, const bool bClosed, UI::TVerticies& vVerticies )
{
	UI::C_AdobePoint* pCurrentPoint = NULL;
	UI::C_AdobePoint* pNextPoint = NULL;

	// Allocate enough space for the existing points and the new points.
	S32 size = vSegments.Size();
	S32 count = 0;

	S32 sortedSize = (S32)( size + ( size * (S32)m_CurveStep ) );
	vVerticies.SetCapacity( sortedSize );

	// pre-pass to find max curve length
	// - most of this is copy-pasta-ed from below - meh!
	// - TODO - pre-store lengths
	F32 fMaxCurveLength = 0.0f;

	for ( S32 i = 0, next = 1; i < size; i++, next++ )
	{
		const UI::C_AdobePoint& currentPoint = vSegments[i];
		const UI::C_AdobePoint& nextPoint = vSegments[ ( next < size ) ? next : 0 ];

		if ( !bClosed && ( next >= size ) )
			break;

		const Maths::HVec2 &p1 = currentPoint.m_Pos;
		const Maths::HVec2 &p2 = currentPoint.m_Out;
		const Maths::HVec2 &p3 = nextPoint.m_In;
		const Maths::HVec2 &p4 = nextPoint.m_Pos;

		F32 curveLength = BezierLength( p1, p2, p3, p4 );
		F32 lineLength = Maths::Abs( ( p4 - p1 ).Length() );

		if ( curveLength > 0.0f && lineLength > 0.0f )
		{
			F32 fraction = curveLength / lineLength;

			if ( fraction > kStrightLineTolerence && fraction <= 1.0f )
				continue;
		}

		if ( curveLength > fMaxCurveLength )
			fMaxCurveLength = curveLength;
	}

	// Iterate through all of the points on this polygon.
	for ( S32 i = 0, next = 1; i < size; i++, next++ )
	{
		// Get the current and next points.
		const UI::C_AdobePoint& currentPoint = vSegments[i];
		const UI::C_AdobePoint& nextPoint = vSegments[ ( next < size ) ? next : 0 ];

		// Add the current point.
		vVerticies.PushBack( UI::C_AdobeVertex( currentPoint.m_Pos ) );

		// need a isClosed flag!
		if ( !bClosed && ( next >= size ) )
			break;

		count++;

		// calculate curve point between current and next
		const Maths::HVec2 &p1 = currentPoint.m_Pos;
		const Maths::HVec2 &p2 = currentPoint.m_Out;
		const Maths::HVec2 &p3 = nextPoint.m_In;
		const Maths::HVec2 &p4 = nextPoint.m_Pos;

		// Check to see how striaght the segment is.
		F32 curveLength = BezierLength( p1, p2, p3, p4 );
		F32 lineLength = Maths::Abs( ( p4 - p1 ).Length() );

		// Check for divide by zero
		if ( curveLength > 0.0f && lineLength > 0.0f )
		{
			F32 fraction = curveLength / lineLength;

			if ( fraction > kStrightLineTolerence && fraction <= 1.0f )
				continue;
		}

		// Calc number of steps to make
		F32 fStep = m_CurveStep;

		/*
		fStep *= ( curveLength / fMaxCurveLength );
		fStep = Maths::Ceil( fStep );
		*/

		// Change to step increment
		fStep = 1.0f / fStep;

		for ( F32 a = ( 0.0f + fStep ); a < 1.0f; a += fStep )
		{
			F32 mu = a;
			F32 mum1 = 1 - mu;
			F32 mum13 = mum1 * mum1 * mum1;
			F32 mu3 = mu * mu * mu;

			Maths::HVec2 pos = (p1*mum13) + (p2*3.0f*mu*mum1*mum1) + (p3*3.0f*mu*mu*mum1) + (p4*mu3);

			UI::C_AdobePoint point( pos.X(), pos.Y() );
			vVerticies.PushBack( UI::C_AdobeVertex( point.m_Pos ) );
		}
		if ( count >= size )
			break;
	}
	return true;
}


const bool C_ShapeBuilder::EvaluateStrokeSegments( const UI::TVerticies& vSegments, const bool bClosed, const F32 fWidth, UI::TVerticies& vStroke )
{
	vStroke.Clear(); // just to be sure

	S32 numSegments = vSegments.Size();

	S32	numVertices = ( bClosed ) ? numSegments + 1 : numSegments;
	F32	outlineLength = 0.0f;

	// This shouldn't happen
	if(numVertices < 2)
		return false;

	Maths::HVec3 p1( 0.0f, 0.0f, 0.0f );
	Maths::HVec3 p2( 0.0f, 0.0f, 0.0f );
	Maths::HVec3 p3( 0.0f, 0.0f, 0.0f );

	Maths::HVec3 r1( 0.0f, 0.0f, 0.0f );
	Maths::HVec3 r2( 0.0f, 0.0f, 0.0f );

	F32 totalDist = 0.0f;
	Btl::Vector<F32> uvTable;
	uvTable.Resize(numVertices + ( numVertices / 2 )); // Incase we insert points.
	U32 uvIndex = 0;
	bool addExtraPoint = false;

	// Always be 0.0f
	uvTable[0] = 0.0f;

	// Is it a closed shape?
	if ( bClosed )
	{
		for ( S32 i = 0; i < numVertices; i++ )
		{
			addExtraPoint = false; // reset

			p1 = ( ( i % numSegments ) == 0 ) ? vSegments[ numSegments - 1 ].m_Pos : vSegments[ i - 1 ].m_Pos;
			p2 = vSegments[ i % numSegments ].m_Pos;
			p3 = vSegments[ ( i + 1 ) % numSegments ].m_Pos;

			// Get outside / top point.
			GetOutlinePoint( p1, p2, p3, r1, fWidth, 1.0f );

			// Get inside / bottom point.
			GetOutlinePoint( p1, p2, p3, r2, fWidth, -1.0f );

			// Trim the corner if needed and add the points to the list.
			//BuildCorner( p1, p2, p3, r1, r2, vStroke, fWidth, addExtraPoint );
			// ==
			Maths::HVec3 norm1 = ( p2 - r2 );
			Maths::HVec3 norm2 = ( p2 - r1 );

			vStroke.PushBack( UI::C_AdobeVertex( Maths::HVec2( r2 ), Maths::HVec4( norm1 ) ) );
			vStroke.PushBack( UI::C_AdobeVertex( Maths::HVec2( r1 ), Maths::HVec4( norm2 ) ) );

			// Store the distance so that the UV table can do some maths.
			if ( i > 0 )
			{
				totalDist += p2.Distance( p3 );
				uvTable[uvIndex] = totalDist;

				if ( addExtraPoint )
					uvTable[++uvIndex] = totalDist;
			}
			uvIndex++;
		}
	}
	else
	{
		Maths::HVec3 n1( 0.0f, 0.0f, 0.0f );

		for ( S32 i = 0; i < numVertices; i++ )
		{
			if ( i != 0 && i != numVertices - 1 )
			{
				addExtraPoint = false; // reset

				p1 = vSegments[ i - 1 ].m_Pos;
				p2 = vSegments[ i ].m_Pos;
				p3 = vSegments[ i + 1 ].m_Pos;

				// Outside / Top
				GetOutlinePoint( p1, p2, p3, r1, fWidth, 1.0f );

				// Inside / Bottom
				GetOutlinePoint( p1, p2, p3, r2, fWidth, -1.0f );

				// Trim the corner if needed and add the points to the list.
				//BuildCorner( p1, p2, p3, r1, r2, vStroke, fWidth, addExtraPoint );
				// ==
				Maths::HVec3 norm1 = ( p2 - r2 );
				Maths::HVec3 norm2 = ( p2 - r1 );

				vStroke.PushBack( UI::C_AdobeVertex( Maths::HVec2( r2 ), Maths::HVec4( norm1 ) ) );
				vStroke.PushBack( UI::C_AdobeVertex( Maths::HVec2( r1 ), Maths::HVec4( norm2 ) ) );

				// Store the distance so that the UV table can do some maths.
				if ( i > 0 )
				{
					totalDist += p2.Distance( p3 );
					uvTable[uvIndex] = totalDist;

					if ( addExtraPoint )
						uvTable[++uvIndex] = totalDist;
				}
			}
			else
			{
				// End pieces.
				p1 = vSegments[ ( i == 0 ) ? i : i - 1 ].m_Pos;
				p2 = vSegments[ ( i == 0 ) ? i + 1 : i ].m_Pos;

				n1 = ( p2 - p1 ) ^ Maths::HVec3( 0.0f, 0.0f, 1.0f );
				n1.Normalise();

				Maths::HVec3 p = ( i == 0 ) ? p1 : p2;

				r1 = p + ( -n1 * fWidth );
				r2 = p + (  n1 * fWidth );

				Maths::HVec3 norm1 = ( p2 - r1 );
				Maths::HVec3 norm2 = ( p2 - r2 );

				vStroke.PushBack( UI::C_AdobeVertex( Maths::HVec2( r1 ), Maths::HVec4( norm1 ) ) );
				vStroke.PushBack( UI::C_AdobeVertex( Maths::HVec2( r2 ), Maths::HVec4( norm2 ) ) );

				// Store the distance so that the UV table can do some maths.
				if ( i > 0 )
				{
					totalDist += p1.Distance( p2 );
					uvTable[uvIndex] = totalDist;
				}
			}
			uvIndex++;
		}
	}

	// Build the UV table and normals
	S32 index = 0;
	F32	u = 0.0f;
	//Maths::HVec3 norm( 0.0f, 0.0f, 0.0f );

	for ( S32 i = 0; i < vStroke.Size(); i++ )
	{
		if ( index > 0 )
			u = uvTable[ index ] / totalDist;	

		// Generate normals.
		/*
		if ( !(i & 1) )
		{
			norm = vStroke[i + 1].m_Pos - vStroke[i].m_Pos;
			norm.Normalise();
		}
		*/

		vStroke[i].m_UV = Maths::HVec2( u, 1.0f * ( i & 1 ) );
		//vStroke[i].m_Normal = norm;

		index += 1 * ( i & 1 );
	}

	return true;
}


const bool C_ShapeBuilder::EvaluateFixedSpaceStrokeSegments( const UI::AdobePointList& vSegments, const F32 fSpacing, UI::TVerticies& vStroke )
{
	UI::AdobePointList sortedPointList;
	UI::C_AdobePath path;

	for ( S32 i = 0; i < vSegments.Size(); i++ )
		path.AddPoint( vSegments[i] );

	path.CalculateLengths();
	path.EvaluateSplineFixedSpacing( sortedPointList, fSpacing );
	path.RemovePoints();

	for ( S32 i = 0; i < sortedPointList.Size(); i++ )
		vStroke.PushBack( UI::C_AdobeVertex( sortedPointList[i].m_Pos ) );

	return true;
}


void C_ShapeBuilder::GetOutlinePoint( Maths::HVec3& p1, Maths::HVec3& p2, Maths::HVec3& p3, Maths::HVec3& p, F32 length, float z )
{
	// outline point is the intersection of the two extruded incident edges of this vertex
	Maths::HVec3 n1 = ( p2 - p1 ) ^ Maths::HVec3( 0.0f, 0.0f, z );
	n1.Normalise();
	Maths::HVec3 n2 = ( p3 - p2 ) ^ Maths::HVec3( 0.0f, 0.0f, z );
	n2.Normalise();

	Maths::HVec2 i1 = Maths::HVec2( p1 + ( n1 * length ) );
	Maths::HVec2 i2 = Maths::HVec2( p2 + ( n1 * length ) );
	Maths::HVec2 j1 = Maths::HVec2( p2 + ( n2 * length ) );
	Maths::HVec2 j2 = Maths::HVec2( p3 + ( n2 * length ) );

	F32 a = 0.0f;
	F32 b = 0.0f;

	bool bIntersect = LineIntersect( i1, i2, j1, j2, a, b, false );

	if ( bIntersect )
		p = i1 + ( i2 - i1 ) * a; //p = j1 + ( j2 - j1 ) * b;
	else
	{
		// use the approximate normal - it's not perfect but does the job in this situation
		Maths::HVec3 d1 = ( p2 - p1 ) ^ Maths::HVec3( 0.0f, 0.0f, z );
		d1.Normalise();
		Maths::HVec3 d2 = ( p3 - p2 ) ^ Maths::HVec3( 0.0f, 0.0f, z );
		d2.Normalise();

		Maths::HVec3 ex = ( d1 + d2 ) * 0.5f;
		ex.Normalise();

		p = p2 + ( ex * length );
	}
}

void C_ShapeBuilder::BuildCorner
(
	Maths::HVec3& p1, 
	Maths::HVec3& p2, 
	Maths::HVec3& p3, 
	Maths::HVec3& r1, 
	Maths::HVec3& r2,
	UI::TVerticies& vStroke,
	F32 length,
	bool& extraPoint
)
{
	// If angle is less than the threshold then some magic is needed.
	Maths::HVec3 n1 = ( p1 - p2 );
	n1.Normalise();
	Maths::HVec3 n2 = ( p3 - p2 );
	n2.Normalise();

	F32 dot = Maths::DotProduct( n1, n2 );
	F32 theata = acos( Maths::Clamp( dot, 0.0f, Maths::Pi ) );
	F32 angle = Maths::RadToDeg( theata );

	if ( false ) //angle >= 0 && angle <= kTrimOutlineThreshold )
	{
		// ***********
		// PR - TODO - normals for these
		// ***********
		Maths::HVec3 r3( 0.0f, 0.0f, 0.0f );

		//------------------------------------------------------------
		// Need to work out the rotation order to stop mesh twisting.
		// 1						2
		// p1.				p3.	||			p2.
		// 			p2.			||	p1.				p3.		
		//------------------------------------------------------------
		n1 = ( p2 - p1 ) ^ Maths::HVec3( 0.0f, 0.0f, 1.0f );
		n1.Normalise();

		// Check the rotation order.
		F32 angle = n1 | ( p3 - p2 );
		if ( angle < 0 )
		{
			n1 = ( p2 - r1 ) ^ Maths::HVec3( 0.0f, 0.0f, 1.0f );
			n1.Normalise();

			r1 = p2 + ( n1 * length );
			r2 = p2 + ( n1 * -length );

			vStroke.PushBack( Maths::HVec2( r1 ) );
			vStroke.PushBack( Maths::HVec2( r2 ) );
			vStroke.PushBack( Maths::HVec2( r2 ) );
			vStroke.PushBack( Maths::HVec2( r1 ) );

			// This will get added twice.
			extraPoint = true;
		}
		else
		{
			n1 = ( p2 - r1 ) ^ Maths::HVec3( 0.0f, 0.0f, 1.0f );
			n1.Normalise();

			r2 = p2 + ( n1 * length );
			r1 = p2 + ( n1 * -length );

			vStroke.PushBack( Maths::HVec2( r1 ) );
			vStroke.PushBack( Maths::HVec2( r2 ) );
			vStroke.PushBack( Maths::HVec2( r2 ) );
			vStroke.PushBack( Maths::HVec2( r1 ) );

			// This will get added twice.
			extraPoint = true;
		}
	}
	else
	{
		vStroke.PushBack( Maths::HVec2( r2 ) );
		vStroke.PushBack( Maths::HVec2( r1 ) );
	}
}


const F32 C_ShapeBuilder::BezierLength(const Maths::HVec3& b0, const Maths::HVec3& b1, const Maths::HVec3&b2, const Maths::HVec3& b3 )
{
	Maths::HVec3 p0(b0 - b1);
	Maths::HVec3 p1(b2 - b1);
	Maths::HVec3 p2;
	Maths::HVec3 p3(b3 - b2);
	F32 l0 = p0.Length();
	F32 l1 = p1.Length();
	F32 l3 = p3.Length();
	if (l0 > 0.f) p0 /= l0;
	if (l1 > 0.f) p1 /= l1;
	if (l3 > 0.f) p3 /= l3;
	p2 = -p1;
	F32 a = Maths::Abs(p0 | p1) + Maths::Abs(p2 | p3);

//	if ((a > 1.98f) || ((l0+l1+l3) < 1.0f ) )
	if ((a > 1.98f) || ((l0+l1+l3) < ((4.f - a)*8.f)))
	{
		return l0 + l1 + l3;
	} 

	Maths::HVec3 bl0( b0 );
	Maths::HVec3 bl1( (b0 + b1) * 0.5f );
	Maths::HVec3 mid( (b1 + b2) * 0.5f );

	Maths::HVec3 bl2( (bl1 + mid) * 0.5f );
	Maths::HVec3 br3( b3 );

	Maths::HVec3 br2( (b2 + b3) * 0.5f );
	Maths::HVec3 br1( (br2 + mid) * 0.5f );
	Maths::HVec3 br0( (br1 + bl2) * 0.5f );

	Maths::HVec3 bl3( br0 );

	return BezierLength(bl0, bl1, bl2, bl3) + BezierLength(br0, br1, br2, br3);
}


const bool C_ShapeBuilder::LineIntersect( const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& q1, const Maths::HVec2& q2, F32& a, F32& b, const bool bSegmentTest )
{
	Maths::HVec2 q2q1 = q2 - q1;
	Maths::HVec2 p2p1 = p2 - p1;

	F32 fDenominator = ( q2q1.Y() * p2p1.X() ) - ( q2q1.X() * p2p1.Y() );

	if ( Maths::Abs( fDenominator ) < 0.01f )	// Hmmmm ...
		return false;		// almost/are parallel

	// else they intersect

	Maths::HVec2 p1q1 = p1 - q1;

	F32 fNumeratorA = ( q2q1.X() * p1q1.Y() ) - ( q2q1.Y() * p1q1.X() );
	F32 fNumeratorB = ( p2p1.X() * p1q1.Y() ) - ( p2p1.Y() * p1q1.X() );

	a = fNumeratorA / fDenominator;
	b = fNumeratorB / fDenominator;

	return !bSegmentTest || !( a < 0.0f || a > 1.0f || b < 0.0f || b > 1.0f );
}


void C_ShapeBuilder::Triangulate( UI::UIPolygonList& results, UI::UIPolygonIndexList& indices )
{
	UI::UIPolygonList originalList = results;

	UI::Triangulate( results, indices, true );

	// make sure original list is winding correctly
	if ( !originalList[ 0 ].CW() )
		originalList[ 0 ].ReverseWindingOrder();

	// do the finning
	DoSoftwareAA_Shape( originalList, results, indices );
}

void C_ShapeBuilder::DoSoftwareAA_Shape( UI::UIPolygonList& vertexList, UI::UIPolygonList& resolvedList, UI::UIPolygonIndexList& indexList )
{
	if ( m_bDisableFinning )
	{
		// just make sure z-normal is set
		for ( S32 i = 0; i < resolvedList.Size(); i++ )
		{
			UI::C_UIResultPolygon& poly = resolvedList[ i ];

			for ( U32 j = 0; j < poly.NumPoints(); j++ )
			{
				poly[ j ].VX( 0.0f );
				poly[ j ].VY( 0.0f );
				poly[ j ].VZ( 1.0f );
				poly[ j ].VW( 0.0f );
			}
		}

		return;
	}

	// the hole-resolver should make sure we're winded correctly

	// calculate and store inwards-normals
	for ( S32 i = 0; i < vertexList.Size(); i++ )
	{
		for ( U32 j = 0; j < vertexList[ i ].NumPoints(); j++ )
		{
			Maths::HVec2 p2 = vertexList[ i ][ j ].GetPoint();
			Maths::HVec2 p = vertexList[ i ].GetOutlinePoint( j );

			// new outline uses a length ratio, then we control the size of the outline in the vertex shader
			Maths::HVec2 ratio = ( p - p2 );

			vertexList[ i ][ j ].VX( -ratio.X() * 0.5f );
			vertexList[ i ][ j ].VY( -ratio.Y() * 0.5f );
			vertexList[ i ][ j ].VZ( 1.0f );
			vertexList[ i ][ j ].VW( 0.0f );
		}
	}
	
	// now find the corresponding vertex in the resolved and triangulated list
	for ( S32 i = 0; i < resolvedList.Size(); i++ )
	{
		for ( U32 j = 0; j < resolvedList[ i ].NumPoints(); j++ )
		{
			bool bFound = false;
			Maths::HVec2 v;

			for ( S32 x = 0; x < vertexList.Size(); x++ )
			{
				for ( U32 y = 0; y < vertexList[ x ].NumPoints(); y++ )
				{
					if ( resolvedList[ i ][ j ] == vertexList[ x ][ y ] )
					{
						// copy over the normal
						resolvedList[ i ][ j ].VX( vertexList[ x ][ y ].VX() );
						resolvedList[ i ][ j ].VY( vertexList[ x ][ y ].VY() );
						resolvedList[ i ][ j ].VZ( vertexList[ x ][ y ].VZ() );
						resolvedList[ i ][ j ].VW( vertexList[ x ][ y ].VW() );

						bFound = true;
						break;
					}
				}

				if ( bFound )
					break;
			}

			BIZ_ASSERT( bFound );
		}
	}

	// build the outlines
	for ( S32 i = 0; i < vertexList.Size(); i++ )
	{
		resolvedList.PushBack( UI::C_UIResultPolygon() );
		UI::C_UIResultPolygon& outline = resolvedList.Back();

		for ( U32 j = 0; j < vertexList[ i ].NumPoints(); j++ )
		{
			UI::C_UIResultPoint p1( vertexList[ i ][ j ].GetPoint(), -vertexList[ i ][ j ].GetVector4() );
			UI::C_UIResultPoint p2( vertexList[ i ][ j ].GetPoint(),  vertexList[ i ][ j ].GetVector4() );

			p1.VZ( 0.0f );
			p1.VW( 2.0f );

			p2.VZ( 1.0f );
			p2.VW( 0.0f );

			outline.AddPoint( p1 );
			outline.AddPoint( p2 );
		}

		indexList.PushBack( UI::C_ResultIndexList() );
		UI::C_ResultIndexList& outlineIndexList = indexList.Back();

		for ( U32 j = 0; j < outline.NumPoints(); j++ )
		{
			if ( j & 1 )
			{
				outlineIndexList.AddIndex( j );
				outlineIndexList.AddIndex( ( j + 1 ) % outline.NumPoints() );
				outlineIndexList.AddIndex( ( j + 2 ) % outline.NumPoints() );
			}
			else
			{
				outlineIndexList.AddIndex( j );
				outlineIndexList.AddIndex( ( j + 2 ) % outline.NumPoints() );
				outlineIndexList.AddIndex( ( j + 1 ) % outline.NumPoints() );
			}
		}
	}
}

S32 C_ShapeBuilder::GetCurveStepOverride( Util::C_String& name )
{
	S32 sPos = name.Search( "__" );

	if ( sPos == -1 )
		return -1;

	S32 sNum = -1;
	sscanf( name.Chars() + sPos + 2, "%d", &sNum );

	name.NonConstChars()[ sPos ] = 0;

	return sNum;
}

}; // namespace Ipf

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", on ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif