#ifndef EDITOR_CONFIG_H
#define EDITOR_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Utility/string.h>

#include <EditorLib/Misc/Definitions.h>

namespace Editor
{
	struct EditorConfig
	{
		Btl::String				configPath;
		Btl::String				macroFilePath;

		Btl::String				dumpPath;
		EditorLib::E_AppType	appType;
		Btl::String				startupCommand;
		Btl::String				startupScript;
		Btl::String				xtdbOverridePath;
	};

	bool LoadConfig (const char* configPath, const char* macroPath, EditorConfig& config);
}

#endif