#include "stdafx.h"

#include "AftkUt\AftkUtCreateMaterial.h"

using namespace AftkUt;

namespace AftkUt
{

/// constructor
MaterialCreator::MaterialCreator(std::string name)
{
	m_name = name;
}

/// add a texture to the material
void MaterialCreator::AddTexture(std::string name, std::string filename)
{
	m_textures.PushBack();
	m_textures.Back().name = name;
	m_textures.Back().filename = filename;
}


/// add a parameter to the material
void MaterialCreator::AddParameter(std::string name, float fValue)
{
	m_floats.PushBack();
	m_floats.Back().name = name;
	m_floats.Back().fValue = fValue;
}

/// create the material using the given parameters/textures
const Aftk::C_MaterialResource *MaterialCreator::CreateMaterial(Aftk::C_Scene* pScene, U32 materialTypeId)
{
	Aftk::C_MaterialNode aftkMaterialNode;
		
	// create the shader node
	Aftk::C_ShaderNode* pMyShader = new Aftk::C_ShaderNode;
	aftkMaterialNode.AddShaderNodePtr(pMyShader);
	pMyShader->SetName(m_name + "Shader");

	// create a plug on the material and attach the shader node to this
	Aftk::C_DynamicPlug* pNewPlug = new Aftk::C_DynamicPlug();
	aftkMaterialNode.AddPlugPtr(pNewPlug);
	pNewPlug->SetName("mayaSurfaceShader");
	pNewPlug->SetSourceConnectionPtr(pMyShader);

	// give the material a name
	if (m_textures.Size() > 0)
		aftkMaterialNode.SetName(m_name + "Material " + m_textures[0].name);
	else
		aftkMaterialNode.SetName(m_name + "Material ?");

	// create an image node and connect it to the shadernode
	for (TextureList::Iterator i=m_textures.Begin(); i!=m_textures.End(); ++i)
	{
		Aftk::C_ShaderImageNode* pImageNode = new Aftk::C_ShaderImageNode;
		aftkMaterialNode.AddShaderNodePtr(pImageNode);
		// give this an unique id so we can identify it at export
		aftkMaterialNode.SetMaterialTypeId(materialTypeId);
		// emulate maya material for the translator
		aftkMaterialNode.SetOrigin(Aftk::E_MaterialOrigin_Maya_CgFX);
		pImageNode->SetName(i->name);
		// get the path the textures are stored at (should be same as SPT file)
		pImageNode->GetFilePlug().SetValue(i->filename);
		pImageNode->GetImageTypePlug().SetValue("");

		// create a plug for the image node
		Aftk::C_DynamicPlug* pOutColourPlug = new Aftk::C_DynamicPlug();
		pImageNode->AddPlugPtr(pOutColourPlug);
		pOutColourPlug->SetName("outColor");

		// create a plug for the shader node and connect the two
		Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
		pMyShader->AddPlugPtr(pPlug);
		pPlug->SetName(i->name);
		pPlug->SetSourceConnectionPtr(pOutColourPlug);
	}

	// create a plug for each parameter
	for (FloatList::Iterator i=m_floats.Begin(); i!=m_floats.End(); ++i)
	{
		// create a plug for the shader node and connect the two
		Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
		pMyShader->AddPlugPtr(pPlug);
		pPlug->SetName(i->name);
		pPlug->SetValue(i->fValue);
	}
	
	// add the material to the scene
	return pScene->AddMaterialNode(aftkMaterialNode);
}

}
