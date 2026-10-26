#pragma once

#include "UIDefines.h"

enum UILANG
{
	UILANG_UNKNOWN,

	UILANG_ENGLISH,
	UILANG_JAPANESE,
	UILANG_GERMAN,
	UILANG_FRENCH,
	UILANG_SPANISH,
	UILANG_ITALIAN,
	UILANG_KOREAN,
	UILANG_CHINESE,
	UILANG_PORTUGUESE,

	UILANG_MAX,
};

enum UIEXPORT
{
	UIEXPORT_FULL,
	UIEXPORT_EAFONTS,
	UIEXPORT_MAX,
};

typedef struct
{
	char UIMakerDir[128];
	char UIMakerTextureDir[128];
	char UIMakerTextDir[128];

	char SourceDir[128];

	char UIMakerXMLDir[128];
	char TargetDir[128];
	char ServerDir[128];
	char ExternalEditor[128];
	char LuaDir[128];

	bool CopytoServer;
	bool JustErrorDebug;

	int XboxIPAddress;

	UIEXPORT Export;
} UIMakerConfig;

class UI
{
public:
	UIMakerConfig Config;
	void DefaultConfig();
	void LoadConfig();
	void SaveConfig();

	void SetCurrentResourceFile(char *f){strcpy(mCurrentResourceFile,f);};
	char *GetCurrentResourceFile(){return mCurrentResourceFile;};

	void SetLanguage(UILANG l);

protected:

	char mCurrentResourceFile[64];
};

extern UI gUI;
