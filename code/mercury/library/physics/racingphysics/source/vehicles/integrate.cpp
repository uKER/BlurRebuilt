#include "stdafx.h"
#include "System\Scheduler.h"
#include "racingphysics\vehicles\vehicle.h"
#include "racingphysics\vehicles\car\car.h"
#include "racingphysics\physics\physicstimer.h"

#ifdef PLATFORM_SPU
#include "core/maths/hvec/VectorConstGlobals.inl"
#include "../../../../common/core/source/maths/hvec/Trigonometry.cpp.inl"
#include "../../../../common/core/source/maths/hvec/Constants.cpp.inl"
#endif

extern Scheduler::jqModule IntegrationModule;
extern Scheduler::jqModule NetworkUpdateModule;

namespace RacingPhysics
{

#if !PLATFORM_SPU 

Scheduler::GroupHandle IntegrationCallback::s_networkUpdateGroup = Scheduler::JQ_GROUP_NONE;
Scheduler::GroupHandle IntegrationCallback::s_integrationGroup = Scheduler::JQ_GROUP_NONE;

Scheduler::GroupHandle IntegrationCallback::BeginIntegrationTask(Scheduler::GroupHandle group, Scheduler::PolicyHandle policy, float time)
{
	PERF_EVENT("IntegrationCallback::BeginIntegrationTask");

	if (s_networkUpdateGroup == Scheduler::JQ_GROUP_NONE)
	{
		s_networkUpdateGroup = Scheduler::CreateGroup();
		s_integrationGroup = Scheduler::CreateGroup();
		Scheduler::AddGroupDependency(s_integrationGroup, s_networkUpdateGroup);
	}

	m_pVehicle->BeginFrame(0.f);

	IntegrationCommand command;
	command.pVehicle = m_pVehicle;
	command.time = time;

#if PLATFORM_PS3
	m_pVehicle->GetDmaData(&command.vehicleDmaData);
	Surfaces::GetDmaData(&command.surfaceDmaData);
	PhysicsTimer::Instance().GetDmaData(&command.physicsTimer);
#endif

	if (m_pVehicle->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK)
	{
		Scheduler::AddTask(NetworkUpdateModule, NULL, NULL, s_networkUpdateGroup, policy, &command, sizeof(IntegrationCommand));
	}

	Scheduler::AddTask(IntegrationModule, NULL, NULL, s_integrationGroup, policy, &command, sizeof(IntegrationCommand));	

	return s_integrationGroup;
}

void IntegrationCallback::PostIntegration(const Maths::C_TimeS PCREF time)
{
	PERF_EVENT("IntegrationCallback::PostIntegration");
	m_pVehicle->EndFrame(time);
}

#endif

}

namespace RacingPhysics_Integration
{

RacingPhysics::Vehicle* InitialiseVehicle(RacingPhysics::Vehicle* pVehicle, void* dmaData)
{
#if !PLATFORM_SPU
	return pVehicle;
#else
	static RacingPhysics::Car car;
	car.SpuFetch(dmaData);
	return &car;
#endif
}

void FinaliseVehicle(const RacingPhysics::Vehicle* pLocal, void* dmaData)
{ 
#ifdef PLATFORM_SPU
	pLocal->SpuStore(dmaData);
#endif
}

void IntegrationTask(Scheduler::Task* pTask)
{
	PERF_EVENT("RacingPhysics::IntegrationTask");

	//DebugBreak();

	RacingPhysics::IntegrationCommand* pCommand = (RacingPhysics::IntegrationCommand*)&pTask->ParamData[0];

	RacingPhysics::Vehicle* pVehicle = InitialiseVehicle(pCommand->pVehicle, pCommand->vehicleDmaData);

	RacingPhysics::Surfaces::SpuFetch(pCommand->surfaceDmaData);
	RacingPhysics::PhysicsTimer::Instance().SpuFetch(pCommand->physicsTimer);

	pVehicle->CreateGravityCache();
	pVehicle->Integrate(pCommand->time);

	FinaliseVehicle(pVehicle, pCommand->vehicleDmaData);
}

}

JQ_DECLARE_SPU_MODULE(IntegrationModule, RacingPhysics_Integration::IntegrationTask, IntegrateSpu, "RacingPhysics.Integrate");
//JQ_DECLARE_PPU_MODULE(IntegrationModule, RacingPhysics_Integration::IntegrationTask);

#if PLATFORM_SPU

extern "C" void _exit() {snPause();}

#endif
