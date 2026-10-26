#ifndef __CUSTOMSEMANTIC_H__
#define __CUSTOMSEMANTIC_H__

#include "converter_shared/renderdataconversion/CustomSemanticType.h"

namespace FX
{
struct Parameter;
struct Annotation;
}

namespace RenderDataConversion
{

class AftkMaterialConverter;
class TextureAnnotationReader;

class I_ParseAnnotation
{
public:
	virtual bool ParseAnnotation(FX::Annotation *pAnnotation) = 0;
};

/// virtual interface for an input semantic
class I_CustomSemantic
{
public:
	virtual bool IdentifySemantic(const char *pszSource, E_CustomSemanticType type, FX::Parameter *pParam) = 0;

	I_ParseAnnotation *m_pParent;
	/// utility functions for setting an annotation on the parent
	void SetAnnotation(const char *psz, const char *pszValue);
	void SetAnnotation(const char *psz, bool bValue);
};

/// virtual interface for a texture input semantic
class I_CustomTextureSemantic : public I_CustomSemantic
{
public:
	virtual Util::C_String GetTextureFilename(AftkMaterialConverter *pMtl) = 0;
};

/// virtual interface for a custom shader constant input semantic
class I_CustomConstantSemantic : public I_CustomSemantic
{
public:
	virtual Util::C_String GetConstantData(AftkMaterialConverter *pMtl, float *pRtnData) = 0;
};

/// interface for factory for instantiating custom semantic handlers
class I_CustomSemanticFactory
{
public:
	virtual I_CustomSemantic *Instantiate() const = 0;
};

/// implementation of factory for instantiating custom semantic handlers
template<typename T>
class CustomSemanticFactory : public I_CustomSemanticFactory
{
public:
	virtual I_CustomSemantic *Instantiate() const
	{
		return new T;
	}
};

/// functions for registering a custom input semantic
void RegisterSemantic(const I_CustomSemanticFactory *);

/// use this function to register a custom semantic
/// usage: RegisterSemantic<MySemantic>();
template<typename T>
void RegisterSemantic()
{
	RegisterSemantic(new CustomSemanticFactory<T>);
}

}

#endif

