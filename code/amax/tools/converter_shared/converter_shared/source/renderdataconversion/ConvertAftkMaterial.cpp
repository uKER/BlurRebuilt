#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\MaterialManager.h"
#include "converter_shared\renderdataconversion\ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\Effect.h"
#include <converter_shared/helpers/AftkMaterialHelpers.h>
#include <system/misc/StringUtil.h>

// to generate the empty texture for the NULL material
#include <texture_export/texture.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <system/serialise.h>

namespace RenderDataConversion
{

/// constructor
AftkMaterialConverter::AftkMaterialConverter(MaterialManager *pMgr, const Aftk::C_MaterialNode *pMtl, E_NodeType type = E_NodeType_Mesh)
{
	m_pMtl = pMtl;
	m_pNode = NULL;
	m_pManager = pMgr;
	m_iSortType = RenderingData::SORT_TYPE_KEY;
	m_NodeType = type;	
	m_pAftkShaderNode = NULL;
	m_pAftkLightingNode = NULL;
	m_blendState = (int&)RenderingData::BlendState();
	Setup();
}

/// constructor (for exporting material data from an Aftk Node, e.g. UV offset, SH lighting etc.)
AftkMaterialConverter::AftkMaterialConverter(MaterialManager *pMgr, const Aftk::C_SceneNode *pNode)
{
	m_pMtl = NULL;
	m_pNode = pNode;
	m_pManager = pMgr;
	m_iSortType = RenderingData::SORT_TYPE_KEY;
	m_NodeType = E_NodeType_Instance;
	m_pAftkShaderNode = NULL;
	m_pAftkLightingNode = NULL;
	Setup();
}

AftkMaterialConverter::~AftkMaterialConverter()
{
}

/// specify the permutated .fx name to use
void AftkMaterialConverter::SetPermutatedFXName(const Util::C_String &sName)
{
	m_sPermutatedFXName = sName;
}

// 
RenderingData::Build::Scene *AftkMaterialConverter::GetSceneExport()
{
	return m_pManager->m_pSceneExport;
}

TextureExport::TextureManager *AftkMaterialConverter::GetTextureManager()
{
	return m_pManager->m_pTextureManager;
}

EffectManager *AftkMaterialConverter::GetEffectManager()
{
	return m_pManager->m_pEffectManager;
}

/// setup some shared variables to be used by the conversion process
void AftkMaterialConverter::Setup()
{
	m_sPermutatedFXName = GetFXName();

	const Aftk::C_MaterialNode* pMaterialNode = m_pMtl;
	
	m_pAftkShaderNode = NULL;
	m_pAftkLightingNode = NULL;

	if (m_pMtl)
	{
		// get material node
		const Aftk::I_Plug* plug = AftkHelpers::MaterialGetPlug(pMaterialNode, "mayaSurfaceShader");
		m_materialSourceType = E_MaterialSourceType_Maya;

		if (!plug)
		{			
			for (U32 i = 0; i < pMaterialNode->GetNumShaderNodes(); i++)
			{
				const Aftk::C_ShaderNode* pShader = pMaterialNode->GetShaderNodePtr(i);

				if (pShader && (pShader->GetName().find("XSIMaterial") != std::string::npos || pShader->GetName().find("BaseMaterial") != std::string::npos))
				{
					m_pAftkShaderNode = pShader;
					m_materialSourceType = E_MaterialSourceType_XSI;
					break;
				}
			}

			if (!m_pAftkShaderNode)
			{
				plug = AftkHelpers::MaterialGetPlug(pMaterialNode, "RealTime");
				if (plug) m_materialSourceType = E_MaterialSourceType_XSI;
			}

		}

		if (plug)
			m_pAftkShaderNode = AftkHelpers::PlugGetConnectedShader(*plug);

		// get lighting node
		plug = AftkHelpers::MaterialGetPlug(pMaterialNode, "lux");

		if (plug)
			m_pAftkLightingNode = AftkHelpers::PlugGetConnectedShader(*plug);
	}

	if (m_pNode)
	{
		//
	}
}

/// return the name of the .fx file that is associated with this material
Util::C_String AftkMaterialConverter::GetFXName()
{
	if (m_pMtl && m_pMtl->GetOrigin() == Aftk::E_MaterialOrigin_Maya_Hlsl_2008)
	{
		Util::C_String sShader = AftkHelpers::GetPlugValue(m_pAftkShaderNode, "shader");

		// strip path
		return Util::ExtractFilename(sShader.Chars());
	}
	else if (m_pMtl && m_pMtl->GetOrigin() == Aftk::E_MaterialOrigin_XSI)
	{
		//Hard-code this for vehicle shaders for the mo...
		Util::C_String sShader = "";
		if (m_pAftkShaderNode)
		{
			int i = m_pAftkShaderNode->GetName().find("BaseMaterial_");
			if (i != std::string::npos && i + 12 < m_pAftkShaderNode->GetName().size())
			{				
				std::string fileName("vehicle");
				fileName.append(m_pAftkShaderNode->GetName().substr(i+12));
				fileName.append(".fx");
				return Util::C_String(fileName.c_str());
			}
		}
		
		if (sShader == "")
			return g_szDefaultFXName;

		// strip path
		return Util::ExtractFilename(sShader.Chars());
	}

	return g_szDefaultFXName;
}

/// return the filename of the texture on the material with the given plug name
Util::C_String AftkMaterialConverter::GetTextureFilenameFromPlug(const char* plugName)
{
	std::string filename;

	//If it's an XSI material, we have to find the relevant shader node
	if (m_pMtl && m_pMtl->GetOrigin() == Aftk::E_MaterialOrigin_XSI)
	{
		for (U32 iShader = 0; iShader < m_pMtl->GetNumShaderNodes(); iShader++)
		{
			const Aftk::C_ShaderNode* pShader = m_pMtl->GetShaderNodePtr(iShader);
				
			if (pShader->GetName() == plugName)
			{
				if (const Aftk::I_Plug* pPlug = pShader->GetPlugPtr("tex"))
				{
					if (const Aftk::C_ShaderNode* map = AftkHelpers::PlugGetConnectedShader(*pPlug))
					{
						if (map->GetType() == Aftk::E_ObjType_ShaderImage)
						{
							((Aftk::C_ShaderImageNode*)map)->GetFilePlug().GetValue(filename);
							return Util::C_String(filename.c_str());
						}
					}
				}
			}
		}
	}


	if (plugName && m_pAftkShaderNode)
	{
		for (U32 i = 0; i < m_pAftkShaderNode->GetNumPlugs(); i++)
		{
			const Aftk::I_Plug* pPlug = m_pAftkShaderNode->GetPlugPtr(i);
			if (pPlug->GetName() == plugName)
			{
				const Aftk::C_ShaderNode* map = AftkHelpers::PlugGetConnectedShader(*pPlug);

				if (map)
				{
					if (map->GetType() == Aftk::E_ObjType_ShaderImage)
					{
						((Aftk::C_ShaderImageNode*)map)->GetFilePlug().GetValue(filename);
						return Util::C_String(filename.c_str());
					}
					else if (map->GetType() == Aftk::E_ObjType_Shader) 
					{
						for (U32 i = 0; i < map->GetNumPlugs(); i++)
						{
							if (map->GetPlugPtr(i)->GetName() == "tex") // xsi
							{
								const Aftk::C_ShaderNode* map2 = AftkHelpers::PlugGetConnectedShader(*map->GetPlugPtr(i));

								if (map2->GetType() == Aftk::E_ObjType_ShaderImage)
								{
									((Aftk::C_ShaderImageNode*)map2)->GetFilePlug().GetValue(filename);
									return Util::C_String(filename.c_str());
								}
							}
						}
					}
				}
			}
		}
	}
	return "";
}

/// return the filename of the diffuse texture
Util::C_String AftkMaterialConverter::GetDiffuseTextureFilename()
{
	std::string texturePlugName;
	if (m_materialSourceType == E_MaterialSourceType_Maya)
		texturePlugName = "color";
	else
		texturePlugName = "diffuse";

	return GetTextureFilenameFromPlug(texturePlugName.c_str());
}

/// return the filename of the lightmap texture
Util::C_String AftkMaterialConverter::GetLightmapTextureFilename()
{
	if (m_pAftkLightingNode)
	{
		// find the plug containing the lightmap filename
		for (U32 i = 0; i < m_pAftkLightingNode->GetNumPlugs(); i++)
		{
			const Aftk::C_ShaderNode* map = AftkHelpers::PlugGetConnectedShader(*m_pAftkLightingNode->GetPlugPtr(i));

			if (map && map->GetType() == Aftk::E_ObjType_ShaderImage)
			{
				std::string fileName;
				((Aftk::C_ShaderImageNode*) map)->GetFilePlug().GetValue(fileName);

				return Util::C_String(fileName.c_str());
			}
		}
	}
	else if (m_pNode)
	{
		// try to get the lightmap filename off the instance
		const Aftk::C_GenericAttribute* attribute = AftkHelpers::FindAttribute(*m_pNode, "AtlasName");
		if (attribute)
		{
			std::string fileName;
			attribute->GetValue(fileName);
			return Util::C_String(fileName.c_str());
		}
	}

	return "";
}

/// set the sort-type for this material - called by the TextureAnnotationReader class
void AftkMaterialConverter::SetSortType(int type)
{
	if (m_iSortType == RenderingData::SORT_TYPE_BACK_FRONT || type == RenderingData::SORT_TYPE_BACK_FRONT)
		m_iSortType = RenderingData::SORT_TYPE_BACK_FRONT;
}

/// returns the name of the Aftk material that we're converting
Util::C_String AftkMaterialConverter::GetMtlName()
{
	if (m_pMtl)
		return Util::C_String(m_pMtl->GetName().c_str());
	else
		return Util::C_String();
}

void AftkMaterialConverter::SetAlphaBlendEnable(bool b)
{
	((RenderingData::BlendState*)&m_blendState)->SetAlphaBlendEnable(b);
}

void AftkMaterialConverter::SetAlphaTestEnable(bool b, U8 ref)
{
	((RenderingData::BlendState*)&m_blendState)->SetAlphaTestEnable(b);
	((RenderingData::BlendState*)&m_blendState)->SetAlphaRef(ref);
}

/// return the node's world-matrix
void AftkMaterialConverter::GetWorldMatrix(float *pOutValue)
{
	assert(m_pNode && !m_pMtl);
	Aftk::Matrix44 worldMatrix = m_pNode->GetWorldMatrix();
	
	memcpy(pOutValue, &worldMatrix, sizeof(Aftk::Matrix44));
}

// convert a hex digit into an int
int HexToI(char c)
{
	int v = c-'0';
	if (v>=0 && v<10)
		return v;
	v = c-'a';
	if (v>=0 && v<6)
		return v + 10;
	v = c-'A';
	if (v>=0 && v<6)
		return v + 10;
	return -1;
}

// utility function to convert a HEX string to binary data
U8 *ConvertHexToBin(const std::string &name, int *piDataSize)
{
	int c = name.length()/2;
	U8 *pData = new U8[c];
	*piDataSize = c;
	U8 *pBinData = pData;

	// convert hex number to binary data
	const char *psz = name.c_str();
	for (int i=0; i<c; i++, pBinData++, psz+=2)
	{
		int v1 = HexToI(psz[0]);
		if (v1 == -1)
			return false;
		int v2 = HexToI(psz[1]);
		if (v2 == -1)
			return false;
		*pBinData = (v1 << 4) + v2;
	}
	return pData;
}

/// return lux-custom data (used for SH lighting etc.)
void AftkMaterialConverter::GetLuxCustomData(float *pOutValue, int iOffset)
{
	assert(m_pNode);
	const Aftk::I_Plug *pPlug = m_pNode->GetPlugPtr("luxCustom");
	std::string text;
	if (!pPlug || !pPlug->GetValue(text))
		return;

	int iSize;
	float *pData = (float*)ConvertHexToBin(text, &iSize);

	if (iOffset >= iSize/4)
	{
		PrintWarning("Reading past the end of LuxCustomData with size %d at offset %d on node %s.\n",
			iSize/4, iOffset, m_pNode->GetName().c_str());
	}

	for (int i=0; i<16; i++)
	{
		pOutValue[i] = pData[iOffset + i];
		if (i > iSize/4)
			break;
	}
	for (;i<16; i++)
		pOutValue[i] = 0;

	delete pData;
}

/// return the value of a paremeter plug
void AftkMaterialConverter::GetFloat4FromPlug(const char *plugName, float *pOutValue)
{
	std::string newPlugName(plugName);

	const Aftk::C_ShaderNode* pAftkShaderNode = const_cast<Aftk::C_ShaderNode*>(m_pAftkShaderNode);

	if (m_pMtl && m_pMtl->GetOrigin() == Aftk::E_MaterialOrigin_XSI)
	{
		std::string origPlugName(plugName);
		std::string nodeName(plugName);
		
		int pos = origPlugName.find(".");

		if (pos != std::string::npos)
		{
			nodeName.resize(pos);
			newPlugName = origPlugName.substr(pos+1);
			for (U32 i = 0; i < m_pMtl->GetNumShaderNodes(); i++)
			{
				const Aftk::C_ShaderNode* pShader = m_pMtl->GetShaderNodePtr(i);

				if (pShader->GetName() == nodeName)
				{
					pAftkShaderNode = pShader;
				}
			}
		}
	}

	Aftk::Vector4F32 v(0,0,0,0);
	if (newPlugName.c_str())
	{
		if (pAftkShaderNode)
		{
			// get it off the material
			const Aftk::I_Plug* pPlug = pAftkShaderNode->GetPlugPtr(newPlugName);
			if (pPlug)
			{
				if (pPlug->GetCompoundType() == Aftk::E_GenericAttributeType_Vector4F32) 
					pPlug->GetCompoundValue(v);
				else if (pPlug->GetCompoundType() == Aftk::E_GenericAttributeType_Vector3F32)
					pPlug->GetCompoundValue((Aftk::Vector3F32&)v);
				else if (pPlug->GetCompoundType() == Aftk::E_GenericAttributeType_Vector2F32)
					pPlug->GetCompoundValue((Aftk::Vector2F32&)v);
				else if (!pPlug->IsCompound() && pPlug->GetType() == Aftk::E_ObjType_DynamicPlug && ((Aftk::C_DynamicPlug*)pPlug)->GetDataType() == Aftk::E_GenericAttributeType_F32)
					pPlug->GetValue(v.x);
				else if (pPlug->GetNumChildren() >= 2 && pPlug->GetChildPtr(0)->IsCompound() &&
					pPlug->GetChildPtr(0)->GetCompoundType() == Aftk::E_GenericAttributeType_Vector3F32)
				{
					//Get XYZ
					pPlug->GetChildPtr(0)->GetCompoundValue((Aftk::Vector3F32&)v);
					//Get W
					if (!pPlug->GetChildPtr(1)->GetValue(v[3]))
						return;
				}
				else
					return;
			}
		}
		else if (m_pNode)
		{
			// get it off the node
			const Aftk::C_GenericAttribute* attribute = AftkHelpers::FindAttribute(*m_pNode, newPlugName);
			if (attribute)
				attribute->GetValue(v);
			else
				return;
		}
	}	

	pOutValue[0] = v.x;
	pOutValue[1] = v.y;
	pOutValue[2] = v.z;
	pOutValue[3] = v.w;
}

/// creates a RenderingData::Material from an Aftk Material
/// returns the GUID for the given Aftk material
void AftkMaterialConverter::Convert(Material *pOut)
{
	Effect *pEffect = m_pManager->m_pEffectManager->GetEffect(m_sPermutatedFXName);

	// create a new material from the material type given by the effect
	RenderingData::Material material;
	m_pManager->m_pSceneExport->GetMaterialList().CreateMaterial(material, pEffect->GetMaterialTypeIndex(m_NodeType));

	// allocate space for the material data
	int c = pEffect->GetMaterialDataSize(m_NodeType);
	U8 *pData = (U8*)alloca(c);
	U8 *_pData = pData;
	memset(_pData, 0, c);

	// generate data
	pEffect->GetMaterialData(this, &_pData, m_NodeType);
	assert(_pData-pData == c);
	material.SetData(pData, c);

	pOut->sortType = (RenderingData::SortType)m_iSortType;
	pOut->blendState = *(RenderingData::BlendState*)&m_blendState;
	pOut->iMaterialID = m_pManager->m_pSceneExport->GetMaterialList().InsertMaterial(material);
	pOut->pEffect = pEffect;
}

};
