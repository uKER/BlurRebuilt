#ifndef TVCAMERACONTROL_H
#define TVCAMERACONTROL_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>


//=========================================================================
//						TV CAMERA CONTROL CLASSES
//=========================================================================



namespace Actors
{


class TVCameraControlCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	const Actor::C_ActorTypeId					GetType()		const {return Game::E_ActorTypeId_TVCameraControl;}

	XtSphere									m_Region;
	ActorMsg::C_XtChangeCameraBehaviourMsg		*m_CameraMessage;
	ActorMsg::C_XtChangeCameraBehaviourMsg		*m_OptionalSecondMessage;
};

//=========================================================================


class TVCameraControlActor : public Game::Actors::C_GameActor
{
public:
	TVCameraControlActor();
	virtual ~TVCameraControlActor();

	static const Actor::C_ActorTypeId			TypeId()		{ return Game::E_ActorTypeId_TVCameraControl; }
	const TVCameraControlCreationData*			GetCreationData() const {return static_cast<const TVCameraControlCreationData*>(C_GameActor::GetCreationData().GetData());}
	virtual const Maths::HMat43&				GetMatrix(void) const {return m_WorldMatrix;}
	virtual const Maths::C_Vertex				GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);
	void										Initialise();
	bool										PlayerInRegion();


protected:
	virtual bool								InitialiseImpl();
	virtual bool								CreateImpl();

private:
	Maths::HMat43								m_WorldMatrix;

	bool										m_Active;
	Maths::HVec3								m_Centre;
	F32											m_Radius2;

	Game::Actors::C_VehicleActor				*m_PlayerVehicle;
};

}// End namespace Actors


#endif // TVCAMERACONTROL_H
