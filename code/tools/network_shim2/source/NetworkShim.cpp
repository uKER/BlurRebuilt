#include "stdafx.h"
#include "Options.h"

#include "assets/AftkSceneCollection.h"

#include "processes/SubversionWrapper.h"
#include "processes/AlienBrainWrapper.h"
#include "processes/UberCopyWrapper.h"

#include <Helpers/XmlHelper.h>
#include "actions/Action.h"
#include "actions/ScmAction.h"
#include "actions/UberCopyAction.h"

#include "NetworkShim.h"
#include "GetLatestTools.h"

#include <System/Time/Time.h>


#include <tools_shared/shim/ShimErrorHelpers.h>

C_NetworkShim::C_NetworkShim(
	const char *szName,
	const char *szSubtitle,
	const char *szLogger,
	const char *szDmpPath,
	bool bExecuter)
	:
	Tools::C_ToolsApplication(
		szName,
		szSubtitle,
		szLogger,
		szDmpPath,
		bExecuter)
{
}

C_NetworkShim::~C_NetworkShim()
{
}

#undef GetCurrentTime

bool DeleteOldFiles(const Util::C_String& folder, const Util::C_String& strPrefix, int minNumDaysOld, bool recursive)
{
	Btl::String lowerPrefix = strPrefix.Lower();

	Sys::C_DirectoryInfo dirInfo;
	Sys::GetDefaultFileSystem()->DirectoryInfo(folder, &dirInfo);

	Time::C_Time currentTime = Time::GetCurrentTime();
	for (U32 i=0; i<dirInfo.NumFiles(); i++)
	{
		Sys::C_FilePath fileName = dirInfo.FileInfo(i).m_FileName;

		if (dirInfo.FileInfo(i).m_FileType == Sys::E_FileType_Directory)
		{
			if (recursive)
				DeleteOldFiles(fileName.Chars(), strPrefix, minNumDaysOld, recursive);
			continue;
		}

		if (Sys::ExtractFilename(fileName).Search(lowerPrefix) != 0)
			continue;

		if (currentTime < dirInfo.FileInfo(i).m_FileTime)
		{
			ERROR_INFO("File time is in the future, skipping : %s", fileName.Chars());
			continue;
		}

		Time::C_Time diff = currentTime - dirInfo.FileInfo(i).m_FileTime;
		Time::C_DecomposedTime diffDecomp(diff);
		int daysOld = diffDecomp.m_yday;
		
		if (diffDecomp.m_yday > minNumDaysOld)
		{
			ERROR_INFO("Deleting old file : %s", fileName.Chars());

			Sys::GetDefaultFileSystem()->DeleteFileA(fileName);
		}
	}

	return true;
}

bool RemoveEmptyFolders(const Util::C_String& folder)
{
	Sys::C_DirectoryInfo dirInfo;
	Sys::GetDefaultFileSystem()->DirectoryInfo(folder, &dirInfo);

	Time::C_Time currentTime = Time::GetCurrentTime();
	for (U32 i=0; i<dirInfo.NumFiles(); i++)
	{
		if (dirInfo.FileInfo(i).m_FileType == Sys::E_FileType_Directory)
		{
			Sys::C_DirectoryInfo subDirInfo;
			Sys::GetDefaultFileSystem()->DirectoryInfo(dirInfo.FileInfo(i).m_FileName, &subDirInfo);

			if (subDirInfo.NumFiles() == 0)
			{
				ERROR_INFO("Deleting empty directory : %s", dirInfo.FileInfo(i).m_FileName.Chars());

				if (RemoveDirectory(dirInfo.FileInfo(i).m_FileName.Chars())==0)
				//if (!Sys::GetDefaultFileSystem()->DeleteFile(dirInfo.FileInfo(i).m_FileName))
				{
					ERROR_INFO("Couldn't delete directory : %s",  dirInfo.FileInfo(i).m_FileName.Chars());
				}
			}
		}
	}

	return true;
}

S32 C_NetworkShim::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_Options options;
	bool success = true;

	Util::C_String cleanupPath; // TODO: Cleanup

	Sys::C_XmlDom dom;
	//Set application options
	{
		Util::C_String xmlFilePath;
		success &= options.ParseCommandLine(xmlFilePath, argc, argv);
		BEGIN_CHECK_SUCCESS ERROR_FATAL("Failed to parse the command line") END_GOTO_EXIT;

		success &= dom.Read(xmlFilePath.Chars());
		BEGIN_CHECK_SUCCESS ERROR_FATAL("Failed to read XML file: %s", xmlFilePath.Chars()) END_GOTO_EXIT;

		//Fill out options embedded in XML
		LOGINFO(Tools::g_pMyApplication->GetLogger(), "Parsing: %s\n", xmlFilePath.Chars());
		success &= options.ParseXml(xmlFilePath, dom);
		BEGIN_CHECK_SUCCESS ERROR_FATAL("Failed to parse XML file: %s", xmlFilePath.Chars()) END_GOTO_EXIT;
	}

	//Grab latest tools if running on a robot
	bool isNetworkExport = (options.GetExportInfo() & Tools::Shim::E_ExportInfoMask_Location) == Tools::Shim::E_ExportInfo_LocationNetwork;
	if (isNetworkExport)
	{
		success &= C_GetLatestTools::GetLatestTools(options);
		BEGIN_CHECK_SUCCESS ERROR_FATAL("Failed to get latest tools") END_GOTO_EXIT;

		success &= MapDrives(options, dom);
		BEGIN_CHECK_SUCCESS ERROR_FATAL("Failed to map drives") END_GOTO_EXIT;
	}

	//Grab latest content from SCM if required.
	bool isUsingContentFromSCM = (options.GetExportInfo() & Tools::Shim::E_ExportInfoMask_DataSource) == Tools::Shim::E_ExportInfo_DataSourceSCM;
	if (isUsingContentFromSCM)
	{
//		success &= SubversionWrapper::GetLatest(options, dom);
//		BEGIN_CHECK_SUCCESS ERROR_FATAL("Failed to get latest copy of files from SVN") END_DO_NOTHING;

		success &= AlienBrainWrapper::GetLatest(options, dom, true);
		BEGIN_CHECK_SUCCESS ERROR_FATAL("Failed to get copy of data from alienbrain") END_DO_NOTHING;
	}

	cleanupPath = options.GetXmlFilePath().Replace("/", "\\");
	S32 lastSlash = cleanupPath.RevSearch('\\');
	DeleteOldFiles(cleanupPath(0, lastSlash), "Ns_", 4, true);
	RemoveEmptyFolders(cleanupPath(0, lastSlash));

	success &= LaunchAction(options, dom);
	BEGIN_CHECK_SUCCESS ERROR_ERROR("Couldn't launch action") END_GOTO_EXIT;

exit:
	if (success)
	{
		ERROR_INFO("Processing completed succesfully!\n");
	}
	else
	{
		ERROR_INFO("Processing failed!\n");
	}

	Sleep(500);

	if (success)
		return 0;
	else
		return 1;
}

bool C_NetworkShim::MapDrives(const C_Options& options, const Sys::C_XmlDom& dom) const
{
	Btl::String commandFilePath = "subst.exe";
	Btl::String argumentsValue = "/d %BIZARRE_CONTENT%";
	Btl::String workingFolder = "";

	Tools::C_MacroExpander::Instance().Expand(argumentsValue);
	Sys::ShellExec::ExecuteCommandAndWait(commandFilePath.Chars(), argumentsValue.Chars(), workingFolder.Chars(), options.GetShellExecLaunchState(), &Tools::g_pMyApplication->GetLogger());

	S32 exitCode;

	if ((options.GetExportInfo() & Tools::Shim::E_ExportInfoMask_DataSource) == Tools::Shim::E_ExportInfo_DataSourceNetwork)
	{
		Btl::String dataSource = Btl::String("\\\\") + options.GetLaunchComputerName() + "\\" + ALIEN_BRAIN_FOLDER;
		ERROR_INFO("Mapping content to %s:", dataSource.Chars());
		argumentsValue = Btl::String("%BIZARRE_CONTENT% ") + dataSource;
		Tools::C_MacroExpander::Instance().Expand(argumentsValue);
		
		exitCode = Sys::ShellExec::ExecuteCommandAndWait(commandFilePath.Chars(), argumentsValue.Chars(), workingFolder.Chars(), options.GetShellExecLaunchState(), &Tools::g_pMyApplication->GetLogger());
	} else {
		ERROR_INFO("Mapping content to root");
		argumentsValue = "%BIZARRE_CONTENT% %BIZARRE_ROOT%\\";
		Tools::C_MacroExpander::Instance().Expand(argumentsValue);
		exitCode = Sys::ShellExec::ExecuteCommandAndWait(commandFilePath.Chars(), argumentsValue.Chars(), workingFolder.Chars(), options.GetShellExecLaunchState(), &Tools::g_pMyApplication->GetLogger());
	}

	BEGIN_CHECK_CUSTOM (exitCode != 0) ERROR_ERROR ("Couldn't map data drives") END_RETURN_FALSE
	
	return true;
}

bool C_NetworkShim::LaunchAction(const C_Options& options, const Sys::C_XmlDom& dom) const
{
	switch (options.GetMode())
	{
	case E_ShimMode_Action:
		{
			C_Action action(options);
			bool success = action.Execute(*dom.GetRootNodePtr()->GetChildren()[0]);

			{
				Ut::Vector<const Sys::C_XmlDomBaseNode*> postExport;
				Sys::C_XmlDomUtility::XpathEvaluate(options.GetActionNode(), "actiondata/shim_mode/postexport", postExport);
				BEGIN_CHECK_CUSTOM(postExport.Size() != 1) ERROR_ERROR("Number of post export definitions does not equal one") END_RETURN_FALSE
				BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)postExport[0])->GetAttributes().GetValueByName("frequency")) ERROR_XML_ATTR(postExport[0]) END_RETURN_FALSE
				Util::C_String frequency = ((Sys::C_XmlDomElement*)postExport[0])->GetAttributes().GetValueByName("frequency");

				if (frequency == "perbatch")
				{
					Ut::Vector<Util::C_String> failedExports;

					int i = 1;
					for (Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString>::Iterator it = UberCopyWrapper::GetTasks().Begin(); it != UberCopyWrapper::GetTasks().End(); ++it, ++i)
					{
						ERROR_INFO("Running post export copy %d/%d (%s):", i, UberCopyWrapper::GetTasks().Size(), (*it).GetKey().Chars());

						if (!UberCopyWrapper::PostExportCopy(options, *it))
							failedExports.PushBack(*it);
					}

					if (failedExports.Size() != 0)
					{
						ERROR_ERROR("The following nodes failed their post export copy:");

						for (int i=0; i<failedExports.Size(); i++)
						{
							ERROR_ERROR(failedExports[i].Chars());
						}

						success = false;
					}

					action.PrintStatus();

					if (failedExports.Size() != 0)
					{
						ERROR_ERROR("Post-export failed");
					} else {
						ERROR_INFO("Post-export completed successfully");
					}
				} else {
					action.PrintStatus();
				}
			}

			return success;
		}
	case E_ShimMode_Ubercopy:
		{
			return UberCopyAction::Execute(options, dom);
		}
	case E_ShimMode_Scm:
		{
			return ScmAction::Execute(options, dom);
		}
	default:
		{
			BEGIN_ALWAYS ERROR_ERROR("An unknown shim mode was specified.\n") END_RETURN_FALSE;
		}
	}
}
