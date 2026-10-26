#ifndef SHAREDACTORS_PHYSICSBODYUSERDATA_H
#define SHAREDACTORS_PHYSICSBODYUSERDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <game_physics/BodyUserDataImpl.h>


namespace Actor
{
	class C_Actor;
}

namespace Components
{
	class I_PhysicsBody;
}


namespace Actors
{

/// Base user data for physics bodies owned by actors.
class C_PhysicsBodyUserData : public GamePhysics::C_BodyUserData
{
public:
	const Actor::C_Actor* GetOwner() const {return m_pOwner;} ///< @returns the actor that owns the physics body we're associated with

protected:
	C_PhysicsBodyUserData(const U32 typeUid);
	~C_PhysicsBodyUserData();

	void SetOwner(const Actor::C_Actor* pOwner) {m_pOwner = pOwner;} ///< @param pOwner actor that owns the physics body.

private:
	const Actor::C_Actor* m_pOwner; ///< weak ref, ok to store actor ptr instead of handle as our lifetime is governed by the owner

	C_PhysicsBodyUserData(const C_PhysicsBodyUserData& copy); // can not copy
	C_PhysicsBodyUserData& operator = (const C_PhysicsBodyUserData& copy); // can not copy

};

/// User data for physics bodies owned by actors requiring no special attention.
class C_SimplePhysicsBodyUserData : public GamePhysics::C_BodyUserDataImpl<C_SimplePhysicsBodyUserData, C_PhysicsBodyUserData>
{
public:
	C_SimplePhysicsBodyUserData(); ///< not for general consumption, use Create instead.
	~C_SimplePhysicsBodyUserData(); ///< not for general consumption, use Destroy instead.

	static C_SimplePhysicsBodyUserData* Create(const Actor::C_Actor* pOwner);
	static void Destroy(C_SimplePhysicsBodyUserData* pUserData);

private:
	C_SimplePhysicsBodyUserData(const C_SimplePhysicsBodyUserData& copy); // can not copy
	C_SimplePhysicsBodyUserData& operator = (const C_SimplePhysicsBodyUserData& copy); // can not copy

};

/// User data for physics bodies owned by actors using I_PhysicsBody components.
class C_PBCUserData : public GamePhysics::C_BodyUserDataImpl<C_PBCUserData, C_PhysicsBodyUserData>
{
public:
	C_PBCUserData(); ///< not for general consumption, use Create instead.
	~C_PBCUserData(); ///< not for general consumption, use Destroy instead.

	static C_PBCUserData* Create(const Actor::C_Actor* pOwner, const Components::I_PhysicsBody* pPBC);
	static void Destroy(C_PBCUserData* pUserData);

	const Components::I_PhysicsBody* GetPBC() const {return m_pPBC;} ///< @returns the Physics Body Component that owns this data

private:
	const Components::I_PhysicsBody* m_pPBC; ///< weak ref, ok to store ptr as our lifetime is governed by the PBC

	C_PBCUserData(const C_PBCUserData& copy); // can not copy
	C_PBCUserData& operator = (const C_PBCUserData& copy); // can not copy

};

} // namespace Actors


#endif // SHAREDACTORS_PHYSICSBODYUSERDATA_H
