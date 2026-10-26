#pragma once

class UberCopyWrapper
{
public:
	static bool PostExportCopy(const C_Options& options, const Util::C_String& nodeName);

	//Determines what to copy from xml
	static bool CopyToGame(const C_Options& options, const Util::C_String& platform, const Util::C_String& taskDescriptionXmlFilePath);
	static Util::C_String GetTaskDescriptionXmlFilePath(const C_Options& options, const Util::C_String& nodeName);
	
	static bool AddTask(const Util::C_String& nodeName);
	static const Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString>& GetTasks();

private:
	static Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString> m_DestinationSet;
	static Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString> m_TaskSet;

	static bool ValidityCheck(const C_Options& options, const Util::C_String& xmlFilePath);

	static bool CopySingle(const C_Options& options, const Util::C_String& fileToCopy);

	static bool Copy(
		const C_Options& options,
		const Util::C_String& xmlFilePath,
		const Util::C_String* pPlatform = 0);

	static bool GetCopyToGameXmlFilePath(Util::C_String& copyToGameFilePath, const Util::C_String& taskFilePath);
	static bool GetCopyToWorkingXmlFilePath(Util::C_String& copyToWorkingFilePath, const Util::C_String& taskFilePath);
};
