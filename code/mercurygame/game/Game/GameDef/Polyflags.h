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

const U32 kVehicleSolid			= 1 << 0; //1
const U32 kWheelSolid			= 1 << 1; //2
const U32 kCharacterSolid		= 1 << 2; //4
const U32 kDynamicObjectSolid	= 1 << 3; //8
const U32 kRagdollSolid			= 1 << 4; //16
const U32 kShootable			= 1 << 5; //32
const U32 kCameraSolid			= 1 << 6; //64
const U32 kPuddle				= 1 << 16; //65536
const U32 kFluid				= 1 << 17; //131072


const U32 kAllSolid				=	kVehicleSolid |
									kWheelSolid |
									kCharacterSolid |
									kDynamicObjectSolid |
									kRagdollSolid |
									kShootable |
									kCameraSolid |
									kPuddle |
									kFluid;									

const Util::C_String GetCollideFlagsName( const U32 flag );

} // namespace PolyFlags
} // namespace Game

#endif // GAME_POLYFLAGS_H
