#ifndef SPLITLINEACTOR_H
#define SPLITLINEACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>
#include "ui/Graphics/Localisation/Loc.h"

#include <SafeString.inl>
#include <shared_actors/XtActorBase.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>


#include "Game/ActorDef/Actortypes.h"

namespace Actors
{
//Used to create the actor
class SplitLineCreate : public C_PosActorCreate
{
public:
	const Actor::C_ActorTypeId					GetType() const			{return Game::E_ActorTypeId_SplitLine;}

	S32											mSplitNumber;
//	SplitLineCreate*							m_pOtherPoint;			// Was never used
};

//Split Line Actor
class SplitLineActor : public Actor::C_Actor
{
public:
	SplitLineActor(){}

	static const Actor::C_ActorTypeId			TypeId()				{ return Game::E_ActorTypeId_SplitLine; }
	static const Actor::C_ActorTypeName			TypeName()				{ return "SplitLine"; }
	bool										InitialiseImpl();

	const SplitLineCreate*						GetCreationData() const {return static_cast<const SplitLineCreate*>(C_Actor::GetCreationData().GetData());}
	const Maths::HMat43&						GetMatrix() const		{return mWorldMatrix;}
	const Maths::C_Vertex						GetPosition() const		{return mWorldMatrix.AW();}
	const Maths::C_Vector						GetDirection() const	{return mWorldMatrix.AZ();}
	const Maths::C_Vertex						GetUp() const			{return mWorldMatrix.AY();}
	const Maths::C_Vertex						GetRight() const		{return mWorldMatrix.AX();}

//	const SplitLineCreate*						GetOtherPoint() const	{return (static_cast<const SplitLineCreate*>(C_Actor::GetCreationData().GetData()))->m_pOtherPoint;}

	const S32									GetSplitNumber() const	{ return GetCreationData()->mSplitNumber; }

	void										BroadcastHandle() const;

	bool										OnMessage(const Messaging::C_MessageBase *pMessage);

private:
	Maths::HMat43								mWorldMatrix;
	bool										mDisplay;
};
}; //Namespace actors
#endif //SPLITLINEACTOR_H
