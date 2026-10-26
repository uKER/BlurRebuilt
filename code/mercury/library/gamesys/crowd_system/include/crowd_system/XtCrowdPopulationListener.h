#ifndef XT_CROWD_POPULATION_LISTENER_H
#define XT_CROWD_POPULATION_LISTENER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/list.h>

struct XtRecord;

namespace AnimatedCrowd
{

class C_XtCrowdPopulation;
class C_XtCrowdCharacter;
class C_XtCrowdCycleGroup;
class C_XtCrowdCycleGroupItem;
class C_XtCrowdEffectSystem;
class C_XtCrowdAnimationEventEffect;

class C_XtCrowdPopulationListener: public Btl::ListElementDouble<C_XtCrowdPopulationListener>
{
public:
	C_XtCrowdPopulationListener();
	virtual ~C_XtCrowdPopulationListener();

	virtual void OnXtCrowdPopulationCreated(XtRecord* pRecord, const C_XtCrowdPopulation* pPopulation);
	virtual void OnXtCrowdPopulationDestroyed(XtRecord* pRecord, const C_XtCrowdPopulation* pPopulation);

	virtual void OnXtCrowdCharacterCreated(XtRecord* pRecord, const C_XtCrowdCharacter* pCharacter);
	virtual void OnXtCrowdCharacterDestroyed(XtRecord* pRecord, const C_XtCrowdCharacter* pCharacter);

	virtual void OnXtCrowdCycleGroupCreated(XtRecord* pRecord, const C_XtCrowdCycleGroup* pCycleGroup);
	virtual void OnXtCrowdCycleGroupDestroyed(XtRecord* pRecord, const C_XtCrowdCycleGroup* pCycleGroup);

	virtual void OnXtCrowdCycleGroupItemCreated(XtRecord* pRecord, const C_XtCrowdCycleGroupItem* pCycleGroupItem);
	virtual void OnXtCrowdCycleGroupItemDestroyed(XtRecord* pRecord, const C_XtCrowdCycleGroupItem* pCycleGroupItem);

	virtual void OnXtCrowdEffectSystemCreated(XtRecord* pRecord, const C_XtCrowdEffectSystem* pEffectSystem);
	virtual void OnXtCrowdEffectSystemDestroyed(XtRecord* pRecord, const C_XtCrowdEffectSystem* pEffectSystem);

	virtual void OnXtCrowdAnimationEventEffectCreated(XtRecord* pRecord, const C_XtCrowdAnimationEventEffect* pAnimationEventEffect);
	virtual void OnXtCrowdAnimationEventEffectDestroyed(XtRecord* pRecord, const C_XtCrowdAnimationEventEffect* pAnimationEventEffect);
};

}; // namespace AnimatedCrowd

#endif // XT_CROWD_POPULATION_LISTENER_H
