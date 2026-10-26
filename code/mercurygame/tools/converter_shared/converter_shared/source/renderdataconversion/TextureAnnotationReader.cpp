#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>

#include <renderingdata/renderingdata.h>
#include <renderingdata/render.h>
#include "converter_shared\renderdataconversion\Effect.h"
#include <fx/effect.h>

#include "converter_shared\renderdataconversion\TextureAnnotationReader.h"

#include "texture_export\TextureManager.h"
#include "texture_export\TextureProcessing.h"
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\MaterialManager.h"
#include "converter_shared\renderdataconversion\ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\TextureTags.h"

#include <texture_export/texture.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <system/serialise.h>
#include "system/misc/StringUtil.h"

namespace RenderDataConversion
{

void RegisterDefaultTextureAnnotationReader()
{
	RenderDataConversion::RegisterAnnotationReader<TextureAnnotationReader>();
}

/// constructor
TextureAnnotationReader::TextureAnnotationReader(FX::Parameter *pParam, const Util::C_String &sEffectName) :
	ParamAnnotationReader(pParam, sEffectName)
{
	m_bDetectAlpha = false;
	m_bConvertNormalMap = false;
	m_bInvertNormalMapX = false;
	m_bINR0 = false;
	m_bReduce = false;
	m_Source = E_Source_None;
	m_iDefaultColour = 0xffffffff;
	m_bWarnMissing = false;
	m_iUVSet = 0;
	m_fuScale = 1.f;
	m_fvScale = 1.f;
	m_fWeight = 1.f;
	m_nMipMaps = 0;
	m_bEnhanceMipmapAlpha = false;
	m_MipFilter = ActTools::FT_LANCZOS;
}

/// destructor
TextureAnnotationReader::~TextureAnnotationReader()
{
}

/// list of default texture colours
const TextureAnnotationReader::DefaultColourItem TextureAnnotationReader::m_DefaultColours[] = 
{
	// name  ABGR
	{ "black", 0xff000000 },
	{ "white", 0xffffffff },
	{ "red", 0xff0000ff },
	{ "green", 0xff00ff00 },
	{ "blue", 0xffff0000 },
	{ "normal", 0xffff7f7f },
	{ "noalpha", 0 },
	{ "" },
};

/// identify source semantic
bool TextureAnnotationReader::IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type)
{
	if (type != E_CustomSemanticType_Texture)
		return false;

	return ParamAnnotationReader::IdentifySemantic(pszSource, type);
}

/// parse a single annotation
bool TextureAnnotationReader::ParseAnnotation(FX::Annotation *pAnnotation)
{
	if (!stricmp(pAnnotation->Name, "AlphaDetect"))
	{
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_BOOL && pAnnotation->Bool)
		{
			m_bDetectAlpha = true;
			return true;
		}
	}
	else if (!stricmp(pAnnotation->Name, "ConvertNormalMap"))
	{
		// convert heightmap to normal map annotation
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_BOOL && pAnnotation->Bool)
		{
			m_bConvertNormalMap = true;
			return true;
		}
	}
	else if (!stricmp(pAnnotation->Name, "InvertNormalMapX"))
	{
		// flip normal map x
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_BOOL && pAnnotation->Bool)
		{
			m_bInvertNormalMapX = true;
			return true;
		}
	}
	else if (!stricmp(pAnnotation->Name, "INR0"))
	{
		// convert to INR0 annotation
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_BOOL && pAnnotation->Bool)
		{
			m_bINR0 = true;
			return true;
		}
	}
	else if (!stricmp(pAnnotation->Name, "MipFilter"))
	{
		// convert to filter name
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
		{
			ActTools::FilterType f = ActTools::GetFilterType(pAnnotation->String); 
			if (f != ActTools::FT_INVALID)
			{
				m_MipFilter = f;
			}
			else
			{
				LOGWARN( g_Log, "%S: unrecognised mip filter type '%s' on parameter '%s'\n", m_sEffectName.Chars(), pAnnotation->String, m_pParam->Name.Chars());
			}
			return true;
		}
	}
	else if (!stricmp(pAnnotation->Name, "Default"))
	{
		// interpret default texture type annotation
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
		{
			int x;
			for (x=0; m_DefaultColours[x].sName[0]!=0; x++)
			{
				if (!stricmp(m_DefaultColours[x].sName, pAnnotation->String))
				{
					m_iDefaultColour = m_DefaultColours[x].iColour;
					return true;
				}
			}
			if (m_DefaultColours[x].sName[0] == 0)
			{
				PrintWarning( "%s: unrecognised default texture type '%s' on parameter '%s'\n", m_sEffectName.Chars(), pAnnotation->String, m_pParam->Name.Chars());
			}
			return true;
		}
		// interpret default texture type annotation
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_INT)
		{
			m_iDefaultColour = pAnnotation->Int;
			return true;
		}
	}
	else if (!stricmp(pAnnotation->Name, "UVSet"))
	{
		// interpret UV-set annotation
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
		{
			m_sUVSet = pAnnotation->String;
			return true;
		}
		else if (pAnnotation->Type == FX::NGLFX_ANNOTATION_INT)
		{
			m_iUVSet = pAnnotation->Int;
			return true;
		}
	}
	else if (!stricmp(pAnnotation->Name, "Format"))
	{
		// interpret texture format override annotation
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
		{
			m_sTexelFmtOverride = pAnnotation->String;
			return true;
		}
	}
	// interpret additional UV-mapping variables for the streamer
	// if the type is string, interpret it as a maya material parameter (e.g. scale etc.)
	else if (!stricmp(pAnnotation->Name, "uScale"))
	{
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_FLOAT)
			m_fuScale = pAnnotation->Float;
		else if (pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
			m_suScale = pAnnotation->String;
		m_iUVSet = -1;
		return true;
	}
	else if (!stricmp(pAnnotation->Name, "vScale"))
	{
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_FLOAT)
			m_fvScale = pAnnotation->Float;
		else if (pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
			m_svScale = pAnnotation->String;
		m_iUVSet = -1;
		return true;
	}
	else if (!stricmp(pAnnotation->Name, "weight"))
	{
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_FLOAT)
			m_fWeight = pAnnotation->Float;
		else if (pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
			m_sWeight = pAnnotation->String;
		m_iUVSet = -1;
		return true;
	}
	// number of mipmaps
	else if (!stricmp(pAnnotation->Name, "NumMipMaps"))
	{
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_INT)
			m_nMipMaps = pAnnotation->Int;
		return true;
	}
	else if (!stricmp(pAnnotation->Name, "EnhanceMipMapAlpha"))
	{
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_BOOL && pAnnotation->Bool)
		{
			m_bEnhanceMipmapAlpha = true;
			return true;
		}
	}
	return (ParamAnnotationReader::ParseAnnotation(pAnnotation));
}

void SetSamplerStateFromTextureTags(RenderingData::SamplerState& samplerState, const C_TextureTags& textureTags )
{
	switch(textureTags.m_AddressModeU)
	{
		default:
		case E_TextureTagAddressMode_Wrap: samplerState.SetAddressU( RenderingData::TEXTURE_ADDRESS_WRAP ); break;
		case E_TextureTagAddressMode_Mirror: samplerState.SetAddressU( RenderingData::TEXTURE_ADDRESS_MIRROR ); break;
		case E_TextureTagAddressMode_Clamp: samplerState.SetAddressU( RenderingData::TEXTURE_ADDRESS_CLAMP ); break;
	}

	switch(textureTags.m_AddressModeV)
	{
		default:
		case E_TextureTagAddressMode_Wrap: samplerState.SetAddressV( RenderingData::TEXTURE_ADDRESS_WRAP ); break;
		case E_TextureTagAddressMode_Mirror: samplerState.SetAddressV( RenderingData::TEXTURE_ADDRESS_MIRROR ); break;
		case E_TextureTagAddressMode_Clamp: samplerState.SetAddressV( RenderingData::TEXTURE_ADDRESS_CLAMP ); break;
	}

	switch(textureTags.m_FilteringType)
	{
		case E_TextureTagFilter_Linear:
			samplerState.SetMagFilter( RenderingData::TEXTURE_FILTER_LINEAR );
			samplerState.SetMinFilter( RenderingData::TEXTURE_FILTER_LINEAR );
			samplerState.SetMipFilter( RenderingData::TEXTURE_FILTER_LINEAR );
		break;
		case E_TextureTagFilter_Anisotropic:
			samplerState.SetMagFilter( RenderingData::TEXTURE_FILTER_LINEAR );
			samplerState.SetMinFilter( RenderingData::TEXTURE_FILTER_ANISOTROPIC );
			samplerState.SetMipFilter( RenderingData::TEXTURE_FILTER_LINEAR );
		break;
		case E_TextureTagFilter_Point:
			samplerState.SetMagFilter( RenderingData::TEXTURE_FILTER_POINT );
			samplerState.SetMinFilter( RenderingData::TEXTURE_FILTER_LINEAR );
			samplerState.SetMipFilter( RenderingData::TEXTURE_FILTER_LINEAR );
		break;
	}

	samplerState.SetMaxAnisotropy( textureTags.m_MaxAnisotropy );
	samplerState.SetMipMapLodBias( textureTags.m_MipmapBias );
}

// work out the source texture filename
Util::C_String TextureAnnotationReader::GetTextureFilename(AftkMaterialConverter *pMtl)
{
	Util::C_String textureName;

	switch (m_Source)
	{
	default:
		assert(0); // shouldn't get here
		break;
	// TODO: potentially refactor this so each source is it's own annotation reader class
	case E_Source_NamedParam:
		{
			m_bWarnMissing = true;
			char uvSetName[MAX_PATH];
			memset(uvSetName, 0, sizeof(char)*MAX_PATH);
			textureName = pMtl->GetTextureFilenameFromPlug(m_sNamedParamName, uvSetName);
			if (m_sUVSet == "" && strcmp(uvSetName, "") != 0)
				m_sUVSet = uvSetName;
		}
		break;
	case E_Source_Diffuse:
		m_bWarnMissing = true;
		textureName = pMtl->GetDiffuseTextureFilename();
		break;
	case E_Source_Lightmap:
		textureName = pMtl->GetLightmapTextureFilename();
		break;
	case E_Source_InvalidTexture:
		textureName = pMtl->GetTextureManager()->GetMissingTextureFilename();
		break;
	}

	return textureName;
}

/// read out data for this parameter, given an Aftk Material
/// automatically advances the pData pointer
bool TextureAnnotationReader::GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **pData)
{
	Util::C_String sTextureName;
	Util::C_String originalTextureName;

	bool bOk = false;

	// setup material element description
	pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX;
	pMtlElement->count = 1;
	pMtlElement->name = m_pParam->Name.Chars();

	// should this texture be exposed?
	if (m_Source == E_Source_NamedParam)
	{
		Util::C_String sName = pMtl->GetExposedParameterName(m_sNamedParamName);
		if (sName.Length() > 0)
		{
			pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR;
			// export this texture as a named exposed parameter
			SetupAnimatedParameter(pMtl, pMtlElement, pData, sName);

			return true;
		}
	}

	// work out the source texture filename
	sTextureName = GetTextureFilename(pMtl);

	// HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK
	Util::C_String sFile = Util::ExtractFilename(sTextureName);
	if (!stricmp(sFile.Chars(), "smashedglass.tga"))
		sTextureName = Util::ReplaceFileExtension(sTextureName,"dds");
	// HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK

	// Get the original filename to use as the texture name.
	{
		Sys::C_FileInfo info;
		Sys::C_FileSystem::Instance().FileInfo(sTextureName, &info );
		originalTextureName = info.m_originalFilename.Chars();
	}

	// setup texture info
	TextureExport::TextureInfo info;
	TextureExport::InputFile item;

	item.bRequired = m_bWarnMissing;
	item.sName = sTextureName;
	info.m_InputFilenames.PushBack(item);
	info.m_sTexelFmtOverride = m_sTexelFmtOverride;

	if (m_bINR0)
	{
		{
			Util::C_String sTmp = "";
			if (sTextureName.ISearch("_IPR0") != -1)
			{
				sTmp = sTextureName.IReplace("_IPR0", "_NEON");
				if (!pMtl->TextureFileExists(sTmp))
				{
					sTmp = "";
				}
			}
			TextureExport::InputFile item;
			item.bRequired = m_bWarnMissing;
			item.sName = sTmp;
			info.m_InputFilenames.Resize(2);
			info.m_InputFilenames[1] = item;
		}
	}

	// load texture and return texture index
	// TODO: replace "Texture" with the filename of the texture (without path).

	//read texture tags
	C_TextureTags textureTags;
	// TODO : pass in an I_TextureTagsReader externally from somewhere
	C_DefaultTextureTagsReader defaultReader;
	defaultReader.ReadTextureTags(sTextureName, textureTags);

	info.m_iDefaultColour = m_iDefaultColour;
	info.convertToNormalMap = m_bConvertNormalMap;
	info.invertNormalMapX = m_bInvertNormalMapX;
	info.bINR0 = m_bINR0;
	info.bReduce = m_bReduce;
	info.bSRGBTexture = ((m_pParam->Sampler->iSamplerState>>20)&1)?true:false;
	info.m_nMipMaps = m_nMipMaps;
	info.m_MipFilter = m_MipFilter;
	info.bEnhanceMipMapAlpha = m_bEnhanceMipmapAlpha;

	 // IPR processing hack
	{
		Util::C_String sTmp = "";
		if (sTextureName.ISearch("_IPR0") != -1)
		{
			sTmp = sTextureName.IReplace("_IPR0", "_DIFF");
			if (!pMtl->TextureFileExists(sTmp))
			{
				sTmp = "";
			}
			else
			{
				if (!textureTags.m_OverrideIprReduce)
				{
					info.bReduce = true;
				}

				TextureExport::InputFile item;
				item.bRequired = m_bWarnMissing;
				item.sName = sTmp;
				info.m_InputFilenames.Resize(3);
				info.m_InputFilenames[2] = item;
			}
		}
	}

	if (textureTags.m_AlphaType != E_TextureTagAlphaType_Punchthrough)
		info.bForceGreyScaleAlpha = true;

	if (textureTags.m_AddressModeU == E_TextureTagAddressMode_Clamp)
		info.m_EdgeFilterU = ActTools::EDGE_CLAMP;

	if (textureTags.m_AddressModeV == E_TextureTagAddressMode_Clamp)
		info.m_EdgeFilterV = ActTools::EDGE_CLAMP;

	U32 iHash;
	
	if (sTextureName.Length() > 0 && sTextureName.ISearch("null_texture") == -1 && sTextureName.ISearch("ignoreme_") == -1)
	{
		iHash = Btl::Hash(sTextureName.Chars());
		bOk = true;

		// check whether the texture exists
		if (pData && !pMtl->TextureFileExists(sTextureName))
		{
			if (m_bWarnMissing)
			{
				// only warn once for each texture
				static Btl::Set<Util::C_String> filenameSet;
				Btl::Set<Util::C_String>::Iterator iter = filenameSet.Find(sTextureName);
				if (iter == filenameSet.End())
				{
					filenameSet.Insert(sTextureName);
					LOGWARN( g_Log, "Required texture '%s' not found.\n", sTextureName.Chars());
				}
			}
			info.m_InputFilenames[0].sName = pMtl->GetTextureManager()->GetMissingTextureFilename();
		}
	}
	else
	{
		if (m_bAllowOmit)
			return false;
		iHash = info.m_iDefaultColour;
	}

	int textureCacheIndex = pMtl->GetTextureManager()->GetTextureIndex(&info);

	if (!pData)
		return bOk;

	// get exported texture data and read into ActTools::Texture
	ActTools::Texture &texture = *pMtl->GetTextureManager()->GetTexture(textureCacheIndex, info);

	// if 'AlphaDetect' was specified, detect the alpha on this texture and set that to the material
	if (m_bDetectAlpha)
	{
		enum E_AlphaType
		{
			E_AlphaType_Opaque = 0,
			E_AlphaType_AlphaDither,
			E_AlphaType_Punchthrough,
			E_AlphaType_AlphaBlend,
			E_AlphaType_Decal
		};

		E_AlphaType alphaType = E_AlphaType_Opaque;

		// check texture and determine alpha type
		const S32 alphaPresence = texture.GetAlphaPresence();
		bool hasAlpha = false;
		switch( alphaPresence )
		{
			default:									
			break;
			
			case ActTools::Texture::ALPHA_BLACKWHITE:
			case ActTools::Texture::ALPHA_GREYSCALE:	
				hasAlpha = true;
			break;
		}

		if( hasAlpha )
		{
			switch( textureTags.m_AlphaType )
			{
				case E_TextureTagAlphaType_AlphaDither:
					alphaType = E_AlphaType_AlphaDither;
				break;
				case E_TextureTagAlphaType_Punchthrough:
					alphaType = E_AlphaType_Punchthrough;
				break;
				case E_TextureTagAlphaType_AlphaBlend:
					alphaType = E_AlphaType_AlphaBlend;
				break;
				case E_TextureTagAlphaType_Decal:
					alphaType = E_AlphaType_Decal;
				break;
			}
		}

		switch (alphaType)
		{
			case E_AlphaType_Opaque:
				pMtl->SetSortType(RenderingData::SORT_TYPE_KEY); 
			break;
			case E_AlphaType_AlphaDither:
				pMtl->SetSortType(RenderingData::SORT_TYPE_KEY); 
				pMtl->SetAlphaTestEnable(true, 0);
				pMtl->SetAlphaToMaskEnable(true);
			break;
			case E_AlphaType_Punchthrough:
				pMtl->SetSortType(RenderingData::SORT_TYPE_KEY); 
				pMtl->SetAlphaTestEnable(true, textureTags.m_AlphaRef);
			break;
			case E_AlphaType_AlphaBlend:
				pMtl->SetSortType(RenderingData::SORT_TYPE_BACK_FRONT);
				pMtl->SetAlphaBlendEnable(true);
				pMtl->SetAlphaTestEnable(true, 0);
			break;
			case E_AlphaType_Decal:				
				pMtl->SetSortType(RenderingData::SORT_TYPE_KEY); 
				pMtl->SetAlphaBlendEnable(true);
				pMtl->SetAlphaTestEnable(true, 0);
				pMtl->SetDecalOrder( textureTags.m_DecalOrder );
			break;
		}
	}

	bool shared = (pMtl->GetTextureManager()->GetShared() || texture.GetType() == ActTools::Texture::CUBETEXTURE || textureTags.m_Shared);

	if (shared) // IPR processing hack
	{
		info.bReduce = false;
	}

	RenderingData::Build::Texture t1;	
	t1.Initialise(Btl::String(bOk ? Util::RemoveFileExtension(originalTextureName) : "DefaultColour"), textureCacheIndex, iHash, pMtl->GetTextureManager());

	SetSamplerStateFromTextureTags(t1.GetSamplerState(), textureTags);

	if (info.bReduce) // IPR processing hack
	{
		t1.GetSamplerState().SetMipMapLodBias(16);
	}

	t1.GetSamplerState().SetSRGB(info.bSRGBTexture ? 1 : 0);

	//t1.m_RenderState. ... <- set this
	int iTextureIndex;
	
	if (shared)
		iTextureIndex = pMtl->GetSceneExport()->InsertSharedTexture(t1);
	else
		iTextureIndex = pMtl->GetSceneExport()->InsertStreamedTexture(t1, 0); // Putting all textures in block 0 - they get moved later.

	// add textureinfo which will be used for the streamer
	pMtl->AddTextureInfo(iTextureIndex, m_sUVSet, m_iUVSet, m_suScale, m_fuScale, m_svScale, m_fvScale, m_sWeight, m_fWeight);

	*(int*)(*pData) = iTextureIndex;
	(*pData)+=4;

	return bOk;
}

};
