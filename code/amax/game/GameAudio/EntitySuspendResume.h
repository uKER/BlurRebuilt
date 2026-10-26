#ifndef _ENTITY_SUSPEND_RESUME_H_
#define _ENTITY_SUSPEND_RESUME_H_

#ifdef _MSC_VER
#pragma once
#endif

namespace Components 
{
	class BaseMessageHandler;
}

//namespace Audio
//{

enum E_EntityAudioState
{
	E_EntityAudioState_Unknown = 0,		// entity/instance state is unknown. may be active or not active
	E_EntityAudioState_Playing,			// entity/instance is active and producing audio
	E_EntityAudioState_Finished			// entity/instance is finished and suspended
};

enum E_SuspendResumeType
{
	E_SuspendResumeManagerBase = 0,
	E_NullSuspenderResumer,
	E_EntitySuspenderResumer,
	E_InstanceSuspenderResumer,
};

class I_AudioEntityDataSource
{
public:
	static const U32 uInvalidDataMemberId = 0xffffffff;
	static const U32 uInvalidDataMemberHandle = 0xffffffff;

	virtual ~I_AudioEntityDataSource() {}

	virtual U32									GetNumDataMembers() const = 0;
	virtual const char*							GetDataMemberName(const U32 id) const = 0;
	virtual Audio::EntityDataMemberHandle		GetDataMemberHandle(const U32 id) const = 0;

};

//-----------------------------------------------------------------------------
// C_SuspendResumeManagerBase()
//
// abstract base class.
//-----------------------------------------------------------------------------
class C_SuspendResumeManagerBase
{
public:
	static const U32 uSuspendAll = 0xfffffffe;

	virtual ~C_SuspendResumeManagerBase() {}

	virtual const E_SuspendResumeType Type() const { return E_SuspendResumeManagerBase; }

	virtual bool Suspend(const Audio::EntityDataMemberHandle hCom = uSuspendAll) = 0;
	virtual bool Resume(const Audio::EntityDataMemberHandle hCom = uSuspendAll) = 0;

	virtual void Init(const Actor::C_ActorHandle& hActor, Audio::EntityDefinitionHandle hDef, AudioEntityHandle hEntity) = 0;

	virtual bool IsCallbackSet() const = 0;

	virtual E_EntityAudioState GetState() const = 0;
};

//-----------------------------------------------------------------------------
// C_NullSuspendResumeManager()
//
// does no suspending or resuming. 
// use this if you dont want your entity to ever be suspended or resumed
//-----------------------------------------------------------------------------
class C_NullSuspenderResumer : public C_SuspendResumeManagerBase
{
public:
	C_NullSuspenderResumer(const I_AudioEntityDataSource* pDataSource) {}
	~C_NullSuspenderResumer() {}

	const E_SuspendResumeType Type() const { return E_NullSuspenderResumer; }

	bool Suspend(const Audio::EntityDataMemberHandle hCom) { return true; }
	bool Resume(const Audio::EntityDataMemberHandle hCom) { return true; }

	void Init(const Actor::C_ActorHandle& hActor, Audio::EntityDefinitionHandle hDef, AudioEntityHandle hEntity) { }

	bool IsCallbackSet() const { return true; }

	E_EntityAudioState GetState() const { return E_EntityAudioState_Unknown; }
};

//-----------------------------------------------------------------------------
// EntitySuspendResumer()
//
// entire entity is resumed when any actionin triggers and suspended
// on "AudioStopped" external trigger.
//-----------------------------------------------------------------------------
class C_EntitySuspenderResumer : public C_SuspendResumeManagerBase
{
public:
	C_EntitySuspenderResumer(const I_AudioEntityDataSource* pDataSource) : m_pDataSource(pDataSource), m_eAudioState(E_EntityAudioState_Unknown) {}
	~C_EntitySuspenderResumer() {}

	const E_SuspendResumeType Type() const { return E_EntitySuspenderResumer; }

	bool Suspend(const Audio::EntityDataMemberHandle hCom);
	bool Resume(const Audio::EntityDataMemberHandle hCom);

	void Init(const Actor::C_ActorHandle& hActor, Audio::EntityDefinitionHandle hDef, AudioEntityHandle hEntity);

	bool IsCallbackSet() const { return m_hAudioStoppedCallback.IsValid(); }

	E_EntityAudioState GetState() const { return m_eAudioState; }
	void SetState(E_EntityAudioState eState) { m_eAudioState = eState; }
	
	const I_AudioEntityDataSource* GetDataSource() const { return m_pDataSource; }

	const AudioEntityHandle& GetEntity() const { return m_hEntity; }
	const Audio::EntityDefinitionHandle& GetEntityDef() const { return m_hEntityDef; }

protected:
	void SetCallbacks(const Actor::C_ActorHandle& hNotifyActor);

protected:
	Audio::PagedArrayHandle	m_hAudioStoppedCallback;

	E_EntityAudioState m_eAudioState;

	const I_AudioEntityDataSource* m_pDataSource;

	// we can look these up from the actor once we have derived components
	AudioEntityHandle m_hEntity;
	Audio::EntityDefinitionHandle m_hEntityDef;
};

//-----------------------------------------------------------------------------
// C_InstanceSuspendResumer()
//
// instances of audio objects within the entity can be suspended and resumed
// independently from each other.
//-----------------------------------------------------------------------------
class C_InstanceSuspenderResumer : public C_SuspendResumeManagerBase
{
public:
	static const U32 uInvalidInstanceId = 0xffffffff;
	static const U32 uActionOutNone = 0xfffffffe;

	C_InstanceSuspenderResumer(const I_AudioEntityDataSource* pDataSource);
	~C_InstanceSuspenderResumer() {}

	const E_SuspendResumeType Type() const { return E_InstanceSuspenderResumer; }

	Audio::PagedArrayHandle SubscribeToCallback(const Audio::EntityDataMemberHandle& hActionOut, U32 uInstanceId, const Actor::C_ActorHandle& hActor);

	virtual U32 GetNumInstances() const { return 0; }
	S32	InstancePlayingCount() const { return m_InstancesPlaying.Size(); }

	E_EntityAudioState GetState() const;

	void Init(const Actor::C_ActorHandle& hActor, Audio::EntityDefinitionHandle hDef, AudioEntityHandle hEntity);

	bool Suspend(const Audio::EntityDataMemberHandle hCom);

	virtual void SuspendInstance(U32 uInstanceId);
	void ResumeInstance(U32 uInstanceId);

	const I_AudioEntityDataSource* GetDataSource() const { return m_pDataSource; }

	const AudioEntityHandle& GetEntity() const { return m_hEntity; }
	const Audio::EntityDefinitionHandle& GetEntityDef() const { return m_hEntityDef; }

	bool IsCallbackSet() const; // TODO 

protected:
	//virtual U32	GetActionOutId( U32 uInstanceId ) = 0; /*const { return 0xffffffff; }*/
	//virtual U32 GetInstanceId(U32 uComponentId) const = 0;

	U32 LookupComponentId(const Audio::EntityDataMemberHandle hCom) const;
	
	Audio::EntityDataMemberHandle GetActionOutHandle( U32 uInstanceId ) const { return m_ahActionOutHandle[ uInstanceId ]; }
	Audio::EntityDataMemberHandle GetInstanceHandle( U32 uInstanceId ) const { return m_ahInstanceHandle[ uInstanceId ]; }

	virtual char* GetInstanceString( U32 uId ) const = 0;
	virtual char* GetActionOutString( U32 uId ) const = 0;

	void SuspendAll();

	bool IsInstancePlaying(U32 uInstanceId) const;
	void SetInstancePlaying(U32 uInstanceId, bool bPlaying);

	void SetCallbacks(const Actor::C_ActorHandle& hNotifyActor);

protected:

	Btl::Map<Audio::EntityDataMemberHandle, U32>	m_ComIdLookupMap;			// lookup componentid from handle
	Btl::Map<U32, bool>								m_InstancesPlaying;			// which instances are playing?
	Btl::Vector< Audio::EntityDataMemberHandle >	m_ahActionOutHandle;		// instance handles. one for each instance
	Btl::Vector< Audio::EntityDataMemberHandle >	m_ahInstanceHandle;			// instance handles. one for each instance
	Btl::Vector< Audio::PagedArrayHandle >			m_ahActionOutCallback;		// action out callback handles handles. one for each instance

	const I_AudioEntityDataSource* m_pDataSource;

	// we can look these up from the actor once we have derived components
	AudioEntityHandle m_hEntity;
	Audio::EntityDefinitionHandle m_hEntityDef;
};

//-----------------------------------------------------------------------------
// EntityCallbacks
//
// triggered when a sound has finished playing
//-----------------------------------------------------------------------------
class C_AudioEntityCallback : public Audio::EntityCallbackBase
{
public:
	C_AudioEntityCallback(const AudioEntityHandle& hEnt, const Actor::C_ActorHandle& hActor) : 
		m_hActor(hActor) 
		{ 
			m_hEntity = hEnt;
		}

	void Callback();
	Actor::C_ActorHandle m_hActor;			// the actor handle of the audio component which started the sound
};

class C_AudioInstanceCallback : public Audio::EntityCallbackBase
{
public:
	C_AudioInstanceCallback(const AudioEntityHandle& hEnt, const Actor::C_ActorHandle& hActor, U32 uInstanceId) : 
		m_hActor(hActor) 
		{ 
			m_hEntity = hEnt;
			m_uInstanceId = uInstanceId;
		}

	void Callback();
	Actor::C_ActorHandle m_hActor;			// the actor handle of the audio component which started the sound
	U32 m_uInstanceId;						// the instance id of the instance which is playing the sound
};
//} // namespace Audio
#endif	//	_ENTITY_SUSPEND_RESUME_H_
