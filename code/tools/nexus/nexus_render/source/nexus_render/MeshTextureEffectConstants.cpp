#include <nexus_render/stdafx.h>

#include <nexus_render/D3dImageLoader.h>
#include <nexus_render/D3dTextureManager.h>

#include <nexus_render/EffectConstants.h>
#include <nexus_render/MeshTextureEffectConstants.h>
#include <core/utility/StringProc.h>

using namespace NexusRender;

static const U32 COLOUR_TYPE = 0;
static const U32 LIGHTMAP_TYPE = 1;

MeshTextureEffectConstants::MeshTextureEffectConstants() : EffectConstants()
{
	OneTimeInit();
}

MeshTextureEffectConstants::MeshTextureEffectConstants(const MeshTextureEffectConstants& rhs)
{
	OneTimeInit();
	*this = rhs;
}

void MeshTextureEffectConstants::OneTimeInit()
{
	m_pColourTexture = 0;
	m_pCropTexture = 0;
	m_cropSheetColourTextureIndex = -1;
}

MeshTextureEffectConstants::~MeshTextureEffectConstants()
{
}

const MeshTextureEffectConstants& MeshTextureEffectConstants::operator=(const MeshTextureEffectConstants& rhs)
{
	EffectConstants::operator =(rhs);

	m_pColourTexture = rhs.m_pColourTexture;
	m_pCropTexture = rhs.m_pCropTexture;

	m_colourTextureInfo = rhs.m_colourTextureInfo;
	return *this;
}

void MeshTextureEffectConstants::GetTextureInfo(Ut::UtVector<Ut::DynamicVector<const EffectConstants::TextureInfo*> >& textureInfo) const
{
	textureInfo.PushBack(&m_colourTextureInfo);
}

void MeshTextureEffectConstants::LoadTextures(SceneRenderer& sceneRenderer)
{
	if (m_colourTextureInfo.fileName.length() == 0)
		return;

	m_pColourTexture = sceneRenderer.GetTextureManagerPtr()->Find(m_colourTextureInfo.fileName);
}

Tu::Status MeshTextureEffectConstants::LoadCropSheetTexture(SceneRenderer& sceneRenderer)
{
	if (m_colourTextureInfo.fileName.length() == 0)
		return Tu::Status::SUCCESS;

	m_cropSheetColourTextureIndex = sceneRenderer.GetCropSheetImageRecord(m_colourTextureInfo.fileName);
	if (m_cropSheetColourTextureIndex == -1)
	{
		std::stringstream ss;
		ss << m_colourTextureInfo.fileName << " has no entry in crop sheet database.";
		return Tu::Status(Tu::Status::FAILURE, ss.str());
	}

	return Tu::Status::SUCCESS;
}

Tu::Status MeshTextureEffectConstants::Build(const Aftk::C_MaterialNode* pAftkMaterialNode)
{
	Tu::Status status;

	if (!pAftkMaterialNode)
		return Tu::Status::SUCCESS;

	std::string imageType;
	for (U32 iImage = 0; iImage < pAftkMaterialNode->GetNumShaderImageNodes(); iImage++)
	{
		const Aftk::C_ShaderImageNode* pImage = pAftkMaterialNode->GetShaderImageNodePtr(iImage);
		//Use the first texture we find that _isn't_ a lightmap as our colour texture.
		pImage->GetImageTypePlug().GetValue(imageType);
		if (strcmp(imageType.c_str(),"lightmap"))
		{
			pImage->GetFilePlug().GetValue(m_colourTextureInfo.fileName);
			std::transform(
				m_colourTextureInfo.fileName.begin(),
				m_colourTextureInfo.fileName.end(),
				m_colourTextureInfo.fileName.begin(), 
				std::tolower);
			Tu::SearchAndReplace(m_colourTextureInfo.fileName, "\\", "/");
			m_colourTextureInfo.forceFullRes = false;
			break;
		}
	}

	return Tu::Status::SUCCESS;
}

IDirect3DTexture9* MeshTextureEffectConstants::GetColourTexturePtr()
{
	return m_pColourTexture;
}

IDirect3DTexture9* MeshTextureEffectConstants::GetCropSheetTexturePtr()
{
	return m_pCropTexture;
}

U32 MeshTextureEffectConstants::GetHash() const
{
	return (U32)m_colourTextureInfo.fileName.length();
}

bool MeshTextureEffectConstants::operator==(const EffectConstants& effectConstants) const
{
	if (GetEffectType() != effectConstants.GetEffectType())
		return false;

	const MeshTextureEffectConstants& rhs = (const MeshTextureEffectConstants&)effectConstants;
	if (m_colourTextureInfo.fileName != rhs.m_colourTextureInfo.fileName)
		return false;

	return true;
}

EffectWrapper::EEffectType MeshTextureEffectConstants::GetEffectType() const
{
	return EffectWrapper::EFFECT_TYPE_MESH_TEXTURE;
}

