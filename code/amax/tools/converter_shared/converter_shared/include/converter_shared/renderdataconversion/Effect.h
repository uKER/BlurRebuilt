#ifndef __CONVERTER_SHARED_EFFECT_H__
#define __CONVERTER_SHARED_EFFECT_H__

#include "converter_shared/renderdataconversion/common.h"

namespace FX
{
class Effect;
struct Parameter;
}

namespace Aftk
{
class C_Polygon;
};

namespace RenderDataConversion
{

class ParamAnnotationReader;
class AftkMaterialConverter;
class AftkMeshConverter;
class EffectPermutationSelector;
class VtxInputAnnotationReader;

/// contains the information the converter needs about a .fx file
/// such as which vertex declaration to use, which parameters are used etc.
class Effect
{
public:
	/// constructor
	Effect();
	/// destructor
	~Effect();

	/// initialises an effect by loading the associated .fx file
	void Initialise(RenderingData::Build::Scene *pSceneExport, const Util::C_String &sFXBPath, const Util::C_String &sName);

	/// returns the index of the material type associated with this effect
	int GetMaterialTypeIndex(E_NodeType type);

	/// returns the size (in bytes) of the material data
	int GetMaterialDataSize(E_NodeType type);

	/// read out data for this effect, given an Aftk Material
	void GetMaterialData(AftkMaterialConverter *pMtl, U8 **ppData, E_NodeType type);

	/// get the renderingdata vertex format for this effect
	const RenderingData::VertexFormat *GetVertexFormat(E_NodeType type);

	/// get the vertex buffer data for a single Aftk vertex
	void GetVertexData(const AftkMeshConverter *pMesh, int iPoly, int iVertex, E_NodeType nodeType, U8 **ppData);

	/// get the .fx #define permutation for the given node + material
	void GetPermutation(EffectPermutationSelector *pEPS);

	/// get name
	Util::C_String GetName();

private:
	/// load the Effect from the .fx file
	void Load(const Util::C_String &sFXBPath);

	/// construct the rendering data material type
	void SetupMaterialType(RenderingData::Build::Scene *pSceneExport);

	/// construct the vertex format
	void SetupVertexFormats();

	/// construct the rendering data effect
	void SetupRenderingDataEffect(RenderingData::Build::Scene *pSceneExport);

	/// returns the annotation reader for a particular parameter type
	ParamAnnotationReader *GetAnnotationReaderForParameter(FX::Parameter *pParam);

	/// initialise all of the annotation readers
	void SetupAnnotationReaders();

	int m_iMaterialTypeIndex[E_NodeType_Max];
	int m_iEffectIndex;
	int m_iMaterialDataSize[E_NodeType_Max];

	// one annotation reader for each parameter
	Btl::Vector<ParamAnnotationReader*>		m_ParamAnnotationReaders;
	// one annotation reader for each vtx input semantic
	Btl::Vector<VtxInputAnnotationReader*>	m_VtxAnnotationReaders;

	FX::Effect *m_pFX;

	Util::C_String m_sName;
	RenderingData::VertexFormat m_VertexFormat[E_NodeType_Max];
};

}

#endif

