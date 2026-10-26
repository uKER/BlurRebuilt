#ifndef _ENTITY_H_
#define _ENTITY_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <System/DebugLog.h>

namespace InterfaceLayerLogger 
{
	extern Debug::Logger &g_Logger;
}


#define	MAX_NUM_FUNCTION_EXECUTION_HISTORY_ENTRIES	8

extern Audio::InstanceComponentHandle gInvalidInstComHnd;
extern Audio::Handle gInvalidHnd;

struct AudioEntityHandle
{
	AudioEntityHandle() { m_u16SubsystemId = 0; }


	//AudioEntityHandle(Audio::PagedArrayHandle h)
	//{
	//	m_u32PagedArrayHandle = h;
	//	m_u16SubsystemId = 0;
	//}


	AudioEntityHandle(Audio::PagedArrayHandle h, U16 u16Id)
	{
		m_u32PagedArrayHandle = h;
		m_u16SubsystemId = u16Id;
	}


	void operator =(const Audio::PagedArrayHandle h)
	{
		m_u32PagedArrayHandle = h;
		m_u16SubsystemId = 0;
	}


	bool operator ==(AudioEntityHandle& hComp)
	{
		return (hComp.m_u32PagedArrayHandle == m_u32PagedArrayHandle && hComp.m_u16SubsystemId == m_u16SubsystemId);
	}


	bool operator !=(AudioEntityHandle& hComp)
	{
		return (hComp.m_u32PagedArrayHandle != m_u32PagedArrayHandle || hComp.m_u16SubsystemId != m_u16SubsystemId);
	}

	bool									IsValid() const { return m_u32PagedArrayHandle.IsValid(); }
//	static AudioEntityHandle				InvalidHandle() { return AudioEntityHandle(Audio::PagedArrayInvalidHandle(), 0); }
	void									Zero()  { m_u32PagedArrayHandle.SetInvalidHandle();m_u16SubsystemId=0;}


	inline const Audio::PagedArrayHandle	ArrayHandle() const { return m_u32PagedArrayHandle; }
	inline const U32						SubsystemId() const { return (U32)m_u16SubsystemId; }

	Audio::PagedArrayHandle					m_u32PagedArrayHandle;
	U16										m_u16SubsystemId;
};


enum FunctionType
{
	eEntityTypeFunction = 0,
	eEntityInstanceFunction,
	eParameterisedEntityInstanceFunction
};


struct FunctionExecutionHistoryEntry
{
	U32					hFunction;
	AudioEntityHandle	hEntity;
	AudioEntityHandle	hParameterEntity;
	FunctionType		eFunctionType;

	FunctionExecutionHistoryEntry() { hFunction = 0; eFunctionType = eEntityTypeFunction; }
};


typedef AudioEntityHandle AudioEntityTypeHandle;


namespace Audio
{

	class EntityBase
	{

	public:
		EntityBase() { m_pDataStore = 0; m_pDataStore2 = 0; m_u32DataStore1Size = 0; m_u32DataStore2Size = 0; }

		bool			Init(U32 u32DataStore1Size, U32 u32DataStore2Size)
		{
			m_pDataStore = new U32[u32DataStore1Size + u32DataStore2Size];

			if (! m_pDataStore)
				return false;

			m_pDataStore2 = &(m_pDataStore[u32DataStore1Size]);

			m_u32DataStore1Size = u32DataStore1Size;
			m_u32DataStore2Size = u32DataStore2Size;

			return true;
		}
		void							SetDefinition(EntityDefinitionHandle hDefinition) { m_hDefinition = hDefinition; }
		inline EntityDefinitionHandle	GetDefinitionHandle() { return m_hDefinition; }
		bool							PushFunctionExecutionEntry(FunctionExecutionHistoryEntry& entry);
		bool							PopFunctionExecutionEntry(FunctionExecutionHistoryEntry& entry);
		// if an entity is destroyed, we'll need to remove any references to it from the history of other entities...
		void							RemoveHistoryEntryWithSignature(U32 hFunction, AudioEntityHandle& hEntity);
		inline U32						GetNumFunctionExecutionHistoryEntries() { return m_aFunctionHistory.Size(); }
		bool							GetFunctionExecutionEntry(U32 u32Index, FunctionExecutionHistoryEntry& entry);


	protected:
		inline const U32*				DataStore1() const { return m_pDataStore; }
		inline const U32*				DataStore2() const { return m_pDataStore2; }
		inline U32						GetDataStore1Size() const { return m_u32DataStore1Size; }
		inline U32						GetDataStore2Size() const { return m_u32DataStore2Size; }
		void							Release() { if (m_pDataStore) delete [] m_pDataStore; m_pDataStore = 0; }

	private:
		EntityDefinitionHandle			m_hDefinition;
		Btl::Vector<FunctionExecutionHistoryEntry>	m_aFunctionHistory;
		U32*							m_pDataStore;
		U32*							m_pDataStore2;
		U32								m_u32DataStore1Size;
		U32								m_u32DataStore2Size;
		MT::Lock						m_Lock;
	};


	class EntityType : public EntityBase
	{

	public:
		EntityType() { m_u32InstanceRefCount = 0; }
		~EntityType() { Release(); }

		bool							Init(U32 u32NumObjects, U32 u32NumComponents);
		ObjectHandle					GetObject(U32 u32Index) { if (u32Index == 0xffffffff || u32Index >= GetNumObjects()) { ObjectHandle hInvalid; return hInvalid; } return Objects()[u32Index]; }
		ObjectComponentHandle			GetComponent(U32 u32Index) { if (u32Index == 0xffffffff || u32Index >= GetNumComponents()) { ObjectComponentHandle hInvalid; return hInvalid; } return Components()[u32Index]; }
		bool							SetObjectHandle(U32 u32Index, ObjectHandle hObj) { if (u32Index >= GetNumObjects()) return false; Objects()[u32Index] = hObj; return true; }
		bool							SetComponentHandle(U32 u32Index, ObjectComponentHandle hComp) { if (u32Index >= GetNumComponents()) return false; Components()[u32Index] = hComp; return true; }
		inline U32						GetInstanceRefCount() { return m_u32InstanceRefCount; }
		inline void						IncRefCount() { m_u32InstanceRefCount++; }
		inline void						DecRefCount() { assert (m_u32InstanceRefCount != 0); m_u32InstanceRefCount--; }

	private:
		inline ObjectHandle*			Objects() const { return (ObjectHandle*)DataStore1(); }
		inline ObjectComponentHandle*	Components() const { return (ObjectComponentHandle*)DataStore2(); }
		inline U32						GetNumObjects() const { return GetDataStore1Size(); }
		inline U32						GetNumComponents() const { return GetDataStore2Size(); }

		U32								m_u32InstanceRefCount;
	};


	class EntityInstance : public EntityBase
	{

	public:
		EntityInstance() { m_pParentEntityType = 0; /*m_af32ReturnValues = 0;*/ m_bSuspended = false; m_u32CallbackRefCount = 0; m_u32DataTrackerRefCount = 0; m_pCustomData = 0; m_bResumeAfterPause = false; }
		~EntityInstance() 
		{ 
			//if (m_af32ReturnValues) 
			//	delete [] m_af32ReturnValues; 
			//m_af32ReturnValues = 0; 
			
			if (m_pCustomData) 
			{
				//BIZ_ASSERT(0 && "Trying to call destructor on Entity instance when custom data has not been freed");
				// temporarily downgraded to an error
				//AUDIO_SYS_ERROR("Trying to call destructor on Entity instance when custom data has not been freed");
				LOGERROR(InterfaceLayerLogger::g_Logger, "Trying to call destructor on Entity instance when custom data has not been freed");
			}

			Release(); 
		}

		bool							Init(U32 u32NumInstances, U32 u32NumConnectors/*, U32 u32NumReturnValues*/);
		inline InstanceHandle			GetInstance(U32 u32Index) const { if (u32Index == 0xffffffff || u32Index >= GetNumInstances()) { InstanceHandle hInvalid; return hInvalid; } return Instances()[u32Index]; }
		inline InstanceComponentHandle	GetConnector(U32 u32Index) const { if (u32Index == 0xffffffff || u32Index >= GetNumConnectors()) return gInvalidInstComHnd; return Connectors()[u32Index]; }
		bool							SetEntityType(EntityType* pType) { if (m_pParentEntityType) return false; m_pParentEntityType = pType; return true; }
		void							SetAudioEntityHandle(AudioEntityHandle hEntity) { m_hAudioEntityHandle = hEntity; }
		inline EntityType*				GetParentEntityType() const { return m_pParentEntityType; }
		bool							SetInstanceHandle(U32 u32Index, InstanceHandle hInst) { if (u32Index >= GetNumInstances()) return false; Instances()[u32Index] = hInst; return true; }
		bool							SetConnectorHandle(U32 u32Index, InstanceComponentHandle hComp) { if (u32Index >= GetNumConnectors()) return false; Connectors()[u32Index] = hComp; return true; }
		bool							SetSuspendState(bool bSuspend);
		inline bool						IsSuspended() { return m_bSuspended; }
		bool							SetMuteState(bool bMute);
		inline bool						IsMuted() { return m_bMuted; }
//		inline F32						GetOutputValue(U32 u32Index) { return m_af32ReturnValues[u32Index]; }
		inline void						IncrementCallbackRefCount() { m_u32CallbackRefCount++; }
		inline void						DecrementCallbackRefCount() { m_u32CallbackRefCount--; }
		inline U32						GetCallbackRefCount() const { return m_u32CallbackRefCount; }
		inline void						IncrementDataTrackerRefCount() { m_u32DataTrackerRefCount++; }
		inline void						DecrementDataTrackerRefCount() { m_u32DataTrackerRefCount--; }
		inline U32						GetDataTrackerRefCount() const { return m_u32DataTrackerRefCount; }
		const AudioEntityHandle&		GetAudioEntityHandle() const { return m_hAudioEntityHandle; }
		const void*						GetCustomDataPtr() const { return m_pCustomData; }
		void							SetCustomDataPtr(void* pData) { assert(!m_pCustomData); m_pCustomData = pData; }					
		void							SetCustomDataFreed() { m_pCustomData = 0; }												// you must call this after freeing the custom data
//		void							SetReturnValue(F32 f32Value, U32 u32Index) { m_af32ReturnValues[u32Index] = f32Value; }
		void							SetResumeAfterPause(bool bResumeAfterPause);
		bool					    	GetResumeAfterPause();

	private:
		inline InstanceHandle*			Instances() const { return (InstanceHandle*)DataStore1(); }
		inline InstanceComponentHandle*	Connectors() const { return (InstanceComponentHandle*)DataStore2(); }
		inline U32						GetNumInstances() const { return GetDataStore1Size(); }
		inline U32						GetNumConnectors() const { return GetDataStore2Size(); }

		EntityType*						m_pParentEntityType;
//		F32*							m_af32ReturnValues;
		AudioEntityHandle				m_hAudioEntityHandle;
		U32								m_u32CallbackRefCount;
		U32								m_u32DataTrackerRefCount;
		bool							m_bSuspended;
		void*							m_pCustomData;
		bool							m_bMuted;
		bool							m_bResumeAfterPause;
	};
}

#endif	//	#ifndef _ENTITY_H_
