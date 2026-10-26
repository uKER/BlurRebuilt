#include "stdafx.h"
#include "game_shared/ExampleExportData.h"
#include "Options.h"
#include "ExampleConverter.h"
#include "renderingdata/renderingdata.h"

#include "renderingdata/archive.h"
#include "system/serialise/C_FileSystem.h"
#include "system/serialise/C_FileBlock.h"

#include <converter_shared/RenderDataConverter.h>

//#pragma optimize("", off)

static Debug::Logger &g_Log = Debug::GetLogger("ModelConverter");

std::string GetLower(const std::string& string)
{
	std::string output;
	output.resize(string.size());
	transform(string.begin(), string.end(), output.begin(), tolower);
	return output;
}

int GetRoute(const Aftk::C_SceneNode& aftkSceneNode)
{
	int route = -1;
	
	const Aftk::C_LayerNode* layer = aftkSceneNode.GetLayerPtr(0);
	
	if (layer && GetLower(layer->GetName().substr(0, 5)) == "route")
	{
		int offsetToNum = (GetLower(layer->GetName().substr(5, 1)) == "_") ? 6 : 5;	// Incase they've got Route_XX

		route = atoi(layer->GetName().substr(offsetToNum, 2).c_str());
	}

	return route;
}

// Take out some nodes from Gotham scenes.
bool ShouldExport(const Aftk::C_SceneNode& aftkSceneNode)
{
	const Aftk::C_LayerNode* layer = aftkSceneNode.GetLayerPtr(0);

	if (layer)
	{
		std::string lower = GetLower(layer->GetName());
		if (lower == "sky"
			|| lower == "noexport"
			|| lower == "occluders"
			|| lower.substr(0, 9) == "rotoscope"
			|| lower == "crowd_pvs_layer")
			return false;
	}

	if (GetLower(aftkSceneNode.GetName()).find("occluder") != std::string::npos)
		return false;

	if( (Aftk::C_DynamicPlug*)aftkSceneNode.GetPlugPtr("collisionPrimitive") != NULL)
		return false;

	if (GetRoute(aftkSceneNode) > -1)
		return false;

	return true;
}

bool C_ExampleConverter::Convert(
	C_ExampleExportData& output,
	Aftk::C_Scene& aftkScene,
	const C_Options& options)
{
	// use the converter_shared renderingdata converter
	RenderingData::C_Converter converter("c:\\temp", "z:\\mercurygame\\tools\\data\\shaders");

	converter.SetEffectVariation("LS");

	// this enables skinning
	//converter.EnableSkinning(true, 24, 4, "_skin");

	if (0)
	{
		// generate test scene
		converter.TestProcess();
	}
	else
	{
		Aftk::C_SceneNodeVisit::Traverse(
			&aftkScene, 
			Aftk::C_MeshTriangulate::C_TriangulateVisitor());

		// find all meshnodes and add them to the static list
		RenderingData::C_AftkSceneNodeList list;
		for (Aftk::C_SceneNodeConstIterator it(&aftkScene); &(*it); ++it)
		{
			Aftk::C_SceneNode &node = *it;
			if (node.GetType() == Aftk::E_ObjType_SceneMesh || node.GetType() == Aftk::E_ObjType_SceneInstance
				|| node.GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance )
				if (ShouldExport(node))
					list.PushBack(&node);
		}
		converter.AddStaticNodes(list);
		converter.Process();
	}

	aftkScene.Clear();

	{
		LOGINFO(g_Log, "Convert::Export360\n");

		Sys::C_FileSystem::Instance().CreateDirectory("output/360");
		Sys::I_File* pSceneFile = Sys::C_FileSystem::Instance().Open("output/360/model.dat", Sys::E_FileAccess_Write);
		Sys::I_File* pStreamFile = Sys::C_FileSystem::Instance().Open("output/360/model.stream", Sys::E_FileAccess_Write);

		pSceneFile->SetEndianSwapFlag(true);
		pStreamFile->SetEndianSwapFlag(true);

		if (!pSceneFile)
		{
			assert(!"Could not open file.");
			return false;
		}
		else
		{
			// root block
			Sys::C_FileBlock rootBlock(pSceneFile, "Render");

			// put scene in a block called scene
			{
				Sys::C_FileBlock sceneBlock(&rootBlock, "Scene");
				converter.ExportBinary(&sceneBlock, pStreamFile, RenderingData::PLATFORM_TYPE_360);
			}
		}

		delete pSceneFile;
		delete pStreamFile;
	}

	{
		LOGINFO(g_Log, "Convert::ExportPS3\n");

		Sys::C_FileSystem::Instance().CreateDirectory("output/PS3");
		Sys::I_File* pSceneFile = Sys::C_FileSystem::Instance().Open("output/PS3/model.dat", Sys::E_FileAccess_Write);
		Sys::I_File* pStreamFile = Sys::C_FileSystem::Instance().Open("output/PS3/model.stream", Sys::E_FileAccess_Write);

		pSceneFile->SetEndianSwapFlag(true);

		if (!pSceneFile)
		{
			assert(!"Could not open file.");
			return false;
		}
		else
		{
			// root block
			Sys::C_FileBlock rootBlock(pSceneFile, "Render");

			// put scene in a block called scene
			{
				Sys::C_FileBlock sceneBlock(&rootBlock, "Scene");
				converter.ExportBinary(&sceneBlock, pStreamFile, RenderingData::PLATFORM_TYPE_PS3);
			}
		}

		delete pSceneFile;
		delete pStreamFile;
	}

	{
		LOGINFO(g_Log, "Convert::ExportPC\n");

		Sys::C_FileSystem::Instance().CreateDirectory("output/PC");
		Sys::I_File* pSceneFile = Sys::C_FileSystem::Instance().Open("output/PC/model.dat", Sys::E_FileAccess_Write);
		Sys::I_File* pStreamFile = Sys::C_FileSystem::Instance().Open("output/PC/model.stream", Sys::E_FileAccess_Write);

		pSceneFile->SetEndianSwapFlag(false);

		if (!pSceneFile)
		{
			assert(!"Could not open file.");
			return false;
		}
		else
		{
			// root block
			Sys::C_FileBlock rootBlock(pSceneFile, "Render");

			// put scene in a block called scene
			{
				Sys::C_FileBlock sceneBlock(&rootBlock, "Scene");
				converter.ExportBinary(&sceneBlock, pStreamFile, RenderingData::PLATFORM_TYPE_PC);
			}
		}

		delete pSceneFile;
		delete pStreamFile;
	}

	{
		LOGINFO(g_Log, "Convert::ExportXML\n");

		Sys::C_FileSystem::Instance().CreateDirectory("output");
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open("output/model.xml", Sys::E_FileAccess_Write);

		if (!pFile)
		{
			assert(!"Could not open file.");
			return false;
		}
		else
		{
			converter.ExportXml(pFile, RenderingData::PLATFORM_TYPE_360);
		}

		delete pFile;
	}

	return true;
}
