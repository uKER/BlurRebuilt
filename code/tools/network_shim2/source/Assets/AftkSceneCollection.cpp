#include "stdafx.h"
#include "Utility.h"
#include <tools_shared/shim/ShimErrorHelpers.h>

#include <Helpers/XmlHelper.h>

#include <assets/AftkSceneCollection.h>

bool AftkSceneCollection::GetOutputDirectory(
	Util::C_String& outputDirectory,
	const Sys::C_XmlDomBaseNode& root,
	const C_Options& options)
{
	Util::C_String partialDirectory;

	Ut::Vector<const Sys::C_XmlDomBaseNode*> outputPath;
	Sys::C_XmlDomUtility::XpathEvaluate(&root, "**/container[@shim_tag=\"shim_output_path\"]", outputPath);

	BEGIN_CHECK_CUSTOM(outputPath.Size() > 1) ERROR_ERROR("Multiple output paths specified for scene collection, zero or one expected.") END_RETURN_FALSE

	if (!outputPath.Size())
		partialDirectory = "networkshim/unnamed";
	else
	{
		const char* pathAttribute = ((Sys::C_XmlDomElement*)outputPath[0])->GetAttributes().GetValueByName("value");
		BEGIN_CHECK_NOTNULL(pathAttribute) ERROR_ERROR("Path has no value") END_RETURN_FALSE
		partialDirectory = pathAttribute;
	}

	Util::C_String outputFolder = partialDirectory;
	outputFolder = Util::FormatWindowsFileString(outputFolder);
	outputFolder = Util::RemoveTrailingSlash(outputFolder);
	outputFolder = Util::RemoveTrailingSpaces(outputFolder);
	outputFolder = Util::RemoveLeadingSlash(outputFolder);

	outputDirectory = options.GetRootTempPath() + "\\" + outputFolder.Chars() + "\\";

	return true;
}

bool AftkSceneCollection::GetOutputFilename(
	Util::C_String& outputFilename,
	const Sys::C_XmlDomBaseNode& root,
	const C_Options& options)
{
	const Sys::C_XmlDomBaseNode* outputFileNode = XmlHelper::FindXmlNodeByShimTag(&root, "shim_output_file");
	
	if (outputFileNode)
	{
		if (((Sys::C_XmlDomElement*)outputFileNode)->GetAttributes().GetValueByName("value"))
		{
			outputFilename.Printf("%s.a3d", ((Sys::C_XmlDomElement*)outputFileNode)->GetAttributes().GetValueByName("value"));
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		const char* name = ((Sys::C_XmlDomElement&)root).GetAttributes().GetValueByName("name");
		BEGIN_CHECK_NOTNULL(name) ERROR_ERROR("Root node has no name, unable to determine output filename") END_RETURN_FALSE;

		outputFilename.Printf("%s.a3d",name);
	}

	return true;
}

bool AftkSceneCollection::GetAssetArray(
	Ut::UtVector<Ut::DynamicVector<Asset> >& assets,
	const Util::C_String& outputDirectory,
	const Sys::C_XmlDomBaseNode* pNode,
	const C_Options& options)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> assetNodes;
	Sys::C_XmlDomUtility::XpathEvaluate(pNode, "**/container[@shim_tag=\"shim_aftk_scene\"]", assetNodes);

	for (int i=0; i<assetNodes.Size(); i++)
	{
		const Sys::C_XmlAttribute* pFileName = ((Sys::C_XmlDomElement*)assetNodes[i])->GetAttributes().GetAttributeByName("value");
		BEGIN_CHECK_NOTNULL(pFileName) ERROR_XML_ATTR(assetNodes[i]) END_RETURN_FALSE;

		if (!XmlHelper::ParseDependsOn(assetNodes[i]))
		{
			ERROR_INFO("Scene %s has been flagged with \"don't export\" and will be skipped.", pFileName->value.Chars());
			continue;
		}

		assets.PushBack();
		AftkSceneCollection::Asset& asset = assets[assets.Size()-1];

		asset.inputFullPath = Util::FormatWindowsFileString(pFileName->value);

		BEGIN_CHECK_CUSTOM (!GetFullA3DPath(
			asset.outputFullPath,
			outputDirectory,
			asset.inputFullPath,
			options))
			ERROR_ERROR("Failed to get A3D path");
		END_RETURN_FALSE;

		// TODO: Properly handle contains references
		asset.containsReferences = false;
	}

	return true;
}

bool AftkSceneCollection::GetFullA3DPath(
	Util::C_String& outputFullPath, 
	const Util::C_String& outputDirectory,
	const Util::C_String& inputFullPath,
	const C_Options& options)
{
	Util::C_String outputFilename;
	outputFilename.Printf("%s_%x.a3d", Util::RemoveFileExtension(Util::ExtractFilename(inputFullPath)).Chars(), Btl::HashKeyOpString::GetHash(inputFullPath));

	outputFullPath = outputDirectory + outputFilename;

	LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "A3D output path [%s] generated from input path [%s].\n", 
		outputFullPath.Chars(), 
		inputFullPath.Chars());

	return true;
}