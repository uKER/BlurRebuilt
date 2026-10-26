#ifndef SPATIAL_DIVIDER_HEADERS_H
#define SPATIAL_DIVIDER_HEADERS_H

/*! \mainpage Spatial Divider raytracing library
*
* \section intro_sec Introduction
*
* The Spatial Divider library is a tool for raytracing against an Aftk mesh. Features:
* - Fast ray-tracing (over 1 million rays/sec on 200,000 polygons)
* - Fast collision tree building
* - Ability to trace 4 shadow rays simultaneously using SSE
* - Switching layers on and off without rebuilding the collision tree
* - Instanced geometry support
* - Optional alpha texture support
*
* \section install_sec Usage
*
* Using this library requires including the Aftk in your project. Once the Aftk include and
* library paths are set up, the Spatial Divider library should build without any further changes.
* To use the library in your own project, include the "SpatialDividerHeaders.h" file. Below
is a typical usage example.

<code><pre>

##include "spatial_hierarchy/SpatialDividerHeaders.h"

int _tmain(int argc, char* argv[])
{
	Aftk aftk;

	SD::SpatialHierarchy tree;
	
	tree.Build( &aftk );

	SD::Ray ray(Float3(0,0,0), Float3(1,0,0));
	SD::RayTraceResult result;

	tree.RayTrace(ray, result);

	return 0;
}

</code></pre>
*/

//#include "spatial_hierarchy/stdafx.h"

#ifdef GCC
#define DONT_USE_SIMD
#endif

#include <spatial_hierarchy/typedefs.h>

#include <Aftk/Aftk.h>
#include <core/containers/vector.h>
#include <core/containers/bitvector.h>

#include <Core\Maths\Storage.h>
#include <spatial_hierarchy/StorageMaths.h>

#include "Common.h"
#include "RayTraceResult.h"
#include "Ray.h"
#include "spatial_hierarchy/Tree-Common.h"
#include "spatial_hierarchy/Kd-TreeBuilder.h"
#include "spatial_hierarchy/SerialiseUtVectorToOst.h"
#include "AlphaTextureInfo.h"
#include "AlphaTextureManager.h"
#include "SpatialHierarchy.h"

#endif
