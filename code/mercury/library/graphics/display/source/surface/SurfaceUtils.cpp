#include "stdafx.h"
#include "Display/Surface/SurfaceUtils.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{

U32 GetSurfaceSize(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample)
{
	return Display2::GetSurfaceSize(width, height, flags, GetSurfaceFormatFromFormat(format), (Display2::E_Multisample)multisample);
}

}; // namespace Display
