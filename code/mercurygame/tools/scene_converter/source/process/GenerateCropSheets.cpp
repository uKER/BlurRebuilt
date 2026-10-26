#include "stdafx.h"

#include <tex_atlas/TexAtlas.h>

#include <core/containers/genesis_containers/Sort.h>
#include <AftkUt/AftkUt.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkProgress.h>
#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>
#include <data/SceneDescription.h>
#include <data/CropSheetData.h>

#include <process/GenerateCropSheets.h>

#include <tex_atlas/TexAtlas.h>
#include <tex_atlas/Atlas.h>

#include <btl/utility/string.h>
#include <btl/utility/pair.h>
#include <btl/containers/vector.h>
#include <btl/containers/openhashmap.h>

Btl::Vector<const Aftk::C_MaterialNode*>	g_HighVisMaterialNodes;
Btl::OpenHashMap<const char*, U32>			g_HighVisMaterialNodeMap;


void OnMessage(const std::string& msg)
{
	LOGDEBUG( Tools::g_log, "%s", msg.c_str());
}

void OnWarning(const std::string& msg)
{
	LOGWARN( Tools::g_log, "%s", msg.c_str());
}

void OnError(const std::string& msg)
{
	LOGERROR( Tools::g_log, "%s", msg.c_str());
}

std::string CrowdNormalAtlasName(const std::string& baseName, U32 atlasIndex, U32 layerIndex)
{
	std::stringstream ss;
	ss << Tu::RemoveFileExtension(baseName) << "_" << atlasIndex << "_" << layerIndex << "." << Tu::FileExtension(baseName);
	return ss.str();
}

std::string CrowdDiffIPRNeonAtlasName(const std::string& baseName, U32 atlasIndex, U32 layerIndex)
{
	std::stringstream ss;
	std::string layerName ;
	switch (layerIndex)
	{
	case 0: layerName = "DIFF"; break;
	case 1: layerName = "IPR0"; break;
	case 2: layerName = "NEON"; break;		
	};

	ss << Tu::RemoveFileExtension(baseName) << "_" << atlasIndex << "_" << layerName << "." << Tu::FileExtension(baseName);
	return ss.str();
}

bool TextureFileExists(const Util::C_String& filename)
{
	const bool bFileExistsOnDisk = Sys::C_FileSystem::Instance().FileExists(filename);

	if(!bFileExistsOnDisk)
		return false;

	const FileTag::C_Tag* pTag = 
		FileTag::C_TagDBManager::Instance().GetTagPtr( filename.Chars() );
	
	if (!pTag)
		return true;

	bool bTemp;
	if (pTag->GetAttributeValue("exclude", bTemp))
		return bTemp;
	else
		return true;
}

const Util::C_String C_ProcessGenerateCropSheets::GetName() const
{
	return Util::C_String("C_ProcessGenerateCropSheets");
}

bool C_ProcessGenerateCropSheets::Process( Progress::I_ProgressBar* pProgress )
{
	bool success = BuildCrowdCropSheet();
	if (success)
	{
		C_DataCropSheet *pCropSheetData = (C_DataCropSheet*) GetOutput(E_GraphData_CropSheet);
		BIZ_ASSERT(pCropSheetData);

		RemapCrowdMaterialAndUVs(pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_Crowd));
		RemapCrowdMaterialAndUVs(pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdNeon));
		RemapCrowdMaterialAndUVs(pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdHighVis));
		RemapCrowdMaterialAndUVs(pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdImposters));

#if 0
		const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
		std::string path = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\remapped.a3d";

		const C_DataAftkScene* pSceneData = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );
		assert(pSceneData);
		const Aftk::C_Scene *pScene = &pSceneData->GetAftkScene();
		pScene->Write(path);
#endif

	}

	return true;
}

void C_ProcessGenerateCropSheets::GetCrowdNodes(C_AftkNodeList& nodes)
{
#if 0
	const C_DataSceneDescription* pInputSceneDescription = (C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	const C_AftkNodeList& sceneNodes =  pInputSceneDescription->GetCrowdBaseInstanceNodes();
#else

	//TODO: remove dependency on the 'CrowdCropSheet' flag & use only the scene description...

	const C_DataAftkScene* pSceneData = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );
	assert(pSceneData);
	const Aftk::C_Scene *pScene = &pSceneData->GetAftkScene();

	for (Aftk::C_SceneNodeConstIterator it(pScene); it; ++it)
	{
		bool crowdCropSheet = false;
		AftkUt::C_NodeFlagParser nodeFlags((*it));
		nodeFlags.GetParameterValue("CrowdCropSheet", crowdCropSheet);
		if (crowdCropSheet)
		{
			if (it->HasType(Aftk::E_ObjType_SceneMesh))
				nodes.PushBack(it);
		}
		else if (it->HasType(Aftk::E_ObjType_SceneMesh))
		{
			Aftk::C_SceneNode* pParent = it->GetParentPtr();
			while (pParent)
			{
				AftkUt::C_NodeFlagParser nodeFlags(*pParent);
				nodeFlags.GetParameterValue("CrowdCropSheet", crowdCropSheet);
				if (crowdCropSheet)
				{
					nodes.PushBack(it);
					break;
				}
				pParent = pParent->GetParentPtr();
			}
		}
	}
#endif
}

void C_ProcessGenerateCropSheets::BuildCrowdTextureList(C_ProcessGenerateCropSheets::TextureDataArray& textures)
{
	Ut::Vector<Btl::Pair<U32, const Aftk::C_SceneNode*> > materialIds;
	//const C_DataSceneDescription* pInputSceneDescription = (C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	//const C_AftkNodeList& sceneNodes =  pInputSceneDescription->GetCrowdBaseInstanceNodes();

	const C_DataAftkScene* pSceneData = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );
	assert(pSceneData);
	const Aftk::C_Scene *pScene = &pSceneData->GetAftkScene();

	C_AftkNodeList sceneNodes;
	GetCrowdNodes(sceneNodes);

	for (S32 i=0; i<sceneNodes.Size(); ++i)
	{
		if (sceneNodes[i]->HasType(Aftk::E_ObjType_SceneMesh))
		{
			const Aftk::C_SceneMeshNode *pMesh = static_cast<const Aftk::C_SceneMeshNode*>(sceneNodes[i]);			

			for (U16 j=0; j<pMesh->GetNumPolygonMaterialMappings(); ++j)
				if (pMesh->GetPolygonMaterialMappingPtr(j))
					materialIds.PushBack(Btl::Pair<U32, const Aftk::C_SceneNode*>(pMesh->GetPolygonMaterialMappingPtr(j)->GetIndex(), pMesh));
		}

		//recurse into children
		//if (sceneNodes[i]->GetNumChildren())
		//{
		//	Aftk::C_SceneNodeConstIterator it(sceneNodes[i]->GetChildPtr());
		//	while (it)
		//	{
		//		if (it->HasType(Aftk::E_ObjType_SceneMesh))
		//		{
		//			const Aftk::C_SceneMeshNode *pMesh = static_cast<const Aftk::C_SceneMeshNode*>((Aftk::C_SceneNode*)it);			

		//			for (U16 j=0; j<pMesh->GetNumPolygonMaterialMappings(); ++j)
		//				if (pMesh->GetPolygonMaterialMappingPtr(j))
		//					materialIds.PushBack(pMesh->GetPolygonMaterialMappingPtr(j)->GetIndex());
		//		}
		//		++it;
		//	}
		//}
	}

	Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString > textureList;
	for (S32 i=0; i<materialIds.Size(); ++i)
	{
		if (materialIds[i].first >= pScene->GetNumMaterialNodes() || !pScene->GetMaterialResourcePtr(materialIds[i].first))
			continue;
	
		const Aftk::C_MaterialNode* pMaterial = &pScene->GetMaterialResourcePtr(materialIds[i].first)->GetContent();
		if (pMaterial->GetNumShaderImageNodes() == 0)
			continue;

		if (materialIds[i].second && pMaterial->GetOrigin() == Aftk::E_MaterialOrigin_Maya_Hlsl_2008)
		{
			if (pMaterial->GetName().find("HighVis") != std::string::npos)
			{
				g_HighVisMaterialNodes.PushBack(pMaterial);
				g_HighVisMaterialNodeMap.Insert(materialIds[i].second->GetName().c_str(), static_cast<U32>(g_HighVisMaterialNodes.Size()) - 1);
			}
		}

		//use the first texture
		const Aftk::C_ShaderImageNode* pImage = pMaterial->GetShaderImageNodePtr(0);
		std::string path;
		pImage->GetFilePlug().GetValue(path);
		std::transform(path.begin(), path.end(), path.begin(), tolower);

		textureList.Insert(Util::C_String(path.c_str()));
	}

	Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString >::Iterator it = textureList.Begin();
	while (it != textureList.End())
	{
		textures.PushBack();
		textures.Back().path = it->GetKey();
		textures.Back().flags.value = 0;

		if (it->GetKey().ISearch("_diff") != -1)
		{
			Util::C_String tmp;
			
			tmp = it->GetKey().IReplace("_diff", "_ipr0");
			if (TextureFileExists(tmp))
				textures.Back().flags.hasIPR0 = true;
			else
				textures.Back().flags.hasIPR0 = false;

			tmp = it->GetKey().IReplace("_diff", "_neon");
			if (TextureFileExists(tmp))
				textures.Back().flags.hasNEON = true;
			else
				textures.Back().flags.hasNEON = false;

			if (it->GetKey().ISearch("hivis") != -1)
				textures.Back().flags.isHighVis = true;
			else
				textures.Back().flags.isHighVis = false;
		}
		if ((it->GetKey().ISearch("_front") != -1) || (it->GetKey().ISearch("_back") != -1) ||
			(it->GetKey().ISearch("_side") != -1))
		{
			textures.Back().flags.isImposter = true;
		}

		it++;
	}
}

bool C_ProcessGenerateCropSheets::BuildCrowdCropSheet()
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);
	if (!pInputConversionOptions->GetToolConfig().GetCrowdCropSheetOptions().IsValid())
	{
		LOGDEBUG( Tools::g_log, "GenerateCropSheets: Skipping Crowd CropSheet Generation");
		return true;
	}

	const C_DataAftkScene* pSceneData = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );
	assert(pSceneData);

	//HACK: ideally need another aftk scene data
	Aftk::C_Scene *pScene = const_cast<Aftk::C_Scene*>(&pSceneData->GetAftkScene());
	BIZ_ASSERT(pScene);

	TextureDataArray textureList;
	BuildCrowdTextureList(textureList);

	if (!textureList.Size())
		return true;

	Sys::C_FileSystem::Instance().CreateDirectory( pInputConversionOptions->GetCrowdCropSheetDirectory());
	const C_CrowdCropSheetOptions& csOptions = pInputConversionOptions->GetToolConfig().GetCrowdCropSheetOptions();

	//create normal texture atlas
	{
		TexAtlas::TextureKeyArray textureKeys;
		for (S32 i=0; i<textureList.Size(); ++i)
		{
			if (textureList[i].flags.value == 0)
			{
				textureKeys.PushBack();
				TexAtlas::TextureKey& key = textureKeys.Back();
				key.SetFileName(textureList[i].path.Chars());
				key.SetBaseIndex(textureKeys.Size()-1);
				key.SetLayerIndex(0);
			}
		}

		if (textureKeys.Size())
		{
			std::string imageDb = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\cropsheet_normal.dat";
			std::string atlas = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\crowdatlas.tga";

			Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > backgroundColours;
			backgroundColours.PushBack(LiteMaths::Vec4U8(0, 0, 0, 0));

			TexAtlas::Atlas::SetAtlasNameCallback(&CrowdNormalAtlasName);
			Attk::C_Status status = TexAtlas::AutoScaleCropSheet::Build(textureKeys, false, imageDb, atlas, csOptions.width, csOptions.height, csOptions.border, 
				0.f, csOptions.minImageSize, csOptions.maxImageSize, false, true, backgroundColours, 
				TexAtlas::AutoScaleCropSheet::AUTO_FIND_CROP_SHEET_SIZE_MULTI,
				OnMessage, OnWarning, OnError, NULL);

			TexAtlas::Atlas::SetAtlasNameCallback(0);
			if (status)
			{
				C_DataCropSheet *pCropSheetData = (C_DataCropSheet*) GetOutput(E_GraphData_CropSheet);
				BIZ_ASSERT(pCropSheetData);
				pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_Crowd).Read(imageDb);
			}
		}
	}

	//create neon texture atlas
	{
		TexAtlas::TextureKeyArray textureKeys;
		U32 baseIndex = 0;
		for (S32 i=0; i<textureList.Size(); ++i)
		{
			if (textureList[i].flags.hasIPR0 && textureList[i].flags.hasNEON && !textureList[i].flags.isHighVis)
			{
				textureKeys.PushBack();
				{
					TexAtlas::TextureKey& key = textureKeys.Back();
					key.SetFileName(textureList[i].path.Chars());
					key.SetBaseIndex(baseIndex);
					key.SetLayerIndex(0);
				}
				textureKeys.PushBack();
				{
					TexAtlas::TextureKey& key = textureKeys.Back();
					key.SetFileName(textureList[i].path.IReplace("_diff", "_ipr0").Chars());
					key.SetBaseIndex(baseIndex);
					key.SetLayerIndex(1);
				}
				textureKeys.PushBack();
				{
					TexAtlas::TextureKey& key = textureKeys.Back();
					key.SetFileName(textureList[i].path.IReplace("_diff", "_neon").Chars());
					key.SetBaseIndex(baseIndex);
					key.SetLayerIndex(2);
				}
				++baseIndex;
			}
		}

		if (textureKeys.Size())
		{
			std::string imageDb = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\cropsheet_neon.dat";
			std::string atlas = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\crowdatlas_neon.tga";

			Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > backgroundColours;
			backgroundColours.PushBack(LiteMaths::Vec4U8(0, 0, 0, 0));
			backgroundColours.PushBack(LiteMaths::Vec4U8(0, 0, 0, 0));
			backgroundColours.PushBack(LiteMaths::Vec4U8(0, 0, 0, 0));

			TexAtlas::Atlas::SetAtlasNameCallback(&CrowdDiffIPRNeonAtlasName);
			Attk::C_Status status = TexAtlas::AutoScaleCropSheet::Build(textureKeys, false, imageDb, atlas, csOptions.width, csOptions.height, csOptions.border, 
				0.f, csOptions.minImageSize, csOptions.maxImageSize, false, true, backgroundColours, 
				TexAtlas::AutoScaleCropSheet::AUTO_FIND_CROP_SHEET_SIZE_MULTI,
				OnMessage, OnWarning, OnError, NULL);
			TexAtlas::Atlas::SetAtlasNameCallback(0);



			if (status)
			{
				C_DataCropSheet *pCropSheetData = (C_DataCropSheet*) GetOutput(E_GraphData_CropSheet);
				BIZ_ASSERT(pCropSheetData);
				pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdNeon).Read(imageDb);
			}
		}
	}

	//create highvis texture atlas
	{
		TexAtlas::TextureKeyArray textureKeys;
		U32 baseIndex = 0;
		for (S32 i=0; i<textureList.Size(); ++i)
		{
			if (textureList[i].flags.hasIPR0 && textureList[i].flags.hasNEON && textureList[i].flags.isHighVis)
			{
				textureKeys.PushBack();
				{
					TexAtlas::TextureKey& key = textureKeys.Back();
					key.SetFileName(textureList[i].path.Chars());
					key.SetBaseIndex(baseIndex);
					key.SetLayerIndex(0);
				}
				textureKeys.PushBack();
				{
					TexAtlas::TextureKey& key = textureKeys.Back();
					key.SetFileName(textureList[i].path.IReplace("_diff", "_ipr0").Chars());
					key.SetBaseIndex(baseIndex);
					key.SetLayerIndex(1);
				}
				textureKeys.PushBack();
				{
					TexAtlas::TextureKey& key = textureKeys.Back();
					key.SetFileName(textureList[i].path.IReplace("_diff", "_neon").Chars());
					key.SetBaseIndex(baseIndex);
					key.SetLayerIndex(2);
				}
				++baseIndex;
			}
		}

		if (textureKeys.Size())
		{
			std::string imageDb = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\cropsheet_highvis.dat";
			std::string atlas = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\crowdatlas_highvis.tga";

			Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > backgroundColours;
			backgroundColours.PushBack(LiteMaths::Vec4U8(0, 0, 0, 0));
			backgroundColours.PushBack(LiteMaths::Vec4U8(0, 0, 0, 0));
			backgroundColours.PushBack(LiteMaths::Vec4U8(0, 0, 0, 0));

			TexAtlas::Atlas::SetAtlasNameCallback(&CrowdDiffIPRNeonAtlasName);
			Attk::C_Status status = TexAtlas::AutoScaleCropSheet::Build(textureKeys, false, imageDb, atlas, csOptions.width, csOptions.height, csOptions.border, 
				0.f, csOptions.minImageSize, csOptions.maxImageSize, false, true, backgroundColours, 
				TexAtlas::AutoScaleCropSheet::AUTO_FIND_CROP_SHEET_SIZE_MULTI,
				OnMessage, OnWarning, OnError, NULL);
			TexAtlas::Atlas::SetAtlasNameCallback(0);



			if (status)
			{
				C_DataCropSheet *pCropSheetData = (C_DataCropSheet*) GetOutput(E_GraphData_CropSheet);
				BIZ_ASSERT(pCropSheetData);
				pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdHighVis).Read(imageDb);
			}
		}
	}

	//create imposters texture atlas
	{
		TexAtlas::TextureKeyArray textureKeys;
		for (S32 i=0; i<textureList.Size(); ++i)
		{
			if (textureList[i].flags.isImposter)
			{
				textureKeys.PushBack();
				TexAtlas::TextureKey& key = textureKeys.Back();
				key.SetFileName(textureList[i].path.Chars());
				key.SetBaseIndex(textureKeys.Size()-1);
				key.SetLayerIndex(0);
			}
		}

		if (textureKeys.Size())
		{
			std::string imageDb = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\cropsheet_imposters.dat";
			std::string atlas = std::string(pInputConversionOptions->GetCrowdCropSheetDirectory().Chars()) + "\\crowdatlas_imposters.tga";

			Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > backgroundColours;
			backgroundColours.PushBack(LiteMaths::Vec4U8(0, 0, 0, 0));

			TexAtlas::Atlas::SetAtlasNameCallback(&CrowdNormalAtlasName);
			Attk::C_Status status = TexAtlas::AutoScaleCropSheet::Build(textureKeys, false, imageDb, atlas, csOptions.width, csOptions.height, csOptions.border, 
				0.f, csOptions.minImageSize, csOptions.maxImageSize, false, true, backgroundColours, 
				TexAtlas::AutoScaleCropSheet::AUTO_FIND_CROP_SHEET_SIZE_MULTI,
				OnMessage, OnWarning, OnError, NULL);

			TexAtlas::Atlas::SetAtlasNameCallback(0);
			if (status)
			{
				C_DataCropSheet *pCropSheetData = (C_DataCropSheet*) GetOutput(E_GraphData_CropSheet);
				BIZ_ASSERT(pCropSheetData);
				pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdImposters).Read(imageDb);
			}
		}
	}

	const C_DataCropSheet *pCropSheetData = (C_DataCropSheet*) GetOutput(E_GraphData_CropSheet);
	return (pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_Crowd).NumImageRecords() + 
			pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdNeon).NumImageRecords() +
			pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdHighVis).NumImageRecords() +
			pCropSheetData->GetDatabase(C_DataCropSheet::E_CropSheetType_CrowdImposters).NumImageRecords()) > 0;

}

F32 FixTexCoord(const F32 f)
{
	F32 value = 0.f;
	
	if (f > AFTK_EPSILON && ::fmodf(::abs(f), 1.f) <= AFTK_EPSILON)
		value = 1.f;
	else
		value = ::abs(f) - ::floorf(::abs(f));
	
	return ((f >= 0.f) ? value : (1.f - value));
}

const Aftk::C_MaterialResource* C_ProcessGenerateCropSheets::CreateAtlasMaterialResource(Aftk::C_Scene *pScene, const std::string& fileName, const Aftk::C_MaterialNode* pMaterialNode)
{
	if (pMaterialNode && pMaterialNode->GetNumShaderImageNodes())
	{
		Aftk::C_MaterialNode aftkMaterialNode(*pMaterialNode);

#if 0
		for (U32 iNode = 0, count = aftkMaterialNode.GetNumShaderNodes(); iNode < count; ++iNode)
		{
			Aftk::C_ShaderNode *pShaderNode = aftkMaterialNode.GetShaderNodePtr(iNode);

			if (pShaderNode && pShaderNode->GetOriginalNodeTypeName() == "hlslShader" && pShaderNode->GetNumDynamicPlugs())
			{
				for (U32 iPlug = 0, plugCount = pShaderNode->GetNumDynamicPlugs(); iPlug < plugCount; ++iPlug)
				{
					Aftk::C_DynamicPlug *pPlug = pShaderNode->GetDynamicPlugPtr(iPlug);

					if (pPlug && pPlug->GetDataType() == Aftk::E_GenericAttributeType_F32)
					{
						F32 value = 0.f;
						if (pPlug->GetValue(value))
						{
							volatile S32 breakhere = 1;
						}
					}
				}
			}
		}
#endif

		for (U32 iImage = 0, count = aftkMaterialNode.GetNumShaderImageNodes(); iImage < count; ++iImage)
		{
			Aftk::C_ShaderImageNode *pShaderImageNode = aftkMaterialNode.GetShaderImageNodePtr(iImage);

			if (pShaderImageNode)
			{
				std::string imagePath;
				if (pShaderImageNode->GetFilePlug().GetValue(imagePath))
				{
					if (imagePath.find("_DIFF") != std::string::npos)
					{
						pShaderImageNode->GetFilePlug().SetValue(fileName);
						pShaderImageNode->GetImageTypePlug().SetValue("");
						break;
					}
				}
			}
		}

		aftkMaterialNode.SetName("CrowdCropSheetMaterial");

		return pScene->AddMaterialNode(aftkMaterialNode);
	}
	else
	{
		Aftk::C_MaterialNode aftkMaterialNode;
		aftkMaterialNode.SetOrigin(Aftk::E_MaterialOrigin_Maya);
		
		Aftk::C_ShaderNode *pShaderNode = new Aftk::C_ShaderNode();
		pShaderNode->SetName("shader");
		aftkMaterialNode.AddShaderNodePtr(pShaderNode);
		{
			Aftk::C_ShaderImageNode *pImageNode = new Aftk::C_ShaderImageNode;
			aftkMaterialNode.AddShaderNodePtr(pImageNode);
			pImageNode->SetName("file1");

			pImageNode->GetFilePlug().SetValue(fileName);
			pImageNode->GetImageTypePlug().SetValue("");

			Aftk::C_DynamicPlug *pOutColorPlug = new Aftk::C_DynamicPlug();
			pImageNode->AddPlugPtr(pOutColorPlug);
			pOutColorPlug->SetName("outColor");

			Aftk::C_DynamicPlug* pColorPlug = new Aftk::C_DynamicPlug();
			pShaderNode->AddPlugPtr(pColorPlug);
			pColorPlug->SetName("color");
			pColorPlug->SetSourceConnectionPtr(pOutColorPlug);
		}

		{
			Aftk::C_DynamicPlug* pNewPlug = new Aftk::C_DynamicPlug();
			aftkMaterialNode.AddPlugPtr(pNewPlug);
			pNewPlug->SetName("mayaSurfaceShader");
			pNewPlug->SetSourceConnectionPtr(pShaderNode);
		}

		aftkMaterialNode.SetName("CrowdCropSheetMaterial");

		return pScene->AddMaterialNode(aftkMaterialNode);
	}
}

bool C_ProcessGenerateCropSheets::RemapCrowdMaterialAndUVs(const TexAtlas::Database& database)
{
	const C_DataAftkScene* pSceneData = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );
	assert(pSceneData);

	//HACK: we ideally need another aftk scene data
	Aftk::C_Scene *pScene = const_cast<Aftk::C_Scene*>(&pSceneData->GetAftkScene());
	BIZ_ASSERT(pScene);

	if (database.NumAtlasRecords() == 0)
		return true;

	Ut::Vector<const Aftk::C_MaterialResource*> materialResources;
	materialResources.Resize(static_cast<S32>(database.NumAtlasRecords()));
	for (U32 i = 0, count = database.NumAtlasRecords(); i < count; ++i)
		materialResources[i] = NULL;

	C_AftkNodeList sceneNodes;
	GetCrowdNodes(sceneNodes);

	//now do the remapping...
	for (S32 i=0; i<sceneNodes.Size(); ++i)
	{
		Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)sceneNodes[i];

		Ut::Vector<S32> cropSheetImageRecordIds;
		if (!pMesh->GetNumPolygonMaterialMappings() || !pMesh->GetNumPolygons(0) || !pMesh->GetNumPolygonVertices())
			continue;

		cropSheetImageRecordIds.Resize(pMesh->GetNumPolygonMaterialMappings());
		for (S16 j=0; j<(S16)pMesh->GetNumPolygonMaterialMappings(); ++j)
		{
			cropSheetImageRecordIds[j] = -1;

			const Aftk::C_MaterialResource *pOldMaterialRes =  pMesh->GetPolygonMaterialMappingPtr(j);
			const Aftk::C_MaterialNode* pOldMaterial = &pOldMaterialRes->GetContent();
			if (pOldMaterial->GetNumShaderImageNodes() == 0)
				continue;

			const Aftk::C_ShaderImageNode* pImage = pOldMaterial->GetShaderImageNodePtr(0);
			std::string path;
			pImage->GetFilePlug().GetValue(path);
			std::transform(path.begin(), path.end(), path.begin(), tolower);

			//nasty linear search for the texture in database
			for (S32 k=0; k<(S32)database.NumImageRecords(); ++k)
			{
				if (database.ImageRecord(k).key.FileName() == path)
				{
					cropSheetImageRecordIds[j] = k;
					break;
				}
			}
		}

		Aftk::PolygonArray& polygons = pMesh->GetPolygonArray(0);
		Ut::BitVector remappedPolyVertices;
		remappedPolyVertices.Resize(pMesh->GetNumPolygonVertices());
		remappedPolyVertices.ClearBits();

		for (S32 j=0; j<polygons.Size(); ++j)
		{
			Aftk::C_Polygon& polygon = polygons[j];

			S16 material = polygon.GetMaterialMappingIndex();
			if (material == -1 || cropSheetImageRecordIds[material]==-1)
				continue;

			const TexAtlas::Database::DBImageRecord& imageRecord = database.ImageRecord(cropSheetImageRecordIds[material]);
			
			if (materialResources[imageRecord.atlasIndex] == NULL)
			{
				const Aftk::C_MaterialNode* pMaterialNode = NULL;
				Btl::OpenHashMap<const char*, U32>::Iterator iter = g_HighVisMaterialNodeMap.Find(pMesh->GetName().c_str());

				if (iter != g_HighVisMaterialNodeMap.End())
					pMaterialNode = g_HighVisMaterialNodes[iter->GetValue()];

				materialResources[imageRecord.atlasIndex] = CreateAtlasMaterialResource(pScene, database.AtlasRecord(imageRecord.atlasIndex).fileName, pMaterialNode);
			}

			S16 newMaterialIndex = pMesh->AddPolygonMaterialMapping(materialResources[imageRecord.atlasIndex]);

			polygon.SetMaterialMappingIndex(newMaterialIndex);
			for (S32 k=0; k<(S32)polygon.GetNumPolygonVertices(); ++k)
			{
				U32 pvIndex = polygon.GetPolygonVertexIndex(k);
				if (!remappedPolyVertices.GetBitValue(pvIndex))
				{
					Aftk::C_PolygonVertex& pv = polygon.GetPolygonVertex(k);
					S32 uvIndex = pv.GetTextureCoordinateIndex(0);
					if (uvIndex != -1)
					{
						//add a new uv coordinate rather than modifying the existing one as it may be shared.
						Aftk::TextureCoordinate4 uv = pMesh->GetTextureCoordinate(0, uvIndex);
						uv[0] = imageRecord.uOffset + (imageRecord.uScale * FixTexCoord(uv[0]));
						uv[1] = imageRecord.vOffset + (imageRecord.vScale * FixTexCoord(uv[1]));

						S32 newUvIndex = pMesh->AddTextureCoordinate(0, uv);
						pv.SetTextureCoordinateIndex(0, newUvIndex);
					}
					remappedPolyVertices.SetBit(pvIndex);
				}
			}
		}
	}

	return true;
}