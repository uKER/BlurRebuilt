#ifndef __ANNOTATIONREADERLIST_H__
#define __ANNOTATIONREADERLIST_H__

namespace FX
{
class Effect;
struct Parameter;
struct FunctionBinding;
}

namespace Aftk
{
class C_Polygon;
}

namespace RenderingData
{
struct MaterialElement;
class BlendState;
}

namespace RenderDataConversion
{

class ParamAnnotationReader;
class AftkMaterialConverter;
class AftkMeshConverter;
class EffectPermutationSelector;
class VtxInputAnnotationReader;
class Effect;

/// a list of annotation readers and some utility functions related to the list
class AnnotationReaderList
{
public:
	/// read out data for this effect, given an Aftk Material
	void GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialType *pMaterialType, int *piSize, U8 **ppData, E_NodeType type);

	/// add all annotation readers from a given effect to the current effect, checks for duplicates by name
	void AddAnnotationReadersFromEffect(Effect *pEffect, E_NodeType type);

protected:
	/// add an annotation reader (checks for duplicates by name)
	void AddAnnotationReader(ParamAnnotationReader *pA);

	// one annotation reader for each parameter
	Btl::Vector<ParamAnnotationReader*>		m_ParamAnnotationReaders;
};

}

#endif
