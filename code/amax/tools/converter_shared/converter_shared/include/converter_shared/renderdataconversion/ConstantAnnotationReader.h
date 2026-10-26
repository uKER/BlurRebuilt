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

	Util::C_String m_sEffectName;		// name of the effect of this annotation, for debugging purposes
	int m_iFormat;
	int m_iElementCount;
	float m_fDefault[16];

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

