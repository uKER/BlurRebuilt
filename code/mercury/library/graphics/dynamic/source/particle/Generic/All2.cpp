#include "stdafx.h"

// [Jan] removing FORCE_INLINE changes the compile time of this file from 33.67 secs to 19.06 secs
#undef FORCE_INLINE
#define FORCE_INLINE inline

// need this for first include of UpdateXX.h
#include "Dynamic/Defines.h"

#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>

#include "Dynamic/Particle/Tools_Shared/UserShape3DData.h"
#include "Dynamic/Particle/Tools_Shared/UserShape3DVariants.h"
#include "Dynamic/Particle/Tools_Shared/UserParticleContainer.h"
#include "Dynamic/Particle/UserParticleLibrary.h"

// C_Particle3D_RV3D, ShapePolicy::Shape3D
#include "Shape3DSimple.cpp"
#include "Shape3DSimpleSuper.cpp"

// C_Particle3D_RV3D_CP, ShapePolicy::Shape3D
#include "Shape3DCollision.cpp"
#include "Shape3DCollisionSuper.cpp"

