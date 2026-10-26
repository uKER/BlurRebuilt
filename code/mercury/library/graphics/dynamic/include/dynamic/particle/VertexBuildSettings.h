#ifndef RENDER_DYNAMICEFFECT_PARTICLE_VERTEXBUILDSETTINGS_H
#define RENDER_DYNAMICEFFECT_PARTICLE_VERTEXBUILDSETTINGS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Enums.h"
#include "Dynamic/Enums.h"
#include "Dynamic/Common.h"

namespace DynamicEffect
{
namespace Particle
{

class C_UserShape3DVariants;

// these are required for the vertex build jobs
struct ALIGN16 VertexBuildSettings
{
	E_VertexBuild					eVertexBuild;
	E_ParticleShape					eParticleShape;				// particle shape type
	E_ParticleSort					eParticleSort;				// particle sorting type
	LookupTable						sizeTable;
	LookupTable						colourTable;
	E_ParticleLimitSize				eLimitSize;
	MathsStorage::Float4A16			limitSize;
	MathsStorage::Float4A16			pivot;
	E_AtlasType						eAtlasType;
	F32								fAtlasAnimPeriod;
	U32								iAtlasSize;					// the number of rows/columns of sprites in texture
	U32								iAtlasOffset;
	U32								iElementSize;
	S32								iParticleLibraryIndex;		// for mesh particles
	const C_UserShape3DVariants*	pUserShape3DVariants;
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EFFECTINTERFACE_H
