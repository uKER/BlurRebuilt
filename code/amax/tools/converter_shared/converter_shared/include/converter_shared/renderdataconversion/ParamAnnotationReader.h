#ifndef __PARAMANNOTATIONREADER_H__
#define __PARAMANNOTATIONREADER_H__

namespace FX
{
struct Parameter;
struct Annotation;
}

namespace RenderDataConversion
{

class AftkMaterialConverter;

/// interpret a list of annotations belonging to an FX parameter
class ParamAnnotationReader : public AnnotationReader
{
public:
	/// constructor
	ParamAnnotationReader(FX::Parameter *p, const Util::C_String &sEffectName);
	/// destructor
	virtual ~ParamAnnotationReader() {}

	/// return a RenderingData::MaterialElement that describes this parameter
	/// returns false if this parameter is not supposed to be stored in the renderingdata material
	virtual bool GetMaterialElement(RenderingData::MaterialElement *pMtlElement) = 0;

	/// read out data for this parameter, given an Aftk Material
	/// automatically advances the pData pointer
	virtual bool GetMaterialData(AftkMaterialConverter *pMtl, U8 **pData) = 0;

protected:
	/// returns true if this parameter should go on the material, false if it's set from the game
	virtual bool IsMaterialParameter() = 0;

	/// interpret an annotation
	virtual bool ParseAnnotation(FX::Annotation *pAnnotation);

	/// interpret a 'Source' annotation and set variables accordingly
	void InterpretSourceAnnotation(FX::Annotation *pAnnotation);

	enum E_Source
	{
		E_Source_None,
		E_Source_Diffuse,
		E_Source_Lightmap,
		E_Source_WorldMatrix,
		E_Source_LuxCustom,
		E_Source_NamedParam,
		E_Source_Custom,
	};

	// this list of source names matches the enum above
	static const char *m_pszSourceNames[];

	E_Source m_Source;
	Util::C_String m_sNamedParamName;
	Util::C_String m_sEffectName;
	FX::Parameter *m_pParam;	// pointer to the FX parameter that we're reading annotations off
	int m_iOffset;				// for E_Source_LuxCustom
};

}

#endif
