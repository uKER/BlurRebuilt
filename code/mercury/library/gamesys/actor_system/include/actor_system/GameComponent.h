#ifndef GAMECOMPONENT_H
#define GAMECOMPONENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <actor_system/MemoryTracker.h>
#include <actor_system/SerialisationConfig.h>
#include <misc_shared/DataRef/Ptr.h>
#include <misc_shared/DataRef/Sorcery.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <misc_shared/uid/TypeChecker.h>
#include <system/Debug/I_DebugRender.h>
#include <Core/BizAssert.h>


namespace Sys
{
	class C_ContextualStream;
}


namespace Component
{

/// Base class used to create components.
class I_ComponentCreator
{
public:
	typedef DataRef::C_Ptr<I_ComponentCreator> C_DataRef;

	// Xt annotated data
	U32							m_SorceryUid;	///< unique value assigned by sorcery

	I_ComponentCreator();
	virtual ~I_ComponentCreator();

	/**
	@brief Each component type is uniquely identified by a user-defined type 
	identifier.
	@returns type identifier for the component to create.
	*/
	virtual C_ComponentTypeId	GetComponentTypeID() const = 0;

	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);

	static void					XtCallback(XtEventType eventType, XtRecord* pRecord, I_ComponentCreator* pData, XtCallbackContext* pContext);

	DataRef::C_SorceryUid		GetSorceryUid() const {return static_cast<DataRef::C_SorceryUid>(m_SorceryUid);} ///< @returns unique identifier

	inline void					DoSorceryCallback(F_ComponentCreatorSorceryCallback pCallbackFn) {pCallbackFn(this);} ///< for DataRef::SorceryCallback
	static C_DataRef			FactoryCreate(const C_ComponentTypeId typeId, I_ComponentCreator** ppData = NULL, const C_TypeChecker* pTypeChecker = NULL); ///< Wrapper for DataRef::FactoryCreate

#ifndef CFG_GOLD
	virtual void				DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43 &actorMatrix) const;
#endif
};

/// Helper class for dynamic component creation data.
template <class T_ComponentCreator>
class C_ComponentCreatorFactoryHelper
{
public:
	typedef T_ComponentCreator C_ComponentCreatorType;									///< Class being created.
	static C_ComponentTypeId GetType() {return C_ComponentCreatorType::ClassType();}	///< Type identifier being created.

	C_ComponentCreatorFactoryHelper() : m_DataRef(), m_Data(NULL)
	{
		I_ComponentCreator* pCreationData = NULL;
#ifndef CFG_SPEED
		m_DataRef = I_ComponentCreator::FactoryCreate(GetType(), &pCreationData, GetTypeChecker<T_ComponentCreator>());
#else
		m_DataRef = I_ComponentCreator::FactoryCreate(GetType(), &pCreationData);
#endif
		BIZ_ASSERT((pCreationData != NULL) && (pCreationData->GetComponentTypeID() == GetType()));
		m_Data = static_cast<C_ComponentCreatorType*>(pCreationData);
	}

	~C_ComponentCreatorFactoryHelper() {}

	const I_ComponentCreator::C_DataRef& Get() const {return m_DataRef;} ///< Gets the created data reference.

	C_ComponentCreatorType& operator * () {return *m_Data;} ///< Writable data reference.
	C_ComponentCreatorType* operator -> () {return m_Data;} ///< Writable data pointer.

private:
	I_ComponentCreator::C_DataRef	m_DataRef;	///< Created data reference.
	C_ComponentCreatorType*			m_Data;		///< Writable data pointer.

	C_ComponentCreatorFactoryHelper(const C_ComponentCreatorFactoryHelper&);				// can not copy
	C_ComponentCreatorFactoryHelper& operator = (const C_ComponentCreatorFactoryHelper&);	// can not copy

};

/// Component abstract base class interface.
class I_GameComponent : public Messaging::C_MessageHandler
{
public:
	I_GameComponent();
	virtual ~I_GameComponent();

	virtual C_ComponentInterfaceId	GetInterfaceID(void) const = 0;		///< @returns component interface ID.
	virtual C_ComponentTypeId		GetComponentTypeID(void) const = 0;	///< @returns component type ID.

	const I_ComponentCreator::C_DataRef&	GetCreator() const {return m_pCreator;}
	ActorSys::E_LifetimeState		GetLifetimeState() const {return m_LifetimeState;}

	bool							Initialise(const I_ComponentCreator::C_DataRef& pCreationData);
	bool							Initialise(const I_ComponentCreator::C_DataRef& pCreationData, Sys::C_ContextualStream& stream);
	bool							Prime();
	bool							Create();
	void							Kill();

	inline void						InternalDependenciesCreated() {InternalDependenciesCreatedImpl();}
	inline void						ExternalDependenciesCreated() {ExternalDependenciesCreatedImpl();}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

#ifndef CFG_GOLD
	virtual void					DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected:
	virtual void					SetPrimed(); ///< Called by derived classes when all resources have loaded.

	virtual bool					InitialiseImpl();	///< Called when the component lifecycle changes from Constructed.
	virtual bool					PrimeImpl();		///< Called when the component lifecycle changes from Initialised.
	virtual bool					CreateImpl();		///< Called when the component lifecycle changes from Primed.
	virtual void					KillImpl();			///< Called when the component lifecycle ends.

	virtual void					InternalDependenciesCreatedImpl();	///< Called when the containing object and all sibling components have been created.
	virtual void					ExternalDependenciesCreatedImpl();	///< Called when all related objects have been created.

private:
	I_ComponentCreator::C_DataRef	m_pCreator;	///< creation data
	ActorSys::E_LifetimeState		m_LifetimeState;	///< current lifetime state

	void							SetLifetimeState(const ActorSys::E_LifetimeState state) {m_LifetimeState = state;}

};

} // namespace Component


void SerialiseStorage(const DataRef::I_Storage<Component::I_ComponentCreator>& storage, Sys::C_ContextualStream& stream);
void DeserialisePtr(const DataRef::C_StorageTypeId storageType, DataRef::C_Ptr<Component::I_ComponentCreator>& ptr, Sys::C_ContextualStream& stream);


#endif
