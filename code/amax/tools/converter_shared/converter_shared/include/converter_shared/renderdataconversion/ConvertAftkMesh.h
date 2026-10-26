#ifndef __CONVERTAFTKMESH_H__
#define __CONVERTAFTKMESH_H__

namespace RenderDataConversion
{

class Effect;

/// convert a polygons of a single Aftk meshnode into to a renderdata primitive
class AftkMeshConverter
{
	friend class VtxInputAnnotationReader;
public:
	/// constructor
	//AftkMeshConverter(const Aftk::C_SceneMeshNode *pMesh);
	/// constructor (for instanced mesh)
	AftkMeshConverter(const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_SceneNode *pInstance = NULL);
	/// destructor
	~AftkMeshConverter();

	/// loads vertex data onto a primitive, given an effect and polygon index
	void GetVertexData(Effect *pEffect, int iPolygon, RenderingData::PrimitiveInfo *pPrim, E_NodeType type) const;

	void SetMatrices();
	void SetMatrices(const Maths::HMat44& localMatrix);
	
	/// get pointer to current meshnode
	const Aftk::C_SceneMeshNode *GetMeshNode() const;

	struct TextureScale {float uScale; float vScale; float weight;};
	void CalculateTextureScale(const Aftk::C_Polygon& aftkPolygon, int set, TextureScale& output);

protected:
	/// get vertex position
	void GetPosition(const Aftk::C_Polygon *pPoly, int iVtx, float *pData, int nComponents = 4) const;

	/// get vertex normal
	void GetNormal(const Aftk::C_Polygon *pPoly, int iVtx, float *pData) const;

	/// get UV
	bool GetUV(const Aftk::C_Polygon *pPoly, int iVtx, int iUVSet, float *pData, int nComponents = 4) const;

	/// get tangent
	bool GetTangent(const Aftk::C_Polygon *pPoly, int iVtx, int iUVSet, float *pData) const;
	
	/// get binormal
	bool GetBinormal(const Aftk::C_Polygon *pPoly, int iVtx, int iUVSet, float *pData) const;

	/// get vertex data channel
	bool GetVertexData(const Aftk::C_Polygon *pPoly, int iVtx, int iChannel, float *pData) const;

	/// get vertex colours
	bool GetVertexColours(const Aftk::C_Polygon *pPoly, int iVtx, float *pData) const;

	/// return the index of the UV set with the given name
	int FindNamedUVSet(const char *pszName, int iPolygon) const;

	/// return the index of the vertex data channel with the given name
	int FindNamedVtxDataChannel(const char *pszName) const;

private:
	void CalculateTangentandBiNormal(const Aftk::C_Polygon& aftkPolygon, int vertex, int set, Aftk::Vector3F32* tangent, Aftk::Vector3F32* binormal);

	/// initialise member variables
	void SetMatrices(const Maths::HMat44& worldMatrix, const Maths::HMat44& localMatrix);

	//RenderingData::Build::Scene *m_pSceneExport;
	const Aftk::C_SceneNode *m_pInstance;
	const Aftk::C_SceneMeshNode *m_pMesh;
	bool m_bCalculatedTangentSpace;

	Aftk::Matrix44 m_PositionMatrix;
	Aftk::Matrix44 m_NormalMatrix;

};

}

#endif
