#include "ResourceUtilsPch.h"

#include <resource_utils/CycleResourceDef.h>
#include <resource_utils/GameModel.h>
#include <resource_utils/GameTexture.h>
#include <resource_utils/ResourceUtilsDeclare.h>
#include <resource_utils/SkeletonResourceDef.h>


XT_TYPE(ResourceUtils::E_GameFileFlags, "GameFileFlags")
	XT_FLAG_STORAGE(XtU32)
	XT_FLAG(ResourceUtils::E_GameFileFlag_DownloadableContent, "DownloadableContent")
XT_END_TYPE

XT_LIBRARY(ResourceUtils)
	XT_REGISTER_TYPE(ResourceUtils::C_GameModel)
	XT_REGISTER_TYPE(ResourceUtils::C_GameTexture)
	XT_REGISTER_TYPE(ResourceUtils::C_XtCycleResourceDef)
	XT_REGISTER_TYPE(ResourceUtils::C_XtSkeletonResourceDef)
	XT_REGISTER_TYPE(ResourceUtils::E_GameFileFlags)
XT_END_LIBRARY
