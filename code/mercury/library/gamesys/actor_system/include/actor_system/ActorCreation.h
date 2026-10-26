#ifndef ACTORCREATION_H
#define ACTORCREATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <misc_shared/DataRef/Ptr.h>
#include <misc_shared/DataRef/Sorcery.h>
#include <misc_shared/uid/TypeChecker.h>
#include <system/Debug/I_DebugRender.h>

namespace Sys
{
	class C_ContextualStream;
}


namespace Actor
{

/// Actor creation interface.
class I_ActorCreationData
{
public:
	virtual ~I_ActorCreationData() {}

	/**
	@brief Identifies the type of actor to create.
	Each actor class is uniquely identified by a user-defined type identifier.
	The implementation of I_ActorCreationData used to create an actor should
	override this method to supply the type identifier of the actor.
	@returns unique type identifier for the actor class to create.
	*/
	virtual const C_ActorTypeId		GetType() const = 0;

	/**
	@brief Identifies the actor by name.
	If it does not make sense to name the actor then a suitable default string
	should be returned.
	@returns name of the actor, not necessarily unique.
	*/
	virtual const char*				GetName() const = 0;

	/**
	@brief Places the actor in the game world.
	If the actor does not have a concept of physical placement then a suitable
	default matrix should be returned.
	@returns .
	*/
	virtual const Maths::HMat43 	GetCreationMatrix() const  = 0;

#ifndef CFG_GOLD
	/**
	@brief Allows the creation object to display debug info.
	@param debugRenderPtr context to render in.
	*/
	virtual void					DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const {}
#endif
};

/// Base concrete actor creation type.
class C_ActorCreationData : public I_ActorCreationData
{
	friend class C_ActorList;

public:
	typedef DataRef::C_Ptr<C_ActorCreationData> C_DataRef;

	// Xt annotated data
	U32								m_SorceryUid;	///< unique value assigned by sorcery

	C_ActorCreationData();
	virtual ~C_ActorCreationData();

	DataRef::C_SorceryUid			GetSorceryUid() const {return static_cast<DataRef::C_SorceryUid>(m_SorceryUid);} ///< @returns unique identifier
	C_ActorHandle					GetActorHandle() const {return m_ActorHandle;} ///< @returns handle of the actor created

	// from I_ActorCreationData
	virtual const char*				GetName() const;
	virtual const Maths::HMat43 	GetCreationMatrix() const;

	virtual void					Serialise(Sys::C_ContextualStream& stream) const;
	virtual void					Deserialise(Sys::C_ContextualStream& stream);

	inline void						DoSorceryCallback(F_ActorCreationDataSorceryCallback pCallbackFn) {pCallbackFn(this);} ///< for DataRef::SorceryCallback<Actor::C_ActorCreationData>
	virtual void					DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn); ///< for DataRef::SorceryCallback<Component::I_ComponentCreator>

	static void						XtCallback(XtEventType eventType, XtRecord* pRecord, C_ActorCreationData* pData, XtCallbackContext* pContext);
	static C_DataRef				FactoryCreate(const C_ActorTypeId actorType, C_ActorCreationData** ppData = NULL, const C_TypeChecker* pTypeChecker = NULL); ///< Wrapper for DataRef::FactoryCreate

	virtual void					LinkActorHandle(const C_ActorHandle& handle) const;

protected:
	static void						DoComponentSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn, Component::I_ComponentCreator& data, DataRef::C_Ptr<Component::I_ComponentCreator>* ppData = NULL);

private:
	mutable C_ActorHandle			m_ActorHandle;	///< handle of created actor

	C_ActorCreationData(const C_ActorCreationData&);				// can not copy
	C_ActorCreationData& operator = (const C_ActorCreationData&);	// can not copy

};

/// Helper class for dynamic actor creation data.
template <class T_ActorCreationData>
class C_ActorCreationDataFactoryHelper
{
public:
	typedef T_ActorCreationData	C_ActorCreationDataType;						///< Class being created.
	static C_ActorTypeId GetType() {return T_ActorCreationData::ClassType();}	///< Type identifier being created.

	C_ActorCreationDataFactoryHelper() : m_DataRef(), m_Data(NULL)
	{
		C_ActorCreationData* pCreationData = NULL;
#ifndef CFG_SPEED
		m_DataRef = C_ActorCreationData::FactoryCreate(GetType(), &pCreationData, GetTypeChecker<T_ActorCreationData>());
#else
		m_DataRef = C_ActorCreationData::FactoryCreate(GetType(), &pCreationData);
#endif
		BIZ_ASSERT((pCreationData != NULL) && (pCreationData->GetType() == GetType()));
		m_Data = static_cast<C_ActorCreationDataType*>(pCreationData);
	}

	~C_ActorCreationDataFactoryHelper() {}

	const C_ActorCreationData::C_DataRef& Get() const {return m_DataRef;} ///< Gets the created data reference.

	C_ActorCreationDataType& operator * () {return *m_Data;} ///< Writable data reference.
	C_ActorCreationDataType* operator -> () {return m_Data;} ///< Writable data pointer.

private:
	C_ActorCreationData::C_DataRef	m_DataRef;	///< Created data reference.
	C_ActorCreationDataType*		m_Data;		///< Writable data pointer.

	C_ActorCreationDataFactoryHelper(const C_ActorCreationDataFactoryHelper&);				// can not copy
	C_ActorCreationDataFactoryHelper& operator = (const C_ActorCreationDataFactoryHelper&);	// can not copy

};

} // namespace Actor


void SerialiseStorage(const DataRef::I_Storage<Actor::C_ActorCreationData>& storage, Sys::C_ContextualStream& stream);
void DeserialisePtr(const DataRef::C_StorageTypeId storageType, DataRef::C_Ptr<Actor::C_ActorCreationData>& ptr, Sys::C_ContextualStream& stream);


#endif // ACTORCREATION_H
