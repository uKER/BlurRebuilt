#include "stdafx.h"
#include "ui/Graphics/Polygons/UIClipOperations.h"
#include "ui/Graphics/Polygons/UIHoleResolver.h"

namespace UI
{

void ClipOperation( C_UIClipPolygon& subject, C_UIClipPolygon& clip, Btl::Vector< C_UIClipPolygon* >& result, const E_UIPolyClip eClipTest )
{
	// stage 1
	// -- determine the intersection points and sort into both polys any intersection vertices
	bool bHasIntersections = Stage1( subject, clip );

	if ( !bHasIntersections )
	{
		// polys are disjoint - may need to resolve a hole
		HandleDisjointPolys( subject, clip, result, eClipTest );
	}
	else
	{
		// exclude is a special case
		if ( eClipTest == E_UIPolyClip_Exclude )
		{
			// A-B
			Stage2( subject, clip, E_UIPolyClip_AMinusB );
			Stage3( subject, clip, result );
			// B-A
			Stage2( subject, clip, E_UIPolyClip_BMinusA );
			Stage3( subject, clip, result );
		}
		else
		{
			// stage 2
			// -- trace each polygon and mark the entry and exit points to the other polys interior
			Stage2( subject, clip, eClipTest );

			// stage 3
			// -- process all of the intersection vertices in the subject poly to create the clip poly(s)
			Stage3( subject, clip, result );
		}
	}
}

static bool Stage1( C_UIClipPolygon& subject, C_UIClipPolygon& clip )
{
	bool bHasIntersections = false;

	F32 a, b;

	S32 sSize1 = subject.NumVertices();
	S32 sSize2 = clip.NumVertices();

	// test all original edges for any intersections

	for ( S32 i = 0; i < sSize1; i++ )
	{
		for ( S32 j = 0; j < sSize2; j++ )
		{
			C_UIClipVertex* p1 = subject[ i ];
			C_UIClipVertex* p2 = subject[ ( i + 1 ) % sSize1 ];
			C_UIClipVertex* q1 = clip[ j ];
			C_UIClipVertex* q2 = clip[ ( j + 1 ) % sSize2 ];

			if ( UI::LineIntersect( p1->m_Pos, p2->m_Pos, q1->m_Pos, q2->m_Pos, a, b ) )
			{
				bHasIntersections = true;

				// calculate intersection vertex
				F32 fX = p1->m_Pos.X() + a * ( p2->m_Pos.X() - p1->m_Pos.X() );
				F32 fY = p1->m_Pos.Y() + a * ( p2->m_Pos.Y() - p1->m_Pos.Y() );
				
				//C_UIClipVertex* pVertex2 = new C_UIClipVertex(
				//	q1->m_Pos.X() + b * ( q2->m_Pos.X() - q1->m_Pos.X() ),
				//	q1->m_Pos.Y() + b * ( q2->m_Pos.Y() - q1->m_Pos.Y() ) );

				C_UIClipVertex* pVertex1 = new C_UIClipVertex( fX, fY );
				C_UIClipVertex* pVertex2 = new C_UIClipVertex( fX, fY );

				// create the neighbour links
				pVertex1->m_bIntersect = true;
				pVertex1->m_fAlpha = a;
				pVertex1->m_pNeighbour = pVertex2;

				pVertex2->m_bIntersect = true;
				pVertex2->m_fAlpha = b;
				pVertex2->m_pNeighbour = pVertex1;

				// ** START
				// TODO - THIS PART BETTER!
				bool bDiscard1 = false;
				// check for dupe vertices first and discard if it is
				for ( S32 k = 0; k < subject.NumVertices(); k++ )
				{
					// TODO - some "better" limit based on something
					if ( ( subject[ k ]->m_Pos - pVertex1->m_Pos ).Length() < 1.0f )
					{
						bDiscard1 = true;
						SAFE_DELETE_PTR( pVertex1 );

						// update the dupe
						pVertex1 = subject[ k ];
						pVertex1->m_bIntersect = true;
						pVertex1->m_fAlpha = a;
						pVertex1->m_pNeighbour = pVertex2;
						pVertex2->m_pNeighbour = pVertex1;
					}
				}

				bool bDiscard2 = false;
				// check for dupe vertices first and discard if it is
				for ( S32 k = 0; k < clip.NumVertices(); k++ )
				{
					// TODO - some "better" limit based on something
					if ( ( clip[ k ]->m_Pos - pVertex2->m_Pos ).Length() < 1.0f )
					{
						bDiscard2 = true;
						SAFE_DELETE_PTR( pVertex2 );

						// update the dupe
						pVertex2 = clip[ k ];
						pVertex2->m_bIntersect = true;
						pVertex2->m_fAlpha = a;
						pVertex2->m_pNeighbour = pVertex1;
						pVertex1->m_pNeighbour = pVertex2;
					}
				}
				// ** END

				// now add and sort into both
				if ( !bDiscard1 )
					subject.AddAndSortInto( pVertex1, p1, p2 );

				if ( !bDiscard2 )
					clip.AddAndSortInto( pVertex2, q1, q2 );
			}
		}
	}

	return bHasIntersections;
}

static void Stage2( C_UIClipPolygon& subject, C_UIClipPolygon& clip, const E_UIPolyClip eClipTest )
{
	// traverse the subject
	bool bIsInside1 = UI::IsVertexInsidePoly( subject[ 0 ], clip );

	bool bEntryStatus1 = false;

	switch ( eClipTest )
	{
		case E_UIPolyClip_Intersection:
			bEntryStatus1 = bIsInside1 ? false : true;
			break;
		case E_UIPolyClip_AMinusB:
			bEntryStatus1 = bIsInside1 ? true : false;
			break;
		case E_UIPolyClip_BMinusA:
			bEntryStatus1 = bIsInside1 ? false : true;
			break;
		case E_UIPolyClip_Union:
			bEntryStatus1 = bIsInside1 ? true : false;
			break;
		default:
			assert( false );
			break;
	}

	for ( C_UIClipVertex* p = subject[ 0 ]; p->m_pNext != subject[ 0 ]; p = p->m_pNext )
	{
		if ( p->m_bIntersect )
		{
			p->m_bEntryStatus = bEntryStatus1;
			bEntryStatus1 = !bEntryStatus1;
		}
	}

	// traverse the clip
	bool bIsInside2 = UI::IsVertexInsidePoly( clip[ 0 ], subject );

	bool bEntryStatus2 = false;

	switch ( eClipTest )
	{
		case E_UIPolyClip_Intersection:
			bEntryStatus2 = bIsInside2 ? false : true;
			break;
		case E_UIPolyClip_AMinusB:
			bEntryStatus2 = bIsInside2 ? false : true;
			break;
		case E_UIPolyClip_BMinusA:
			bEntryStatus2 = bIsInside2 ? true : false;
			break;
		case E_UIPolyClip_Union:
			bEntryStatus2 = bIsInside2 ? true : false;
			break;
		default:
			assert( false );
			break;
	}

	for ( C_UIClipVertex* p = clip[ 0 ]; p->m_pNext != clip[ 0 ]; p = p->m_pNext )
	{
		if ( p->m_bIntersect )
		{
			p->m_bEntryStatus = bEntryStatus2;
			bEntryStatus2 = !bEntryStatus2;
		}
	}
}

static void Stage3( C_UIClipPolygon& subject, C_UIClipPolygon& clip, Btl::Vector< C_UIClipPolygon* >& result )
{
	subject.ResetProcessed();
	clip.ResetProcessed();

	C_UIClipVertex* pStart = NULL;
	C_UIClipVertex* pCurrent = NULL;

	// process all of the intersection vertices
	for ( C_UIClipVertex* p = subject[ 0 ]; p->m_pNext != subject[ 0 ]; p = p->m_pNext )
	{
		if ( p->m_bIntersect && !p->m_bProcessed )
		{
			// start a new poly
			C_UIClipPolygon* pPolygon = new C_UIClipPolygon;
			assert( pPolygon );
			result.PushBack( pPolygon );

			pStart = p;
			pCurrent = pStart;

			pStart->m_bProcessed = true;

			do 
			{
				if ( pCurrent->m_bEntryStatus )
				{
					do
					{
						result[ result.Size() - 1 ]->AddVertex( pCurrent->m_Pos );
						pCurrent = pCurrent->m_pNext;
					}
					while ( !pCurrent->m_bIntersect );
				}
				else
				{
					do
					{
						result[ result.Size() - 1 ]->AddVertex( pCurrent->m_Pos );
						pCurrent = pCurrent->m_pPrev;
					}
					while ( !pCurrent->m_bIntersect );
				}

				pCurrent->m_bProcessed = true;
				pCurrent = pCurrent->m_pNeighbour;
				pCurrent->m_bProcessed = true;
			} 
			while ( pCurrent != pStart );
		}
	}

	// create the next/back links for the result polys
	for ( S32 i = 0; i < result.Size(); i++ )
		result[ i ]->CreateInitialNextAndBackLinks();
}

static void HandleDisjointPolys( C_UIClipPolygon& p1, C_UIClipPolygon& p2, Btl::Vector< C_UIClipPolygon* >& result, const E_UIPolyClip eTest )
{
	switch ( eTest )
	{
		case E_UIPolyClip_Intersection:
		{
			if ( UI::IsVertexInsidePoly( p1[ 0 ], p2 ) )
				AddToResult( p1, result );	// p1 is the hole
			else if ( UI::IsVertexInsidePoly( p2[ 0 ], p1 ) )
				AddToResult( p2, result );	// p2 is the hole
			break;
		}
		case E_UIPolyClip_AMinusB:
		{
			if ( UI::IsVertexInsidePoly( p1[ 0 ], p2 ) );
				// NULL - p1 is the hole
			else if ( UI::IsVertexInsidePoly( p2[ 0 ], p1 ) )
			{
				C_UIClipPolygon* pPoly = UI::ResolveHole( p2, p1 );	// p2 is the hole
				
				if ( pPoly )
					AddToResult( *pPoly, result );
			}
			else
				AddToResult( p1, result ); // exclusive - add the first
			break;
		}
		case E_UIPolyClip_BMinusA:
		{
			if ( UI::IsVertexInsidePoly( p2[ 0 ], p1 ) );
				// NULL - p2 is the hole
			else if ( UI::IsVertexInsidePoly( p1[ 0 ], p2 ) )
			{
				C_UIClipPolygon* pPoly = UI::ResolveHole( p1, p2 );	// p1 is the hole

				if ( pPoly )
					AddToResult( *pPoly, result );
			}
			else
				AddToResult( p2, result ); // exclusive - add the second
			break;
		}
		case E_UIPolyClip_Union:
		{
			if ( UI::IsVertexInsidePoly( p1[ 0 ], p2 ) )
				AddToResult( p2, result );	// p1 is the hole
			else if ( UI::IsVertexInsidePoly( p2[ 0 ], p1 ) )
				AddToResult( p1, result );	// p2 is the hole
			else
			{
				AddToResult( p1, result );	// add both
				AddToResult( p2, result );
			}
			break;
		}
		case E_UIPolyClip_Exclude:
		{
			if ( UI::IsVertexInsidePoly( p1[ 0 ], p2 ) )
			{
				C_UIClipPolygon* pPoly = UI::ResolveHole( p1, p2 );	// p1 is the hole

				if ( pPoly )
					AddToResult( *pPoly, result );
			}
			else if ( UI::IsVertexInsidePoly( p2[ 0 ], p1 ) )
			{
				C_UIClipPolygon* pPoly = UI::ResolveHole( p2, p1 );	// p2 is the hole
				
				if ( pPoly )
					AddToResult( *pPoly, result );
			}
			else
			{
				AddToResult( p1, result );	// add both
				AddToResult( p2, result );
			}
			break;
		}
	}
}

static void AddToResult( C_UIClipPolygon& p, Btl::Vector< C_UIClipPolygon* >& result )
{
	C_UIClipPolygon* pPoly = new C_UIClipPolygon;
	assert( pPoly );

	for ( S32 i = 0; i < p.NumVertices(); i++ )
		pPoly->AddVertex( p[ i ]->m_Pos.X(), p[ i ]->m_Pos.Y() );

	pPoly->CreateInitialNextAndBackLinks();

	result.PushBack( pPoly );
}

};	// namespace UI
