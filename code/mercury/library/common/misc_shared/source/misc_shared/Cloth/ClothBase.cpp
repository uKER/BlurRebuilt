#include "MiscSharedPch.h"
#include "misc_shared/Cloth/Cloth.h"

#include <misc_shared/Noise/perlin.h>

namespace Cloth {
//-------------------------------------------------------------------------------


Debug::Logger & g_Log = Debug::GetLogger("mercury.misc_shared.cloth");


Debug::Bool g_DebugDebugRender("Gravity.NotGravity.Cloth.DebugRender.Enable", false);
Debug::Float g_DebugDebugRenderNormalLength("Gravity.NotGravity.Cloth.DebugRender.NormalLength", 0.5f, Debug::Range(0.0f, 1.0f));
Debug::Float g_DebugDebugRenderTangentLength("Gravity.NotGravity.Cloth.DebugRender.TangentLength", 0.5f, Debug::Range(0.0f, 1.0f));
Debug::Float g_DebugDebugRenderOrientationMatrixSize("Gravity.NotGravity.Cloth.DebugRender.OrientationMatrixSize", 0.5f,
	Debug::Range(0.0f, 1.0f));

#ifdef CLOTH_USE_SCALING_FACTORS
Debug::Float g_DebugMaxMinorTimeStepScalingFactor("Gravity.NotGravity.Cloth.MaxMinorTimeStepScalingFactor",
	1.0f, Debug::Range(0.001f, 0.1f));
	
Debug::Float g_DebugFrictionCoeffScalingFactor("Gravity.NotGravity.Cloth.FrictionCoeffScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugWindVelocityScalingFactor("Gravity.NotGravity.Cloth.WindVelocityScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugWindResistanceCoeffScalingFactor("Gravity.NotGravity.Cloth.WindResistanceCoeffScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugBendDampCoeffScalingFactor("Gravity.NotGravity.Cloth.BendDampCoeffScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugBendSpringConstScalingFactor("Gravity.NotGravity.Cloth.BendSpringConstScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugShearDampCoeffScalingFactor("Gravity.NotGravity.Cloth.ShearDampCoeffScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugShearSpringConstScalingFactor("Gravity.NotGravity.Cloth.ShearSpringConstScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugStretchDampCoeffScalingFactor("Gravity.NotGravity.Cloth.StretchDampCoeffScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugStretchSpringConstScalingFactor("Gravity.NotGravity.Cloth.StretchSpringConstScalingFactor",
	1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugParticleMassScalingFactor("Gravity.NotGravity.Cloth.ParticleMassScalingFactor",
	1.0f, Debug::Range(0.1f, 5.0f));
#endif

Debug::Float g_DebugGravitationalFieldStrength("Gravity.NotGravity.Cloth.GravitationalFieldStrength", 9.8f, Debug::Range(0.0f, 9.8f));


//-------------------------------------


void C_WindGenerator::Init(const C_WindInitParams & initParams)
{
	m_direction = initParams.direction;
	m_averageSpeed = initParams.averageSpeed;
	m_maxSpeedDeviation = initParams.maxSpeedDeviation;
	m_noiseTimeScale = initParams.noiseTimeScale;
}


void C_WindGenerator::Update(const hf & dt)
{
	C_PerlinNoiseDefault noiseGenerator;

	hf speedDeviation = m_maxSpeedDeviation * noiseGenerator.SoftNoise1D( FP32(m_time / m_noiseTimeScale) );
	m_velocity = (m_averageSpeed + speedDeviation) * m_direction;

	m_time += dt;
}


//-------------------------------------------------------------------------------
};
