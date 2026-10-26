#include "stdafx.h"
#include "Display/Manager/GCM/GCMDebug.h"

namespace Display
{
namespace GCM
{

void GCMCheckCallback(uint32_t* cmd)
{
	LOGFATAL(g_Log, "ERROR: Bad command 0x%X at memory location 0x%x ###\n", *cmd, cmd);
	
	BIZ_ASSERT_FAIL("ERROR: Bad command building gcm command buffer");
}

void GCMEnableDebug()
{
	#if defined(BUILD_CONFIG_DEBUG) && defined(CELL_GCM_DEBUG)
	gCellGcmDebugCheckCallback = GCMCheckCallback;
	cellGcmDebugCheckEnable(CELL_GCM_TRUE);
	#endif
}

void GCMDisableDebug()
{
	#if defined(BUILD_CONFIG_DEBUG) && defined(CELL_GCM_DEBUG)
	gCellGcmDebugCheckCallback = NULL;
	cellGcmDebugCheckEnable(CELL_GCM_FALSE);
	#endif
}

}; // namespace GCM
}; // namespace Display
