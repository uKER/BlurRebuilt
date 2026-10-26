#ifndef __CONVERTER_SHARED_AFTK_MATERIAL_HELPERS__
#define __CONVERTER_SHARED_AFTK_MATERIAL_HELPERS__

namespace AftkHelpers
{

inline const Aftk::C_MaterialResource* GetMaterialResourcePtr(const Aftk::C_Polygon* pPoly)
{
	const Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	S16 localMaterialIndex = pPoly->GetMaterialMappingIndex();
	if (localMaterialIndex == -1)
	{
		return 0;
	}


	return pMesh->GetPolygonMaterialMappingPtr(localMaterialIndex);
}

// get a value from a plug (see specialisation below)
template <typename T>
void AFTKGetPlugValue(T& value, const Aftk::I_Plug& plug)
{
	plug.GetValue(value);
}

// if it's a vector type, look for compound value.
template <typename T>
void AFTKGetPlugValue(UtMaths::MVector<T>& value, const Aftk::I_Plug& plug)
{
	if (plug.GetCompoundType() != Aftk::E_GenericAttributeType_Null)
		plug.GetCompoundValue(value);
}

// find the plug with the given name on the shadernode and return it's value
template <typename T>
bool GetPlugValue(const Aftk::C_ShaderNode* shaderNode, T& value, const std::string& name)
{
	if (shaderNode)
	{
		for (U32 i = 0; i < shaderNode->GetNumPlugs(); i++)
		{
			const Aftk::I_Plug& plug = *shaderNode->GetPlugPtr(i);

			if (plug.GetName() == name)
			{
				AFTKGetPlugValue(value, plug);
				return true;
			}
		}
	}

	return false;
}

// find the plug with the given name on the shadernode and return it's string value
inline Util::C_String GetPlugValue(const Aftk::C_ShaderNode *shaderNode, const Util::C_String& name)
{
	std::string _name = name.Chars();
	std::string out = "";
	GetPlugValue(shaderNode, out, _name);
	return Util::C_String(out.c_str());
}

// find the plug with the specified name on the material
inline const Aftk::I_Plug* MaterialGetPlug(const Aftk::C_MaterialNode* pMaterial, const std::string& name)
{
	for (U32 i = 0; i < pMaterial->GetNumPlugs(); i++)
	{
		const Aftk::I_Plug& plug = *pMaterial->GetPlugPtr(i);
		if (plug.GetName() == name)
			return &plug;
	}

	return 0;
}

// get the shadernode connecte to the given plug
inline const Aftk::C_ShaderNode* PlugGetConnectedShader(const Aftk::I_Plug& plug)
{
	if (plug.IsConnected())
	{
		const Aftk::I_Object* pSourceConnection = plug.GetSourceConnectionPtr();
		if (pSourceConnection->HasType(Aftk::E_ObjType_Shader))
			return (Aftk::C_ShaderNode*)pSourceConnection;
		if (pSourceConnection->HasType(Aftk::E_ObjType_IPlug))
			return (Aftk::C_ShaderNode*)((Aftk::I_Plug*)pSourceConnection)->GetTopLevelParentPtr();
	}

	return 0;
}

// get attribute off node
inline const Aftk::C_GenericAttribute* FindAttribute(const Aftk::C_SceneNode& node, const std::string& name)
{
	if (node.GetBlindDataResourcePtr() == 0)
		return 0;
	else
	{
		const Aftk::C_BlindDataNode& bd = node.GetBlindDataResourcePtr()->GetContent();

		for (int i = 0; i < (int) bd.GetNumAttributes(); i++)
			if (bd.GetAttribute(i).GetName() == name)
				return &bd.GetAttribute(i);

		return 0;
	}
}


};

#endif
