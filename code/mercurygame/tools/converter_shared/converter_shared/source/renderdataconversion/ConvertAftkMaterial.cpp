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
#include "converter_shared\renderdataconversion\TextureTags.h"
#include <converter_shared/helpers/AftkMaterialHelpers.h>
#include <converter_shared/config/ABFolderListConfig.h>
#include <system/misc/StringUtil.h>

// to generate the empty texture for the NULL material
#include <texture_export/texture.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <system/serialise.h>

namespace RenderDataConversion
{

/// constructor
AftkMaterialConverter::AftkMaterialConverter(MaterialManager *pMgr, const Aftk::C_MaterialNode *pMtl, const Aftk::C_SceneNode *pNode, int iCustomKey, E_NodeType type = E_NodeType_Mesh)
{
	m_pMtl = pMtl;
	m_pNode = pNode;
	m_pManager = pMgr;
	m_iSortType = RenderingData::SORT_TYPE_KEY;
	m_NodeType = type;
	m_iCustomKey = iCustomKey;
	m_pAftkShaderNode = NULL;
	m_pAftkLightingNode = NULL;
	m_blendState = (int&)RenderingData::BlendState();
	m_DecalOrder = 128;
	m_iCustomFlags = 0;
	m_iStream = -1;
	Setup();
}

/// constructor (for exporting material data from an Aftk Node, e.g. UV offset, SH lighting etc.)
AftkMaterialConverter::AftkMaterialConverter(MaterialManager *pMgr, const Aftk::C_SceneNode *pNode, bool bInstanceResetMtl)
{
	m_pMtl = NULL;
	m_pNode = pNode;
	m_pManager = pMgr;
	m_iSortType = RenderingData::SORT_TYPE_KEY;
	m_NodeType = bInstanceResetMtl?E_NodeType_InstanceReset:E_NodeType_Instance;
	m_pAftkShaderNode = NULL;
	m_pAftkLightingNode = NULL;
	m_DecalOrder = 128;
	m_iCustomFlags = 0;
	m_iStream = -1;
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
			if (i != std::string::npos && i + 12 < (int)m_pAftkShaderNode->GetName().size())
			{				
				std::string fileName("veh");
				std::string matName = m_pAftkShaderNode->GetName().substr(i+12);
				if (stricmp(matName.c_str(), "_lightoverlay") == 0)
				{
					matName = "_neon";
					LOGWARN(g_Log, "Deprecating 'veh_lightoverlay.fxb', replacing with 'veh_neon.fxb'");
				}
				else if (stricmp(matName.c_str(), "_paintl") == 0)
				{
					matName = "_paint";
					LOGWARN(g_Log, "Deprecating 'veh_paintL.fxb', replacing with 'veh_paint.fxb'");
				}
				else if (m_pNode && (m_pNode->GetName().find("_windowinterior_") != std::string::npos) && stricmp(matName.c_str(), "_glass") == 0)
				{
					matName = "_winint";
					LOGWARN(g_Log, "Replacing 'veh_glass.fxb' with 'veh_winint.fxb'");
				}
				fileName.append(matName);
				std::string win32FileName = fileName;
				win32FileName.append(".fxb");
				fileName.append(".fx");

				Util::C_String effectPath = GetEffectManager()->GetEffectPath();
				effectPath = effectPath + "\\" + win32FileName.c_str();
				if (!Sys::C_FileSystem::Instance().FileExists(effectPath))
					fileName = "veh_default.fx";

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

/// checks whether the given material parameter is exposed, and if so returns the name for the exposed parameter name
Util::C_String AftkMaterialConverter::GetExposedParameterName(const char *pszParameter)
{
	if (!m_pAftkShaderNode)
		return "";

	std::string sPlugName = std::string(pszParameter) + "_ExposedValue";

	Util::C_String sValue = AftkHelpers::GetPlugValue(m_pAftkShaderNode, sPlugName.c_str());
	return sValue;
}

/// return the filename of the texture on the material with the given plug name
Util::C_String AftkMaterialConverter::GetTextureFilenameFromPlug(const char* plugName, char* pUVSetName)
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

							if (pUVSetName)
							{
								if (const Aftk::C_DynamicPlug* pUVPlug = pShader->GetDynamicPlugPtr("tspace_id"))
								{
									std::string plugValue;
									pUVPlug->GetValue(plugValue);
									strcpy(pUVSetName, plugValue.c_str());
								}
							}

							return Util::FormatLocalFileString(Util::C_String(filename.c_str()).Lower());
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
						return Util::FormatLocalFileString(Util::C_String(filename.c_str()).Lower());
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
									return Util::FormatLocalFileString(Util::C_String(filename.c_str()).Lower());
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

				return Util::FormatLocalFileString(Util::C_String(fileName.c_str()).Lower());
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
			return Util::FormatLocalFileString(Util::C_String(fileName.c_str()).Lower());
		}
	}

	return "";
}

/// returns whether a texture exists on disk or false if the "exclude" texture tagger option is set
bool AftkMaterialConverter::TextureFileExists(const Util::C_String& filename)
{
	const bool bFileExistsOnDisk = Sys::C_FileSystem::Instance().FileExists(filename);

	if(!bFileExistsOnDisk)
		return false;

	const bool bFileInABFolderList = ABFolderList::C_Config::IsFileInABFolderList(filename);
	
	if(!bFileInABFolderList)
	{
		//return false; //TODO next game
		LOGERROR(g_Log, "Texture file not in Aliebrain Folder List %s", filename.Chars());
	}

	//read texture tags
	C_TextureTags textureTags;

	// TODO : pass in an I_TextureTagsReader externally from somewhere
	C_DefaultTextureTagsReader defaultReader;
	defaultReader.ReadTextureTags(filename, textureTags);

	if(textureTags.m_Exclude)
		return false;

	return true;
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

void AftkMaterialConverter::SetAlphaToMaskEnable(bool b)
{
	((RenderingData::BlendState*)&m_blendState)->SetAlphaToMaskEnable(b);
}

void AftkMaterialConverter::SetAlphaTestEnable(bool b, U8 ref)
{
	((RenderingData::BlendState*)&m_blendState)->SetAlphaTestEnable(b);
	((RenderingData::BlendState*)&m_blendState)->SetAlphaRef(ref);
}

void AftkMaterialConverter::SetDecalOrder( const U8 decalOrder )
{
	m_DecalOrder = decalOrder;
}

/// return the custom key passed in by RenderDataConverter.cpp
int AftkMaterialConverter::GetCustomKey()
{
	return m_iCustomKey;
}

/// return the node's world-matrix
void AftkMaterialConverter::GetWorldMatrix(float *pOutValue)
{
	assert(m_pNode);
	Aftk::Matrix44 worldMatrix;
	if (m_NodeType == E_NodeType_Instance)
		worldMatrix = m_pNode->GetWorldMatrix();
	else
	{
		Maths::HMat44 identity;
		identity.SetIdentity();
		Aftk::NightmareToTemplateMatrix44(worldMatrix, identity);
	}
	memcpy(pOutValue, &worldMatrix, sizeof(Aftk::Matrix44));
}

/// return the node's world bounding box
Maths::C_AxisBox AftkMaterialConverter::GetWorldBoundingBox()
{
	assert(m_pNode);
	return Aftk::TemplateToNightmare(m_pNode->GetWorldBounds());
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
	if (iSize<4)
	{
		delete pData;
		return;	// not floating point data
	}

	if (iOffset >= iSize/4)
	{
		PrintWarning("Reading past the end of LuxCustomData with size %d at offset %d on node %s.\n",
			iSize/4, iOffset, m_pNode->GetName().c_str());
	}

	int i;
	for (i=0; i<16; i++)
	{
		if (i+iOffset >= iSize/4)
			break;
		pOutValue[i] = pData[iOffset + i];
	}
	for (;i<16; i++)
		pOutValue[i] = 0;

	delete pData;
}

// Returns true if plug is animated in some way.
bool IsAnimatedPlug(const Aftk::I_Plug* plug)
{
	if ( !plug )
	{
		return false;
	}
	
	// Check to see if plug itself is animated.
	const Aftk::I_Plug* currPlug = plug;
	const Aftk::I_Object* src = currPlug->GetSourceConnectionPtr();
	while (src != NULL)
	{
		if (src->HasType(Aftk::E_ObjType_AnimationCurve))
		{
			const Aftk::C_AnimationCurveNode* animCurveNode = static_cast<const Aftk::C_AnimationCurveNode*>(src);
			
			// Is this animation curve driven directly by time?
			if (animCurveNode->IsTimeInput())
			{
				// Yes, so consider it keyframed if there are one or more keys.
				return animCurveNode->GetNumKeys() > 0;
			}
			else
			{
				// No, so follow the animation curve input.
				currPlug = &animCurveNode->GetInputPlug();
			}
		}
		else if (src->HasType(Aftk::E_ObjType_IPlug))
		{
			currPlug = static_cast<const Aftk::I_Plug*>(src);
		}
		else
		{
			// Unrecognised node type.
			return false;
		}
		src = currPlug->GetSourceConnectionPtr();
	}
	
	// If plug doesn't seem to be animated, check its children.
	if (plug->IsCompound())
	{
		for (U32 i = 0; i < plug->GetNumChildren(); ++i)
		{
			const Aftk::I_Plug* childPlug = plug->GetChildPtr(i);
			if (IsAnimatedPlug(childPlug))
			{
				return true;
			}
		}
	}
	
	return false;
}


/// return the value of a paremeter plug
int AftkMaterialConverter::GetFloat4FromPlug(const char *plugName, float *pOutValue, bool *pbAnimated)
{
	std::string newPlugName(plugName);

	int c = 0;
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
				if (pbAnimated)
					*pbAnimated = IsAnimatedPlug(pPlug);

				if (pPlug->GetCompoundType() == Aftk::E_GenericAttributeType_Vector4F32) 
				{
					c = 4;
					pPlug->GetCompoundValue(v);
				}
				else if (pPlug->GetCompoundType() == Aftk::E_GenericAttributeType_Vector3F32)
				{
					c = 3;
					pPlug->GetCompoundValue((Aftk::Vector3F32&)v);
				}
				else if (pPlug->GetCompoundType() == Aftk::E_GenericAttributeType_Vector2F32)
				{
					c = 2;
					pPlug->GetCompoundValue((Aftk::Vector2F32&)v);
				}
				else if (!pPlug->IsCompound() && pPlug->GetType() == Aftk::E_ObjType_DynamicPlug && ((Aftk::C_DynamicPlug*)pPlug)->GetDataType() == Aftk::E_GenericAttributeType_F32)
				{
					c = 1;
					pPlug->GetValue(v.x);
				}
				else if (pPlug->GetNumChildren() >= 2 && pPlug->GetChildPtr(0)->IsCompound() &&
					pPlug->GetChildPtr(0)->GetCompoundType() == Aftk::E_GenericAttributeType_Vector3F32)
				{
					c = 4;
					//Get XYZ
					pPlug->GetChildPtr(0)->GetCompoundValue((Aftk::Vector3F32&)v);
					//Get W
					if (!pPlug->GetChildPtr(1)->GetValue(v[3]))
						return 0;
				}
				else
					return 0;
			}
		}
		else if (m_pNode)
		{
			// get it off the node
			const Aftk::C_GenericAttribute* attribute = AftkHelpers::FindAttribute(*m_pNode, newPlugName);
			if (attribute)
			{
				c = 4;
				attribute->GetValue(v);
			}
			else
				return 0;
		}
	}	

	if (c>0)
		pOutValue[0] = v.x;
	if (c>1)
		pOutValue[1] = v.y;
	if (c>2)
		pOutValue[2] = v.z;
	if (c>3)
		pOutValue[3] = v.w;

	return c;
}

/// get the stream we are exporting for
int AftkMaterialConverter::GetStream()
{
	return m_iStream;
}

/// add TextureInfo entry - this contains a list of texture indices and associated UV sets etc. for the streamer
/// if UVSetName == "", uvSet is used. if uvSet == -1, uScale, vScale and weight are used
void AftkMaterialConverter::AddTextureInfo(S32 textureIndex, const char *pszUVSetName, int uvSet, const char *suScale, float uScale, const char *svScale, float vScale, const char *sWeight, float weight)
{
	float buffer[4];
	if (strlen(suScale)>0)
	{
		GetFloat4FromPlug(suScale, buffer, NULL);
		uScale = buffer[0];
	}
	if (strlen(svScale)>0)
	{
		GetFloat4FromPlug(svScale, buffer, NULL);
		vScale = buffer[0];
	}
	if (strlen(sWeight)>0)
	{
		GetFloat4FromPlug(sWeight, buffer, NULL);
		weight = buffer[0];
	}

	RenderingData::PrimitiveInfo::TextureInfo texInfo;
	texInfo.textureIndex = textureIndex;
	texInfo.sUVSet = pszUVSetName;
	texInfo.uvSet = uvSet;
	texInfo.uScale = uScale;
	texInfo.vScale = vScale;
	texInfo.weight = weight;
	m_TextureInfo.PushBack(texInfo);
}

/// creates a RenderingData::Material from an Aftk Material
/// returns the GUID for the given Aftk material (amongst other things)
void AftkMaterialConverter::Convert(Material *pOut, const Aftk::C_SceneNode *pMesh, bool bSeparateStreams)
{
	m_TextureInfo.Clear();
	Effect *pEffect = m_pManager->m_pEffectManager->GetEffect(m_sPermutatedFXName);
	if (!pEffect)
	{
		pEffect = m_pManager->m_pEffectManager->GetEffect(g_szInvalidFXName);
		if (pMesh)
			LOGERROR(g_Log, "%s: Failed to load .fxb file for shader '%s'.\n", pMesh->GetPath().c_str(), m_sPermutatedFXName.Chars());
	}
	if (!pEffect)
	{
		LOGFATAL(g_Log, "Failed to load the .fxb file for the default .fx file '%s'. Exiting.\n", g_szInvalidFXName.Chars());
		exit(-1);
	}

	if (!bSeparateStreams)
	{
		// get all material stream data in one material
		m_iStream = -1;
		Convert(pEffect, pOut, pMesh, &pOut->iMaterialID);
	}
	else
	{
		// get separate material for each stream
		int iLastNonEmptyStream = -1;
		pOut->materialIDs.Resize(MAX_MATERIAL_STREAMS);
		for (int iStream=0; iStream<MAX_MATERIAL_STREAMS; iStream++)
		{
			m_iStream = iStream;
			if (Convert(pEffect, pOut, pMesh, &pOut->materialIDs[iStream]))
				iLastNonEmptyStream = iStream;
		}
		pOut->materialIDs.Resize(iLastNonEmptyStream+1);
	}

	// collect other output information
	pOut->sortType = (RenderingData::SortType)m_iSortType;
	pOut->blendState = *(RenderingData::BlendState*)&m_blendState;
	pOut->pEffect = pEffect;
	pOut->textureInfo = m_TextureInfo;
	pOut->decalOrder = m_DecalOrder;

	// get blend state overrides from shader
	pEffect->GetDefaultBlendState(&pOut->blendState);

	// get sort type override from shader
	pEffect->GetSortTypeAnnotation(&pOut->sortType);

	// get decal sortkey override
	int iDecalOrder;
	if (pEffect->GetAnnotation("SortOrder", &iDecalOrder))
		pOut->decalOrder = (U8)Maths::Clamp(128-iDecalOrder, 0, 255);

	// get custom flags from shader
	pEffect->GetAnnotation("Flags", &m_iCustomFlags);
	pOut->iCustomFlags = m_iCustomFlags;
}

/// internal function which converts a material for a single stream
bool AftkMaterialConverter::Convert(Effect *pEffect, Material *pOut, const Aftk::C_SceneNode *pMesh, int *piMtlId)
{
	RenderingData::MaterialType materialType;
	int iSize = 0;
	U8 pData[1024*16]; // TODO: FIXME: make this a dynamically resizable buffer instead of 16K fixed size
	U8 *_pData = pData;
	pEffect->GetMaterialData(this, &materialType, &iSize, &_pData, m_NodeType);
	if (iSize == 0)
	{
		*piMtlId = -1;
		return false;
	}

	// create material type
	int iMaterialType = m_pManager->m_pSceneExport->GetMaterialList().InsertMaterialType(materialType);

	// create a new material from the material type
	RenderingData::Material material;
	m_pManager->m_pSceneExport->GetMaterialList().CreateMaterial(material, iMaterialType);

	// set data
	material.SetData(pData, iSize);

	*piMtlId = m_pManager->m_pSceneExport->GetMaterialList().InsertMaterial(material);
	return true;
};

}
