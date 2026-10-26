#include "stdafx.h"

#include <Polyflags.h>

void GetPolyflagItemNames( Btl::Vector< Util::C_String >& polyFlagItemNames )
{
	polyFlagItemNames.PushBack( Util::C_String("Invisible") );
	polyFlagItemNames.PushBack( Util::C_String("DoNotLight") );
	polyFlagItemNames.PushBack( Util::C_String("DoubleSided") );
	polyFlagItemNames.PushBack( Util::C_String("NoBackfaceCulling") );
	polyFlagItemNames.PushBack( Util::C_String("CollideMaterial") );
	polyFlagItemNames.PushBack( Util::C_String("CollideFlags") );
	polyFlagItemNames.PushBack( Util::C_String("CollideUserData") );
}

void GetObjectflagItemNames( Btl::Vector< Util::C_String >& objectFlagItemNames )
{
	objectFlagItemNames.PushBack( Util::C_String("Element") );
	objectFlagItemNames.PushBack( Util::C_String("Model") );
	objectFlagItemNames.PushBack( Util::C_String("NavMesh") );
	objectFlagItemNames.PushBack( Util::C_String("PortalRooms") );
	objectFlagItemNames.PushBack( Util::C_String("PvsZones") );
	objectFlagItemNames.PushBack( Util::C_String("NoLightSamples") );
	objectFlagItemNames.PushBack( Util::C_String("DontLight") );
	objectFlagItemNames.PushBack( Util::C_String("DontCastShadows") );
}
