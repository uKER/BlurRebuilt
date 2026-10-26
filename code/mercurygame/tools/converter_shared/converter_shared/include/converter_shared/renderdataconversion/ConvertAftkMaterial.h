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
class Effect;
struct Material;

/// generate renderdata material from a single Aftk material or an Aftk Node
class AftkMaterialConverter
{
public:
	/// constructor
	AftkMaterialConverter(MaterialManager *pMgr, const Aftk::C_MaterialNode *pMtl, const Aftk::C_SceneNode *pNode, int iCustomKey, E_NodeType type);

	/// constructor (for exporting material data from an Aftk Node, e.g. UV offset, SH lighting etc.)
	AftkMaterialConverter(MaterialManager *pMgr, const Aftk::C_SceneNode *pNode, bool bInstanceResetMtl);

	/// destructor
	~AftkMaterialConverter();

	/// specify the permutated .fx name to use
	void SetPermutatedFXName(const Util::C_String &sName);

	/// creates a RenderingData::Material from an Aftk Material
	/// fills out the pMtlOut structure with the converted data
	void Convert(Material *pMtlOut, const Aftk::C_SceneNode *pMesh, bool bSeparateStreams = false);

	/// return the filename of the texture on the material with the given plug name
	Util::C_String GetTextureFilenameFromPlug(const char* plugName, char* pUVSetName = NULL);

	/// return the filename of the diffuse texture
	Util::C_String GetDiffuseTextureFilename();

	/// return the filename of the lightmap texture
	Util::C_String GetLightmapTextureFilename();

	/// returns whether a texture exists on disk or false if the "exclude" texture tagger option is set
	bool TextureFileExists(const Util::C_String& filename);

	/// return the name of the .fx file that is associated with this material
	Util::C_String GetFXName();

	/// return lux-custom data (used for SH lighting etc.)
	void GetLuxCustomData(float *pOutValue, int iOffset);

	/// return the value of a paremeter plug
	int GetFloat4FromPlug(const char *plugName, float *pOutValue, bool *pbAnimated);

	/// return the node's world-matrix
	void GetWorldMatrix(float *pOutValue);

	/// return the node's world bounding box
	Maths::C_AxisBox GetWorldBoundingBox();

	/// return the custom key passed in by RenderDataConverter.cpp
	int GetCustomKey();

	/// add TextureInfo entry - this contains a list of texture indices and associated UV sets etc. for the streamer
	void AddTextureInfo(S32 textureIndex, const char *pszUVSetName, int uvSet, const char *suScale, float uScale, const char *svScale, float vScale, const char *sWeight, float weight);

	/// checks whether the given material parameter is exposed, and if so returns the name for the exposed parameter name
	Util::C_String GetExposedParameterName(const char *pszParameter);

	/// get the stream we are exporting for
	int GetStream();

	/// set the sort-type for this material - called by the TextureAnnotationReader class
	void SetSortType(int type); //RenderingData::SortType
	void SetAlphaTestEnable(bool b, U8 ref);
	void SetAlphaBlendEnable(bool b);
	void SetAlphaToMaskEnable(bool b);
	void SetDecalOrder( const U8 decalOrder );

	bool GetAlphaBlendEnable() const;

	/// returns the name of the Aftk material that we're converting
	Util::C_String GetMtlName();

	const Aftk::C_SceneNode* GetSceneNode() const {return m_pNode;}

	E_NodeType GetNodeType() { return m_NodeType; }

public:
	RenderingData::Build::Scene *GetSceneExport();
	TextureExport::TextureManager *GetTextureManager();
	EffectManager *GetEffectManager();

private:
	/// internal function which converts a material for a single stream
	bool Convert(Effect *pEffect, Material *pOut, const Aftk::C_SceneNode *pMesh, int *piMtlId);

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
	int m_iCustomFlags;
	int m_iStream;			// index of the stream that is currently being converted for. -1 means ignore streams
	U8 m_DecalOrder;
	E_NodeType m_NodeType;
	int m_iCustomKey;
	Btl::Vector<RenderingData::PrimitiveInfo::TextureInfo> m_TextureInfo;
	Util::C_String m_sPermutatedFXName;
};

}

#endif
