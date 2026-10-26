#include "stdafx.h"

#include <game/gamedef/PhysicsConstants.h>

namespace Game
{

namespace PhysicsConstants
{

Collision::C_CollisionFilter	kDynamicObjectCollisionFilter( 0, PolyFlags::kDynamicObjectSolid );
Collision::C_CollisionFilter	kVehicleBodyCollisionFilter( 0, PolyFlags::kVehicleSolid );
Collision::C_CollisionFilter	kVehicleWheelCollisionFilter( 0xffffffff, PolyFlags::kWheelSolid );
Collision::C_CollisionFilter	kVehicleOverlayCollisionFilter( 0xffffffff, PolyFlags::kPuddle );

Collision::C_CollisionFilter	kCharacterCollisionFilter(	PolyFlags::kCharacterSolid | 
														  	PolyFlags::kVehicleSolid |
														  	PolyFlags::kWheelSolid |
														  	PolyFlags::kDynamicObjectSolid |
														  	PolyFlags::kRagdollSolid |
															PolyFlags::kCameraSolid, 
															PolyFlags::kCharacterSolid );

Collision::C_CollisionFilter	kRagdollBodyCollisionFilter( 0, PolyFlags::kRagdollSolid );

F32 kDefaultSlop = 0.000005f;
F32 kDefaultVehicleSlop = 0.00000001f;

F32 kDefaultBaumgarte = 0.8f;

} // namespace PhysicsConstants

} // namespace Game
