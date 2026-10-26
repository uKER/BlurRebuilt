#include "stdafx.h"
#include "DynamicsContext.h"
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/skeleton/WindGenerator.h>
#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>
#include <system/debug/I_DebugRender.h>

C_DynamicsContext::C_DynamicsContext()
{
	SetDefaults();
}

void C_DynamicsContext::SetDefaults()
{
	SetGravity(Maths::MakeVector3(0.f, -9.81f, 0.f));

	SetWindEnabled(false);
	SetWindDirection(Maths::MakeVector3(-1.f, 0.f, 1.f));
	SetWindDirectionVariation(0.4f);
	SetWindStrength(100.f);
	SetWindStrengthVariation(0.2f);
	SetWindVariationRate(10.f);
	
	SetBuoyancyEnabled(false);
	SetWaterPlaneHeight(0.f);
	
	SetDynamicJointsEnabled(true);
	ShowWindVector(true);
	
	Reset();
}

void C_DynamicsContext::Reset()
{
	m_reset = true;
}

void C_DynamicsContext::Update(Animation::C_SkeletonInstance& skeletonInstance, const Maths::Mat43& modelWorldMatrix, F32 deltaTime)
{
	skeletonInstance.SetGravity(GetGravity());
	skeletonInstance.SetDynamicJointsEnabled(AreDynamicJointsEnabled());
	
	if (m_reset)
	{
		skeletonInstance.Reset();
		m_reset = false;
	}
	
	if (m_windEnabled)
	{
		skeletonInstance.ApplyWindForces(m_windParams, deltaTime);
		
		if (ShowWindVector())
		{
			Animation::C_WindGenerator windGen;
			windGen.SetTime(skeletonInstance.GetWindTime());
			windGen.Update(m_windParams, 0.f);
			
			Maths::Vector3 windForce = windGen.GenerateWindForce(0);
			DbUtil::DefaultDebugRenderSys()->Deferred()->SetDepthEnable(false);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(
				Maths::HVec3(Maths::RowW(modelWorldMatrix)),
				Maths::HVec3(Maths::MakePoint3(windForce)),
				0.4f * Maths::Reciprocal(m_windParams.GetWindStrength()),
				0.05f,
				Maths::RGBA(0xff, 0xff, 0x00, 0xff)
			);
			DbUtil::DefaultDebugRenderSys()->Deferred()->SetDepthEnable(true);
		}
		
	}
	skeletonInstance.UpdateDynamicJoints(deltaTime);
}

