 #include "stdafx.h"

#include <game/gamedef/PhysicsConstants.h>

namespace Game
{

namespace PhysicsConstants
{
//Kier - changed this so that I can put in the kVehicleBlockCollisionFilter flag for all dynamic objects!
Collision::C_CollisionFilter	kDynamicObjectCollisionFilter( PolyFlags::kVehicleBlocker, PolyFlags::kDynamicObjectSolid, 0, 0, 0, PolyFlags::kbDynamicObject );
Collision::C_CollisionFilter	kVehicleWheelCollisionFilter( 0xffffffff, PolyFlags::kWheelSolid );
Collision::C_CollisionFilter	kVehicleOverlayCollisionFilter( 0xffffffff, PolyFlags::kPuddle | PolyFlags::kWackyMagic );

//Vehicle body, collides with everything other than WheelSolid and Vehicles
Collision::C_CollisionFilter	kVehicleBodyCollisionFilter( 0, PolyFlags::kVehicleSolid, 0, 0, 0, PolyFlags::kbVehicle, PolyFlags::kbWorld );

//The old "skirt" for the car, used for dynamic object collisions only
Collision::C_CollisionFilter	kVehicleBlockCollisionFilter( PolyFlags::kAllSolid & (~PolyFlags::kWheelSolid), PolyFlags::kVehicleBlocker, 0, 0, 0, PolyFlags::kbVehicle );

//The new vehicle-and-dynamic-object-only collision filter
Collision::C_CollisionFilter	kVehicleVehicleCollisionFilter( PolyFlags::kVehicleBlocker |
															   PolyFlags::kDynamicObjectSolid, PolyFlags::kVehicleBlocker, 0, 0, 0, PolyFlags::kbVehicle, PolyFlags::kbVehicle | PolyFlags::kbDynamicObject | PolyFlags::kbPerk | PolyFlags::kbOuterMine );

Collision::C_CollisionFilter	kCharacterCollisionFilter(	PolyFlags::kCharacterSolid | 
														  	PolyFlags::kVehicleSolid |
														  	PolyFlags::kWheelSolid |
														  	PolyFlags::kDynamicObjectSolid |
														  	PolyFlags::kRagdollSolid |
															PolyFlags::kCameraSolid, 
															PolyFlags::kCharacterSolid );

Collision::C_CollisionFilter	kGroundPlaneFilter( PolyFlags::kCharacterSolid | 
														  	PolyFlags::kVehicleSolid |
														  	PolyFlags::kWheelSolid |
														  	PolyFlags::kDynamicObjectSolid |
														  	PolyFlags::kRagdollSolid |
															PolyFlags::kCameraSolid, 
															PolyFlags::kDynamicObjectSolid | 
															PolyFlags::kWheelSolid |
															PolyFlags::kVehicleSolid | 
															PolyFlags::kCharacterSolid);

Collision::C_CollisionFilter	kRagdollBodyCollisionFilter( 0, PolyFlags::kRagdollSolid );

Collision::C_CollisionFilter	kPerkCollisionFilter( PolyFlags::kDynamicObjectSolid, 0xffffffff, 0, 0, 0, PolyFlags::kbPerk );


F32 kDefaultSlop = 0.000005f;
F32 kDefaultVehicleSlop = 0.00000001f;

F32 kDefaultBaumgarte = 0.8f;
F32 kVehicleBaumgarte = 2.f;

} // namespace PhysicsConstants

} // namespace Game