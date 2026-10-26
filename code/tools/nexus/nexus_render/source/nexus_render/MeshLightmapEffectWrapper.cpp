#include <nexus_render/stdafx.h>
#include <nexus_render/camera.h>

using namespace NexusRender;

MeshLightmapEffectWrapper::MeshLightmapEffectWrapper() : MeshTextureEffectWrapper()
{
	m_lightmapTextureHandle = 0;
	m_lightMapUVOffsetsHandle = 0;
}

Tu::Status MeshLightmapEffectWrapper::Build(D3DShaderManager* pShaderManager)
{
	Tu::Status status;
	status = __super::Build(pShaderManager);
	if (!status) return status;

	//Build variable handles
	m_lightmapTextureHandle = 
		m_pEffect->GetParameterByName(0, "g_lightmapTexture");
	m_lightMapUVOffsetsHandle = 
		m_pEffect->GetParameterByName(0, "g_fLmUVOffsets");

	return Tu::Status::SUCCESS;
}

Tu::Status MeshLightmapEffectWrapper::SetTechnique(
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
		strcat(techniqueName, "_CM");
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

/*
Tu::Status MeshLightmapEffectWrapper::UpdateTextures(
	SceneRenderer* pSceneRenderer,
	const SceneRendererSettings& settings,
	MeshLightmapEffectConstants& effectConstants)
{
	if (settings.useCropSheet)
	{
		if (effectConstants.m_cropSheetColourTextureIndex == -1)
		{
			//No crop sheet texture found, use blank texture.
			if (FAILED(m_pEffect->SetTexture(
				m_textureCSHandle,
				pSceneRenderer->GetTextureManagerPtr()->Blank())))
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
		}
		else
		{
			const SceneRenderer::CropSheetImageRecord& csRecord = 
				pSceneRenderer->GetCropSheetImageRecord(effectConstants.m_cropSheetColourTextureIndex);
			const SceneRenderer::CropSheetAtlasRecord& atlasRecord = 
				pSceneRenderer->GetCropSheetAtlasRecord(csRecord.atlasIndex);

			//From ShaderX3
			{
				float const kUOffset = csRecord.uOffset;
				float const kVOffset = csRecord.vOffset;
				float const kWidth   = csRecord.uScale;
				float const kHeight  = csRecord.vScale;
				// clamp final repeat coordinates against these min/max values;
				// should use half-texel adjustment -- fudging these, however, 
				// generally yields better results, although they
				// are actually wrong (as visible in very low-res (32x32) textures)
				float const kFudge = 3.0f;
				float const kMinU  = kUOffset + kFudge*0.5f/256.f;
				float const kMinV  = kVOffset + kFudge*0.5f/512.f;
				float const kMaxU  = kUOffset + kWidth  - kFudge*0.5f/256.f;
				float const kMaxV  = kVOffset + kHeight - kFudge*0.5f/512.f;

				if (FAILED(m_pEffect->SetVector(m_offsetWidthHeightCSHandle, &D3DXVECTOR4(kUOffset,  kVOffset, kWidth, kHeight))))
					return Tu::Status(Tu::Status::FAILURE, "Failed to set CS.");
				if (FAILED(m_pEffect->SetVector(m_widthHeightRecipCSHandle, &D3DXVECTOR4( 1.f/kWidth, 1.f/kHeight,  0, 0))))
					return Tu::Status(Tu::Status::FAILURE, "Failed to set CS.");
				if (FAILED(m_pEffect->SetVector(m_clampCSHandle, &D3DXVECTOR4( kMinU, kMinV, kMaxU, kMaxV))))
					return Tu::Status(Tu::Status::FAILURE, "Failed to set CS.");
			}
			int rotated;
			if (csRecord.rotated)
				rotated = 1;
			else
				rotated = 0;
			if (FAILED(m_pEffect->SetValue( m_rotatedCSHandle, &rotated, sizeof(int))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set rotation.");

			if (atlasRecord.pTexture)
			{
				if (FAILED(m_pEffect->SetTexture( m_textureCSHandle, atlasRecord.pTexture)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
			}
			//Lightmap textures are enabled.
			if (effectConstants.m_pLightmapTexture)
			{
				//Use lightmap texture.
				if (FAILED(m_pEffect->SetTexture(
					m_texture1Handle, 
					effectConstants.m_pLightmapTexture)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
			}
			else
			{
				//No lightmap texture found, use blank texture.
				if (FAILED(m_pEffect->SetTexture(
					m_texture1Handle,
					pSceneRenderer->GetTextureManagerPtr()->Blank())))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
			}

		}
	}
	else if (settings.useTextures)
	{
		if (effectConstants.m_pColourTexture)
		{
			//Use diffuse texture.
			if (FAILED(m_pEffect->SetTexture(
				m_texture0Handle,
				effectConstants.m_pColourTexture)))
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
		}
		else
		{
			//No diffuse texture found, use blank texture.
			if (FAILED(m_pEffect->SetTexture(
				m_texture0Handle, 
				pSceneRenderer->GetTextureManagerPtr()->Blank())))
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
		}

		if (!settings.useLightmapTextures)
		{
			//We're not using lightmap textures.
			if (FAILED(m_pEffect->SetTexture(
				m_texture1Handle, 
				pSceneRenderer->GetTextureManagerPtr()->Blank())))
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
		}
		else
		{
			//Lightmap textures are enabled.
			if (effectConstants.m_pLightmapTexture)
			{
				//Use lightmap texture.
				if (FAILED(m_pEffect->SetTexture(
					m_texture1Handle, 
					effectConstants.m_pLightmapTexture)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
			}
			else
			{
				//No lightmap texture found, use blank texture.
				if (FAILED(m_pEffect->SetTexture(
					m_texture1Handle, 
					pSceneRenderer->GetTextureManagerPtr()->Blank())))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
			}
		}
	}
	else
	{
		//No diffuse texture found, use blank texture.
		if (FAILED(m_pEffect->SetTexture(
			m_texture0Handle, 
			pSceneRenderer->GetTextureManagerPtr()->Blank())))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);

		//No lightmap texture found, use blank texture.
		if (FAILED(m_pEffect->SetTexture(
			m_texture1Handle, 
			pSceneRenderer->GetTextureManagerPtr()->Blank())))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);

		//No CS texture found, use blank texture.
		if (FAILED(m_pEffect->SetTexture(
			m_textureCSHandle, 
			pSceneRenderer->GetTextureManagerPtr()->Blank())))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TEXTURE);
	}

	return Tu::Status::SUCCESS;
}
*/

void MeshLightmapEffectWrapper::UpdateNodeConstants(const RenderNode& renderNode) 
{
	__super::UpdateNodeConstants(renderNode);

	if (renderNode.HasType(RenderNode::RENDER_NODE_TYPE_MESH_REMAPPABLE))
	{
		m_pEffect->SetFloatArray(
			m_lightMapUVOffsetsHandle,
			((const RenderMeshRemappableNode&)renderNode).GetLightmapUVOffsets(),
			2);
	}
}

void MeshLightmapEffectWrapper::UpdateMaterialConstants(
	SceneRenderer& sceneRenderer,
	const EffectConstants& constants)
{
	__super::UpdateMaterialConstants(
		sceneRenderer,
		constants);

	MeshLightmapEffectConstants& meshLightmapEffectConstants = (MeshLightmapEffectConstants&)constants;
	if (sceneRenderer.GetSettingsPtr()->useTextures)
	{
		if (meshLightmapEffectConstants.m_pLightmapTexture)
			//Use lightmap texture.
			m_pEffect->SetTexture(m_lightmapTextureHandle, meshLightmapEffectConstants.m_pLightmapTexture);
		else
			//No lightmap texture, set blank texture.
			m_pEffect->SetTexture(m_lightmapTextureHandle, sceneRenderer.GetTextureManagerPtr()->Blank());
	}
	else
	{
		//No lightmap texture, set blank texture.
		m_pEffect->SetTexture(m_lightmapTextureHandle, sceneRenderer.GetTextureManagerPtr()->Blank());
	}
}
