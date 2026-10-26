#pragma once

class Config
{
public:
	char mBuildFile[128];
	char mTextDirectory[128];

	char mTextureExport[128];
	char mPCExport[128];
	char mServerExport[128];

	char mEditorExe[128];
	char mBundlerExe[128];

	void Default();

};

extern Config gConfig;
