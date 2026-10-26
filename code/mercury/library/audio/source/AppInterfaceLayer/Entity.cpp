#include "AudioPch.h"
#include "..\..\Include\AppInterfaceLayer.h"

Audio::InstanceComponentHandle gInvalidInstComHnd;
Audio::Handle gInvalidHnd;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PushFunctionExecutionEntry
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityBase::PushFunctionExecutionEntry(FunctionExecutionHistoryEntry& entry)
{
	m_Lock.Enter();
	m_aFunctionHistory.PushBack(entry);
	m_Lock.Exit();

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PopFunctionExecutionEntry
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityBase::PopFunctionExecutionEntry(FunctionExecutionHistoryEntry& entry)
{
	m_Lock.Enter();
	U32 u32ListSize = m_aFunctionHistory.Size();

	if (! u32ListSize)
	{
		m_Lock.Exit();
		return false;
	}

	entry = m_aFunctionHistory[u32ListSize - 1];

	m_aFunctionHistory.Resize(u32ListSize - 1);

	m_Lock.Exit();

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RemoveHistoryEntryWithSignature
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
//void Audio::EntityBase::RemoveHistoryEntryWithSignature(U32 hFunction, AudioEntityHandle& hEntity)
//{
//	for (S32 i = 0; i < m_aFunctionHistory.Size(); i++)
//	{
//		if (m_aFunctionHistory[i].hFunction == hFunction &&
//			(m_aFunctionHistory[i].hEntity == hEntity || m_aFunctionHistory[i].hParameterEntity == hEntity))
//		{
//			for (S32 i2 = i; i2 < (S32)m_aFunctionHistory.Size() - 1; i2++)
//			{
//				m_aFunctionHistory[i2] = m_aFunctionHistory[i2 + 1];
//			}
//
//			m_aFunctionHistory.Resize(m_aFunctionHistory.Size() - 1);
//			i--;
//		}
//	}
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetFunctionExecutionEntry
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityBase::GetFunctionExecutionEntry(U32 u32Index, FunctionExecutionHistoryEntry& entry)
{
	m_Lock.Enter();

	if ((S32)u32Index >= m_aFunctionHistory.Size())
	{
		m_Lock.Exit();
		return false;
	}

	entry = m_aFunctionHistory[u32Index];

	m_Lock.Exit();

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Init
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityInstance::Init(U32 u32NumInstances, U32 u32NumConnectors/*, U32 u32NumReturnValues*/)
{
	if (EntityBase::Init(u32NumInstances, u32NumConnectors))
	{
		InstanceHandle hInvalidInst;
		for (U32 i=0; i<u32NumInstances; i++)
			SetInstanceHandle(i, hInvalidInst);

		InstanceComponentHandle hInvalidInstCom;
		for (U32 i=0; i<u32NumConnectors; i++)
			SetConnectorHandle(i, hInvalidInstCom);

		//if (u32NumReturnValues)
		//{
		//	m_af32ReturnValues = new F32[u32NumReturnValues];

		//	if (! m_af32ReturnValues)
		//		return false;

		//	for (U32 i = 0; i < u32NumReturnValues; i++)
		//		m_af32ReturnValues[i] = 0.f;
		//}

		return true;
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetSuspendState
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityInstance::SetSuspendState(bool bSuspend)
{
	if (m_bSuspended == bSuspend)
		return true;

	m_bSuspended = bSuspend;

	bool bSuccess = true;

	for (U32 i = 0; i < GetNumInstances(); i++)
		bSuccess = bSuccess && System::Get()->GetObjectManager()->Suspend(GetInstance(i), bSuspend);

	return bSuccess;
}

void	Audio::EntityInstance::SetResumeAfterPause(bool bResumeAfterPause)
{
	m_bResumeAfterPause = bResumeAfterPause;
}

bool	Audio::EntityInstance::GetResumeAfterPause() 
{
	return m_bResumeAfterPause;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Init
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityType::Init(U32 u32NumObjects, U32 u32NumComponents)
{
	if (EntityBase::Init(u32NumObjects, u32NumComponents))
	{
		ObjectHandle hInvalidObj;
		for (U32 i=0; i<u32NumObjects; i++)
			SetObjectHandle(i, hInvalidObj);

		ObjectComponentHandle hInvalidObjCom;
		for (U32 i=0; i<u32NumComponents; i++)
			SetComponentHandle(i, hInvalidObjCom);

		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetMuteState
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityInstance::SetMuteState(bool bMute)
{
	if (m_bMuted == bMute)
		return false;

	m_bMuted = bMute;

	bool bSuccess = true;

	for (U32 i = 0; i < GetNumInstances(); i++)
		bSuccess = bSuccess && System::Get()->GetObjectManager()->Mute(GetInstance(i), bMute);

	return bSuccess;
}

