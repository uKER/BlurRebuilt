#ifndef __CONVERTAFTKMATERIAL_H__
#define __CONVERTAFTKMATERIAL_H__

#include "converter_shared/renderdataconversion/common.h"

// forward declarations
namespace Aftk
{
class C_MaterialNode;
class C_SceneNode;
class C_ShaderNode;
}
namespace RenderingData
{
namespace Build
{
class Scene;
}
}
namespace TextureExport
{
class TextureManager;
}

namespace RenderDataConversion
{

class EffectManager;
class MaterialManager;
struct Material;

/// generate renderdata material from a single Aftk material or an Aftk Node
class AftkMaterialConverter
{
public:
	/// constructor
	AftkMaterialConverter(MaterialManager *pMgr, const Aftk::C_MaterialNode *pMtl, E_NodeType type);

	/// constructor (for exporting material data from an Aftk Node, e.g. UV offset, SH lighting etc.)
	AftkMaterialConverter(MaterialManager *pMgr, const Aftk::C_SceneNode *pNode);

	/// destructor
	~AftkMaterialConverter();

	/// specify the permutated .fx name to use
	void SetPermutatedFXName(const Util::C_String &sName);

	/// creates a RenderingData::Material from an Aftk Material
	/// fills out the pMtlOut structure with the converted data
	void Convert(Material *pMtlOut);

	/// return the filename of the texture on the material with the given plug name
	Util::C_String GetTextureFilenameFromPlug(const char* plugName);

	/// return the filename of the diffuse texture
	Util::C_String GetDiffuseTextureFilename();

	/// return the filename of the lightmap texture
	Util::C_String GetLightmapTextureFilename();

	/// return the name of the .fx file that is associated with this material
	Util::C_String GetFXName();

	/// return lux-custom data (used for SH lighting etc.)
	void GetLuxCustomData(float *pOutValue, int iOffset);

	/// return the value of a paremeter plug
	void GetFloat4FromPlug(const char *plugName, float *pOutValue);

	/// return the node's world-matrix
	void GetWorldMatrix(float *pOutValue);

	/// set the sort-type for this material - called by the TextureAnnotationReader class
	void SetSortType(int type); //RenderingData::SortType
	void SetAlphaTestEnable(bool b, U8 ref);
	void SetAlphaBlendEnable(bool b);

	/// returns the name of the Aftk material that we're converting
	Util::C_String GetMtlName();

public:
	RenderingData::Build::Scene *GetSceneExport();
	TextureExport::TextureManager *GetTextureManager();
	EffectManager *GetEffectManager();

private:
	/// setup some shared variables to be used by the conversion process
	void Setup();

	const Aftk::C_MaterialNode *m_pMtl;	// the Aftk Material node we're processing
	const Aftk::C_SceneNode *m_pNode;	// the Aftk Scene node we're processing
	MaterialManager *m_pManager;

	const Aftk::C_ShaderNode* m_pAftkShaderNode;
	const Aftk::C_ShaderNode* m_pAftkLightingNode;

	enum E_MaterialSourceType
	{
		E_MaterialSourceType_Maya,
		E_MaterialSourceType_XSI,
	};

	E_MaterialSourceType m_materialSourceType;
	int m_iSortType;
	int m_blendState;
	E_NodeType m_NodeType;
	Util::C_String m_sPermutatedFXName;
};

}

#endif
