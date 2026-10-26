#ifndef EDITOR_LIB_CORE_CONFIG_H
#define EDITOR_LIB_CORE_CONFIG_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	struct C_CoreConfig
	{
		bool Load(const char* configFilePath_);

		//From Xml
		Btl::String	configPath;
		Btl::String	macroFilePath;
		Btl::String	xtDbPath;
		Btl::String	a3dCachePath;
		Btl::String	conversionToolPath;
		Btl::String	conversionToolParams;
		Btl::String conversionToolMetaQueryParams;

		Btl::String	uberCopyToolPath;
		Btl::String	alienbrainConfigPath;

		Btl::String	projectName;
		Btl::String	rootProjectPath;
		Btl::String	rootTempPath;
		Btl::String	rootWorkingPath;

		Btl::String	computerName;
		Btl::String	dumpPath;

		//Set manually
		E_AppType		appType;
	};
}

#endif
