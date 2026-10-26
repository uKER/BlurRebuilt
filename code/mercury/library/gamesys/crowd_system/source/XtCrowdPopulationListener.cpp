#include "CrowdSystemPch.h"

#include <crowd_system/XtCrowdPopulationListener.h>
#include <crowd_system/XtCrowd.h>

namespace AnimatedCrowd
{

C_XtCrowdPopulationListener::C_XtCrowdPopulationListener()
{
	C_XtCrowdPopulation::AddListener(this);
}

C_XtCrowdPopulationListener::~C_XtCrowdPopulationListener()
{
	C_XtCrowdPopulation::RemoveListener(this);
}

void C_XtCrowdPopulationListener::OnXtCrowdPopulationCreated(XtRecord* pRecord, const C_XtCrowdPopulation* pPopulation)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdPopulationDestroyed(XtRecord* pRecord, const C_XtCrowdPopulation* pPopulation)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdCharacterCreated(XtRecord* pRecord, const C_XtCrowdCharacter* pCharacter)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdCharacterDestroyed(XtRecord* pRecord, const C_XtCrowdCharacter* pCharacter)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdCycleGroupCreated(XtRecord* pRecord, const C_XtCrowdCycleGroup* pCycleGroup)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdCycleGroupDestroyed(XtRecord* pRecord, const C_XtCrowdCycleGroup* pCycleGroup)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdCycleGroupItemCreated(XtRecord* pRecord, const C_XtCrowdCycleGroupItem* pCycleGroupItem)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdCycleGroupItemDestroyed(XtRecord* pRecord, const C_XtCrowdCycleGroupItem* pCycleGroupItem)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdEffectSystemCreated(XtRecord* pRecord, const C_XtCrowdEffectSystem* pEffectSystem)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdEffectSystemDestroyed(XtRecord* pRecord, const C_XtCrowdEffectSystem* pEffectSystem)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdAnimationEventEffectCreated(XtRecord* pRecord, const C_XtCrowdAnimationEventEffect* pAnimationEventEffect)
{
}

void C_XtCrowdPopulationListener::OnXtCrowdAnimationEventEffectDestroyed(XtRecord* pRecord, const C_XtCrowdAnimationEventEffect* pAnimationEventEffect)
{
}

}; // namespace AnimatedCrowd
