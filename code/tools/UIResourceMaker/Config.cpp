#include "stdafx.h"
#include "Config.h"

Config gConfig;

void Config::Default()
{
	strcpy(mBuildFile,"");
	strcpy(mTextDirectory,"");

	strcpy(mTextureExport,"");
	strcpy(mPCExport,"");
	strcpy(mServerExport,"");

	strcpy(mEditorExe,"");
	strcpy(mBundlerExe,"");
}
