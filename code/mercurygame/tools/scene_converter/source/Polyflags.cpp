#include "stdafx.h"

#include <Polyflags.h>

void GetPolyflagItemNames( Btl::Vector< Util::C_String >& polyFlagItemNames )
{
	polyFlagItemNames.Resize( E_Polyflag_Count );
	polyFlagItemNames[E_Polyflag_Invisible] =				"Invisible";
	polyFlagItemNames[E_Polyflag_DoNotLight] =				"DoNotLight";
	polyFlagItemNames[E_Polyflag_DoubleSided] =				"DoubleSided";
	polyFlagItemNames[E_Polyflag_NoBackfaceCulling] =		"NoBackfaceCulling";
	polyFlagItemNames[E_Polyflag_EffectMaterial] =			"EffectMaterial";
	polyFlagItemNames[E_Polyflag_PhysicsMaterial] =			"PhysicsMaterial";
	polyFlagItemNames[E_Polyflag_CollideFlags] =			"CollideFlags";
	polyFlagItemNames[E_Polyflag_CollideUserData] =			"CollideUserData";
	polyFlagItemNames[E_Polyflag_NoLightSamples] =			"NoLightSamples";
	polyFlagItemNames[E_Polyflag_ForceLightSamples] =		"ForceLightSamples";
	polyFlagItemNames[E_Polyflag_LowDensityLightSamples] =	"LowDensityLightSamples";
	polyFlagItemNames[E_Polyflag_NoPVSSamples] =			"NoPVSSamples";
	polyFlagItemNames[E_Polyflag_ForcePVSSamples] =			"ForcePVSSamples";
	polyFlagItemNames[E_Polyflag_ForceNoPortalCulling] =	"ForceNoPortalCulling";
	polyFlagItemNames[E_Polyflag_CustomPrimitiveFlags] =	"PrimitiveFlags";
	polyFlagItemNames[E_Polyflag_PvsOccluder] =				"PvsOccluder";
	
}

void GetObjectflagItemNames( Btl::Vector< Util::C_String >& objectFlagItemNames )
{
	objectFlagItemNames.Resize( E_Objectflag_Count );
	objectFlagItemNames[E_Objectflag_Element] =				"Element";
	objectFlagItemNames[E_Objectflag_Model] =				"Model";
	objectFlagItemNames[E_Objectflag_NavMesh] =				"NavMesh";
	objectFlagItemNames[E_Objectflag_PortalRooms] =			"PortalRooms";
	objectFlagItemNames[E_Objectflag_PvsZones] =			"PvsZones";
	objectFlagItemNames[E_Objectflag_NoLightSamples] =		"NoLightSamples";
	objectFlagItemNames[E_Objectflag_NoPVSSamples] =		"NoPVSSamples";
	objectFlagItemNames[E_Objectflag_DontExport] =			"DontExport";	
	objectFlagItemNames[E_Objectflag_DontLight] =			"DontLight";
	objectFlagItemNames[E_Objectflag_DontCastShadows] = 	"DontCastShadows";
	objectFlagItemNames[E_Objectflag_WaterMesh] = 			"WaterMesh";	
}
