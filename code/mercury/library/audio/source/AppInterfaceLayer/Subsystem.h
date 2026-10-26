#ifndef _SUBSYSTEM_H_
#define _SUBSYSTEM_H_

#ifdef _MSC_VER
#pragma once
#endif

namespace Audio
{

	// forward decl
	class InterfaceController;


	static const U32 flagMuted				= 0x00000001;
	static const U32 flagSuspended			= 0x00000002;
	static const U32 flagIgnoringMessages	= 0x00000004;


	enum SubsystemMessage
	{
		eMute = 0,
		eUnmute,
		eSuspend,
		eResume,
		eIgnoreMessages,
		eAcceptMessages,
		eReload,
		eInit,
		eShutdown,
		//cian temp for 1st playable
		eEndLevel,
		eBeginLevel,
		eLoadLevel,				// level is about to load or has started loading
		ePreLoadInGameUI,
		ePreLoadFrontEndUI,
		eLoadUI,		
		eRaceStarted,
		eDumpLevel,
		eQuit,
		eRaceFinished,

		TotalNumSubsystemMessages
	};


	class Subsystem
	{

		friend class InterfaceController;
		friend class EntityCollection;
		friend class DefaultEntityCallback;

	public:
		Subsystem();
		virtual ~Subsystem();

		inline const bool					CheckIdFlags(const U32 u32IdFlags) { return (m_u32IdFlags & u32IdFlags) ? true : false; }
		inline const bool					CheckStateFlags(const U32 u32StateFlags) { return (m_u32StateFlags & u32StateFlags) ? true : false; }

	protected:
		virtual bool						ProcessSystemMessage(const U32 u32MessageId)								{ Error("Subsystem base class method called"); return false; }
		virtual bool						ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0)	{ Error("Subsystem base class method called"); return false; }
		virtual bool						SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)		{ Error("Subsystem base class method called"); return false; }
		virtual void						Update()																		{ Error("Subsystem base class method called"); }
		void								SystemUpdate();
		virtual EntityTypeHandle			CreateEntityType(EntityDefinitionHandle hDefinition);
		virtual EntityInstanceHandle		CreateEntityInstance(EntityTypeHandle hParentEntityType);
		virtual bool						ReleaseEntityType(EntityTypeHandle hEntityType);
		virtual bool						ReleaseEntityInstance(EntityInstanceHandle hEntity);
		inline void							InternalAssert(bool bExp, const char* strMsg = 0);	// renamed so it didnt conflict with Assert #define in util.h
		void								Error(const char* strMsg);
		inline EntityInstance*				GetEntityInstance(EntityInstanceHandle hEntity) { return m_aEntityInstance.GetByHandle(hEntity); }
		inline EntityType*					GetEntityType(EntityTypeHandle hEntityType) { return m_aEntityType.GetByHandle(hEntityType); }
		inline EntityInstance*				GetEntityInstanceByIndex(U32 u32Index) { return m_aEntityInstance.GetByIndex((U16)u32Index); }
		inline EntityType*					GetEntityTypeByIndex(U32 u32Index) { return m_aEntityType.GetByIndex((U16)u32Index); }
		inline U32							GetHighestEntityInstanceIndex() { return m_aEntityInstance.GetHighestNodeIndex(); }
		inline U32							GetHighestEntityTypeIndex() { return m_aEntityType.GetHighestNodeIndex(); }
		inline void							SetId(const U32 u32Id) { m_u32SubsystemId = u32Id; }
		inline AudioEntityHandle			GetAudioEntityHandle(EntityInstanceHandle& hInst) { return AudioEntityHandle(hInst, (U16)m_u32SubsystemId); }

		Audio::PagedArray<EntityType*>		m_aEntityType;
		Audio::PagedArray<EntityInstance*>	m_aEntityInstance;
		U32									m_u32IdFlags;
		U32									m_u32StateFlags;
		U32									m_u32SubsystemId;
	};
}

#endif	//#ifndef _SUBSYSTEM_H_
