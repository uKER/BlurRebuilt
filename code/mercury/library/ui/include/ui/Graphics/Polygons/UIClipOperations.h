#ifndef UICLIPOPERATIONS_H
#define UICLIPOPERATIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Graphics/Polygons/UIPolygonClasses.h"

#include <btl/Containers/containers.h>

namespace UI
{

/// @addtogroup ClipOperation
/// @{

/// Boolean 2D polygon operations.
/// Supports - <ul> <li>Intersection</li> <li>Union</li> <li>Set Difference ( A-B and B-A )</li>  <li>Exclude</li> </ul>
/// @param result - a blank list filled by the algorithm. These are new-ed, <b>so remember to delete them!</b>
	void			ClipOperation( C_UIClipPolygon& subject, C_UIClipPolygon& clip, Btl::Vector< C_UIClipPolygon* >& result, const E_UIPolyClip eClipTest );

/// @}

static bool		Stage1( C_UIClipPolygon& subject, C_UIClipPolygon& clip );
static void		Stage2( C_UIClipPolygon& subject, C_UIClipPolygon& clip, const E_UIPolyClip eClipTest );
static void		Stage3( C_UIClipPolygon& subject, C_UIClipPolygon& clip, Btl::Vector< C_UIClipPolygon* >& result );

static void		HandleDisjointPolys( C_UIClipPolygon& p1, C_UIClipPolygon& p2, Btl::Vector< C_UIClipPolygon* >& result, const E_UIPolyClip eTest );
static void		AddToResult( C_UIClipPolygon& p, Btl::Vector< C_UIClipPolygon* >& result );

};	// namespace UI

#endif	/* UICLIPOPERATIONS_H */
