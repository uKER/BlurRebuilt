#include <nexus_render/stdafx.h>

#include <nexus_render/D3dImageLoader.h>
#include <nexus_render/D3dTextureManager.h>

#include <nexus_render/EffectConstants.h>
#include <nexus_render/MeshLightmapEffectConstants.h>
#include <core/utility/StringProc.h>

using namespace NexusRender;

static const U32 COLOUR_TYPE = 0;
static const U32 LIGHTMAP_TYPE = 1;

MeshLightmapEffectConstants::MeshLightmapEffectConstants() : MeshTextureEffectConstants()
{
	OneTimeInit();
}

MeshLightmapEffectConstants::MeshLightmapEffectConstants(const MeshLightmapEffectConstants& rhs)
{
	*this = rhs;
}

void MeshLightmapEffectConstants::OneTimeInit()
{
}

MeshLightmapEffectConstants::~MeshLightmapEffectConstants()
{
}

const MeshLightmapEffectConstants& MeshLightmapEffectConstants::operator=(const MeshLightmapEffectConstants& rhs)
{
	MeshTextureEffectConstants::operator =(rhs);

	m_lightmapTextureInfo = rhs.m_lightmapTextureInfo;
	m_pLightmapTexture = rhs.m_pLightmapTexture;
	return *this;
}

void MeshLightmapEffectConstants::GetTextureInfo(Ut::UtVector<Ut::DynamicVector<const EffectConstants::TextureInfo*> >& textureInfo) const
{
	__super::GetTextureInfo(textureInfo);
	textureInfo.PushBack(&m_lightmapTextureInfo);
}

void MeshLightmapEffectConstants::LoadTextures(SceneRenderer& sceneRenderer)
{
	__super::LoadTextures(sceneRenderer);

	if (m_lightmapTextureInfo.fileName.length() == 0)
		return;

	m_pLightmapTexture = sceneRenderer.GetTextureManagerPtr()->Find(
		m_lightmapTextureInfo.fileName);
}

Tu::Status MeshLightmapEffectConstants::Build(const Aftk::C_MaterialNode* pAftkMaterialNode)
{
	Tu::Status status;

	status = __super::Build(
		pAftkMaterialNode);
	if (!status) return status;

	m_pLightmapTexture = 0;

	// find associated lightmap texture
	std::string imageType;
	for (U32 iImage=0; iImage < pAftkMaterialNode->GetNumShaderImageNodes(); iImage++)
	{
		const Aftk::C_ShaderImageNode* pImage = pAftkMaterialNode->GetShaderImageNodePtr(iImage);
		pImage->GetImageTypePlug().GetValue(imageType);
		if (!strcmp(imageType.c_str(),	"lightmap"))
		{
			pImage->GetFilePlug().GetValue(m_lightmapTextureInfo.fileName);

			std::transform(
				m_lightmapTextureInfo.fileName.begin(), 
				m_lightmapTextureInfo.fileName.end(), 
				m_lightmapTextureInfo.fileName.begin(), 
				std::tolower);
			Tu::SearchAndReplace(m_lightmapTextureInfo.fileName, "\\", "/");
			m_lightmapTextureInfo.forceFullRes = true;
			break;
		}
	}

	return Tu::Status::SUCCESS;
}

IDirect3DTexture9* MeshLightmapEffectConstants::GetLightmapTexturePtr()
{
	return m_pLightmapTexture;
}

U32 MeshLightmapEffectConstants::GetHash() const
{
	return __super::GetHash() + (U32)m_lightmapTextureInfo.fileName.length();
}

bool MeshLightmapEffectConstants::operator==(const EffectConstants& effectConstants) const
{
	if (!__super::operator ==(effectConstants))
		return false;

	if (GetEffectType() != effectConstants.GetEffectType())
		return false;

	const MeshLightmapEffectConstants& rhs = (const MeshLightmapEffectConstants&)effectConstants;
	if (m_lightmapTextureInfo.fileName != rhs.m_lightmapTextureInfo.fileName)
		return false;

	return true;
}
