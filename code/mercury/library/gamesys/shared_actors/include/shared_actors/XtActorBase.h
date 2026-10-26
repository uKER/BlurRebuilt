#ifndef XT_ACTOR_BASE_H
#define XT_ACTOR_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>
#include <core/xtstdx.h>
#include <actor_system/ActorCreation.h>

namespace Actors
{
// Debug Logger
extern Debug::Logger &g_Log; 

// This is the base type that gets created in the editor
// This actor has a name but no position
class C_GameActorCreate : public Actor::C_ActorCreationData
{
public:
	C_GameActorCreate() {}
	virtual ~C_GameActorCreate() {}

	virtual const char *			GetName() const 	{return m_Name;}

public:
	XtSz8								m_Name;			///< name assigned in Editor
	bool								m_IsCommented;	///< 'Commented out' in the editor - don't create

private:
	C_GameActorCreate(const C_GameActorCreate&);				// can not copy
	C_GameActorCreate& operator = (const C_GameActorCreate&);	// can not copy

};

// This is the base actor type that has a position
class C_PosActorCreate : public C_GameActorCreate
{
public:
	C_PosActorCreate() {}
	virtual ~C_PosActorCreate() {}

	virtual const Maths::HMat43 	GetCreationMatrix() const {return Load(m_WorldMatrix);}
#ifndef CFG_GOLD
	virtual void		DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
	{
		Maths::HMat43 worldMat = GetCreationMatrix();
		debugRenderPtr->Draw3dText(worldMat.AW(),Util::C_String(m_Name));
		debugRenderPtr->Draw3dMatrix(worldMat);
	}
#endif
public:
	XtMat43		m_WorldMatrix;

private:
	C_PosActorCreate(C_PosActorCreate const&);				// can not copy
	C_PosActorCreate& operator = (C_PosActorCreate const&);	// can not copy

};

} //namespace Actors

#endif

