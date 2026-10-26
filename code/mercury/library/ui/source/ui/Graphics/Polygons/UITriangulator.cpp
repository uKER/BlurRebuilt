#include "stdafx.h"
#include "ui/Graphics/Polygons/UITriangulator.h"
//#include "ui/Graphics/Simple2D.h"

namespace UI
{

void Triangulate( UIPolygonList& polys, UIPolygonIndexList& result, const bool bDoConvexify )
{
	if ( bDoConvexify )
	{
		UIPolygonList newPolys;
		Convexify( polys, newPolys );
		polys = newPolys;
	}

	// available index list
    Btl::Vector< S32 > list;

    // triangulate each polys
    for ( S32 i = 0; i < polys.Size(); i++ )
    {
		C_UIResultPolygon& poly = polys[ i ];

        // new result list
        result.PushBack( C_ResultIndexList() );

		if ( poly.NumPoints() < 3 )
            continue;

        // polys must be winded CW
        if ( !poly.CW() )
            poly.ReverseWindingOrder();
 
        list.Clear();

        for ( U32 j = 0; j < poly.NumPoints(); j++ )
            list.PushBack( j );
 
        bool bError = false;

        while ( list.Size() != 0 )
        {
            S32 sOldListSize = (S32)list.Size();

            if ( list.Size() == 3 )
            {
                // just add last triangle
				result[ result.Size() - 1 ].AddIndex( list[ 0 ] );
                result[ result.Size() - 1 ].AddIndex( list[ 1 ] );
                result[ result.Size() - 1 ].AddIndex( list[ 2 ] );
                list.Clear();
			}   
			else
			{
                // build current poly from the available vertex list
				C_UIResultPolygon currentPoly;
 
                for ( S32 k = 1; k < list.Size()-1; k++ )
					currentPoly.AddPoint( poly[ list[ k ] ] );

				S32 i1max = -1;
				F32 maxArea2 = 0.0f;

                // find an ear
                for ( S32 i1 = 0; i1 < list.Size(); i1++ )
                {
                    S32 i0 = ( i1 == 0 ) ? list.Size() - 1 : i1 - 1;
                    S32 i2 = ( i1 == list.Size() - 1 ) ? 0 : i1 + 1;

                    // ear test
					if ( UI::IsAnEar( poly[ list[ i0 ] ], poly[ list[ i1 ] ], poly[ list[ i2 ] ], currentPoly ) )
                    {
						i1max = i1;
						break;

					#if 0
						Maths::HF32 fArea2 = 0.0f;
						fArea2 += ( poly[ list[ i0 ] ].GetPoint() ^ poly[ list[ i1 ] ].GetPoint() );
						fArea2 += ( poly[ list[ i1 ] ].GetPoint() ^ poly[ list[ i2 ] ].GetPoint() );
						fArea2 += ( poly[ list[ i2 ] ].GetPoint() ^ poly[ list[ i0 ] ].GetPoint() );


						if ( F32( fArea2 ) > maxArea2 )
						{
							i1max = i1;
							maxArea2 = fArea2;
						}
						else if ( F32( fArea2 ) == 0.0f )
						{
							// degenerate
							list.EraseByIndex( i1 );
						}
					#endif
                    }
                }

				if ( i1max != -1 )
				{
					S32 i1 = i1max;
					S32 i0 = ( i1 == 0 ) ? list.Size() - 1 : i1 - 1;
                    S32 i2 = ( i1 == list.Size() - 1 ) ? 0 : i1 + 1;

					// add to result
					result[ result.Size() - 1 ].AddIndex( list[ i0 ] );
					result[ result.Size() - 1 ].AddIndex( list[ i1 ] );
					result[ result.Size() - 1 ].AddIndex( list[ i2 ] );

					list.EraseByIndex( i1 );
				}

                if ( sOldListSize == list.Size() )
                {
                    // we could't find an ear - bail to stop infinite looping
                    list.Clear();
                    bError = true;
                }
            }
        }

        result[ result.Size() - 1 ].Ok( !bError );
    }
}

/*
void BuildTriListVertexAndIndexBuffer( UIPolygonList& resolvedList, UIPolygonIndexList& indexList, Display::C_VertexBuffer& vertexBuffer, Display::C_IndexBuffer& indexBuffer, U32& uRenderVertexCount, U32& uRenderIndexCount, Maths::C_AxisBox* pBox )
{
	// vertex buffer
	U32 uVertexCount = 0;
	U32 uIndexCountCorrect = 0;

	for ( S32 i = 0; i < resolvedList.Size(); i++ )
	{
		uVertexCount += resolvedList[ i ].NumPoints();
		uIndexCountCorrect += ( resolvedList[ i ].NumPoints() - 2 ) * 3;
	}

	// To calc bounding area
	Maths::C_AxisBox boundingBox;

	if ( Display::Device()->CreateVertexBuffer( sizeof( C_UICharacterVertex3d ) * uVertexCount, 0, &vertexBuffer ) )
	{
		U32 uCurrentVertex = 0;

		C_UICharacterVertex3d* pVertex = (C_UICharacterVertex3d*)vertexBuffer.Lock();
		BIZ_ASSERT(pVertex);

		for ( S32 i = 0; i < resolvedList.Size(); i++ )
		{
			for ( U32 j = 0; j < resolvedList[ i ].NumPoints(); j++ )
			{
				BIZ_ASSERT(uCurrentVertex < uVertexCount);
				pVertex->SetPos( resolvedList[ i ][ j ].GetPoint3() );

				// Add to bounds
				boundingBox.UpdateExtents( Maths::C_Vertex( resolvedList[ i ][ j ].GetPoint3() ) );

				pVertex++;
				uCurrentVertex++;
			}
		}
		vertexBuffer.Unlock();
	}

	if( pBox )
		*pBox = boundingBox;

	// index buffer
	U32 uIndexCount = 0;

	for ( S32 i = 0; i < indexList.Size(); i++ )
		uIndexCount += indexList[ i ].NumIndices();

	//assert( uIndexCount == uIndexCountCorrect );

	if(uIndexCount == 0)
		return;

	if ( Display::Device()->CreateIndexBuffer( sizeof( U16 ) * uIndexCount, 0, Display::E_Format_Index16, &indexBuffer ) )
	{
		U32 uCurrentIndex = 0;

		U16* pIndex = (U16*)indexBuffer.Lock();
		BIZ_ASSERT(pIndex);

		U32 uStartOffset = 0;

		for ( S32 i = 0; i < resolvedList.Size(); i++ )
		{
			for ( S32 j = 0; j < indexList[ i ].NumIndices(); j++ )
			{
				BIZ_ASSERT(uCurrentIndex < uIndexCount);
				*pIndex++ = indexList[ i ][ j ] + uStartOffset;
				uCurrentIndex++;
			}

			uStartOffset += resolvedList[ i ].NumPoints();
		}

		// add in any extra indices that we've pre-calculated

		for ( S32 i = resolvedList.Size(); i < indexList.Size(); i++ )
		{
			for ( S32 j = 0; j < indexList[ i ].NumIndices(); j++ )
			{
				BIZ_ASSERT(uCurrentIndex < uIndexCount);
				*pIndex++ = indexList[ i ][ j ];
				uCurrentIndex++;
			}
		}

		indexBuffer.Unlock();
	}

	uRenderIndexCount = uIndexCount;
	uRenderVertexCount = uVertexCount;
}
*/

static void Convexify( UIPolygonList& in, UIPolygonList& out )
{
	for ( S32 i = 0; i < in.Size(); i++ )
	{
		C_UIResultPolygon& p = in[ i ];

		// polys must be CW
		if ( !p.CW() )
            p.ReverseWindingOrder();

		while ( !IsConvex( p ) )
		{
			if ( !Convexify( p, out ) )
				break;
		}

		out.PushBack( p );
	}
}

static bool Convexify( C_UIResultPolygon& p, UIPolygonList& out )
{
	S32 sReflexIndex = -1;

	if ( IsConvex( p, &sReflexIndex ) )
	{
		out.PushBack( p );
		return true;
	}

	// else we're concave
	if ( sReflexIndex == -1 )
	{
		LOGERROR( g_Log, "Convexify() - ( sReflexIndex == -1 ) - should never happen! - bailing" );
		return false;
	}
	if ( p.NumPoints() < 3 )
	{
		LOGERROR( g_Log, "Convexify() - ( p.NumPoints() < 3 ) - should never happen! - bailing" );
		return false;
	}
	if ( !p.CW() )
	{
		LOGERROR( g_Log, "Convexify() - ( !p.CW() ) - should never happen! - bailing" );
		return false;
	}

	while ( !IsReflexVertexValid( p, sReflexIndex ) )
	{
		S32 sNewReflexIndex = GetNextReflexVertex( p, sReflexIndex );

		if ( sNewReflexIndex == sReflexIndex )
		{
			LOGERROR( g_Log, "GetNextReflexVertex() - ( sNewReflexIndex == sReflexIndex ) - should never happen! - bailing" );
			return false;
		}

		sReflexIndex = sNewReflexIndex;
	}

	// start from the reflex vertex and build a poly till it's not longer convex
	C_UIResultPolygon r;
	Btl::Vector<S32> ignoreVertices;

	S32 sIndex = sReflexIndex;

	for ( S32 i = 0; i < 3; i++ )
	{
		r.AddPoint( p[ sIndex ] );
		ignoreVertices.PushBack( sIndex );
		sIndex = ( sIndex + 1 ) % p.NumPoints();
	}

	while ( IsConvex( r ) && IsValidPoly( p, r, ignoreVertices ) )
	{
		r.AddPoint( p[ sIndex ] );
		ignoreVertices.PushBack( sIndex );
		sIndex = ( sIndex + 1 ) % p.NumPoints();
	}

	// remove the last point as this is the one that made it concave
	r.GetPointList().PopBack();
	sIndex--;

	// add our newly made convex poly
	out.PushBack( r );

	// remove the used vertices from the current poly
	// -- don't include the first and last
	S32 sNumToErase = r.NumPoints() - 2;

	if ( sNumToErase == 0 )
		return false;	// wtf

	S32 sNumErased = 0;
	S32 sStartVertex = ( sReflexIndex + 1 ) % p.NumPoints();

	for ( ResultPointList::Iterator it = ( p.GetPointList().Begin() + sStartVertex ); sNumErased < sNumToErase; )
	{
		it = p.GetPointList().Erase( it );

		if ( it == p.GetPointList().End() )
			it = p.GetPointList().Begin();

		sNumErased++;
	}

	return true;
}

static bool IsConvex( C_UIResultPolygon& p, S32* pReflexIndex )
{
	if ( p.NumPoints() == 3 )
		return true;

	for ( U32 i = 0; i < p.NumPoints(); i++ )
	{
		if ( IsReflexVertex( p, i ) )
		{
			if ( pReflexIndex )
				*pReflexIndex = i;

			return false;
		}
	}

	if ( pReflexIndex )
		*pReflexIndex = -1;

	return true;
}

static bool IsReflexVertex( C_UIResultPolygon& p, const S32 sIndex )
{
	S32 v1 = ( ( sIndex - 1 ) < 0 ) ? ( p.NumPoints() - 1 ) : ( sIndex - 1 );
	S32 v2 = sIndex;
	S32 v3 = ( ( sIndex + 1 ) == p.NumPoints() ) ? 0 : ( sIndex + 1 );

	Maths::HVec2 v2v1( p[ v2 ].GetPoint() - p[ v1 ].GetPoint() );
	Maths::HVec2 v3v2( p[ v3 ].GetPoint() - p[ v2 ].GetPoint() );

	F32 z = ( ( v2v1.X() * v3v2.Y() ) - ( v2v1.Y() * v3v2.X() ) );

	return ( z < 0.0f );
}

static bool IsReflexVertexValid( C_UIResultPolygon& p, const S32 sIndex )
{
	C_UIResultPolygon r;

	r.AddPoint( p[ sIndex ] );
	r.AddPoint( p[ ( sIndex + 1 ) % p.NumPoints() ] );
	r.AddPoint( p[ ( sIndex + 2 ) % p.NumPoints() ] );

	Btl::Vector<S32> ignoreVertices;
	ignoreVertices.PushBack( sIndex );
	ignoreVertices.PushBack( ( sIndex + 1 ) % p.NumPoints() );
	ignoreVertices.PushBack( ( sIndex + 2 ) % p.NumPoints() );

	bool bCW = r.CW();
	bool bValid = IsValidPoly( p, r, ignoreVertices );

	return ( bCW && bValid );
}

static S32 GetNextReflexVertex( C_UIResultPolygon& p, const S32 sCurrentIndex )
{
	S32 sNewIndex = -1;

	for ( U32 i = ( sCurrentIndex + 1 ); i < p.NumPoints(); i++ )
	{
		bool bReflex = IsReflexVertex( p, i );

		if ( bReflex )
		{
			bool bReflexValid = IsReflexVertexValid( p, i );

			if ( bReflexValid )
			{
				sNewIndex = i;
				break;
			}
		}
	}

	if ( sNewIndex == -1 )
		return sCurrentIndex;

	return sNewIndex;
}

static bool IsValidPoly( C_UIResultPolygon& p, C_UIResultPolygon& r, Btl::Vector<S32>& ignoreVertices )
{
	for ( U32 i = 0; i < p.NumPoints(); i++ )
	{
		bool bIgnore = false;

		for ( S32 j = 0; j < ignoreVertices.Size(); j++ )
		{
			if ( ignoreVertices[ j ] == i )
			{
				bIgnore = true;
				break;
			}
		}

		if ( bIgnore )
			continue;

		if ( IsVertexInsidePoly( p[ i ].GetPoint(), r ) )
			return false;
	}

	return true;
}

static bool IsAnEar( C_UIResultPoint& t1, C_UIResultPoint& t2, C_UIResultPoint& t3, C_UIResultPolygon& poly )
{
	if ( !UI::TriCW( t1, t2, t3 ) )
		return false;

	for ( U32 i = 0; i < poly.NumPoints(); i++ )
	{
		if ( UI::PointInTriangle( poly[ i ], t1, t2, t3 ) )
        {
            if ( ( poly[ i ] != t1 ) && ( poly[ i ] != t2 ) && ( poly[ i ] != t3 ) )
                return false;
        }
	}

	return true;
}

__forceinline static bool TriCW( C_UIResultPoint& t1, C_UIResultPoint& t2, C_UIResultPoint& t3 )
{
	F32 r( ( t1.GetPoint() ^ t2.GetPoint() ) + ( t2.GetPoint() ^ t3.GetPoint() ) + ( t3.GetPoint() ^ t1.GetPoint() ) );
	return ( ( r < 0.0f && r > -0.01f ) || ( r >= 0.0f ) );
}

__forceinline static bool PointInTriangle( C_UIResultPoint& pPoint, C_UIResultPoint& t1, C_UIResultPoint& t2, C_UIResultPoint& t3 )
{
	return ( UI::TriCW( t1, t2, pPoint ) && UI::TriCW( t2, t3, pPoint ) && UI::TriCW( t3, t1, pPoint ) );
}

};	// namespace UI
