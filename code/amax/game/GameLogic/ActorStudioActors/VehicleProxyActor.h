#ifndef VEHICLEPROXYACTOR_H
#define VEHICLEPROXYACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <RaceBook/Data/RaceBookData.h>
#include <RacingPhysics/Vehicles/Vehicle.h>



namespace Game
{
	namespace Actors
	{
		class RaceLogicCore;
	}
}
//=========================================================================
//						VEHICLE PROXY ACTOR CLASSES
//=========================================================================


namespace Actors
{
	
class VehicleProxyCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_VehicleProxy;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_VehicleProxy;}

	XtHandle<RaceBookCharacter>			mCharacter;
	int									mVehicleIndex;

//	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
//	Components::C_MovementCreateInfo*				m_pSorceryMovementComponentCreate;
//	Component::I_ComponentCreator::C_DataRef		m_pMovementCreator;
//	const Components::C_MovementCreateInfo*			GetMovementCreator() const {return static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData());}
//	Components::C_MovementCreateInfo*				GetMovementCreator() {return const_cast<Components::C_MovementCreateInfo*>(static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData()));}
};


//=========================================================================

struct VehicleProxyHingedPart
{
	bool		animating;
	F32			current_angle;
	F32			time_left;
	F32			angle_mod;
};


//=========================================================================

class VehicleProxyActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	VehicleProxyActor();
	virtual ~VehicleProxyActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
//	void								Update();								// Update logic called each frame
//	void								KillThis();								// Cleans up any children etc before killing itself


	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}
	
//	const Maths::C_Vertex				GetDir()		const {return m_WorldMatrix.AZ();}
//	const Maths::C_Vertex				GetUp()			const {return m_WorldMatrix.AY();}
//	const Maths::C_Vertex				GetRight()		const {return m_WorldMatrix.AX();}

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_VehicleProxy; }
	static const Btl::String32			TypeName()		{ return "Vehicle Proxy"; }
	const VehicleProxyCreationData*		GetCreationData() const {return static_cast<const VehicleProxyCreationData*>(C_GameActor::GetCreationData().GetData());}


	Maths::HMat43						m_WorldMatrix;
	int									m_VehicleIndex;

	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
	Game::Actors::RaceLogicCore*		GetRLC();
public:
	Btl::Vector<Actor::C_ActorHandle>	m_Children;
private:
	void								CheckReferences();	// TEMP
	void								SetSpeedRelativeToPoint(Maths::HVec3 pos);

	void								InitialiseProxy();

	void								HingedPart_Init(VehicleProxyHingedPart *part);
	void								HingedPart_Move(VehicleProxyHingedPart *part, F32 deg, F32 duration);
	bool								HingedPart_Update(VehicleProxyHingedPart *part, F32 period);



	Game::Actors::RaceLogicCore*		pRLC;				// Store these after being initialised because they're used everywhere
	Game::Actors::C_VehicleActor*		pVehicleActor;
	
//	bool								applyhandbrake;

	Actor::C_Actor*						relative_actor;			// Actor to drive towards
	Maths::HVec3						relative_offset;		// Position relative to above actor
	F32									aggression;				// How hard to hit the brakes
	bool								relative_speed_only;	// True if we're following a spline but setting the speed relative to another actor

	bool								directcontrol;

	RacingPhysics::CarControls			manual_controls;

	VehicleProxyHingedPart				bonnet;
	VehicleProxyHingedPart				door_left;
	VehicleProxyHingedPart				door_right;
	VehicleProxyHingedPart				window_left;
	VehicleProxyHingedPart				window_right;

	bool								enabled;
};

#ifndef CFG_GOLD
extern Btl::Vector<VehicleProxyActor*> g_VPAList;
#endif

}// End namespace Actors


#endif // VEHICLEPROXYACTOR_H
