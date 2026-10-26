#ifndef CAMERAPOSITIONACTOR_H
#define CAMERAPOSITIONACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
//#include <shared_actors/XtSharedActorMessages.h>


//=========================================================================
//						CAMERA POSITION ACTOR CLASSES
//=========================================================================



namespace Actors
{
	
class CameraPositionCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
//	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_CameraPosition;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_CameraPosition;}

};


//=========================================================================


class CameraPositionActor : public Game::Actors::C_GameActor
{
public:
	CameraPositionActor();
	virtual ~CameraPositionActor();

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_CameraPosition; }
	const CameraPositionCreationData*	GetCreationData() const {return static_cast<const CameraPositionCreationData*>(C_GameActor::GetCreationData().GetData());}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}
	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
	


protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();



private:
	Maths::HMat43						m_WorldMatrix;
};

}// End namespace Actors


#endif // CAMERAPOSITIONACTOR_H
