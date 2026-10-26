#include "stdafx.h"

// [Jan] removing FORCE_INLINE changes the compile time of this file from 33.67 secs to 19.06 secs
#undef FORCE_INLINE
#define FORCE_INLINE inline

// need this for first include of UpdateXX.h
#include "Dynamic/Defines.h"

// C_Particle3D_RV3D, ShapePolicy::Decal
#include "Shape3DDecal.cpp"

// C_Particle3D_R1D, ShapePolicy::SpriteRotating
#include "SpriteRotating.cpp"

// C_Particle3D_R1D_CP, ShapePolicy::SpriteRotating
#include "SpriteRotatingCollision.cpp"

// C_Particle3D_R1D, ShapePolicy::SpriteRotatingSuper
#include "SpriteRotatingSuper.cpp"

// C_Particle3D_P, ShapePolicy::Shape2D
#include "Shape2DSpline.cpp"

// C_Particle3D_R1D_P, ShapePolicy::SpriteRotating
#include "SpriteRotatingSpline.cpp"

