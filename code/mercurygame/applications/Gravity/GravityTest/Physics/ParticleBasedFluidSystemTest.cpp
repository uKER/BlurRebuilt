#include <stdafx.h>

#include "SectionPhysics.h"
#include "gravity/cloths/ParticleBasedFluid.h"

namespace Samples
{
	Debug::Float g_dbFluidSpringCoefficient("Test.ParticleBasedFluid.SpringCoefficient", 30.f, Debug::Range(0.f, 100.f));
	Debug::Float g_dbFluidStiffnessConstant("Test.ParticleBasedFluid.StiffnessConstants", 10.f, Debug::Range(0.f, 100.f));
	Debug::Float g_dbFluidStiffnessNearConstant("Test.ParticleBasedFluid.StiffnessNearConstant", 5.f, Debug::Range(0.f, 100.f));
	Debug::Float g_dbFluidRestDensity("Test.ParticleBasedFluid.RestDensity", 0.5f, Debug::Range(0.f, 100.f));
	Debug::Float g_dbFluidPlasticityConstant("Test.ParticleBasedFluid.PlasticityConstant", 0.5f, Debug::Range(0.f, 1.f));
	Debug::Float g_dbFluidYieldRatio("Test.ParticleBasedFluid.YieldRatio", 0.1f, Debug::Range(0.f, 1.f));
	Debug::Float g_dbFluidParticleRadius("Test.ParticleBasedFluid.ParticleRadius", 0.1f, Debug::Range(0.f, 1.f));
	Debug::Float g_dbFluidLinearViscosityConstant("Test.ParticleBasedFluid.LinearViscosityConstant", 2.f, Debug::Range(1.f, 10.f));
	Debug::Float g_dbFluidQuatraticViscosityConstant("Test.ParticleBasedFluid.QuatraticViscosityConstant", 2.f, Debug::Range(1.f, 10.f));

	static F32 Frand()
	{
		return (F32)(rand()&1023) / 512.f - 1.f;
	}

	void C_TestPhysics::InitialiseParticleBasedFluidSystem()
	{
		using namespace Maths;
		if(m_FluidSystem.m_Particles == NULL)
			m_FluidSystem.SetNumParticles(500);
	
		m_FluidSystem.SetExternalConstants(&m_ExternalConstants);
		m_FluidSystem.SetParticleRadius(HF32(g_dbFluidParticleRadius));
		m_FluidSystem.SetStiffness(HF32(g_dbFluidStiffnessConstant));
		m_FluidSystem.SetStiffnessNear(HF32(g_dbFluidStiffnessNearConstant));
		m_FluidSystem.SetLinearViscosityConstants(HF32(g_dbFluidLinearViscosityConstant));
		m_FluidSystem.SetQuadraticViscosityConstants(HF32(g_dbFluidQuatraticViscosityConstant));
		m_FluidSystem.SetSpringCoefficient(HF32(g_dbFluidSpringCoefficient));

		const HF32 x(14.f);
		const HF32 y(6.f);
		const HF32 z(1.f);
	
		//Initialise the points;
		for(U32 i=0; i<m_FluidSystem.m_NumParticles; ++i)
		{
			Collision::C_FluidParticle& particle = m_FluidSystem.m_Particles[i];
			particle.m_CurPosition = HVec3(Frand(), Frand(), Frand());
			particle.m_Index = i;
		}

		m_FluidSystem.Initialise();

		m_FluidSystem.InitialiseNeighbours();
	}

	void C_TestPhysics::DrawParticleBaseFluidSystem()
	{
		using namespace Maths;
		for(U32 i=0; i< m_FluidSystem.m_NumParticles; ++i)
		{
			Collision::C_FluidParticle& particle = (Collision::C_FluidParticle&)m_FluidSystem.m_Particles[i];
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(particle.m_CurPosition, 0.01f, RGBA(0, 255, 0));
		}
	}


}