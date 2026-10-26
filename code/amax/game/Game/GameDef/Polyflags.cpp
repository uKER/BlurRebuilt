#include "stdafx.h"

#include <game/gamedef/Polyflags.h>

namespace Game
{

namespace PolyFlags
{


void AppendWithNewline(Util::C_String& string, const Util::C_String& newItem)
{
	if(string.Length() > 0)
	{
		string = string + "\n" + newItem;
	}
	else
	{
		string = newItem;
	}
}

const Util::C_String GetCollideFlagsName( const U32 flag )
{
	Util::C_String result;

	if(( flag & kVehicleSolid ) != 0) AppendWithNewline(result, "Vechicle Solid");
	if(( flag & kWheelSolid ) != 0) AppendWithNewline(result, "Wheel Solid");
	if(( flag & kCharacterSolid ) != 0) AppendWithNewline(result, "Character Solid");
	if(( flag & kDynamicObjectSolid ) != 0) AppendWithNewline(result, "Dynamic Object Solid");
	if(( flag & kRagdollSolid ) != 0) AppendWithNewline(result, "Ragdoll Solid");
	if(( flag & kShootable ) != 0) AppendWithNewline(result, "Shootable");
	if(( flag & kCameraSolid ) != 0) AppendWithNewline(result, "Camera Solid");
	if(( flag & kReflectionPlane ) != 0) AppendWithNewline(result, "Reflection Plane");	
	if(( flag & kPuddle ) != 0) AppendWithNewline(result, "Puddle");
	if(( flag & kVehicleBlocker ) != 0 ) AppendWithNewline(result, "VehicleBlocker");
	if(( flag & kFluid ) != 0 ) AppendWithNewline(result, "Fluid");
	if(( flag & kWackyMagic ) != 0 ) AppendWithNewline(result, "WackyMagic");
	
	return result;
}

} // namespace PolyFlags
} // namespace Game
