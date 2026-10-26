#ifndef DMAN_CONFIG_H
#define DMAN_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	struct DialogueManConfig
	{
		Util::C_String	configPath;
		Util::C_String	macroFilePath;
		Util::C_String	XtDbPath;
		Util::C_String	locId;
		Util::C_String	exportPath;
		Util::C_String	audioXMLPath;
		Util::C_String	bankPCPath;
		Util::C_String	bank360Path;
		Util::C_String	bankPS3Path;
		Util::C_String	build360Path;
		TTS				tts;
	};

	struct DialogueManPreferences
	{
	};

	bool LoadConfig (const char* configPath, const char* macroPath, DialogueManConfig& config);

	bool LoadPreferences(const char* prefsPath, DialogueManPreferences& prefs);
}

#endif //DMAN_CONFIG_H