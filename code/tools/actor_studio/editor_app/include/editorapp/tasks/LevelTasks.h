#ifndef EDITOR_LEVELTASKS_H
#define EDITOR_LEVELTASKS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	namespace Tasks
	{
		bool IsLevelLoaded();
		bool IsPackLoaded();
		bool IsDbOpen();

		bool Open(XtId id, bool closeOthers);
		void NewLevel();
		void OpenLevel(bool closeOthers=true);
		void AddLevel();
		int SaveChanges();
		void SetActiveLevel(EditorLib::I_Level* level);
		bool CloseLevel(EditorLib::I_Level* level);
		bool ClosePack(EditorLib::C_LevelPack* pack, bool saveChangesPrompt = true);
		bool CloseAllPacks(bool saveChangesPrompt = true);
		void FastQuit();
		bool SaveLevel();
		void SaveLevelNoReturn();
		void BuildLevel();
		void ExportToLevel();
		void ReloadReferences();
		void LoadAllReferences(bool promptConvert);
		void GetLatestReferences();

		Btl::String GenerateFileName();
		void EraseAutosaveFiles();
		bool ExportUnsavedChanges(const char *pFileName = GenerateFileName().Chars());
		bool ImportUnsavedChanges();
	}
}

#endif
