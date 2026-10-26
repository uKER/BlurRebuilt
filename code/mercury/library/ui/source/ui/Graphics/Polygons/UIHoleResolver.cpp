#include "stdafx.h"
#include "ui/Graphics/Polygons/UIHoleResolver.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

void FindAndResolveHoles( UIPolygonList& polygonList, UIPolygonList& resolvedList )
{
	Btl::Vector< C_UIResultPolygon> holes;

	for ( S32 i = 0; i < polygonList.Size(); i++ )
	{
		if ( polygonList[ i ].HoleProcessed() )
			continue;

		holes.Clear();

		// check other polys for holes
		for ( S32 j = 0; j < polygonList.Size(); j++ )
		{
			if ( ( i == j ) || polygonList[ j ].HoleProcessed() )
				continue;

			bool bOk = true;

			for ( U32 k = 0; k < polygonList[ j ].NumPoints(); k++ )
			{
				if ( !UI::IsVertexInsidePoly( polygonList[ j ][ k ].GetPoint(), polygonList[ i ] ) )
				{
					bOk = false;
					break;
				}
			}
			
			if ( bOk )
			{
				// hole must be CCW
				if ( polygonList[ j ].CW() )
					polygonList[ j ].ReverseWindingOrder();

				holes.PushBack( polygonList[ j ] );
				polygonList[ j ].HoleProcessed( true );
			}
		}

		// resolve the found holes if we're not an island
		if ( holes.Size() != 0 )
		{
			// poly must be CW
			if ( !polygonList[ i ].CW() )
				polygonList[ i ].ReverseWindingOrder();

			C_UIResultPolygon result = UI::ResolveMultipleHoles( polygonList[ i ], holes );
			resolvedList.PushBack( result );
			polygonList[ i ].HoleProcessed( true );
		}
	}

	// do another pass to add in the islands
	for ( S32 i = 0; i < polygonList.Size(); i++ )
	{
		if ( polygonList[ i ].HoleProcessed() )
			continue;
		
		// poly must be CW
		if ( !polygonList[ i ].CW() )
			polygonList[ i ].ReverseWindingOrder();

		resolvedList.PushBack( polygonList[ i ] );
	}
}

C_UIResultPolygon ResolveHole( C_UIResultPolygon& hole, C_UIResultPolygon& poly )
{
	C_UIResultPolygon polygon; // the result

	C_UIResultPoint holeV;
	C_UIResultPoint polyV;

	if ( FindHoleJoinEdge( hole, poly, holeV, polyV ) )
		polygon = BuildJoinedPoly( hole, poly, holeV, polyV );

	return polygon;
}

C_UIClipPolygon* ResolveHole( C_UIClipPolygon& hole, C_UIClipPolygon& poly )
{
	C_UIResultPolygon h( hole );
	C_UIResultPolygon p( poly );

	C_UIResultPolygon result = UI::ResolveHole( h, p );

	C_UIClipPolygon* pResult = new C_UIClipPolygon( result );
	assert( pResult );

	return pResult;
}

C_UIResultPolygon ResolveMultipleHoles( C_UIResultPolygon& outside, Btl::Vector< C_UIResultPolygon >& holes )
{
	C_UIResultPolygon polygon = outside;

	Btl::Vector< C_UIResultPoint > dupeList;

	for ( S32 i = 0; i < holes.Size(); i++ )
	{
		C_UIResultPoint holeV;
		C_UIResultPoint polyV;

		// this should never actually fail now
		if ( FindHoleJoinEdge( holes[ i ], polygon, holeV, polyV, dupeList, holes, i ) )
		{
			// we've found a valid edge
			C_UIResultPolygon tempPoly = BuildJoinedPoly( holes[ i ], polygon, holeV, polyV );

			// add to the list of vertices we can't use in the next iteration
			dupeList.PushBack( holeV );
			dupeList.PushBack( polyV );

			polygon = tempPoly;
		}
		else
		{
			// oh no!
			//assert( false && "failed to the find a edge!" );
		}		
	}

	return polygon;
}

static bool FindHoleJoinEdge( C_UIResultPolygon& hole, C_UIResultPolygon& poly, C_UIResultPoint& holeV, C_UIResultPoint& polyV )
{
	Btl::Vector< C_UIResultPoint > dupeList;
	Btl::Vector< C_UIResultPolygon > holeList;

	return FindHoleJoinEdge( hole, poly, holeV, polyV, dupeList, holeList );
}

static bool FindHoleJoinEdge( C_UIResultPolygon& hole, C_UIResultPolygon& poly, C_UIResultPoint& holeV, C_UIResultPoint& polyV, Btl::Vector< C_UIResultPoint >& dupeList, Btl::Vector< C_UIResultPolygon >& holeList, const S32 sHoleIndex )
{
	// traverse the hole, testing edges with the hole and outer poly for intersections
	// -- if nothing intersects, then we've found two mutually visible vertices
	bool bFound = false;

	for ( U32 i = 0; i < hole.NumPoints(); i++ )
	{
		for ( U32 j = 0; j < poly.NumPoints(); j++ )
		{
			C_UIResultPoint& p1 = hole[ i ];
			C_UIResultPoint& p2 = poly[ j ];

			F32 a, b;

			bool bOk = true;

			// test hole for intersections
			for ( U32 k = 0; k < hole.NumPoints(); k++ )
			{
				C_UIResultPoint& q1 = hole[ k ];
				C_UIResultPoint& q2 = hole[ ( k + 1 ) % hole.NumPoints() ];

				if ( UI::LineIntersect( p1.GetPoint(), p2.GetPoint(), q1.GetPoint(), q2.GetPoint(), a, b ) )
				{
					// discard these cases as it will intersect at two vertices
					if ( ( a != 0.0f && a != 1.0f ) || ( b != 0.0f && b != 1.0f ) )
					{
						bOk = false;
						break;
					}
				}
			}

			if ( bOk )
			{
				// test poly for intersections
				for ( U32 k = 0; k < poly.NumPoints(); k++ )
				{
					C_UIResultPoint& q1 = poly[ k ];
					C_UIResultPoint& q2 = poly[ ( k + 1 ) % poly.NumPoints() ];

					if ( UI::LineIntersect( p1.GetPoint(), p2.GetPoint(), q1.GetPoint(), q2.GetPoint(), a, b ) )
					{
						// discard these cases as it will intersect at two vertices
						if ( ( a != 0.0f && a != 1.0f ) || ( b != 0.0f && b != 1.0f ) )
						{
							bOk = false;
							break;
						}
					}
				}

				if ( bOk )
				{
					// check the dupe list
					for ( S32 k = 0; k < dupeList.Size(); k++ )
					{
						if ( ( dupeList[ k ] == p1 ) || ( dupeList[ k ] == p2 ) )
						{
							bOk = false;
							break;
						}
					}
					
					// check the rest of the holes
					for ( S32 k = ( sHoleIndex + 1 ); k < holeList.Size(); k++ )
					{
						// each edge in the hole
						for ( U32 l = 0; l < holeList[ k ].NumPoints(); l++ )
						{
							if ( UI::LineIntersect( p1.GetPoint(), p2.GetPoint(), holeList[ k ][ l ].GetPoint(), holeList[ k ][ ( l + 1 ) % holeList[ k ].NumPoints() ].GetPoint(), a, b ) )
							{
								// discard these cases as it will intersect at two vertices
								if ( ( a != 0.0f && a != 1.0f ) || ( b != 0.0f && b != 1.0f ) )
								{
									bOk = false;
									break;
								}
							}
						}

						if ( !bOk )
							break;
					}

					if ( bOk )
					{
						bFound = true;
						holeV = p1;
						polyV = p2;
						break;
					}
				}
			}
		}

		if ( bFound )
			break;
	}

	return bFound;
}

static C_UIResultPolygon BuildJoinedPoly( C_UIResultPolygon& hole, C_UIResultPolygon& poly, C_UIResultPoint& holeV, C_UIResultPoint& polyV )
{
	// poly must be CW
	if ( !poly.CW() )
		poly.ReverseWindingOrder();

	// hole must be CCW
	if ( hole.CW() )
		hole.ReverseWindingOrder();

	// build the result poly
	C_UIResultPolygon polygon;

	bool bFound = false;
	bool bFound2 = false;

	// -- traverse the outer poly till we reach the pPolyV vertex
	for ( U32 i = 0; i < poly.NumPoints(); i++ )
	{
		polygon.AddPoint( poly[ i ] );

		if ( ( poly[ i ] == polyV ) && !bFound )
		{
			bFound = true;

			// traverse the hole till we reach the pHoleV vertex
			for ( U32 j = 0; j < hole.NumPoints(); j++ )
			{
				if ( ( hole[ j ] == holeV ) && !bFound2 )
				{
					bFound2 = true;

					// traverse the hole
					for ( U32 k = 0; k < hole.NumPoints(); k++ )
						polygon.AddPoint( hole[ ( j + k ) % hole.NumPoints() ] );

					break;
				}
			}

			// dupe the entry point of the hole
			polygon.AddPoint( holeV );

			// dupe the exit point of the poly
			polygon.AddPoint( polyV );
		}
	}

	return polygon;
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
