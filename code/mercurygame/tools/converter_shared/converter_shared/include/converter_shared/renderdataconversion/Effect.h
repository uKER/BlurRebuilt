#ifndef __CONVERTER_SHARED_EFFECT_H__
#define __CONVERTER_SHARED_EFFECT_H__

#include "converter_shared/renderdataconversion/common.h"
#include "converter_shared/renderdataconversion/AnnotationReaderList.h"

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

/// contains the information the converter needs about a .fx file
/// such as which vertex declaration to use, which parameters are used etc.
class Effect : public AnnotationReaderList
{
public:
	/// constructor
	Effect();
	/// destructor
	~Effect();

	/// initialises an effect by loading the associated .fx file. returns false if the .fx file was not found
	bool Initialise(RenderingData::Build::Scene *pSceneExport, const Util::C_String &sFXBPath, const Util::C_String &sName);

	/// read out data for this effect, given an Aftk Material
	void GetMaterialData(AftkMaterialConverter *pMtl, RenderingData::MaterialType *pMtlDesc, int *piSize, U8 **ppData, E_NodeType type);

	/// get the renderingdata vertex format for this effect
	const RenderingData::VertexFormat *GetVertexFormat(E_NodeType type, int iStream);

	/// get the vertex buffer data for a single Aftk vertex
	void GetVertexData(const AftkMeshConverter *pMesh, int iPoly, int iVertex, E_NodeType nodeType, int iStream, U8 **ppData);

	/// get the .fx #define permutation for the given node + material
	void GetPermutation(EffectPermutationSelector *pEPS);

	/// get name
	Util::C_String GetName();

	/// get the default blend state from the .fx file
	void GetDefaultBlendState(RenderingData::BlendState *pBlendState);

	/// see if we have a sorttype annotation
	void GetSortTypeAnnotation(RenderingData::SortType *pSortType);

	/// returns the value of the annotation with the given name, if found
	const char *GetAnnotation(const char *psz);

	/// returns the value of the annotation with the given name in pbValue
	bool GetAnnotation(const char *psz, bool *pbValue);

	/// returns the value of the annoataion with the given name in piValue
	bool GetAnnotation(const char *psz, int *piValue);

	/// return the index of this effect in the rendering data
	int GetEffectIndex();

	/// returns the number of used vertex streams for the given node type
	int GetNumVertexStreams(E_NodeType nodeType);
	
private:
	/// load the Effect from the .fx file
	void Load(const Util::C_String &sFXBPath);

	/// construct the vertex format
	void SetupVertexFormats();

	/// construct the rendering data effect
	void SetupRenderingDataEffect(RenderingData::Build::Scene *pSceneExport);

	/// initialise all of the annotation readers
	void SetupAnnotationReaders();

	/// dump a vertex declaration element to a string
	Util::C_String DumpVertexDeclElement(const RenderingData::VertexFormat::VertexElement e);

	/// returns false if the parameter with the given index is not used in any of the techniques
	bool IsParameterUsed(int iParam);

	/// return true if the parameter with the given index is referenced in the given bindings vector
	bool IsParameterUsed(Btl::Vector<FX::FunctionBinding> &bindings, int iParam);

	int m_iEffectIndex;

	// one annotation reader for each vtx input semantic
	Btl::Vector<VtxInputAnnotationReader*>	m_VtxAnnotationReaders;

	FX::Effect *m_pFX;

	Util::C_String m_sName;
	RenderingData::VertexFormat m_VertexFormat[E_NodeType_Max][MAX_VERTEX_STREAMS];
	int m_nUsedVertexStreams[E_NodeType_Max];
	RenderingData::Build::Scene *m_pSceneExport;
};

}

#endif

