#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

//#include "gravity/sdk/body.h"
#include "gravity/narrowphase/Ray.h"

namespace Collision
{
	class C_CharacterControllerShape;
}

namespace Gravity
{

	/*
	 * Character controller permits you to control a character.
	 * Provides 4 ways to do this:
	 * (1) Set target position - attempt to place the character controller in a given position
	 * (2) Set target velocity - attempt to set the character controller's velocity
	 * (3) Set target acceleration - attempt to set the character controller's acceleration
	 * (4) Add force - apply a force to the character controller
	 *
	 * The order of precedence follows from case 1 - 4, and at most one of these will be applied per frame.
	 * This means that, for example, if you set the position and velocity, only the set position command will 
	 * be processed.
	 * 
	 * All commands are limitied by the max speed and max acceleration issued to the character controller 
	 * on initialisation.
	 *
	 * In the event that no commands are issued on the character controller, the default behaviour is that
	 * the character controller attempts to come to a halt
	 */

	class C_Body;
	class C_Scene;

	class C_CharacterController
	{
		friend class C_Scene;
		enum E_Controller_State
		{
			E_Normal = 0,
			E_Override_Velocity = 1,
			E_Override_Position = 2,
			E_Override_Acceleration = 4,
			E_External_Force = 8,
			E_Jumping = 16,
			E_Teleport = 32,
			E_NoRotationConstraint = 64,
			E_FlyMode = 128,
			E_Climbing = 256
		};

		C_Body* m_pCharacterBody;
		Collision::C_CharacterControllerShape* m_pCollisionShape;
	public: //[SteveP] had to make these public until accessors and TeleportWithCollide() are implemented :(
		Maths::HF32 m_MaxSpeed;
		Maths::HF32 m_MaxAcceleration;
	private:
		Maths::HVec3 m_TargetVelocity;
		Maths::HVec3 m_TargetAngularVelocity;
		Maths::HVec3 m_TargetPosition;
		Maths::C_Quaternion m_TargetOrientation;
		Maths::HVec3 m_TargetAcceleration;
		Maths::HVec3 m_TargetAngularAcceleration;
		Maths::HVec3 m_ExternalForce;
		Maths::HF32 m_JumpSpeed;
		Maths::HF32 m_OriginalMinPenetration;
		Maths::HF32 m_OriginalRayLength;
	public://[SteveP] had to make this public as need ability to override foot intersection with ground (needs to be much smaller than default)
		Maths::HF32 m_DeadZone;
	private:
		U32 m_ControllerState;
		Collision::C_RayTracingResult m_LastResult;
	public:

		C_CharacterController() : m_pCharacterBody(NULL), m_pCollisionShape(NULL), m_ControllerState(E_Normal),
			m_MaxSpeed(Maths::Constants<Maths::HF32>::Zero()), m_MaxAcceleration(Maths::Constants<Maths::HF32>::Zero()),
			m_TargetVelocity(Maths::Constants<Maths::HVec3>::Zero()), m_TargetAngularVelocity(Maths::Constants<Maths::HVec3>::Zero()),
			m_TargetPosition(Maths::Constants<Maths::HVec3>::Zero()), m_TargetAcceleration(Maths::Constants<Maths::HVec3>::Zero()),
			m_TargetAngularAcceleration(Maths::Constants<Maths::HVec3>::Zero()), m_ExternalForce(Maths::Constants<Maths::HVec3>::Zero()),
			m_JumpSpeed(Maths::Constants<Maths::HF32>::Zero()), m_OriginalMinPenetration(Maths::Constants<Maths::HF32>::Zero()), 
		m_OriginalRayLength(Maths::Constants<Maths::HF32>::Zero()), m_DeadZone(Maths::Constants<Maths::HF32>::Zero())
		{
		}

		C_CharacterController(C_Body* pBody, const Maths::HF32 PCREF maxSpeed, const Maths::HF32 PCREF maxAcceleration);

		void InitialiseConstroller(C_Body* pBody, const Maths::HF32 PCREF maxSpeed, const Maths::HF32 PCREF maxAcceleration);

		void SetTargetPosition(const Maths::HVec3 PCREF position, const Maths::C_Quaternion PCREF rotation);

		void SetTargetVelocity(const Maths::HVec3 PCREF velocity, const Maths::HVec3 PCREF angVel);

		void SetTargetAcceleration(const Maths::HVec3 PCREF acceleration, const Maths::HVec3 PCREF angAccel);

		void Teleport(const Maths::HVec3 PCREF position, const Maths::C_Quaternion PCREF orientation);

		void DisableRotationConstraint();
		
		void EnableRotationConstraint();

		void EnableFlyMode();

		void DisableFlyMode();

		void EnableClimbing();

		void DisableClimbing();

		bool GetOnGroundFlags();

		Maths::HVec3 GetRelativeLinearVelocity() const;

		Maths::HVec3 GetRelativeAngularVelocity();

		void SetFeetMaterial(U16 feetMaterial);

		//This function try to teleport the character controller to the target position, however, it will stop in the place
		//when it hit something. The function will return a t( 0 - 1 ) value
		Maths::HF32 TeleportWithCollision(const Maths::HVec3 PCREF position,
			const Maths::C_Quaternion PCREF orientation);

		void SetRayCollisionFilter(Collision::C_CollisionFilter* pFilter);

		void AddForce(const Maths::HVec3 PCREF force);

		void Update(const Maths::HF32 PCREF dt);

		void Jump(const Maths::HF32 PCREF jumpSpeed);

		Maths::HVec3 Position() const;

		C_Body* GetBody() const;

		void SetBody(C_Body* pBody);

		void ResetState();
		void ResetMinPenetration();

		Collision::C_RayTracingResult& GetFeetCollisions();

		bool operator < (const C_CharacterController& otherController) const;

		bool operator <= (const C_CharacterController& otherController) const;

		bool operator > (const C_CharacterController& otherController) const;

		bool operator >= (const C_CharacterController& otherController) const;

		bool operator == (const C_CharacterController& otherController) const;

		bool operator != (const C_CharacterController& otherController) const;

	};
};

#endif
