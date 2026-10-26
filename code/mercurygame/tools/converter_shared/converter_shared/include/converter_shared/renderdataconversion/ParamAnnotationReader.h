#ifndef __PARAMANNOTATIONREADER_H__
#define __PARAMANNOTATIONREADER_H__

#include "converter_shared/renderdataconversion/CustomSemanticType.h"
#include "converter_shared\renderdataconversion\AnnotationReader.h"

namespace FX
{
struct Parameter;
struct Annotation;
}

namespace RenderingData
{
struct MaterialElement;
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

	/// read out data + MaterialElement for this parameter, given an Aftk Material
	/// automatically advances the pData pointer
	/// returns false if this parameter is not supposed to be stored in the renderingdata material
	virtual bool GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **pOutData) = 0;

	/// set the owner effect's name (for printing warning messages)
	void SetFXName(const char *pszFXName);

	/// set the bUsed member variable
	void SetUsed(bool bUsed);

	/// get the bUsed member variable
	bool IsUsed();

	/// identify source semantic
	virtual bool IdentifySemantic(const char *pszSource, RenderDataConversion::E_CustomSemanticType type);

	/// parse annotations
	virtual void ParseAnnotations();

	/// return the parameter that is associated with this annotation reader
	FX::Parameter *GetFXParameter();

protected:
	/// utility function to output an animated material parameter
	/// this function looks at the material element datatype and generates a dynamic material with that datatype on
	void SetupAnimatedParameter(AftkMaterialConverter *pMtl, RenderingData::MaterialElement *pMtlElement, U8 **ppData, const char *pszName);

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
		E_Source_InvalidTexture,
		E_Source_NamedParam,
	};

	// this list of source names matches the enum above
	static const char *m_pszSourceNames[];

	E_Source m_Source;
	Util::C_String m_sNamedParamName;
	Util::C_String m_sEffectName;
	FX::Parameter *m_pParam;	// pointer to the FX parameter that we're reading annotations off
	int m_iOffset;				// for E_Source_LuxCustom
	bool m_bUsed;				// false if the parameter isn't used in any of the techniques
};

}

#endif
