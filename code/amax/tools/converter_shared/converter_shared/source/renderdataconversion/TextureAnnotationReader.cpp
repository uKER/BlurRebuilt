#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>

#include <renderingdata/renderingdata.h>
#include <renderingdata/render.h>
#include "converter_shared\renderdataconversion\Effect.h"
#include <fx/effect.h>

#include "converter_shared\renderdataconversion\AnnotationReader.h"
#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "converter_shared\renderdataconversion\TextureAnnotationReader.h"

#include "texture_export\TextureManager.h"
#include "texture_export\TextureProcessing.h"
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\MaterialManager.h"
#include "converter_shared\renderdataconversion\ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\CustomSemanticManager.h"

#include <texture_export/texture.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <system/serialise.h>

namespace RenderDataConversion
{

/// constructor
TextureAnnotationReader::TextureAnnotationReader(FX::Parameter *pParam, const Util::C_String &sEffectName) :
	ParamAnnotationReader(pParam, sEffectName)
{
	m_bDetectAlpha = false;
	m_bConvertNormalMap = false;
	m_bInvertNormalMapX = false;
	m_Source = E_Source_None;
	m_iDefaultColour = 0xffffffff;
	m_bWarnMissing = false;
	m_bShared = false;
	m_pCustom = NULL;
	ParseAnnotation();
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

/// returns true if this parameter is supposed to go on the renderingdata material
bool TextureAnnotationReader::IsMaterialParameter()
{
	return m_Source != E_Source_None;
}

/// parse the annotation and work out where to source the texture from
void TextureAnnotationReader::ParseAnnotation()
{
	// does the parameter have 'source'? if so it must come from Maya
	for (int i=0; i<m_pParam->Annotations.Size(); i++)
	{
		FX::Annotation *pAnnotation = &m_pParam->Annotations[i];

		if (!ParseAnnotation(pAnnotation))
		{
			PrintWarning( "%s: unrecognised annotation '%s' on parameter '%s'\n", m_sEffectName.Chars(), pAnnotation->Name.Chars(), m_pParam->Name.Chars());
		}
	}
}

/// parse a single annotation
bool TextureAnnotationReader::ParseAnnotation(FX::Annotation *pAnnotation)
{
	if (!stricmp(pAnnotation->Name, "Source"))
	{
		I_CustomSemantic *p = g_CustomSemanticManager.ParseSourceSemantic(pAnnotation->String, E_CustomSemanticType_Texture, m_pParam, this);
		if (p)
		{
			m_Source = E_Source_Custom;
			m_pCustom = (I_CustomTextureSemantic*)p;
			return true;
		}
	}
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
		// convert heightmap to normal map annotation
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_BOOL && pAnnotation->Bool)
		{
			m_bInvertNormalMapX = true;
			return true;
		}
	}
	else if (!stricmp(pAnnotation->Name, "Default"))
	{
		// interpret default texture type annotation
		if (pAnnotation->Type == FX::NGLFX_ANNOTATION_STRING)
		{
			if (!stricmp(pAnnotation->String, "Warn"))
			{
				m_bWarnMissing = true;
				return true;
			}
			else
			{
				for (int x=0; m_DefaultColours[x].sName[0]!=0; x++)
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
		}
	}

	return (ParamAnnotationReader::ParseAnnotation(pAnnotation));
}

/// return a RenderingData::MaterialElement that describes this parameter
/// returns false if this parameter is not supposed to be stored in the renderingdata material
bool TextureAnnotationReader::GetMaterialElement(RenderingData::MaterialElement *pMtlElement)
{
	if (!IsMaterialParameter())
		return false;

	pMtlElement->type = RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX;
	pMtlElement->count = 1;
	pMtlElement->name = m_pParam->Name.Chars();
	return true;
}

/// read out data for this parameter, given an Aftk Material
/// automatically advances the pData pointer
bool TextureAnnotationReader::GetMaterialData(AftkMaterialConverter *pMtl, U8 **pData)
{
	Util::C_String sTextureName;
	bool bOk = false;

	if (!IsMaterialParameter())
		return false;

	// work out the source texture filename
	switch (m_Source)
	{
	default:
		assert(0); // shouldn't get here
		return false;
	case E_Source_NamedParam:
		sTextureName = pMtl->GetTextureFilenameFromPlug(m_sNamedParamName);
		break;
	case E_Source_Diffuse:
		sTextureName = pMtl->GetDiffuseTextureFilename();
		break;
	case E_Source_Lightmap:
		sTextureName = pMtl->GetLightmapTextureFilename();
		break;
	case E_Source_Custom:
		sTextureName = m_pCustom->GetTextureFilename(pMtl);
		break;
	}

	// setup texture info
	TextureExport::TextureInfo info;
	TextureExport::InputFile item;

	item.bRequired = m_bWarnMissing;
	item.sName = sTextureName;
	info.m_iDefaultColour = m_iDefaultColour;
	info.m_InputFilenames.PushBack(item);
	info.convertToNormalMap = m_bConvertNormalMap;
	info.invertNormalMapX = m_bInvertNormalMapX;
	info.bSRGBTexture = ((m_pParam->Sampler->iSamplerState>>20)&1)?true:false;

	// load texture and return texture index
	// TODO: replace "Texture" with the filename of the texture (without path).

	// TODO: read texture tags
	RenderingData::Build::Texture t1;
	int textureCacheIndex = pMtl->GetTextureManager()->GetTextureIndex(&info);
	U32 iHash;
	
	if (sTextureName.Length() > 0 && sTextureName.ISearch("null_texture") == -1 && sTextureName.ISearch("ignoreme_") == -1)
	{
		iHash = Btl::Hash(sTextureName.Chars());
		bOk = true;
	}
	else
	{
		// optionally warn about no texture specified
		if (m_bWarnMissing)
		{
			PrintWarning( "%s: No texture specified on material %s for parameter '%s'\n",
				m_sEffectName.Chars(), pMtl->GetMtlName().Chars(), m_pParam->Name.Chars() );
		}
		iHash = info.m_iDefaultColour;
	}

	if (!pData)
		return bOk;

	t1.Initialise("Texture", textureCacheIndex, iHash, pMtl->GetTextureManager());
	//t1.m_RenderState. ... <- set this
	int iTextureIndex;
	
	if (m_bShared)
		iTextureIndex = pMtl->GetSceneExport()->InsertSharedTexture(t1);
	else
		iTextureIndex = pMtl->GetSceneExport()->InsertStreamedTexture(t1, 0);

	*(int*)(*pData) = iTextureIndex;
	(*pData)+=4;

	// if 'AlphaDetect' was specified, detect the alpha on this texture and set that to the material
	// TODO: if texture has greyscale alpha, make it alpha mask... unless a polyflag is set to make it alpha blend
	if (m_bDetectAlpha)
	{
		// get exported texture data and read into ActTools::Texture
		ActTools::Texture texture;
		pMtl->GetTextureManager()->GetTextureData(textureCacheIndex, info, "TEX", texture);
		// TODO: read this from a separate header cache which is on the texture manager and stores stuff like
		//       alpha usage, texture size, etc.

		// check texture and determine alpha type
		// TODO: potentially this can be overridden by the texture tag tool, by polyflags, or by the FX file
		// TODO: determine which texture to check by looking at the FX file annotations
		switch (texture.GetAlphaPresence())
		{
		default:									
			pMtl->SetSortType(RenderingData::SORT_TYPE_KEY); 
			break;
		case ActTools::Texture::ALPHA_BLACKWHITE:	
			pMtl->SetSortType(RenderingData::SORT_TYPE_KEY); 
			pMtl->SetAlphaTestEnable(true, 128);
			break;
		case ActTools::Texture::ALPHA_GREYSCALE:	
			pMtl->SetSortType(RenderingData::SORT_TYPE_BACK_FRONT);
			pMtl->SetAlphaBlendEnable(true);
			break;
		}
	}
	return bOk;
}

};
