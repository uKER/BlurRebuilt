#ifndef GAME_POLYFLAGS_H
#define GAME_POLYFLAGS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

namespace PolyFlags
{

// Bit masks for polyflags //

const U32 kVehicleSolid			= 1 << 0;
const U32 kWheelSolid			= 1 << 1;
const U32 kCharacterSolid		= 1 << 2;
const U32 kDynamicObjectSolid	= 1 << 3;
const U32 kRagdollSolid			= 1 << 4;
const U32 kShootable			= 1 << 5;
const U32 kCameraSolid			= 1 << 6;
const U32 kVehicleBlocker		= 1 << 7;
const U32 kReflectionPlane		= 1 << 8;
const U32 kPuddle				= 1 << 16;
const U32 kFluid				= 1 << 17;
const U32 kWackyMagic			= 1 << 18;



const U32 kAllSolid				=	kVehicleSolid |
									kWheelSolid |
									kCharacterSolid |
									kDynamicObjectSolid |
									kRagdollSolid |
									kShootable |
									kCameraSolid |
									kReflectionPlane |
									kPuddle |
									kFluid;									

const U32 kSolidNoLiquid		=	kVehicleSolid |
									kWheelSolid |
									kCharacterSolid |
									kDynamicObjectSolid |
									kRagdollSolid |
									kShootable |
									kCameraSolid |
									kReflectionPlane;

//Body flags

const U32 kbWorld				= 1 << 0;
const U32 kbVehicle				= 1 << 1;
const U32 kbDynamicObject		= 1 << 2;
const U32 kbPerk				= 1 << 3;
const U32 kbOuterMine			= 1 << 4;

const Util::C_String GetCollideFlagsName( const U32 flag );

} // namespace PolyFlags
} // namespace Game

#endif // GAME_POLYFLAGS_H
