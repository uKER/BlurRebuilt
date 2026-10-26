#ifndef UIHOLERESOLVER_H
#define UIHOLERESOLVER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Graphics/Polygons/UIPolygonClasses.h"

#include <btl/Containers/containers.h>

namespace UI
{

/// @addtogroup HoleResolver
/// @{

/// Determine which polygons are "holes" in any of the other ones - then resolves them.
/// @param polygonList - the input list to solve.
/// @param resolvedList - a blank list filled by the algorithm.
/// @remarks - Assumes you don't have holes within holes - this will fail. @n - Bails on infinite loop detection.
void						FindAndResolveHoles( UIPolygonList& polygonList, UIPolygonList& resolvedList );

/// Resolve a single hole once you know about it.
/// @return - the new merged polygon.
C_UIResultPolygon			ResolveHole( C_UIResultPolygon& hole, C_UIResultPolygon& poly );

/// Resolve a single hole once you know about it.
/// @return - the new merged polygon.
/// @remarks - the return polygon is new-ed. <b>Remember to delete it!</b>
C_UIClipPolygon*			ResolveHole( C_UIClipPolygon& hole, C_UIClipPolygon& poly );

/// Resolve multiple holes once you know about them.
/// @return - the new merged polygon.
C_UIResultPolygon			ResolveMultipleHoles( C_UIResultPolygon& outside, Btl::Vector< C_UIResultPolygon >& holes );

/// @}

static bool					FindHoleJoinEdge( C_UIResultPolygon& hole, C_UIResultPolygon& poly, C_UIResultPoint& holeV, C_UIResultPoint& polyV );
static bool					FindHoleJoinEdge( C_UIResultPolygon& hole, C_UIResultPolygon& poly, C_UIResultPoint& holeV, C_UIResultPoint& polyV, Btl::Vector< C_UIResultPoint >& dupeList, Btl::Vector< C_UIResultPolygon >& holeList, const S32 sHoleIndex = 0 );
static C_UIResultPolygon	BuildJoinedPoly( C_UIResultPolygon& hole, C_UIResultPolygon& poly, C_UIResultPoint& holeV, C_UIResultPoint& polyV );

};	// namespace UI

#endif	/* UIHOLERESOLVER_H */
