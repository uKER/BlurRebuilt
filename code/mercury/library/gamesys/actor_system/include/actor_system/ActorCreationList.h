#ifndef ACTORCREATIONLIST_E61097D1_2A68_4C93_8ADF_DD6B505957A2_H
#define ACTORCREATIONLIST_E61097D1_2A68_4C93_8ADF_DD6B505957A2_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorCreation.h>
#include <actor_system/ActorDeclare.h>
#include <btl/containers/Vector.h>


namespace Actor
{

/// Status tracker for staggered actor creation.
class C_ActorCreationStatus
{
public:
	/// Creation phases
	enum E_Phase
	{
		E_Phase_Begin,					///< initial state
		E_Phase_Construct,				///< constructing actor(s)
		E_Phase_Initialise,				///< initialising actor(s)
		E_Phase_Prime,					///< priming actor(s)
		E_Phase_Create,					///< creating actor(s)
		E_Phase_InternalDependencies,	///< patching up dependencies within actors
		E_Phase_ExternalDependencies,	///< patching up dependencies between actors
		E_Phase_End						///< creation complete
	};

	explicit C_ActorCreationStatus(const U32 maxTasks = 0);

	E_Phase GetPhase() const {return m_Phase;}

	void Begin(const E_Phase defaultPhase);
	void Update(const U32 numTasks, U32& o_StartIndex, U32& o_EndIndex);

private:
	E_Phase	m_Phase;		///< current phase
	U32		m_MaxTasks;		///< maximum number of tasks to process in one chunk, zero for unlimited tasks
	U32		m_NumTasks;		///< tasks remaining
	U32		m_StartTask;	///< task to start on

};

/// Configuration data for manipualting I_ActorCreationList objects
struct C_ActorCreationConfig
{
	/// Possible responses if an operation fails on an actor
	enum E_OnFail
	{
		E_OnFail_Continue,	///< continue operation on remaining actors
		E_OnFail_Abort,		///< stop operation immediately
	};

	E_OnFail	m_OnFail; ///< behaviour when an operation fails for an actor

	C_ActorCreationConfig();
};

/// Interface for creating actors.
class I_ActorCreationList
{
public:
	I_ActorCreationList();
	virtual ~I_ActorCreationList();

	/**
	@returns the number of actors to create.
	*/
	virtual U32 CalcNumCreators() const = 0;

	/**
	@param index value less than CalcNumCreators.
	@returns creation data for this actor.
	*/
	virtual C_ActorCreationData::C_DataRef GetCreator(const U32 index) const = 0;

};

/// Used to create a single actor.
class C_ActorCreationListSingle : public I_ActorCreationList
{
public:
	explicit C_ActorCreationListSingle(const C_ActorCreationData::C_DataRef& creator);
	virtual ~C_ActorCreationListSingle();

	virtual U32 CalcNumCreators() const; // from I_ActorCreationList
	virtual C_ActorCreationData::C_DataRef GetCreator(const U32 index) const; // from I_ActorCreationList

private:
	C_ActorCreationData::C_DataRef m_Creator; ///< creation data for a single actor

};

class C_ActorCreationListMultiple : public I_ActorCreationList
{
public:
	C_ActorCreationListMultiple();
	virtual ~C_ActorCreationListMultiple();

	void ReserveActorCreationListItems(const U32 count); ///< Reserve storage for items
	void AddActorCreationListItem(const C_ActorCreationData::C_DataRef& creator); ///< Add an item

	virtual U32 CalcNumCreators() const; // from I_ActorCreationList
	virtual C_ActorCreationData::C_DataRef GetCreator(const U32 index) const; // from I_ActorCreationList

private:
	typedef Btl::Vector<C_ActorCreationData::C_DataRef> C_Creators;

	C_Creators m_Creators; ///< actors to create

};

/// Used to create a collection of actors.
class C_ActorCreationListComposite : public I_ActorCreationList
{
public:
	C_ActorCreationListComposite();
	virtual ~C_ActorCreationListComposite();

	void AddCreationListComposite(const I_ActorCreationList* pContents); ///< Adds a list of actors
	void RemoveCreationListComposite(const I_ActorCreationList* pContents); ///< Removes a list of actors

	virtual U32 CalcNumCreators() const; // from I_ActorCreationList
	virtual C_ActorCreationData::C_DataRef GetCreator(const U32 index) const; // from I_ActorCreationList

private:
	typedef Btl::Vector<const I_ActorCreationList*> C_Contents;

	C_Contents m_Contents; ///< lists of actors to compose

};

} // namespace Actor


#endif // ACTORCREATIONLIST_E61097D1_2A68_4C93_8ADF_DD6B505957A2_H
