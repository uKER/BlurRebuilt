#ifndef STARTPOSITIONACTOR_H
#define STARTPOSITIONACTOR_H

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
class StartPositionCreate : public C_PosActorCreate
{
public:
	const Actor::C_ActorTypeId					GetType() const			{return Game::E_ActorTypeId_StartPosition;}

	S32											mStartNumber;
//	S32											mStartType;
};

// Start Position Actor
class StartPositionActor : public Actor::C_Actor
{
public:
	StartPositionActor(){}

	static const Actor::C_ActorTypeId			TypeId()				{ return Game::E_ActorTypeId_StartPosition; }
	static const Actor::C_ActorTypeName			TypeName()				{ return "StartPosition"; }
	bool										InitialiseImpl();

	const StartPositionCreate*					GetCreationData() const {return static_cast<const StartPositionCreate*>(C_Actor::GetCreationData().GetData());}
	const Maths::HMat43&						GetMatrix() const		{return mWorldMatrix;}
	const Maths::C_Vertex						GetPosition() const		{return mWorldMatrix.AW();}
	const Maths::C_Vector						GetDirection() const	{return mWorldMatrix.AZ();}
	const Maths::C_Vertex						GetUp() const			{return mWorldMatrix.AY();}
	const Maths::C_Vertex						GetRight() const		{return mWorldMatrix.AX();}
	const S32									GetStartNumber() const	{ return GetCreationData()->mStartNumber; }

	bool										OnMessage(const Messaging::C_MessageBase *pMessage);

private:
	Maths::HMat43								mWorldMatrix;
	bool										mDisplay;
};

}; //namespace Actors

#endif //STARTPOSITIONACTOR_H
