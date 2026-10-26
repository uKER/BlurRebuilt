#include "stdafx.h"
#include "UI.h"
#include "Fontmaker.h"
#include "SystemCommands.h"
#include "UIDebug.h"

#define DEG_TO_RAD (0.017453292519943295769236907684886f)

UI gUI;

void UI::DefaultConfig()
{
	strcpy(Config.UIMakerDir,"");

	strcpy(Config.UIMakerXMLDir,"");
	strcpy(Config.UIMakerTextureDir,"");
	strcpy(Config.UIMakerTextDir,"");

	strcpy(Config.SourceDir,"");
	strcpy(Config.TargetDir,"");
	strcpy(Config.ServerDir,"");

	strcpy(Config.UIMakerXMLDir,"");
	strcpy(Config.ExternalEditor,"");
	strcpy(Config.LuaDir,"");

	Config.CopytoServer=false;
	Config.JustErrorDebug=false;

	Config.Export=UIEXPORT_FULL;
}

void UI::LoadConfig()
{
	DefaultConfig();

	FILE *fp;
	char filename[128];

	sprintf(filename,"c:\\uiresource.cfg");

	fp=fopen(filename,"rb");

	if(fp)
	{
		fread(&Config,sizeof(Config),1,fp);
		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Reading Config File",MB_OK|MB_ICONINFORMATION);
	}

	gFontMaker.SetTextDir(Config.UIMakerTextDir);
	gFontMaker.SetTextureDir(Config.UIMakerTextureDir);
}

void UI::SaveConfig()
{
	FILE *fp;
	char filename[128];

	sprintf(filename,"c:\\uiresource.cfg");

	fp=fopen(filename,"wb");
	if(fp)
	{
		fwrite(&Config,sizeof(Config),1,fp);
		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Writing Config File",MB_OK|MB_ICONINFORMATION);
	}
}

void UI::SetLanguage(UILANG l)
{
	char command[128];
	int temp=666;

	sprintf(command,"xbsetcfg -n %d",l);
	gSystemCommands.AddCommand(command);

	gSystemCommands.Execute();

	gUIDebug.SendMessage(UIDEBUG_MESSAGE_REINIT);
}
