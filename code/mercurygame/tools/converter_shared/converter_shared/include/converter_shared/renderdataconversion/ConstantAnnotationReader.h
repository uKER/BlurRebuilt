#ifndef __CONSTANTANNOTATIONREADER_H__
#define __CONSTANTANNOTATIONREADER_H__

namespace FX
{
struct Parameter;
}

namespace RenderDataConversion
{

/// interpret a list of annotations belonging to a texture parameter
class ConstantAnnotationReader : public ParamAnnotationReader
{
public:
	/// constructor
	ConstantAnnotationReader(FX::Parameter *pParam, const Util::C_String &sEffectName);
	/// destructor
	~ConstantAnnotationReader();

	/// read out data + MaterialElement for this parameter, given an Aftk Material
	/// automatically advances the pData pointer
	/// returns false if this parameter is not supposed to be stored in the renderingdata material
	virtual bool GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **pOutData);

	/// identify source semantic
	virtual bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type);

private:
	/// get aftk data into an array of 16 floats (TODO: separate this off into a separate class)
	virtual bool GetAftkData(AftkMaterialConverter *pMtl, float *p, Util::C_String *psAnimatedName) { assert(0); return false; };

	/// parse the annotation and work out where to source the texture from
	bool ParseAnnotation(FX::Annotation *pAnnotation);

	Util::C_String m_sEffectName;		// name of the effect of this annotation, for debugging purposes
	int m_iFormat;
	int m_iElementCount;
	float m_fDefault[512];

	// list of supported format specifications
	struct FormatDef
	{
		char *pszName;
		RenderingData::MaterialElementType type;
	};
	static const FormatDef m_pFormatDefs[];
};

}

#endif

