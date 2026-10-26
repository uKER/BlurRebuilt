#include "stdafx.h"
#include <AftkSpeedTree\AftkSpeedTreeUtility.h>
#include <AftkSpeedTree\AftkSpeedTreeData.h>

using namespace AftkSpeedTree;

const Aftk::C_GenericAttribute* Utility::FindAttribute(const Aftk::C_SceneNode& node, const Util::C_String& name)
{
	if (node.GetBlindDataResourcePtr() == 0)
		return 0;
	else
	{
		const Aftk::C_BlindDataNode& bd = node.GetBlindDataResourcePtr()->GetContent();

		for (int i = 0; i < (int) bd.GetNumAttributes(); i++)
			if (bd.GetAttribute(i).GetName() == name.Chars())
				return &bd.GetAttribute(i);

		return 0;
	}
}

void Utility::AddShaderImageNode(Aftk::C_MaterialNode *pNode, Aftk::C_ShaderNode *pShaderNode, std::string sNodeName, const Util::C_String &strFilename)
{
	Aftk::C_ShaderImageNode* pImageNode = new Aftk::C_ShaderImageNode;
	pNode->AddShaderNodePtr(pImageNode);
	// give this an unique id so we can identify it at export
//	pNode->SetMaterialTypeId(materialTypeId);
	// emulate maya material for the translator
	pNode->SetOrigin(Aftk::E_MaterialOrigin_Maya_Hlsl_2008);
	pImageNode->SetName("file1");
	// get the path the textures are stored at (should be same as SPT file)
	pImageNode->GetFilePlug().SetValue(strFilename.Chars());
	pImageNode->GetImageTypePlug().SetValue("");

	Aftk::C_DynamicPlug* pOutColourPlug = new Aftk::C_DynamicPlug();
	pImageNode->AddPlugPtr(pOutColourPlug);
	pOutColourPlug->SetName("outColor");

	Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
	pShaderNode->AddPlugPtr(pPlug);
	pPlug->SetName(sNodeName);
	pPlug->SetSourceConnectionPtr(pOutColourPlug);
}

const Aftk::C_MaterialResource* Utility::CreateMaterial(Aftk::C_Scene* pScene, const Util::C_String& strFilename, const Util::C_String& strNormalMapFilename, std::string sFXName)
{
	Aftk::C_MaterialNode aftkMaterialNode;
	
	bool bDefaultShader = false;
	{
		Aftk::C_ShaderNode* pSpeedTreeShader = new Aftk::C_ShaderNode;
		aftkMaterialNode.AddShaderNodePtr(pSpeedTreeShader);
		pSpeedTreeShader->SetName("SpeedTreeShader");

		AddShaderImageNode(&aftkMaterialNode, pSpeedTreeShader, "Colour", strFilename.Chars());
		AddShaderImageNode(&aftkMaterialNode, pSpeedTreeShader, "Normal", strNormalMapFilename.Chars());
		
		{
			Aftk::C_DynamicPlug* pShaderPlug = new Aftk::C_DynamicPlug();
			pSpeedTreeShader->AddPlugPtr(pShaderPlug);
			pShaderPlug->SetValue(sFXName);
			pShaderPlug->SetName("shader");
		}
		
		{
			Aftk::C_DynamicPlug* pNewPlug = new Aftk::C_DynamicPlug();
			aftkMaterialNode.AddPlugPtr(pNewPlug);
			pNewPlug->SetName("mayaSurfaceShader");
			pNewPlug->SetSourceConnectionPtr(pSpeedTreeShader); 
		}
	}

	Util::C_String	speedTreeMaterial = "SpeedTreeMaterial ";
	Util::C_String	materialName = speedTreeMaterial + strFilename;
	aftkMaterialNode.SetName(materialName.Chars());
	
	return pScene->AddMaterialNode(aftkMaterialNode);
}

