#ifndef XTACTORCREATION_H
#define XTACTORCREATION_H

#ifdef _MSC_VER
#pragma once
#endif

// This file contains all the classes needed to load and create an actor list from ActorVision
// TODO: This file needs to go into Mercury and is almost ready to do so

#include <Xt/XtGame.h>
#include <core/xtstdx.h>
#include <actor_system/ActorCreation.h>
#include <actor_system/ActorCreationList.h>
#include <shared_actors/XtActorBase.h>

class C_LevelPlatform
{
public:
	XtSz8			m_PlatformName;
};

namespace XtActorCreation
{

bool CreateActors(const Actor::I_ActorCreationList* pCreationList, Actor::C_ActorList* pActorList);

#ifndef CFG_GOLD
void DebugRender(const Actor::I_ActorCreationList* pCreationList, DbUtil::I_DebugRenderPtr debugRenderPtr);
#endif

} // namespace XtActorCreation

namespace Level
{
	class C_GameData
	{
	};
}


// Level Actor List
// This is the record which is output from ActorVision and contains all the 
// actor information for the actors which have been placed.
class C_LevelActorCreationList : public Actor::I_ActorCreationList
{
public:
								C_LevelActorCreationList(){}

	bool						CreateActors(Actor::C_ActorList *pActorList) {return XtActorCreation::CreateActors(this, pActorList);} // TODO: [Oscar] deprecate this method, caller should manage actor priming
#ifndef CFG_GOLD
	void						DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const {return XtActorCreation::DebugRender(this, debugRenderPtr);}
#endif

public:
	XtArray<Actors::C_GameActorCreate*>	m_LevelActors;		///> Array of level actors
	XtArray<C_LevelPlatform>			m_LevelPlatforms;	///> Array of platforms (not used yet)
	XtArray<Level::C_GameData*>			m_LevelData;		///> Array of level data

	void DoSorceryCallback(Actor::F_ActorCreationDataSorceryCallback pCallbackFn); ///< for DataRef::SorceryCallback
	void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn); ///< for DataRef::SorceryCallback

	virtual U32 CalcNumCreators() const; // from I_ActorCreationList
	virtual Actor::C_ActorCreationData::C_DataRef GetCreator(const U32 index) const; // from I_ActorCreationList

	typedef void (*F_CustomSorceryCallback)(C_LevelActorCreationList* pLevelActorCreationList, void* pExtraData);
	static F_CustomSorceryCallback	s_pCustomSorceryCallbackFn;
	static void*					s_pCustomSorceryCallbackData;

};

// Level actor creation data
// This contains the functions and data required to load 
// and hold an actor list in memory
class C_LevelActorCreationData : public Actor::C_ActorCreationListComposite
{
public:
									C_LevelActorCreationData();
									C_LevelActorCreationData(XtHandle<C_LevelActorCreationList> hRecord);
									~C_LevelActorCreationData();
	bool							Load(XtDb* pXtDb,Sys::I_FileSystem &fileSys, const Util::C_String &fullPath);
	void							Free(void);
	bool							CreateActors(Actor::C_ActorList *pActorList) {return XtActorCreation::CreateActors(this, pActorList);} // TODO: [Oscar] deprecate this method, caller should manage actor priming
#ifndef CFG_GOLD
	void							DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const {return XtActorCreation::DebugRender(this, debugRenderPtr);}
#endif

	XtId							GetActorListRecordId() const;

private:
	XtHandle<C_LevelActorCreationList>	m_LevelActorCreationList;	///> Handle to actor list Xt record
	XtPack*								m_LevelActorCreationPack;	///> Pointer to Xt PAK file
	XtDb*								m_pXtDb;

};

// This contains all the current actor lists which have been loaded
class C_LevelActorCreationDataList : public Actor::C_ActorCreationListComposite
{
public:
			C_LevelActorCreationDataList();
			~C_LevelActorCreationDataList();

	bool	AddActorListFromFile(XtDb* pXtDb, Sys::I_FileSystem &fileSys, const Util::C_String &fullPath);
	bool	AddActorListFromRecord(XtHandle<C_LevelActorCreationList> hRecord);
	void	Free(void);
	bool	CreateAllActors(Actor::C_ActorList *pActorList) {return XtActorCreation::CreateActors(this, pActorList);} // TODO: [Oscar] deprecate this method, caller should manage actor priming

#ifndef CFG_GOLD
	void	DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const {return XtActorCreation::DebugRender(this, debugRenderPtr);}
#endif

private:
	Btl::Vector<C_LevelActorCreationData*>	m_LevelCreationDataList;

	bool	AddActorList(C_LevelActorCreationData* pActorList);

};


#endif
