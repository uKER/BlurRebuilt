#include "CrowdSystemPch.h"

#include <crowd_system/CrowdTextureCache.h>
#include <crowd_system/CrowdDebug.h>

#include <crowd_system/XtCrowd.h>

#include <TextureResource/TextureResource.h>
#include <resource_utils/ResourceBagUtils.h>

namespace AnimatedCrowd
{

C_CrowdTexture::C_CrowdTexture()
	: m_Valid(0)
{
}

C_CrowdTexture::~C_CrowdTexture()
{
}

void C_CrowdTexture::Create(const Util::C_String& filename)
{
	MEMORY_TRACKER_PUSH("mercury.crowd");
	
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	if (fs.FileExists(filename))
		m_TextureResourceHandle.Load(filename, this);
	
	MEMORY_TRACKER_POP("mercury.crowd");
}

void C_CrowdTexture::ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData)
{
	switch(result)
	{
	case Resource::E_Notify_DataReady:
		{
			m_Valid = 1;
		}
		break;
		
	default:
		break;
	}
}

const bool C_CrowdTexture::IsValid() const
{
	return (m_Valid > 0);
}

const TextureResource::C_TextureResource* C_CrowdTexture::GetTextureResource() const
{
	if (!IsValid())
		return NULL;

	return static_cast<const TextureResource::C_TextureResource*>(m_TextureResourceHandle);
}

///////////////////////////////////////////////////////////////////////////////

C_CrowdTextureCache::C_CrowdTextureCache()
{ }

C_CrowdTextureCache::~C_CrowdTextureCache()
{
	Destroy();
}

void C_CrowdTextureCache::Destroy()
{
	m_TextureMap.Clear();
	m_Textures.Destroy();

	m_TextureCreationBuffer.Destroy();
}

void C_CrowdTextureCache::BeginBuild()
{
	m_TextureCreationBuffer.Clear();
}

void C_CrowdTextureCache::AddTextureList(const C_XtCrowdTextureList* pTextureList)
{
	if (!pTextureList)
		return;

	for (U32 i = 0, count = static_cast<U32>(pTextureList->m_Textures.count); i < count; ++i)
		m_TextureCreationBuffer.PushBack(pTextureList->m_Textures[i]);
}

void C_CrowdTextureCache::EndBuild(/*ResourceUtils::I_ResourceBag* pResourceBag*/)
{
	MEMORY_TRACKER_PUSH("mercury.crowd");

	//if (!pResourceBag)
	//	return;

	if (!m_TextureCreationBuffer.Size())
		return;

	m_TextureMap.Clear();
	m_Textures.Destroy();

	for (U32 i = 0, count = static_cast<U32>(m_TextureCreationBuffer.Size()); i < count; ++i)
	{
		if (!m_TextureCreationBuffer[i])
			continue;

		Btl::OpenHashMap<const char*, U32>::ConstIterator iter = m_TextureMap.Find(m_TextureCreationBuffer[i]);
		if (iter == m_TextureMap.End())
		{
			/*m_TextureMap.Insert(m_TextureCreationBuffer[i].second, static_cast<U32>(m_Textures.Size()));
			m_Textures.PushBack();
			Util::C_String file(m_TextureCreationBuffer[i].second);
			m_Textures.Back().Load(file + ".texture");*/

			/*Util::C_String file(m_TextureCreationBuffer[i].second);
			TextureResource::C_TextureHandle textureHandle;
			if (textureHandle.Load(file + ".texture"))
			{
				m_TextureMap.Insert(m_TextureCreationBuffer[i].second, static_cast<U32>(m_Textures.Size()));
				m_Textures.PushBack(textureHandle);
			}
			else
				LOGWARN( g_CrowdLog, "C_CrowdTextureCache::EndBuild() - Unable to load texture '%s'", file.Chars() );*/

			/*Util::C_String file(m_TextureCreationBuffer[i].second);

			TextureResource::C_TextureHandle textureHandle;
			textureHandle = pResourceBag->GetResource<TextureResource::C_TextureResource>(MakeResourceBagTag(file));
			//if (!(&*textureHandle))
			if (!((TextureResource::C_TextureResource*)textureHandle))
				pResourceBag->AddResource(MakeResourceBagTag(file), file + ".texture", TextureResource::C_TextureResource::StaticGetResourceTypeId());

			textureHandle = pResourceBag->GetResource<TextureResource::C_TextureResource>(MakeResourceBagTag(file));
			BIZ_ASSERT((TextureResource::C_TextureResource*)textureHandle);
			m_TextureMap.Insert(m_TextureCreationBuffer[i].second, static_cast<U32>(m_Textures.Size()));
			m_Textures.PushBack(textureHandle);*/

			Util::C_String file(m_TextureCreationBuffer[i]);

			m_TextureMap.Insert(m_TextureCreationBuffer[i], static_cast<U32>(m_Textures.Size()));
			m_Textures.PushBack();
			m_Textures.Back().Create(file + ".texture");
		}
	}

	m_TextureCreationBuffer.Clear();

	MEMORY_TRACKER_POP("mercury.crowd");
}

const S32 C_CrowdTextureCache::GetTextureIndex(const char* pTextureName) const
{
	if (pTextureName)
	{
		Btl::OpenHashMap<const char*, U32>::ConstIterator iter = m_TextureMap.Find(pTextureName);
		if (iter != m_TextureMap.End())
			return static_cast<S32>(iter->GetValue());
	}
	
	return -1;
}

const U32 C_CrowdTextureCache::GetNumTextures() const
{
	return static_cast<U32>(m_Textures.Size());
}

const Display::C_Texture2D* C_CrowdTextureCache::GetTexture(const U32 index) const
{
	if (index < static_cast<U32>(m_Textures.Size()))
	{
		/*TextureResource::C_TextureResource* pTextureResource = m_Textures[index];
		if (pTextureResource && pTextureResource->IsDataValid())
			return (const Display::C_Texture2D*) pTextureResource->GetDisplayTexture();*/
		/*TextureResource::C_TextureResource* pTextureResource = m_Textures[index];
		if (pTextureResource && pTextureResource->IsDataValid())
			return (const Display::C_Texture2D*) pTextureResource->GetDisplayTexture();*/

		const C_CrowdTexture& texture = m_Textures[index];
		if (texture.IsValid())
		{
			BIZ_ASSERT(texture.GetTextureResource());
			return texture.GetTextureResource()->GetDisplayTexture();
		}
	}
	
	return NULL;
}

}; // namespace AnimatedCrowd
