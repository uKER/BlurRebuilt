#ifndef __TEXTUREANNOTATIONREADER_H__
#define __TEXTUREANNOTATIONREADER_H__

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

	/// return a RenderingData::MaterialElement that describes this parameter
	/// returns false if this parameter is not supposed to be stored in the renderingdata material
	virtual bool GetMaterialElement(RenderingData::MaterialElement *pMtlElement);

	/// read out data for this parameter, given an Aftk Material
	/// automatically advances the pData pointer
	virtual bool GetMaterialData(AftkMaterialConverter *pMtl, U8 **pOutData);

	/// returns true if this parameter is supposed to go on the renderingdata material
	bool IsMaterialParameter();

private:
	/// parse the annotation and work out where to source the texture from
	void ParseAnnotation();

	/// parse a single annotation
	virtual bool ParseAnnotation(FX::Annotation *pAnnotation);

	Util::C_String m_sEffectName;		// name of the effect of this annotation, for debugging purposes
	bool m_bDetectAlpha;				// true if this texture is to be used to determine the material's alpha type ('AlphaDetect')
	bool m_bConvertNormalMap;			// true if the texture should be converted from height-map to normal-map
	bool m_bInvertNormalMapX;			// true if the normal-map red channel needs negating. Our external normal maps need this as they have been made to be previewable in Maya.
	U32 m_iDefaultColour;				// if the texture is not found, this colour is used instead
	bool m_bWarnMissing;				// print a warning if the texture is not found and this value is true
	bool m_bShared;						// true if the texture is allowed to be shared by other scenes
	I_CustomTextureSemantic *m_pCustom;

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
