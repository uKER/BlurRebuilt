#include <GameAudio/GameAudioLayer.h>
#include "AppInterfaceLayer.h"

#include <Game\Messaging\GameMessages.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <misc_shared/Messaging/MessageHandler.h>











namespace Game
{
	class C_VehicleImpactMsg;

	namespace Actors
	{
		class C_VehicleActor;
	}

	struct CollisionParams
	{
			CollisionParams() : m_bCollisionThisFrame(false),
								m_fMaxImpulseStrength(0.f), m_fDecayingPeakImpulse(0.f){}

			float m_fMaxImpulseStrength;
			float m_fDecayingPeakImpulse;
			bool m_bCollisionThisFrame;
			bool m_bCollidedWithStaticBody;
			Maths::HVec3	m_Position;
			Maths::HVec3	m_Direction;
			S32 m_MaterialType; 
	};


	class VehicleAudioComponent 
{
public:



	VehicleAudioComponent(){}
	~VehicleAudioComponent(){}

	void Init(Audio::XtEntityDef* def,S32 index);

	void Update();

	AudioEntityHandle GetEntityInstanceHandle(){return m_AudioEntityHandle;}
	
	void ProcessMessage(const Messaging::C_MessageBase *pMessage);
	void LoadResources(){if(vehicleAudioDef)vehicleAudioDef->LoadResources();}
	void FreeResources(){if(vehicleAudioDef)vehicleAudioDef->FreeResources();}
	void CreateEntityInstance();
	void ReleaseEntityInstance(){Audio::InterfaceController::Get().ReleaseEntity(m_AudioEntityHandle);FreeResources();}
	void SetAudioDefinition(Audio::XtEntityDef* def){vehicleAudioDef = def;}
	void SetVehicleIndex(S32 index){m_vehicleIndex = index;}
	void UpdateOrientation(Maths::HMat43 objectMatrix,RacingPhysics::Vector3R objectVelocity,const AudioEntityHandle handle);
	void HandleCollision(const Game::Actors::C_ImpactMsg* p_pMsg);
	void TriggerPickUpPickedUp(U8 vehicleIndex,const E_PICKUP_TYPE ePickupType);
	//virtual bool	MessageHandler(const Messaging::C_MessageBase *pMessage);



private:

	AudioEntityHandle				m_AudioEntityHandle;
	Game::Actors::C_VehicleActor	*pVehicle;
	const RLVehiclePickupStore		*pVehiclePickupStore;
	Audio::XtEntityDef*				vehicleAudioDef;
	S32								m_vehicleIndex;
	CollisionParams					m_collideParams;
	 

	

};
}
