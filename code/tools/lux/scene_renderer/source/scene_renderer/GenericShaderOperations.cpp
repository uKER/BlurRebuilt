#include "stdafx.h"
#include <scene_renderer/D3dImageLoader.h>
#include <scene_renderer/D3dTextureManager.h>
#include <scene_renderer/ShaderProperties.h>
#include <scene_renderer/LightmapShaderProperties.h>


// JAN: we can turn off loading textures - this is for faster debugging
//#ifdef KEN
//#define LOAD_TEXTURES
//#endif

extern float g_vSH[4];

namespace SRD
{
	IDirect3DTexture9* g_pTestHackTexture;

	bool InitialiseGenericLightmapShader(ID3DXEffect* pShader)
	{
		D3DXCOLOR colorMtrlDiffuse(1.0f, 1.0f, 1.0f, 1.0f);
		D3DXCOLOR colorMtrlAmbient(0.35f, 0.35f, 0.35f, 0);

		pShader->SetValue("g_MaterialAmbientColor", &colorMtrlAmbient, sizeof(D3DXCOLOR) );
		pShader->SetValue("g_MaterialDiffuseColor", &colorMtrlDiffuse, sizeof(D3DXCOLOR) );    

		return true;
	}

	bool UpdateGenericLightmapShader(
		LightmapShaderProperties* pProperties,
		const D3DXMATRIX& view,
		const D3DXMATRIX& worldView,
		const D3DXMATRIX& worldObject
		)
	{
		ID3DXEffect* pShader = pProperties->GetShaderPtr();
		if ((pProperties == 0) ||(pShader == 0))
			return false;


		// Update the effect's variables.  Instead of using strings, it would 
		// be more efficient to cache a handle to the parameter by calling 
		// ID3DXEffect::GetParameterByName
		pShader->SetMatrix( "g_mWorldViewProjection", &worldView );
		pShader->SetMatrix( "g_mWorld", &worldObject );
		pShader->SetMatrix( "g_mView", &view );

		D3DXCOLOR vWhite = D3DXCOLOR(1,1,1,1);
		pShader->SetValue("g_MaterialDiffuseColor", &vWhite, sizeof(D3DXCOLOR) );

		pShader->SetTexture( "g_MeshTexture", pProperties->GetDiffuseTexturePtr());
		pShader->SetTexture( "g_LightmapTexture", pProperties->GetLightmapTexturePtr());

		pShader->SetFloatArray( "g_vSH", g_vSH, 4);

		return true;
	}

	bool ExtractLightmapShaderPropertiesFromAftk(
		const Aftk::C_MaterialNode* pAftkMaterial, 
		LightmapShaderProperties* pShaderProperties)
	{
		pShaderProperties->SetDiffuseTexturePtr(0);
		pShaderProperties->SetLightmapTexturePtr(0);

		if (pAftkMaterial == 0)
			return true;

		if (pAftkMaterial->GetNumShaderImageNodes() > 0)
		{
			const Aftk::C_ShaderImageNode* pImage = pAftkMaterial->GetShaderImageNodePtr(0);
#ifdef LOAD_TEXTURES
			if (strcmp(pAftkMaterial->GetShaderImageNodePtr(0)->GetImageType().c_str(),"lightmap"))
			{
				IDirect3DTexture9* pTexture = SR_D3D_TEXTURE_MANAGER.Load(pImage->GetFileName());
				pShaderProperties->SetDiffuseTexturePtr(pTexture);
			}
#endif
		}

		// find associated lightmap texture
		for (U32 iMap=0; iMap<pAftkMaterial->GetNumShaderImageNodes(); iMap++)
		{
			std::string imageType;
			pAftkMaterial->GetShaderImageNodePtr(iMap)->GetImageTypePlug().GetValue(imageType);
			if (!strcmp(imageType.c_str(),"lightmap"))
			{
				const Aftk::C_ShaderImageNode* pImage = pAftkMaterial->GetShaderImageNodePtr(iMap);
				std::string fileName;
				pImage->GetFilePlug().GetValue(fileName);
				IDirect3DTexture9* pTexture = //SR_D3D_TEXTURE_MANAGER.Load("c:/texture.bmp");
				SR_D3D_TEXTURE_MANAGER.Load(fileName);
				pShaderProperties->SetLightmapTexturePtr(pTexture);
			}
		}

		return true;
	}

}
