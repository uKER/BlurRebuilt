#include "stdafx.h"

// this define knocks out the setup side of the shapepolicy - ie stuff done on main thread/ppu etc
#include "Dynamic/Particle/Shape/Shape2D.h"

#include "Dynamic/Particle/Build/JobQueueParams.h"
#include "Dynamic/Particle/Build/VertexBuild.h"
#include "Dynamic/TextureCoords.h"

#ifdef PLATFORM_SPU
namespace Maths
{ 
	const vector float Const< vector float >::m_UnitX = (vector float)( 1.0f, 0.0f, 0.0f, 0.0f );
	const vector float Const< vector float >::m_UnitY = (vector float)( 0.0f, 1.0f, 0.0f, 0.0f );
	const vector float Const< vector float >::m_UnitZ = (vector float)( 0.0f, 0.0f, 1.0f, 0.0f );
	const vector float Const< vector float >::m_UnitW = (vector float)( 0.0f, 0.0f, 0.0f, 1.0f );

	const vector unsigned int Const< vector unsigned int >::m_AbsMask  = (vector unsigned int)( 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF );
	const vector unsigned int Const< vector unsigned int >::m_SignMask = (vector unsigned int)( 0x80000000, 0x80000000, 0x80000000, 0x80000000 );
}
#endif

JQ_DECLARE_SPU_MODULE(ParticleVertexBuildShape2DModule, DynamicEffect::Particle::BuildPolicy::VertexBuildTask<DynamicEffect::Particle::ShapePolicy::Shape2D>, dynamic_particle_vertexbuild_shape2D, "dynamic.scheduler");












