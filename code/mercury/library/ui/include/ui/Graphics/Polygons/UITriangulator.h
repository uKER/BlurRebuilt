#ifndef UITRIANGULATOR_H
#define UITRIANGULATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Graphics/Polygons/UIPolygonClasses.h"

namespace UI
{

/// @addtogroup Triangulator
/// @{

/// Triangulate a list of polygon outlines.
/// @param polys - the list of polygons to triangulate.
/// @param result - a blank index list that is filled by the operation. Indices correspond to the polygons in the @e polys list.
/// @param bDoConvexify - split into convex hulls first
/// @remarks - assumes polygon have no self-intersections. @n - bails on failure to stop infinite looping.
void						Triangulate( UIPolygonList& polys, UIPolygonIndexList& result, const bool bDoConvexify = false );

/// Merge the results of the Triangulator into one vertex and index buffer.
/// @param uRenderIndexCount - is modified to contain the final index count.
/// @param pBox - optional : build AABB for this polygon
//void						BuildTriListVertexAndIndexBuffer( UIPolygonList& resolvedList, UIPolygonIndexList& indexList, Display::C_VertexBuffer& vertexBuffer, Display::C_IndexBuffer& indexBuffer, U32& uRenderVertexCount, U32& uRenderIndexCount, Maths::C_AxisBox* pBox = NULL );

				static void Convexify( UIPolygonList& in, UIPolygonList& out );
				static bool Convexify( C_UIResultPolygon& p, UIPolygonList& out );
				static bool IsConvex( C_UIResultPolygon& p, S32* pReflexIndex = NULL );
				static bool IsReflexVertex( C_UIResultPolygon& p, const S32 sIndex );
				static bool IsReflexVertexValid( C_UIResultPolygon& p, const S32 sIndex );
				static S32  GetNextReflexVertex( C_UIResultPolygon& p, const S32 sCurrentIndex );
				static bool IsValidPoly( C_UIResultPolygon& p, C_UIResultPolygon& r, Btl::Vector<S32>& ignoreVertices );

				static bool	IsAnEar( C_UIResultPoint& t1, C_UIResultPoint& t2, C_UIResultPoint& t3, C_UIResultPolygon& poly );
__forceinline	static bool	TriCW( C_UIResultPoint& t1, C_UIResultPoint& t2, C_UIResultPoint& t3 );
__forceinline	static bool	PointInTriangle( C_UIResultPoint& pPoint, C_UIResultPoint& t1, C_UIResultPoint& t2, C_UIResultPoint& t3 );

/// @}

};	// namespace UI

#endif	/* UITRIANGULATOR_H */
