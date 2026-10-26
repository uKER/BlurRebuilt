#include "stdafx.h"
#include "Utility.h"
#include <tools_shared/shim/ShimErrorHelpers.h>

#include <Helpers/XmlHelper.h>
#include <processes/AlienBrainWrapper.h>

#define SHIMXML_AB_PATH_LIST										"shim_ab_folders"


bool AlienBrainWrapper::GetLatest(const C_Options& options, const Sys::C_XmlDom& dom, bool doSync)
{
	ERROR_INFO("Going to try and check necessary files out of AlienBrain.\n");

	BEGIN_CHECK_NOTNULL(dom.GetRootNodePtr()) ERROR_ERROR("Dom root node is null") END_RETURN_FALSE;

	//Our list of folders to update.
	Ut::UtDynamicVector<Util::C_String> abFolderArray;

	// Scan through the node hierachy looking for export info
	Ut::Vector<const Sys::C_XmlDomBaseNode*> abNodes;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "**/container[@shim_tag=\"shim_ab_folders\"]", abNodes);

	for (int i=0; i<abNodes.Size(); i++)
	{
		Util::C_String text = ((Sys::C_XmlDomElement*)abNodes[i])->GetAttributes().GetAttributeByName("value")->value;
		
		//Here we go with some good old fashioned parsing code.
		while(true)
		{
			int offset;

			if ((offset = text.Search("&#xA")) != -1)
			{
				Util::C_String path = GetLeft(text.Chars(), offset);

				text = GetRight(text.Chars(), text.Length() - offset - 4);

				if (text.Chars()[0] == ';')
				{
					text = GetRight(text.Chars(), text.Length() - 1);
				}

				if (path.Length() > 0)
					abFolderArray.PushBack(path);
			}
			else
			{
				if (text.Length() > 0)
					abFolderArray.PushBack(text);

				//We've run out of delimiters and reached the end of our folder list.
				break;
			}
		}
	}

	// Sort the folders into per-project batches
	Ut::OpenHashMap<Util::C_String, Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString>, Ut::HashKeyOpString> projectBatches;
	for (int i=0; i < abFolderArray.Size(); i++)
	{
		Util::C_String path = abFolderArray[i].Replace("\\", "/");

		S32 first = path.ISearch("/");
		S32 second = path.ISearch("/", first+1);
		Util::C_String project = path(first+1, second);

		projectBatches[project].Insert(path);
	}

	for (Ut::OpenHashMap<Util::C_String, Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString>, Ut::HashKeyOpString>::Iterator batch = projectBatches.Begin(); batch != projectBatches.End(); ++batch)
	{
		// Generate an XML file to provide to the alienbrain script
		Util::C_String abFolderListFilename = "%TEMP%";
		abFolderListFilename = abFolderListFilename + "/ab_folder_list.xml";
		Tools::C_MacroExpander::Instance().Expand(abFolderListFilename);

		Util::C_String folderList;
		{
			folderList += "<folders>";
			for (Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString>::Iterator folder=batch->GetValue().Begin(); folder != batch->GetValue().End(); ++folder)
			{
				folderList += "<folder>";
				folderList += folder->GetKey();
				folderList += "</folder>";
			}
			folderList += "</folders>";
		}

		Util::C_String localABScript = ExpandToolPath(options, options.GetAlienbrainScript());

		// Now we write the script out to a file we can execute
		FILE* fh = 0;
		fopen_s(&fh,abFolderListFilename.Chars(), "w");
		if (fh)
		{
			fwrite(folderList.Chars(), folderList.Length(), 1, fh);
			fclose(fh);

			const Util::C_String* server = options.GetAlienbrainServer(batch->GetValue().Begin()->GetKey());

			BEGIN_CHECK_NOTNULL(server) ERROR_ERROR("Unable to find server for alienbrain folder (\"%s\"). Is folder valid?", batch->GetValue().Begin()->GetKey().Chars()) END_RETURN_FALSE
			
			Util::C_String synchroniseCommandLine = 
			localABScript +
			Util::C_String(" ") + 
			"\"" + options.GetAlienbrainUser(*server) + "\"" +
			Util::C_String(" ") + 
			"\"" + options.GetAlienbrainPass(*server) + "\"" +
			Util::C_String(" ") + 
			"\"" + batch->GetKey() + "\"" +
			Util::C_String(" ") + 
			"\"" + *server + "\"" +
			Util::C_String(" ") + 
			"-m \"" + abFolderListFilename +"\"";

			if (doSync)
				synchroniseCommandLine += " --sync";
			else
				synchroniseCommandLine += " --getlatest";


			LOGINFO(Tools::g_log, "Syncing alienbrain folders on server: %s.\n", server->Chars());

			BOOL bWow64Process = FALSE;
			if (!IsWow64Process(GetCurrentProcess(), &bWow64Process))
			{
				//Call failed, handle error
				LOGERROR( Tools::g_log , "Failed to determine machine word length... assuming 32b\n");
			}

			Util::C_String cscriptExe;

			if (bWow64Process)
			{
				cscriptExe = "%SystemRoot%\\syswow64\\cscript.exe";
				Tools::C_MacroExpander::Instance().Expand(cscriptExe);
			}
			else
			{
				cscriptExe = "cscript.exe";
			}

			BEGIN_CHECK_CUSTOM (Sys::ShellExec::ExecuteCommandAndWait(
				cscriptExe, 
				synchroniseCommandLine, 
				"",
				options.GetShellExecLaunchState(),
				&Tools::g_pMyApplication->GetLogger(),
				0, 0, 0, true) != 0)
				ERROR_FATAL("An error occured while trying to synchronise AB.\n");
			END_RETURN_FALSE;
		}
		else
		{
			BEGIN_ALWAYS ERROR_FATAL("An error occured while trying to write the AB folder list XML file (%s).\n", abFolderListFilename.Chars()) END_RETURN_FALSE;
		}
	}

	return true;
}

