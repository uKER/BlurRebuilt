#include "stdafx.h"

// this define knocks out the setup side of the shapepolicy - ie stuff done on main thread/ppu etc
#include "Dynamic/Particle/Shape/All.h"

#include "Dynamic/Particle/Build/JobQueueParams.h"
#include "Dynamic/Particle/Build/VertexBuild.h"
#include "Dynamic/TextureCoords.h"

JQ_DECLARE_PPU_MODULE(ParticleVertexBuildModule, DynamicEffect::Particle::BuildPolicy::VertexBuildTask<DynamicEffect::Particle::ShapePolicy::All>);












