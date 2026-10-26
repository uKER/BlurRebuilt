#include "stdafx.h"
#include <actions/AftkSceneCollectionAction.h>

#include <assets/AftkSceneCollection.h>

#include <processes/NexusExportWrapper.h>
#include <processes/SubversionWrapper.h>
#include <processes/UberCopyWrapper.h>
#include <set>

#include <tools_shared/shim/ShimErrorHelpers.h>

bool AftkSceneCollectionAction::Process(const C_Options& options, const Sys::C_XmlDomBaseNode& dom, E_ShimActionTag tag, Ut::Vector<Util::C_String>& outputAssets)
{
	ERROR_INFO("Launching aftk scene action\n");

	//Grab art_tools folder
	bool isNetworkExport = (options.GetExportInfo() & Tools::Shim::E_ExportInfoMask_Location) == Tools::Shim::E_ExportInfo_LocationNetwork;
	if (isNetworkExport && !SubversionWrapper::GetLatest(options, options.GetArtToolsSvn()))
		return false;

	if (tag == E_ShimActionTag_AftkSceneCombine)
		return ProcessCombine(options, dom, outputAssets);

	if (tag == E_ShimActionTag_AftkSceneMultiple)
		return ProcessMultiple(options, dom, outputAssets);

	if (tag == E_ShimActionTag_AftkSceneSingle)
		return ProcessSingle(options, dom, outputAssets);

	BEGIN_ALWAYS ERROR_ERROR("Unknown aftk scene export type") END_RETURN_FALSE;
}

bool AftkSceneCollectionAction::GetAssetList(const C_Options& options, const Sys::C_XmlDomBaseNode& node, Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> >& assets, const Util::C_String& outputDirectory)
{
	bool success = AftkSceneCollection::GetAssetArray(assets, outputDirectory, &node, options);
	BEGIN_CHECK_SUCCESS ERROR_FATAL("Failed to get asset array") END_RETURN_FALSE;
	
	BEGIN_CHECK_CUSTOM(assets.Size() == 0) ERROR_ERROR("No assets to process!\n") END_RETURN_FALSE;
	{
		int errors = 0;
		std::set<std::string> strSet;
		for (S32 i = 0; i < assets.Size(); i++)
		{
			size_t size = strSet.size();
			strSet.insert(assets[i].outputFullPath.Chars());
			if (strSet.size() != size + 1)
			{
				ERROR_ERROR("The filename of %s is ambiguous!\n", assets[i].inputFullPath.Chars());
				errors++;
			}
		}

		BEGIN_CHECK_CUSTOM(errors) ERROR_ERROR("Ambiguous asset names found! Rename the files mentioned in the error log.\n") END_RETURN_FALSE;
	}

	return true;
}

bool AftkSceneCollectionAction::ProcessMultiple(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, Ut::Vector<Util::C_String>& outputAssets)
{
	Util::C_String outputDirectory;
	BEGIN_CHECK_CUSTOM(!AftkSceneCollection::GetOutputDirectory(
		outputDirectory,
		rootNode,
		options))
		ERROR_ERROR("Failed to get output file path")
	END_RETURN_FALSE;

	Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> > assets;
	bool success = GetAssetList(options, rootNode, assets, outputDirectory);
	BEGIN_CHECK_SUCCESS ERROR_ERROR("Couldn't get aftk scene asset list") END_RETURN_FALSE;

	bool status = Execute(options, rootNode, outputDirectory, assets, outputAssets, false);
	BEGIN_CHECK_CUSTOM(!status) ERROR_ERROR("Asset processing failed") END_RETURN_FALSE;

	return true;
}

bool AftkSceneCollectionAction::ProcessCombine(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, Ut::Vector<Util::C_String>& outputAssets)
{	
	Util::C_String outputDirectory;
	BEGIN_CHECK_CUSTOM(!AftkSceneCollection::GetOutputDirectory(
		outputDirectory,
		rootNode,
		options))
		ERROR_ERROR("Failed to get output file path")
	END_RETURN_FALSE;

	Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> > assets;
	bool success = GetAssetList(options, rootNode, assets, outputDirectory);
	BEGIN_CHECK_SUCCESS ERROR_ERROR("Couldn't get aftk scene asset list") END_RETURN_FALSE;

	bool status = Execute(options, rootNode, outputDirectory, assets, outputAssets, true);
	BEGIN_CHECK_CUSTOM(!status) ERROR_ERROR("Asset processing failed") END_RETURN_FALSE;

	return true;
}

bool AftkSceneCollectionAction::ProcessSingle(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, Ut::Vector<Util::C_String>& outputAssets)
{	
	Util::C_String outputDirectory;
	BEGIN_CHECK_CUSTOM(!AftkSceneCollection::GetOutputDirectory(
		outputDirectory,
		rootNode,
		options))
		ERROR_ERROR("Failed to get output file path")
	END_RETURN_FALSE;

	Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> > assets;
	bool success = GetAssetList(options, rootNode, assets, outputDirectory);
	BEGIN_CHECK_SUCCESS ERROR_ERROR("Couldn't get aftk scene asset list") END_RETURN_FALSE;

	BEGIN_CHECK_CUSTOM(assets.Size()!=1) ERROR_FATAL("File contains %d assets, not 1 as expected", assets.Size()) END_RETURN_FALSE;

	bool status = Execute(options, rootNode, outputDirectory, assets, outputAssets, false);
	BEGIN_CHECK_CUSTOM(!status) ERROR_ERROR("Asset processing failed") END_RETURN_FALSE;

	return true;
}

bool AftkSceneCollectionAction::Execute(const C_Options& options, const Sys::C_XmlDomBaseNode& rootNode, const Util::C_String& outputFilePath, const Ut::UtVector<Ut::DynamicVector<AftkSceneCollection::Asset> >& sceneAssets, Ut::Vector<Util::C_String>& outputAssets, bool doMerge)
{
	Ut::UtVector<Ut::DynamicVector<Util::C_String> > exportedAssets;

	//Launch Aftk converter. Converting and merging as required.
	BEGIN_CHECK_CUSTOM(!NexusExportWrapper::Convert(options, rootNode, sceneAssets, exportedAssets))
		ERROR_ERROR("Failed to process aftk conversion") END_RETURN_FALSE;

	if (doMerge)
	{
		Util::C_String outputDirectory;
		BEGIN_CHECK_CUSTOM(!AftkSceneCollection::GetOutputDirectory(outputDirectory, rootNode, options))
			ERROR_ERROR("Failed to obtain output directory") END_RETURN_FALSE;

		Util::C_String outputFilename;
		BEGIN_CHECK_CUSTOM(!AftkSceneCollection::GetOutputFilename(outputFilename, rootNode, options))
			ERROR_ERROR("Failed to calculate output filename") END_RETURN_FALSE;

		exportedAssets.Clear();
		Util::C_String exportedAsset;
		BEGIN_CHECK_CUSTOM(!NexusExportWrapper::Merge(options, rootNode, outputDirectory+outputFilename, sceneAssets, exportedAsset))
			ERROR_ERROR("Failed to process aftk conversion") END_RETURN_FALSE;
		exportedAssets.PushBack(exportedAsset);
	}

	for (int i=0; i<exportedAssets.Size(); i++)
		outputAssets.PushBack(exportedAssets[i]);

	return true;
}