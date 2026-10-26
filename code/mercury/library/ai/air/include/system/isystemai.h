#ifndef _SYSTEM_AI_I_H
#define _SYSTEM_AI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>
#include <system/scheduler/jobqueue.h>

namespace airacingtech
{
	class IData;

	BEGIN_INTERFACE_DEC(ISystemAI)
	
	public:
		
		//virtual airacingtech::VehicleState *GetVehicleState(int index) = 0;
		//virtual airacingtech::ControlState *GetVehicleControls(int index) = 0;
		virtual IData *GetData(int index) = 0;
		virtual int GetSize() = 0;

		virtual void Update() = 0;
		virtual void PreUpdate() = 0;
		virtual void PostUpdate() = 0;

		virtual void PreUpdate(int index) = 0;

		virtual void SetMultiThreadingGroup(Scheduler::GroupHandle group) = 0;
		virtual void SetMultiThreadingPolicy(Scheduler::PolicyHandle policy) = 0;
		
		virtual const CObjectId &GetObjectIDFromIndex(int index) const = 0;
		virtual int GetIndexFromObjectID(const CObjectId) const = 0; 

		virtual const int GetNumOfAI() const = 0;

	END_INTERFACE_DEC
	

}

#endif //_SYSTEM_AI_I_H
