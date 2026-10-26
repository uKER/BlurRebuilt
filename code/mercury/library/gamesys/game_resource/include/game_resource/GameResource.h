#ifndef GAME_RESOURCE_GAME_RESOURCE_H
#define GAME_RESOURCE_GAME_RESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <game_resource/ModelResource.h>
#include <game_resource/LevelResource.h>

namespace Sys { class I_File; }

namespace GameResource
{

extern Debug::Logger &g_Log;

bool ValidatePlatformId( Sys::I_File* pFile );

} // namespace GameResource

#endif // GAME_RESOURCE_GAME_RESOURCE_H
