#include "stdafx.h"

// [Jan] removing FORCE_INLINE changes the compile time of this file from 33.67 secs to 19.06 secs
#undef FORCE_INLINE
#define FORCE_INLINE inline

// need this for first include of UpdateXX.h
#include "Dynamic/Defines.h"

// C_Particle3D_CP, ShapePolicy::Shape2D
#include "Shape2DCollision.cpp"
#include "Shape2DCollisionSuper.cpp"
// C_Particle3D, ShapePolicy::Shape2D
#include "Shape2DSimple.cpp"
#include "Shape2DSimpleSuper.cpp"


