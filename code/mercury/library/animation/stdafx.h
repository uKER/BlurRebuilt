#if defined (PLATFORM_PC)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <Core/Core.h>
#include <Core/Maths.h>

// Most files involve new maths library.
#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>
//#include <core/maths/hvec/new/IncludeGeneral.h>
//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>

#include <string.h>


// Containers + BTL
#include <Core/Containers.h>
#include <btl/containers/vector.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>

// Often included to access Spu memory functions.
#include <System/Scheduler.h>

#include <core/utility/CoreDebug.h>

#if defined(ANIMATION_TOOL)
#include <Aftk/Aftk.h>
#endif
