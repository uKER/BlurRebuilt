#ifndef UICAMERACONTROLLER_H
#define UICAMERACONTROLLER_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>
#include <GameLogic/ActorStudioActors/DynamicCameraActor.h>

namespace AmaxCam { class C_DirectorActorCreate; }

namespace Actors
{

class UICameraControllerCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_UICameraController;}

	// Actor Specific
	const AmaxCam::C_DirectorActorCreate*			m_Director;
	S32												m_Viewport;
	const DynamicCameraActorCreationData*			m_Camera;
};

//=========================================================================


class UICameraController : public Game::Actors::C_GameActor
{
public:
	UICameraController();
	virtual ~UICameraController();

	static const Actor::C_ActorTypeId			TypeId()		{ return Game::E_ActorTypeId_UICameraController; }
	const UICameraControllerCreationData*		GetCreationData() const {return static_cast<const UICameraControllerCreationData*>(C_GameActor::GetCreationData().GetData());}
	virtual const Maths::HMat43&				GetMatrix(void) const {return m_WorldMatrix;}
	virtual const Maths::C_Vertex				GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);

protected:
	virtual bool								InitialiseImpl();
	virtual bool								CreateImpl();


private:
	Maths::HMat43								m_WorldMatrix;

};

}// End namespace Actors


#endif // UICAMERACONTROLLER_H
