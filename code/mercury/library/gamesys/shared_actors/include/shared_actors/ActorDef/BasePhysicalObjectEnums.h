
#ifndef BASEPHYSICALOBJECTENUMS_H
#define BASEPHYSICALOBJECTENUMS_H

#ifdef _MSC_VER
#pragma once
#endif


namespace BasePhysicalObject
{

// Flags for base physical object:
enum Flags
{
	FlagBlobShadow						= 0x0001,
	FlagBlobShadowOverrideRadius		= 0x0002,
	FlagContinuousCollision				= 0x0004,
	FlagMakeStaticOnSettled				= 0x0008,
	FlagRemoveBodyOnSettled				= 0x0010,
	FlagCollideWithParent				= 0x0020,
	FlagReflectDetachImpulse			= 0x0040
};

// Flags for PhysObj element damage levels:
enum ElementDamageLevelFlags
{
	FlagElementDamageLevelIndestructible					= 0x0001,
	FlagElementDamageLevelNoShadow							= 0x0002,
	FlagElementDamageLevelEnablePivot						= 0x0004,
	FlagElementDamageLevelMakeDynamicOnBreak				= 0x0008,
	FlagElementDamageLevelDetachOnBreak						= 0x0010,
};


// Physical body type enum
enum E_BodyType
{
	E_BodyType_Dynamic = 0,
	E_BodyType_Static,
	E_BodyType_Keyframed
};


} // namespace BasePhysicalObject

#endif
