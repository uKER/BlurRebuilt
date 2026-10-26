#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <system/debug/log/DebugLog.h>
#include <system/debug/log/subscribers/logfilesubscriber.h>
#include <system/debug/log/ExecuterFeedback.h>
#include <tools_shared/executer/ExecuterHelpers.h>
#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>

#include "Application.h"
#include "CreateFileList.h"

#include <aftk/Aftk.h>
#include <camera2/export/AftkToCameraScene.h>
#include <camera2/tv/CameraScene.h>
#include <Camera2/tv/CameraSceneSerialise.h>

static S32 Version = 0x1000;

C_Application::C_Application(
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

C_Application::~C_Application()
{
}

// Search the AFTK Scene for a node which is in a Cameras_... layer
static void FindCameras(Aftk::C_Scene& scene, Cam2::C_CameraScene& outScene)
{
	Aftk::C_SceneNodeIterator it(&scene);

	for ( ; it; it++)
	{
		Aftk::C_SceneNode *node = it;
		Aftk::C_LayerNode *layer = node->GetLayerPtr(0);
		
		if (layer == 0) continue;
		if (layer->GetName().substr(0, 8) == "Cameras_")
		{
			// Convert the node to a C_CameraScene
			Cam2::AftkToCameraScene(&scene, node, outScene);
			break;
		}
	}
}

static void CameraExport(const Util::C_String& sceneName, const Util::C_String& outName)
{
	Sys::I_BinaryStream *file=0;
	
	Aftk::C_Global::Create();
	{
		Aftk::C_Scene scene;
		scene.Read(sceneName.Chars());

		file=Sys::C_FileSystem::Instance().Open(outName,Sys::E_FileAccess_Write);

		Cam2::C_CameraScene cameraScene;
		FindCameras(scene, cameraScene);

		if (file)
		{
			// Write camera scene to disk:
			Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(*file);

			S32 fileVersion = cameraScene.Version, pad = 0;
			archive << fileVersion << pad;
			archive << cameraScene;
		}

		if (file) { delete file; file=0; }
	}
	Aftk::C_Global::Destroy();
}

static bool ParseStringOption(
	const Sys::C_XmlDomBaseNode* pNode,
	const char* attributeName,
	Util::C_String& outValue
)
{
	const Sys::C_XmlDomBaseNode* attrNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pNode, attributeName );
	if (!attrNode)
		return false;

	const Sys::C_XmlAttribute* attr = 
	static_cast< const Sys::C_XmlDomElement* >( attrNode )->GetAttributes().GetAttributeByName("value");
	if (!attr)
		return false;

	outValue = attr->value;
	return true;
}

S32 C_Application::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_Options conversionOptions;
	bool success = true;

	Util::C_String aftkFilePath;

	Sys::C_XmlDom dom;
	//Build options struct
	{
		//Parse our command-line options.
		Util::C_String xmlFilePath;
		success = conversionOptions.ParseCommandLine(xmlFilePath, aftkFilePath, argc, argv);
		if (!success)
			return 1;

		if (!dom.Read(xmlFilePath.Chars()))
			return false;

		conversionOptions.ParseXml(xmlFilePath, dom);
		conversionOptions.SetInputAsset(aftkFilePath);
	}
	
	{
		Tools::Shim::C_FileCopyXmlExport::ClearXmlFileList(
			conversionOptions.GetInputTempFilePath(), 
			conversionOptions.GetExecuterNodeName());

		Tools::FileCopy::C_FileCopyList uberCopyList;

		// ---------------------- Do export to all platforms ----------------------
		for (Tools::E_Platform platform = Tools::E_Platform_Initial; platform < Tools::E_Platform_Count; platform++)
		{
			Util::C_String outputPath = "unknown";
			ParseStringOption(dom.GetRootNodePtr(), "shim_aftk_scene_collection_merged_path", outputPath);

			Util::C_String outputName = "unknown";
			ParseStringOption(dom.GetRootNodePtr(), "shim_aftk_scene_collection_merged_filename", outputName);
			
			Util::C_String csePath = conversionOptions.GetCopyLocations()[platform].workingPath + "\\" + outputPath + "\\" + outputName + ".cse";

			EXECUTER_INFO("camera_converter v%x.%.3x (%s,%s)\n",
				Version >> 12, Version & 0xfff,
				aftkFilePath.Chars(), csePath.Chars());

			CameraExport(aftkFilePath, csePath);

			Tools::FileCopy::C_FileCopyItem item(csePath, "");
			uberCopyList.AddFileCopyItem(platform, item);
		}
		// -----------------------------------------------------------------------

		success = Tools::Shim::C_FileCopyXmlExport::ExportXmlFileList(
			conversionOptions.GetRootTempPath(),
			uberCopyList, 
			conversionOptions.GetRootTempPath() + "\\ubercopy\\",
			conversionOptions.GetCopyLocations(),
			conversionOptions.GetLaunchComputerName(),
			conversionOptions.GetExecuterNodeName());
		if (!success) goto exit;
	}

exit:
	if (success)
	{
		EXECUTER_INFO("Processing completed succesfully!\n");
	}
	else
	{
		EXECUTER_INFO("Processing failed!\n");
	}

	if (success)
		return 0;
	else
		return 1;
}
