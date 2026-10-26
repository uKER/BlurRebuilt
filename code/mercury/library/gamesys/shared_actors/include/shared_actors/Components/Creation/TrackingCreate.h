#ifndef TRACKINGCREATE_3DC1E9FC_3CC8_4803_8790_1B7408CDB3A5_H
#define TRACKINGCREATE_3DC1E9FC_3CC8_4803_8790_1B7408CDB3A5_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/GameComponent.h>
#include <shared_actors/Components/TrackingComponents.h>


namespace Components
{

class C_SpecificActorTrackingCreate : public Component::I_ComponentCreator
{
public:
	C_SpecificActorTrackingCreate();
	virtual ~C_SpecificActorTrackingCreate();

	static Component::C_ComponentTypeId ClassType() {return E_Tracking_SpecificActor;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_Tracking_SpecificActor;} // from Component::I_ComponentCreator

};

} // namespace Components


#endif // TRACKINGCREATE_3DC1E9FC_3CC8_4803_8790_1B7408CDB3A5_H
