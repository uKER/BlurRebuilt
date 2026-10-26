#ifndef APP_POLYFLAGS_H
#define APP_POLYFLAGS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

enum E_Polyflag
{
	E_Polyflag_Invisible = 0,
	E_Polyflag_DoNotLight,
	E_Polyflag_DoubleSided,
	E_Polyflag_NoBackfaceCulling,
	E_Polyflag_EffectMaterial,
	E_Polyflag_PhysicsMaterial,
	E_Polyflag_CollideFlags,
	E_Polyflag_CollideUserData,
	E_Polyflag_NoLightSamples,
	E_Polyflag_ForceLightSamples,
	E_Polyflag_LowDensityLightSamples,
	E_Polyflag_NoPVSSamples,
	E_Polyflag_ForcePVSSamples,
	E_Polyflag_ForceNoPortalCulling,
	E_Polyflag_CustomPrimitiveFlags,
	E_Polyflag_PvsOccluder,
	E_Polyflag_Count
};

void GetPolyflagItemNames( Btl::Vector< Util::C_String >& polyFlagItemNames );

enum E_Objectflag
{
	E_Objectflag_Element = 0,
	E_Objectflag_Model,
	E_Objectflag_NavMesh,
	E_Objectflag_PortalRooms,
	E_Objectflag_PvsZones,
	E_Objectflag_NoLightSamples,	
	E_Objectflag_NoPVSSamples,
	E_Objectflag_DontExport,
	E_Objectflag_DontLight,
	E_Objectflag_DontCastShadows,
	E_Objectflag_WaterMesh,
	E_Objectflag_Count
};

void GetObjectflagItemNames( Btl::Vector< Util::C_String >& objectFlagItemNames );

#endif
