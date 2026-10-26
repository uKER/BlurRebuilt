#include "ResourceUtilsPch.h"
#include <resource_utils/GameTexture.h>

#include <textureresource/TextureResource.h>


XT_TYPE(ResourceUtils::C_GameTexture, "GameTexture")
	XT_FIELD(m_Filename,		"Filename")
XT_END_TYPE


namespace ResourceUtils
{

C_GameTexture::C_GameTexture()
	: m_Filename()
{
}

C_GameTexture::~C_GameTexture()
{
}

Util::C_String C_GameTexture::GetFileName() const
{
	return Util::C_String(m_Filename.Text());
}

Resource::C_ResourceTypeId C_GameTexture::GetResourceTypeId()
{
	return TextureResource::C_TextureResource::StaticGetResourceTypeId();
}

} // namespace ResourceUtils
