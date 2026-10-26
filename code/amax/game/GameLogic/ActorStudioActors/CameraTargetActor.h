#ifndef CAMERATARGETACTOR_H
#define CAMERATARGETACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
//#include <shared_actors/XtSharedActorMessages.h>


//=========================================================================
//						CAMERA TARGET ACTOR CLASSES
//=========================================================================



namespace Actors
{
	
class CameraTargetCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
//	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_CameraTarget;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_CameraTarget;}

};


//=========================================================================


class CameraTargetActor : public Game::Actors::C_GameActor
{
public:
	CameraTargetActor();
	virtual ~CameraTargetActor();

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_CameraTarget; }
	const CameraTargetCreationData*		GetCreationData() const {return static_cast<const CameraTargetCreationData*>(C_GameActor::GetCreationData().GetData());}
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


#endif // CAMERATARGETACTOR_H
