#pragma once

//See also tools_common/network_shim/NetworkShimSharedXmlTags.h for macros shared
//between the shim and other tools.

enum E_ShimActionTag
{
	E_ShimActionTag_AftkSceneMultiple,
	E_ShimActionTag_AftkSceneCombine,
	E_ShimActionTag_AftkSceneSingle,

	E_ShimActionTag_Unknown
};

enum E_ShimMode
{
	E_ShimMode_Action,
	E_ShimMode_Ubercopy,
	E_ShimMode_Scm,

	E_ShimActionType_Unknown
};
