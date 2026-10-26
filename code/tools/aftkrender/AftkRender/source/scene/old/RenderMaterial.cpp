#include "stdafx.h"
#include <algorithm>
#include <system/misc/StringUtil.h>
#include <system/serialise/C_FileSystem.h>
#include "AftkRender.h"

using namespace AftkRender;

C_RenderMaterial::C_RenderMaterial(
	C_SceneRenderer* pSceneRenderer)
:
	m_pSceneRenderer(pSceneRenderer)
{
}

C_RenderMaterial::~C_RenderMaterial()
{
	Clear();
}

void C_RenderMaterial::Clear()
{
	m_TextureInfo.Clear();

	for (U32 i=0; i<(U32)m_Textures2D.Size(); ++i)
	{
		Display::Device()->Destroy(&m_Textures2D[i]);
	}
	m_Textures2D.Clear();
}

Aftk::C_Status C_RenderMaterial::BuildFromAftkMaterial(
	const Aftk::C_MaterialNode* pAftkMaterial)
{
	bool textureTypeFound[TextureInfo::E_TextureUsage_Count];
	Util::C_String textureNames[TextureInfo::E_TextureUsage_Count];

	memset(&textureTypeFound, false, sizeof(bool)*TextureInfo::E_TextureUsage_Count);

	for (U32 iImage = 0; iImage < pAftkMaterial->GetNumShaderImageNodes(); iImage++)
	{
		std::string fileName;
		std::string imageType;
		const Aftk::C_ShaderImageNode* pImage = pAftkMaterial->GetShaderImageNodePtr(iImage);

		pImage->GetFilePlug().GetValue(fileName);
		std::transform(fileName.begin(), fileName.end(), fileName.begin(), tolower);

		pImage->GetImageTypePlug().GetValue(imageType);
		if (!textureTypeFound[TextureInfo::E_TextureUsage_LightMap] && !strcmp(imageType.c_str(),"lightmap"))
		{
			textureNames[TextureInfo::E_TextureUsage_LightMap] = fileName.c_str();
			textureTypeFound[TextureInfo::E_TextureUsage_LightMap] = true;
		}
		else if (!textureTypeFound[TextureInfo::E_TextureUsage_NormalMap] && (fileName.find("normal") != std::string::npos))
		{
			textureNames[TextureInfo::E_TextureUsage_NormalMap] = fileName.c_str();
			textureTypeFound[TextureInfo::E_TextureUsage_NormalMap] = true;
		}
		else if (!textureTypeFound[TextureInfo::E_TextureUsage_DiffuseMap])
		{
			textureNames[TextureInfo::E_TextureUsage_DiffuseMap] = fileName.c_str();
			textureTypeFound[TextureInfo::E_TextureUsage_DiffuseMap] = true;
		}
	}

	if (!textureTypeFound[TextureInfo::E_TextureUsage_NormalMap] && textureTypeFound[TextureInfo::E_TextureUsage_DiffuseMap])
	{
		//No normal map in aftk material but we'll mangle the diffuse texture name and check anyway (eg speedtree nodes)
		Util::C_String& diffuseMap = textureNames[TextureInfo::E_TextureUsage_DiffuseMap];

		Util::C_String normalMap;
		{
			S32 index = diffuseMap.Search("diffuse");
			if(index != -1)
				normalMap = diffuseMap(0, index) + "normal" + diffuseMap(index+7, diffuseMap.Length());
			else
				normalMap = Util::RemoveFileExtension(diffuseMap) + "_normal." + Util::ExtractFileExtension(diffuseMap);

			if (Sys::C_FileSystem::Instance().FileExists(normalMap))
			{
				textureNames[TextureInfo::E_TextureUsage_NormalMap] = normalMap;
				textureTypeFound[TextureInfo::E_TextureUsage_NormalMap] = true;
			}
		}
	}

	if (textureTypeFound[TextureInfo::E_TextureUsage_DiffuseMap])
		m_TextureInfo.PushBack(TextureInfo(textureNames[TextureInfo::E_TextureUsage_DiffuseMap], TextureInfo::E_TextureUsage_DiffuseMap));

	if (textureTypeFound[TextureInfo::E_TextureUsage_NormalMap])
		m_TextureInfo.PushBack(TextureInfo(textureNames[TextureInfo::E_TextureUsage_NormalMap], TextureInfo::E_TextureUsage_NormalMap));

	if (textureTypeFound[TextureInfo::E_TextureUsage_LightMap])
		m_TextureInfo.PushBack(TextureInfo(textureNames[TextureInfo::E_TextureUsage_LightMap], TextureInfo::E_TextureUsage_LightMap));

	return Aftk::C_Status::E_Status_Success;
}

void C_RenderMaterial::OnEnable() const
{

	//TODO: need a mechanism for specifying which sampler to use for each texture type
	for (U32 iTex=0; iTex<(U32)m_Textures2D.Size(); ++iTex)
	{
		switch (m_TextureInfo[iTex].m_Usage)
		{
			case TextureInfo::E_TextureUsage_DiffuseMap:
			{
				if (m_Textures2D[iTex].IsCreated())
					Display::Device()->SetTexture(0, &m_Textures2D[iTex]);
				else
					Display::Device()->SetTexture(0, &m_pSceneRenderer->GetDummyTexture());
				break;
			}
			case TextureInfo::E_TextureUsage_NormalMap:
			{
				if (m_Textures2D[iTex].IsCreated())
					Display::Device()->SetTexture(1, &m_Textures2D[iTex]);
				else
					Display::Device()->SetTexture(1, &m_pSceneRenderer->GetDummyTexture());
				break;
			}
			case TextureInfo::E_TextureUsage_LightMap:
			{
				if (m_Textures2D[iTex].IsCreated())
					Display::Device()->SetTexture(2, &m_Textures2D[iTex]);
				else
					Display::Device()->SetTexture(2, &m_pSceneRenderer->GetDummyTexture());
				break;
			}
			default:
			{
				Display::Device()->SetTexture(0, &m_pSceneRenderer->GetDummyTexture());
				break;
			}
		}
	}
}

void C_RenderMaterial::OnDisable() const
{
}

const Ut::UtVector< Ut::DynamicVector<C_RenderMaterial::TextureInfo> >& C_RenderMaterial::GetTextureInfoList() const
{
	return m_TextureInfo;
}

const Ut::UtVector< Ut::DynamicVector<C_RenderMaterial::TextureInfo> >& C_RenderMaterial::GetTextureInfoList()
{
	return m_TextureInfo;
}

void C_RenderMaterial::AddTextureInfo(TextureInfo& info)
{
	m_TextureInfo.PushBack(info);
}

bool C_RenderMaterial::IsTextureLoaded(U32 index) const
{
	ASSERT(index<(U32)m_Textures2D.Size());
	return m_Textures2D[index].IsCreated();
}

bool C_RenderMaterial::HasAlpha() const
{
	for(U32 iTex=0; iTex<(U32)m_Textures2D.Size(); ++iTex)
	{
		if (m_TextureInfo[iTex].m_Usage == TextureInfo::E_TextureUsage_DiffuseMap)
		{
			Display::E_Format texFormat = m_Textures2D[iTex].GetFormat();
			if (texFormat == Display::E_Format_A8R8G8B8 ||
				texFormat == Display::E_Format_A16B16G16R16 ||
				texFormat == Display::E_Format_A16B16G16R16F ||
				texFormat == Display::E_Format_A2B10G10R10F ||
				//texFormat == Display::E_Format_X8R8G8B8 ||
				texFormat == Display::E_Format_DXT1 ||
				texFormat == Display::E_Format_DXT3 ||
				texFormat == Display::E_Format_DXT5 ||
				texFormat == Display::E_Format_DXN)
			return true;
		}
	}
	return false;
}

bool C_RenderMaterial::HasNormalMap() const
{
	for(U32 iTex=0; iTex<(U32)m_Textures2D.Size(); ++iTex)
		if (m_TextureInfo[iTex].m_Usage == TextureInfo::E_TextureUsage_NormalMap)
			return true;
	return false;
}

U32 C_RenderMaterial::GetNumTextures() const
{
	return m_TextureInfo.Size();
}

U32 C_RenderMaterial::GetHash() const
{
	U32 hash = 0;;
	for (U32 i=0; i<(U32)m_TextureInfo.Size(); ++i)
	{
		hash += Ut::HashKeyOpString::GetHash(m_TextureInfo[i].m_FileName.Chars());
	}
	return hash;
}


Aftk::C_Status C_RenderMaterial::loadTextures2D(bool mipmap)
{
	if (m_Textures2D.Size() < m_TextureInfo.Size())
		m_Textures2D.Resize(m_TextureInfo.Size());

	for (U32 i=0; i<(U32)m_TextureInfo.Size(); ++i)
	{
		if (!m_Textures2D[i].IsCreated() && 
			 m_TextureInfo[i].m_Usage != TextureInfo::E_TextureUsage_Untextured)
		{	
			if(!Display::CreateTexture2DFromFile(m_TextureInfo[i].m_FileName, &m_Textures2D[i]))
				LOGWARN(g_Log, "Failed to create texture from %s\n", m_TextureInfo[i].m_FileName.Chars());
		}
		if (mipmap)
		{
			if (D3DXFilterTexture(m_Textures2D[i].Get(), 0, 0, D3DX_DEFAULT) != S_OK)
				LOGWARN(g_Log, "Failed to mipmap texture %s\n", m_TextureInfo[i].m_FileName.Chars());
			#ifdef _DEBUG
			else
				LOGINFO(g_Log,"%i mip levels for %s\n", m_Textures2D[i].Get()->GetLevelCount(), m_TextureInfo[i].m_FileName.Chars());
			#endif
		}
		
		m_Textures2D[i].GetFormat();
	}
	return Aftk::C_Status::E_Status_Success;
}

Display::C_Texture2D& C_RenderMaterial::GetTexture2D(U32 index)
{
	ASSERT( index < (U32)m_Textures2D.Size() );
	return m_Textures2D[index];
}

const Display::C_Texture2D& C_RenderMaterial::GetTexture2D(U32 index) const
{
	ASSERT( index < (U32)m_Textures2D.Size() );
	return m_Textures2D[index];
}