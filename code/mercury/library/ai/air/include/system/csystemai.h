#ifndef _SYSTEM_AI_C_H
#define _SYSTEM_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/isystemai.h>

namespace airacingtech
{
	class IDriverAI;
	class IData;

	BEGIN_COMPONENT_DEC(CSystemAI, ISystemAI)

	public:
		
		IData *GetData(int index);
		const CObjectId &GetObjectIDFromIndex(int index) const; 
		int GetIndexFromObjectID(const CObjectId) const; 

		void AddAI(CObjectId id);
		void Update();
		void PreUpdate();
		void PostUpdate();
		void QuickRestart();
		inline int GetSize(){return (int)m_AIListDataI.size();}

		void PreUpdateIterate(int i);
		void UpdateIterate(int i);
		void PostUpdateIterate(int i);

		void SetMultiThreadingGroup(Scheduler::GroupHandle group) {m_Group = group;}
		void SetMultiThreadingPolicy(Scheduler::PolicyHandle policy) {m_Policy = policy;}

		const int GetNumOfAI() const
		{
			return static_cast<int>(m_AIlist.size());
		}

		void PreUpdate(int index)
		{
			PreUpdateIterate(index);
		}

	private:

		Scheduler::GroupHandle m_Group;
		Scheduler::PolicyHandle m_Policy;
		std::vector<CObjectId> m_AIlist;
		std::vector<IDriverAI *> m_AIListDriversI;
		std::vector<IData *> m_AIListDataI;
		int		m_DistribtueSplineTrackingCounter;

	END_COMPONENT_DEC
}



#endif //_SYSTEM_AI_C_H
