#include "stdafx.h"
#include "Utility.h"
#include <tools_shared/shim/ShimErrorHelpers.h>

#include <Helpers/XmlHelper.h>
#include <assets/AftkSceneCollection.h>

#include <processes/NexusExportWrapper.h>
#include <tools_shared/utility/AssetMeta.h>
#include <AftkConv/Version.h>

#define SHIMXML_CONFIG_NEXUS_CONFIG									"shim_nexus_config"
#define SHIMXML_CONFIG_NEXUS_ARGS									"shim_nexus_args"

bool NexusExportWrapper::Convert(
	const C_Options& options,
	const Sys::C_XmlDomBaseNode& rootNode,
	const Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> >& assets,
	Ut::UtVector<Ut::DynamicVector<Util::C_String> >& exportedAssets)
{
	C_NexusOptions nexusOptions;
	BEGIN_CHECK_CUSTOM(!ParseOptions(nexusOptions, rootNode)) END_RETURN_FALSE;

	Util::C_String aftkConvExe = ExpandToolPath(options, options.GetAftkConvFilePath());

	BEGIN_CHECK_CUSTOM(!assets.Size()) ERROR_ERROR("ERROR: No files specified to convert.\n") END_RETURN_FALSE;

	//Use the export folder for the first asset as our output location.
	Util::C_String exportFolder = Util::ExtractDirectory(assets[0].outputFullPath);
	//Todo, would be nice to check if the folder existed first.
	Sys::C_FileSystem::Instance().CreateDirectory(exportFolder);

	ERROR_INFO("Converting any Maya files to A3D.\nAbout to process %i files.\n", assets.Size());


	//Perform processing.
	U32 filesProcessed = 0;
	for (S32 iFile = 0; iFile < assets.Size(); iFile++)
	{
		AftkSceneCollection::Asset asset = assets[iFile];

		asset.inputFullPath = 
			GetAssetFullPath(
				asset.inputFullPath, 
				options.GetExportInfo(),
				options.GetLaunchComputerName());

		BEGIN_CHECK_CUSTOM(!Sys::C_FileSystem::Instance().FileExists(asset.inputFullPath)) 
			ERROR_ERROR("Could not find file '%s' which is part of the export.\n", asset.inputFullPath.Chars())
		END_RETURN_FALSE;

		//Handle files which require conversion
		if (Util::ExtractFileExtension(asset.inputFullPath) != "a3d")
		{
			//Convert to unix as we don't want to get into character escape Hell.
			Btl::String cmdLine;

			cmdLine = Btl::String("-c c -o \"") + asset.outputFullPath.Chars() + Btl::String("\" -i \"") + asset.inputFullPath.Chars() + Btl::String("\" -x \"") + nexusOptions.args.Chars() + Btl::String("\"");

			if (Tools::MetaMismatch(asset.outputFullPath, asset.inputFullPath, AFTK_CONV_CURRENT_VERSION))
			{
				BEGIN_CHECK_CUSTOM(!RemoveReadOnlyFlag(asset.outputFullPath)) ERROR_ERROR("Failed to remove read only flag on file %s", asset.outputFullPath.Chars()) END_RETURN_FALSE;
				
				ERROR_INFO("Converting file %i/%i : %s.", filesProcessed+1, assets.Size(), asset.inputFullPath.Chars());
				ERROR_DEBUG("BEGIN Calling %s\n", cmdLine.Chars());
				BEGIN_CHECK_CUSTOM(Sys::ShellExec::ExecuteCommandAndWait(
					aftkConvExe, 
					cmdLine.Chars(),
					Util::ExtractDirectory(aftkConvExe),
					options.GetShellExecLaunchState(),
					&Tools::g_pMyApplication->GetLogger(),
					0,
					"",
					0,
					true,
					true) != 0)
					ERROR_DEBUG("END Calling %s: FAILURE\n", cmdLine.Chars());
					ERROR_FATAL("ERROR: A problem occured trying to convert '%s'. Please review the server logs to help identify the problem.\n", EscapeEscapeCharacters(asset.inputFullPath).Chars());
				END_RETURN_FALSE;
				ERROR_DEBUG("END Calling Nexus: SUCCESS %s\n", cmdLine.Chars());
			}
			else
			{
				ERROR_INFO("File up to date %i/%i : %s.", filesProcessed+1, assets.Size(), asset.inputFullPath.Chars());
			}
		}
		else
		{
			if (asset.inputFullPath.Lower() != asset.outputFullPath.Chars())
			{
				Util::C_String folder = Util::ExtractDirectory(asset.outputFullPath);
				Sys::C_FileSystem::Instance().CreateDirectory(folder);

				BEGIN_CHECK_CUSTOM(!RemoveReadOnlyFlag(asset.outputFullPath))
					ERROR_ERROR("Failed to remove read only flag on file: %s", asset.outputFullPath.Chars())
				END_RETURN_FALSE;
				
				BEGIN_CHECK_CUSTOM(!CopyFile(asset.inputFullPath.Chars(), asset.outputFullPath.Chars(), false))
					ERROR_FATAL("Failed to copy %s to %s.\n", asset.inputFullPath.Chars(), asset.outputFullPath.Chars());
				END_RETURN_FALSE;

				ERROR_DEBUG("Copied file %s to %s.\n", asset.inputFullPath.Chars(), asset.outputFullPath.Chars());
			}
		}

		exportedAssets.PushBack(asset.outputFullPath);

		filesProcessed++;
	}

	
	ERROR_INFO("Finished converting input files.");
	return true;
}

bool NexusExportWrapper::Merge(
	const C_Options& options,
	const Sys::C_XmlDomBaseNode& rootNode,
	const Util::C_String& outputFilePath,
	const Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> >& assets,
	Util::C_String& exportedAsset)
{
	C_NexusOptions nexusOptions;
	BEGIN_CHECK_CUSTOM(!ParseOptions(nexusOptions, rootNode)) END_RETURN_FALSE;

	Util::C_String localAftkConvExe = ExpandToolPath(options, options.GetAftkConvFilePath());

	BEGIN_CHECK_CUSTOM(!assets.Size()) ERROR_ERROR("ERROR: No files specified to convert.\n") END_RETURN_FALSE;

	//Use the export folder for the first asset as our output location.
	Util::C_String exportFolder = Util::ExtractDirectory(assets[0].outputFullPath);
	//Todo, would be nice to check if the folder existed first.
	Sys::C_FileSystem::Instance().CreateDirectory(exportFolder);


	BEGIN_CHECK_CUSTOM (!RemoveReadOnlyFlag(outputFilePath))
		ERROR_ERROR("Failed to remove read-only flag from: %s", outputFilePath.Chars());
	END_RETURN_FALSE;

	exportedAsset = outputFilePath;

	if (assets.Size() == 1)
	{
		BEGIN_CHECK_CUSTOM(!CopyFile(assets[0].outputFullPath.Chars(), outputFilePath.Chars(), false))
			ERROR_ERROR("Unable to copy source file %s to destination %s", assets[0].outputFullPath.Chars(), outputFilePath.Chars());
		END_RETURN_FALSE;
		
		return true;
	}

	Sys::C_FilePath mergeWorkFile = exportFolder;
	mergeWorkFile = mergeWorkFile + "merge_workfile.xml";
	Sys::C_XmlDom mergeDom;
	{
		Sys::C_XmlDomElement* pRoot = new Sys::C_XmlDomElement;
		pRoot->GetName() = "aftkconv";
		Sys::C_XmlDomElement* pInputs = new Sys::C_XmlDomElement;
		pInputs->GetName() = "inputs";
		pRoot->AddChild(pInputs);


		for (S32 iFile = 0; iFile < assets.Size(); iFile++)
		{
			AftkSceneCollection::Asset asset = assets[iFile];
			Sys::C_XmlDomElement* pInput = new Sys::C_XmlDomElement;
			pInput->GetName() = "input";
			pInput->GetAttributes().Add("filepath", asset.outputFullPath);
			pInputs->AddChild(pInput);
		}

		mergeDom.SetRootNodePtr(pRoot);

		// Now we write the merge script out to a file we can execute
		BEGIN_CHECK_CUSTOM (!RemoveReadOnlyFlag(mergeWorkFile.Chars()))
			ERROR_FATAL("Failed to remove read-only flag from: %s", mergeWorkFile.Chars())
		END_RETURN_FALSE;
	}
	if (mergeDom.Write(mergeWorkFile.Chars()))
	{
		Btl::String mergeCommandLine = Btl::String("-c m -o ") + outputFilePath.Chars();
		mergeCommandLine = mergeCommandLine + " -f " + mergeWorkFile.Chars();

		BEGIN_CHECK_CUSTOM(Sys::ShellExec::ExecuteCommandAndWait(
			localAftkConvExe, 
			mergeCommandLine.Chars(),
			Util::ExtractDirectory(localAftkConvExe),
			options.GetShellExecLaunchState(),
			&Tools::g_pMyApplication->GetLogger(),
			0,
			"",
			0,
			false,
			true) != 0)
			ERROR_FATAL("An error occured while trying to merge A3D files together.\n")
		END_RETURN_FALSE;
	}
	else
	{
		ERROR_FATAL("An error occured while trying to write the merge work file (%s).\n", mergeWorkFile.Chars());
		return false;
	}

	ERROR_INFO("Finished merging input files.");
	return true;
}

bool NexusExportWrapper::ParseOptions(C_NexusOptions& options, const Sys::C_XmlDomBaseNode& rootNode)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> nexusArgs;
	Sys::C_XmlDomUtility::XpathEvaluate(&rootNode, "type_entry/tagdata/shim_action/asset/aftk_config/nexus_args", nexusArgs);

	BEGIN_CHECK_CUSTOM(nexusArgs.Size() != 1) ERROR_ERROR("Number of nexus arguments is not equal to one") END_RETURN_FALSE

	const char* nexusArgsAttribute = ((Sys::C_XmlDomElement*)nexusArgs[0])->GetAttributes().GetValueByName("value");
	BEGIN_CHECK_NOTNULL(nexusArgsAttribute) ERROR_ERROR("Aftk conversion arguments have no value") END_RETURN_FALSE
	options.args = nexusArgsAttribute;

	return true;
}
