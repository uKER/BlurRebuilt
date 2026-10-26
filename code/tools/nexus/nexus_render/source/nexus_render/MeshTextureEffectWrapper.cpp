#include <nexus_render/stdafx.h>
#include <nexus_render/camera.h>

using namespace NexusRender;

MeshTextureEffectWrapper::MeshTextureEffectWrapper() : MeshBasicEffectWrapper()
{
	m_offsetWidthHeightCSHandle = 0;
	m_clampCSHandle = 0;

	m_colourTextureHandle = 0;
	m_colourTextureCSHandle = 0;
}

Tu::Status MeshTextureEffectWrapper::Build(D3DShaderManager* pShaderManager)
{
	Tu::Status status;
	status = __super::Build(pShaderManager);
	if (!status) return status;

	m_offsetWidthHeightCSHandle = m_pEffect->GetParameterByName(0, "g_offsetWidthHeight");
	ASSERT(m_offsetWidthHeightCSHandle);
	m_clampCSHandle = m_pEffect->GetParameterByName(0, "g_clamping");
	ASSERT(m_clampCSHandle);
	m_rotatedCSHandle = m_pEffect->GetParameterByName(0, "g_rotatedCS");
	ASSERT(m_rotatedCSHandle);

	m_colourTextureHandle = m_pEffect->GetParameterByName(0, "g_colourTexture");
	ASSERT(m_colourTextureHandle);
	m_colourTextureCSHandle = m_pEffect->GetParameterByName(0, "g_cropTexture");
	ASSERT(m_colourTextureHandle);

	return Tu::Status::SUCCESS;
}

Tu::Status MeshTextureEffectWrapper::SetTechnique(
	const SceneRendererSettings& settings, 
	D3DManager::EVertexDecl decl,
	IRenderMeshNode::EEffectMode mode)
{
	static char techniqueName[256];
	const char* modeStr = IRenderMeshNode::GetEffectModeAsString(mode);
	const char* declStr = D3DManager::GetVertexDeclAsString(decl);
	//Effect mode
	strcpy(techniqueName, modeStr);
	strcat(techniqueName, "_");
	//Vertex declaration
	strcat(techniqueName, declStr);

	//Component mode.
	if (settings.componentMode != 0)
	{
		strcat_s(techniqueName, "_CM");
		static char componentModeStr[11];
		itoa(settings.componentMode, componentModeStr, 10);
		strcat(techniqueName, componentModeStr);
	}

	//Real-time lighting
	if (settings.useRealTimeLighting)
		strcat(techniqueName, "_RTL1");

	//Crop sheet mode
	if (settings.useCropSheet)
		strcat(techniqueName, "_CS1");

	if (FAILED(m_pEffect->SetTechnique(techniqueName)))
		return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TECHNIQUE);

	return Tu::Status::SUCCESS;
}

EffectWrapper::EEffectType MeshTextureEffectWrapper::GetType() const
{
	return EffectWrapper::EFFECT_TYPE_MESH_TEXTURE;
}

const char* MeshTextureEffectWrapper::GetEffectName() const
{
	#ifdef LOAD_SHADERS_FROM_FILE
		static const char* pEffectName = "../nexus_render/Shaders/MeshTexture.fx";
	#else
		static const char* pEffectName = "IDR_MESH_TEXTURE_EFFECT";
	#endif
	return pEffectName;
}

void MeshTextureEffectWrapper::UpdateNodeConstants(const RenderNode& renderNode)
{
	__super::UpdateNodeConstants(renderNode);
}

void MeshTextureEffectWrapper::UpdateMaterialConstants(
	SceneRenderer& sceneRenderer,
	const EffectConstants& constants)
{
	MeshTextureEffectConstants& meshTextureEffectConstants = (MeshTextureEffectConstants&)constants;
	if (sceneRenderer.GetSettingsPtr()->useCropSheet)
	{
		if (meshTextureEffectConstants.m_cropSheetColourTextureIndex == -1)
		{
			m_pEffect->SetTexture( 
				m_colourTextureCSHandle, 
				sceneRenderer.GetTextureManagerPtr()->Blank());
		}
		else
		{
			const SceneRenderer::CropSheetImageRecord& csRecord = 
				sceneRenderer.GetCropSheetImageRecord(meshTextureEffectConstants.m_cropSheetColourTextureIndex);
			const SceneRenderer::CropSheetAtlasRecord& atlasRecord = 
				sceneRenderer.GetCropSheetAtlasRecord(csRecord.atlasIndex);
			
			if (atlasRecord.pTexture)
			{
				float const kUOffset = csRecord.uOffset;
				float const kVOffset = csRecord.vOffset;
				float const kWidth   = csRecord.uScale;
				float const kHeight  = csRecord.vScale;
				//From ShaderX3
				// clamp final repeat coordinates against these min/max values;
				// should use half-texel adjustment -- fudging these, however, 
				// generally yields better results, although they
				// are actually wrong (as visible in very low-res (32x32) textures)
				float const kFudge = 3.f;
				float const kMinU  = kUOffset + kFudge*0.5f / (atlasRecord.width);
				float const kMinV  = kVOffset + kFudge*0.5f / (atlasRecord.height);
				float const kMaxU  = kUOffset + kWidth  - kFudge*0.5f / (atlasRecord.width);
				float const kMaxV  = kVOffset + kHeight - kFudge*0.5f / (atlasRecord.height);
				m_pEffect->SetVector(
					m_offsetWidthHeightCSHandle, &D3DXVECTOR4(kUOffset,  kVOffset, kWidth, kHeight));
				m_pEffect->SetVector(
					m_clampCSHandle, &D3DXVECTOR4( kMinU, kMinV, kMaxU, kMaxV));

				int rotated;
				if (csRecord.rotated)
					rotated = 1;
				else
					rotated = 0;
				m_pEffect->SetValue(m_rotatedCSHandle, &rotated, sizeof(int));

				if (atlasRecord.pTexture)
					m_pEffect->SetTexture(m_colourTextureCSHandle, atlasRecord.pTexture);
			}
			else
			{
				//No CS texture found, use blank texture.
				m_pEffect->SetTexture(m_colourTextureCSHandle, sceneRenderer.GetTextureManagerPtr()->Blank());
			}
		}
	}
	else if (sceneRenderer.GetSettingsPtr()->useTextures)
	{
		if (meshTextureEffectConstants.m_pColourTexture)
			//Use colour texture.
			m_pEffect->SetTexture(m_colourTextureHandle, meshTextureEffectConstants.m_pColourTexture);
		else
			//No diffuse texture found, use blank texture.
			m_pEffect->SetTexture(m_colourTextureHandle, sceneRenderer.GetTextureManagerPtr()->Blank());
	}
	else
	{
		//No diffuse texture found, use blank texture.
		m_pEffect->SetTexture(m_colourTextureHandle, sceneRenderer.GetTextureManagerPtr()->Blank());

		//No CS texture found, use blank texture.
		m_pEffect->SetTexture(m_colourTextureCSHandle, sceneRenderer.GetTextureManagerPtr()->Blank());
	}
}
