#include "stdafx.h"
#include <system/csystemai.h>
#include <vehiclestate/ivehiclestate.h>
#include <controlsstate/icontrolstate.h>
#include <driverai/idriverai.h>
#include <system/Perf/PerfEvent.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

static Debug::Logger &g_DebugLoggerAISystem = Debug::GetLogger("oldtech.ai.aisystem");

namespace airacingtech
{
	struct TaskData
	{
		class CSystemAI		*m_pSystemAI;
		int					m_AIIndex;				
	};

	const U32 MaxTaskBatch = 4;
	struct TaskBatch
	{
		TaskData m_Data[MaxTaskBatch];
		U32 m_TaskSize;
	};


	void PreUpdateIterateTask(Scheduler::Task* pTask)
	{
		/*TaskData *pData = (TaskData*)pTask->ParamData;*/
		TaskBatch* pBatch = (TaskBatch*)pTask->ParamData;
		for(U32 a = 0; a < pBatch->m_TaskSize; ++a)
		{
			pBatch->m_Data[a].m_pSystemAI->PreUpdateIterate(pBatch->m_Data[a].m_AIIndex);
		}
	}

	void UpdateIterateTask(Scheduler::Task* pTask)
	{
		/*TaskData *pData = (TaskData*)pTask->ParamData;
		pData->m_pSystemAI->UpdateIterate(pData->m_AIIndex);*/
		TaskBatch* pBatch = (TaskBatch*)pTask->ParamData;
		for(U32 a = 0; a < pBatch->m_TaskSize; ++a)
		{
			pBatch->m_Data[a].m_pSystemAI->UpdateIterate(pBatch->m_Data[a].m_AIIndex);
		}
	}

	void PostUpdateIterateTask(Scheduler::Task* pTask)
	{
		/*TaskData *pData = (TaskData*)pTask->ParamData;
		pData->m_pSystemAI->PostUpdateIterate(pData->m_AIIndex);*/
		TaskBatch* pBatch = (TaskBatch*)pTask->ParamData;
		for(U32 a = 0; a < pBatch->m_TaskSize; ++a)
		{
			pBatch->m_Data[a].m_pSystemAI->PostUpdateIterate(pBatch->m_Data[a].m_AIIndex);
		}
	}


}

JQ_DECLARE_PPU_MODULE(AIRacingTechPreUpdate, airacingtech::PreUpdateIterateTask);
JQ_DECLARE_PPU_MODULE(AIRacingTechUpdate, airacingtech::UpdateIterateTask);
JQ_DECLARE_PPU_MODULE(AIRacingTechPostUpdate, airacingtech::PostUpdateIterateTask);

namespace airacingtech
{

	





	CSystemAI::CSystemAI()
	{
		m_DistribtueSplineTrackingCounter = 0;
		m_AIlist.clear();
		m_AIListDriversI.clear();
		m_AIListDataI.clear();
	}

	CSystemAI::~CSystemAI()
	{

	}

	// components.. these should really be moved and registered elsewhere...maybe in the old system
	void CSystemAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_SYSTEM_AI, CID_SYSTEM_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_SYSTEM_AI, CSystemAI::CreateMe, CSystemAI::DestroyMe, CHash("SystemAI")); 
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SYSTEM_AI, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SYSTEM_AI, MT_INIT_AI_DRIVER);
		//GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SYSTEM_AI, MT_ADD_AI_DRIVER_TO_LIST);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SYSTEM_AI, MT_INIT_OBJECT);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SYSTEM_AI, MT_QUICK_RESTART);
		
	}

	IComponent *CSystemAI::CreateMe()
	{
		return new CSystemAI;
	}

	bool CSystemAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;
		return true;
	}

	// Virtual IComponent methods
	bool CSystemAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CSystemAI::Deinit(void)
	{
		m_AIlist.clear();
	}

	EMessageResult CSystemAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{
		case MT_QUICK_RESTART:
			{
				QuickRestart();
			}
			break;


		case MT_POST_INIT_AI_SYSTEM:
			{
				msgRet =  MR_TRUE;	

				for(int i=0; i<(int)m_AIlist.size(); i++)
				{
					CObjectId id = m_AIlist[i];
					CMessage msg;
					msg.mType = MT_ADD_AI_DRIVER_TO_LIST;
					msg.mpData = &id;
					GlobalsComponent::GetObjectManager().BroadcastMessage(msg);
				}
			}
			break;

		case MT_INIT_AI_DRIVER:
			{
				CParameterNode *pParams = static_cast<CParameterNode *>(msg.mpData);
				CObjectId id =  GlobalsComponent::GetObjectManager().CreateObject(pParams);

				if ( GlobalsComponent::GetObjectManager().ObjectExists(id) )
				{
					AddAI(id);
				}

				msgRet =  MR_TRUE;	
			}
			break;

			//case MT_ADD_AI_DRIVER_TO_LIST:
			//{
			//}
			//break;

		case MT_INIT_OBJECT:
			{
				CParameterNode *pParams = static_cast<CParameterNode *>(msg.mpData);
				CObjectId id =  GlobalsComponent::GetObjectManager().CreateObject(pParams);
				msgRet =  MR_TRUE;	
			}
			break;


		default:
			break;

		}

		return msgRet;
	}



	EComponentTypeId CSystemAI::GetComponentTypeId(void)
	{
		return CID_SYSTEM_AI;
	}

	void CSystemAI::QuickRestart()
	{
		m_DistribtueSplineTrackingCounter = 0;
	}

	void CSystemAI::AddAI(CObjectId id) 
	{
		m_AIlist.push_back(id);

		IDriverAI *pIDriver = static_cast<IDriverAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DRIVER_AI));
		assert(pIDriver);
		m_AIListDriversI.push_back(pIDriver);

		IData *pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
		assert(pIData);
		
		pIData->GetState().m_index = Maths::Max(static_cast<int>(m_AIListDataI.size()), 0);

		m_AIListDataI.push_back(pIData);

		
	}

	IData *CSystemAI::GetData(int index)
	{

		assert(index >=0 && index < (int)m_AIlist.size());
#if 0	
		CObjectId id = m_AIlist[index];
		IData *pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
		assert(pIData);
#else
		IData *pIData = m_AIListDataI[index];
		assert(pIData);
#endif

		//Darren Hack... this gets called at least once to set the index in the list...
		pIData->GetState().m_index = index;
		return pIData;
	}

	const CObjectId &CSystemAI::GetObjectIDFromIndex(int index) const
	{
		assert(index >=0 && index < (int)m_AIlist.size());
		return m_AIlist[index];
	}

	int CSystemAI::GetIndexFromObjectID(const CObjectId id) const
	{
		for(int i=0;i<(int)m_AIlist.size();i++)
		{
			if(m_AIlist[i] == id )
			{
				return i;
			}
		}
		return -1;
	}

	void CSystemAI::UpdateIterate(int i)
	{
		PERF_EVENT("AI.TechUpdate");
		IDriverAI *pIDriverAI = m_AIListDriversI[i];
		pIDriverAI->Update();
	}


	void CSystemAI::Update()
	{
#ifdef PARTS_OF_AI_MULTI_THREADED
		TaskBatch batch;
		batch.m_TaskSize = 0;
#endif
		for(int i=0; i<(int)m_AIListDriversI.size(); i++)
		{
#ifdef PARTS_OF_AI_MULTI_THREADED
			TaskData& data = batch.m_Data[batch.m_TaskSize++];
			data.m_pSystemAI = this;
			data.m_AIIndex = i;
			if(batch.m_TaskSize == MaxTaskBatch)
			{
				Scheduler::AddTask(AIRacingTechUpdate, 0, 0, m_Group, m_Policy, &batch, sizeof(TaskBatch));	
				batch.m_TaskSize = 0;
			}
#else
			UpdateIterate(i);
#endif
		}

#ifdef PARTS_OF_AI_MULTI_THREADED
		if(batch.m_TaskSize > 0)
		{
			Scheduler::AddTask(AIRacingTechUpdate, 0, 0, m_Group, m_Policy, &batch, sizeof(TaskBatch));	
		}
		Scheduler::Flush(m_Group);
#endif

		CMessage msg;
		msg.mType = MT_UPDATE_AI_SYSTEMS;
		msg.mpData = 0;
		GlobalsComponent::GetObjectManager().BroadcastMessage(msg);

	}


	void CSystemAI::PreUpdateIterate(int i)
	{
		if(m_AIListDriversI.size() > i)
		{
			PERF_EVENT("AI.TechPreUpdate");
			IDriverAI *pIDriverAI = m_AIListDriversI[i];
			pIDriverAI->PreUpdate();
		}
	}

	void CSystemAI::PreUpdate()
	{
#ifdef PARTS_OF_AI_MULTI_THREADED
		TaskBatch batch;
		batch.m_TaskSize = 0;
#endif
		for(int i=0; i<(int)m_AIListDriversI.size(); i++)
		{
#ifdef PARTS_OF_AI_MULTI_THREADED
			TaskData& data = batch.m_Data[batch.m_TaskSize++];
			data.m_pSystemAI = this;
			data.m_AIIndex = i;
			if(batch.m_TaskSize == MaxTaskBatch)
			{
				Scheduler::AddTask(AIRacingTechPreUpdate, 0, 0, m_Group, m_Policy, &batch, sizeof(TaskBatch));
				batch.m_TaskSize = 0;
			}
#else
			PreUpdateIterate(i);
#endif
		}

#ifdef PARTS_OF_AI_MULTI_THREADED
			if(batch.m_TaskSize > 0)
			{
				Scheduler::AddTask(AIRacingTechPreUpdate, 0, 0, m_Group, m_Policy, &batch, sizeof(TaskBatch));
			}
			Scheduler::Flush(m_Group);
#endif

		
			CMessage msg;
			msg.mType = MT_PRE_UPDATE_AI_SYSTEMS;
			msg.mpData = 0;
			GlobalsComponent::GetObjectManager().BroadcastMessage(msg);

	}


	void CSystemAI::PostUpdateIterate(int i)
	{
		PERF_EVENT("AI.TechPostUpdate");
		IDriverAI *pIDriverAI = m_AIListDriversI[i];
		pIDriverAI->PostUpdate();
	}

	void CSystemAI::PostUpdate()
	{

#ifndef PERF_REMOVE_POST_UPDATE_AS_BLUR_DOES_NOT_HAVE_IMPORTANT_CODE_HERE

#ifdef PARTS_OF_AI_MULTI_THREADED
		TaskBatch batch;
		batch.m_TaskSize = 0;
#endif

		for(int i=0; i<(int)m_AIListDriversI.size(); i++)
		{
		
#ifdef PARTS_OF_AI_MULTI_THREADED
			TaskData& data = batch.m_Data[batch.m_TaskSize++];
			data.m_pSystemAI = this;
			data.m_AIIndex = i;
			if(batch.m_TaskSize == MaxTaskBatch)
			{
				Scheduler::AddTask(AIRacingTechPostUpdate, 0, 0, m_Group, m_Policy, &batch, sizeof(TaskBatch));	
				batch.m_TaskSize = 0;
			}
#else

			PostUpdateIterate(i);
#endif
		}

#ifdef PARTS_OF_AI_MULTI_THREADED
		if(batch.m_TaskSize > 0)
		{
			Scheduler::AddTask(AIRacingTechPostUpdate, 0, 0, m_Group, m_Policy, &batch, sizeof(TaskBatch));	
		}
		Scheduler::Flush(m_Group);
#endif

#endif

		CMessage msg;
		msg.mType = MT_POST_UPDATE_AI_SYSTEMS;
		msg.mpData = 0;
		GlobalsComponent::GetObjectManager().BroadcastMessage(msg);

	}
}
#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


