#ifndef __TEXTUREANNOTATIONREADER_H__
#define __TEXTUREANNOTATIONREADER_H__

#include "converter_shared\renderdataconversion\ParamAnnotationReader.h"
#include "texture_export/Rescale.h"

namespace FX
{
struct Parameter;
struct Annotation;
}

namespace RenderDataConversion
{

/// interpret a list of annotations belonging to a texture parameter
class TextureAnnotationReader : public ParamAnnotationReader
{
public:
	/// constructor
	TextureAnnotationReader(FX::Parameter *pParam, const Util::C_String &sEffectName);
	/// destructor
	~TextureAnnotationReader();

	/// read out data + MaterialElement for this parameter, given an Aftk Material
	/// automatically advances the pData pointer
	/// returns false if this parameter is not supposed to be stored in the renderingdata material
	virtual bool GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **pOutData);

	/// work out the source texture filename - this can be overridden by an inheriting class
	virtual Util::C_String GetTextureFilename(AftkMaterialConverter *pMtl);

	/// identify source semantic
	virtual bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type);

	/// add TextureInfo entry into the AftkMaterialConverter for this texture
	void AddStreamerTextureInfo(AftkMaterialConverter *pMtl, int iTextureIndex);

protected:
	
	/// parse the annotation and work out where to source the texture from
	void ParseAnnotation();

	/// parse a single annotation
	virtual bool ParseAnnotation(FX::Annotation *pAnnotation);

	Util::C_String m_sEffectName;		// name of the effect of this annotation, for debugging purposes
	bool m_bDetectAlpha;				// true if this texture is to be used to determine the material's alpha type ('AlphaDetect')
	bool m_bConvertNormalMap;			// true if the texture should be converted from height-map to normal-map
	bool m_bInvertNormalMapX;			// true if the normal-map red channel needs negating. Our external normal maps need this as they have been made to be previewable in Maya.
	bool m_bINR0;						// if INR0 format (neon texture is put into green channel of IPR0)
	bool m_bReduce;						// true if this texture should be reduced in size to half of the third input texture
	U32 m_iDefaultColour;				// if the texture is not found, this colour is used instead
	bool m_bWarnMissing;				// print a warning if the texture is not found and this value is true
	int m_iUVSet;
	int m_nMipMaps;						// number of mipmaps to generate (0 means generate the maximum amount)
	bool m_bEnhanceMipmapAlpha;
	Util::C_String m_sUVSet;
	ActTools::FilterType m_MipFilter;	// filtertype to generate the mipmaps for these textures
	float m_fuScale, m_fvScale, m_fWeight;
	Util::C_String m_suScale, m_svScale, m_sWeight;
	Util::C_String m_sTexelFmtOverride;
	
	// list of default texture colours
	struct DefaultColourItem
	{
		char *sName;
		U32 iColour;
	};
	static const DefaultColourItem m_DefaultColours[];
};

}

#endif
