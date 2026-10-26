#ifndef __CUSTOMSEMANTIC_H__
#define __CUSTOMSEMANTIC_H__

/// this file contains code for register parameter plugins and vertex input plugins

namespace FX
{
struct Parameter;
struct Annotation;
struct InputSemantic;
}

namespace RenderDataConversion
{

class Effect;
class AftkMaterialConverter;
class AftkMeshConverter;
class ParamAnnotationReader;
class VtxInputAnnotationReader;

/// interface class for a custom vtx input semantic handler
class I_CustomVtxInputSemantic
{
public:
	virtual ~I_CustomVtxInputSemantic() {}
	virtual bool IdentifySemantic(const char *pszSource) = 0;
	virtual bool GetData(const AftkMeshConverter *pMeshConv, int iPolygon, int iVtx, float *combinedData) = 0;
	virtual bool DoRecentering() { return false; }
};

/// interface for factory for instantiating custom semantic handlers
class I_CustomSemanticFactory
{
public:
	virtual ParamAnnotationReader *Instantiate(FX::Parameter *pParam, const char *pszFXName) const = 0;
};

/// interface for factory for instantiating custom vtx input semantic handlers
class I_CustomVtxInputSemanticFactory
{
public:
	virtual I_CustomVtxInputSemantic *Instantiate(VtxInputAnnotationReader *pParent) const = 0;
};

/// implementation of factory for instantiating custom semantic handlers
template<typename T>
class CustomSemanticFactory : public I_CustomSemanticFactory
{
public:
	virtual ParamAnnotationReader *Instantiate(FX::Parameter *pParam, const char *pszFXName) const
	{
		return new T(pParam, pszFXName);
	}
};

/// implementation of factory for instantiating custom vtx input semantic handlers
template<typename T>
class CustomVtxInputSemanticFactory : public I_CustomVtxInputSemanticFactory
{
public:
	virtual I_CustomVtxInputSemantic *Instantiate(VtxInputAnnotationReader *pParent) const
	{
		return new T(pParent);
	}
};

/// functions for registering a custom input semantic
void RegisterSemantic(const I_CustomSemanticFactory *);
void RegisterSemantic(const I_CustomVtxInputSemanticFactory *);

/// use this function to register a custom parameter semantic
/// usage: RegisterSemantic<MySemantic>();
template<typename T>
void RegisterAnnotationReader()
{
	RegisterSemantic(new CustomSemanticFactory<T>);
}

/// use this function to register a custom vtx semantic
/// usage: RegisterVtxInputAnnotationReader<MySemantic>();
template<typename T>
void RegisterVtxInputAnnotationReader()
{
	RegisterSemantic(new CustomVtxInputSemanticFactory<T>);
}

}

#endif

