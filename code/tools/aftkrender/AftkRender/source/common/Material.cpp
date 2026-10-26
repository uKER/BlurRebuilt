#include "AftkRenderPch.h"

#include "common/TextureState.h"
#include "common/TextureManager.h"
#include "common/Material.h"

using namespace AftkRender;

C_Material::C_Material()
: m_BlendMode(Display::E_BlendMode_Default)
, m_CullMode(Display::E_CullMode_Default)
, m_TexturesLoaded(false)
, m_MaterialTypeId(-1)
{
}

C_Material::~C_Material()
{
}


C_Material::TextureInfo& C_Material::GetTextureInfo(U32 i)
{
	return m_TextureInfo[i];
}

const C_Material::TextureInfo& C_Material::GetTextureInfo(U32 i) const
{
	return m_TextureInfo[i];
}

void C_Material::AddTextureInfo(const C_Material::TextureInfo& info)
{
	m_TextureInfo.PushBack(info);
}

U32 C_Material::GetNumTextures() const
{
	return m_TextureInfo.Size();
}

C_TextureState& C_Material::GetTextureState()
{
	return m_TextureState;
}

const C_TextureState& C_Material::GetTextureState() const
{
	return m_TextureState;
}

void C_Material::SetTextureState(const C_TextureState& textureState)
{
	m_TextureState = textureState;
}

Display::E_BlendMode C_Material::GetBlendMode() const
{
	return m_BlendMode;
}

void C_Material::SetBlendMode(Display::E_BlendMode mode)
{
	m_BlendMode = mode;
}

Display::E_CullMode C_Material::GetCullMode() const
{
	return m_CullMode;
}

void C_Material::SetCullMode(Display::E_CullMode mode)
{
	m_CullMode = mode;
}

S32 C_Material::GetMaterialTypeId() const
{
	return m_MaterialTypeId;
}

void C_Material::SetMaterialTypeId(S32 id)
{
	m_MaterialTypeId = id;
}

Ut::UtVector<Ut::DynamicVector<C_Material::TextureInfo> >& C_Material::GetTextureInfoArray()
{
	return m_TextureInfo;
}

const Ut::UtVector<Ut::DynamicVector<C_Material::TextureInfo> >& C_Material::GetTextureInfoArray() const
{
	return m_TextureInfo;
}

void C_Material::Enable(U32 pass) const
{
	if (m_CullMode != Display::E_CullMode_Default)
		Display::Device()->SetCullMode(m_CullMode);

	if (m_BlendMode != Display::E_BlendMode_Default)
		Display::Device()->SetBlendMode(m_BlendMode);

	m_TextureState.Enable();
}

void C_Material::Disable() const
{
}

bool C_Material::LoadTextures(C_TextureManager& manager, U32 clampSize)
{
	U32 texturesLoaded = 0;
	for (S32 i=0; i<m_TextureInfo.Size(); ++i)
	{
		switch(m_TextureInfo[i].m_Type)
		{
		case E_TextureType_2D:
		{
			C_ResourcePtr<Display::C_RefTexture2D> ptr = manager.LoadTexture2DFromFile(m_TextureInfo[i].m_FileName, clampSize);
			if (ptr)
			{
				m_TextureState.Set2D(texturesLoaded, ptr);
				++texturesLoaded;
			}
			else
				m_TextureState.Set2D(texturesLoaded, manager.GetDefault2DTexture());
			break;
		}
		case E_TextureType_3D:
		{
			C_ResourcePtr<Display::C_RefTexture3D> ptr = manager.LoadTexture3DFromFile(m_TextureInfo[i].m_FileName, clampSize);
			if (ptr)
			{
				m_TextureState.Set3D(texturesLoaded, ptr);
				++texturesLoaded;
			}
			else
				m_TextureState.Set3D(texturesLoaded, manager.GetDefault3DTexture());
			break;
		}
		case E_TextureType_Cube:
		{
			C_ResourcePtr<Display::C_RefTextureCube> ptr = manager.LoadTextureCubeFromFile(m_TextureInfo[i].m_FileName, clampSize);
			if (ptr)
			{
				m_TextureState.SetCube(texturesLoaded, ptr);
				++texturesLoaded;
			}
			else
				m_TextureState.SetCube(texturesLoaded, manager.GetDefaultCubeTexture());
			break;
		}
		case E_TextureType_Unknown:
		default:
			break;
		}
	}
	m_TexturesLoaded = true;
	return (texturesLoaded == m_TextureInfo.Size());
}

bool C_Material::TexturesLoaded() const
{
	return m_TexturesLoaded;
}

U32 C_Material::GetHash() const
{
	U32 hash = 0;
	for (S32 i=0; i<m_TextureInfo.Size(); ++i)
		hash += Ut::HashKeyOpString::GetHash(m_TextureInfo[i].m_FileName);

	hash += m_MaterialTypeId;
	return hash;
}