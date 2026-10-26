#include "stdafx.h"
#include "converter_shared/CustomSourceSemantics/SkinningProcessor.h"
#include "aftk/Aftk.h"
#include "aftkut/AftkUt.h"
#include "misc_shared/archive/btl.h"
#include <algorithm>

#include <btl/containers/map.h>

namespace SkinningProcessor
{

class C_Weight
{
public:
	C_Weight(){};
	C_Weight(const Aftk::C_SceneNode * pBone, float vertexWeight) : m_pBone(pBone), m_VertexWeight(vertexWeight) {};
	const Aftk::C_SceneNode * m_pBone;
	float m_VertexWeight;

	bool operator<(const C_Weight &b)
	{
		// Sort in order of increasing vertex weights.
		return m_VertexWeight > b.m_VertexWeight;
	}
};

class C_InfluenceVertex
{
public:
	Btl::Vector<C_Weight> m_WeightList;

	void AddWeight(C_Weight weight)
	{
		m_WeightList.PushBack(weight);
	}

	void Normalize(U32 maxVertexBlend)
	{
		if (m_WeightList.Size() > 0)
		{
			//Sort and normalize
			C_Weight* weights = &m_WeightList[0];
			std::sort(weights, weights + m_WeightList.Size());
		}

		//Src size
		S32 srcWeigthSize = m_WeightList.Size();

		//Resize if to many bones, discard the smaller weights
		if (srcWeigthSize > S32(maxVertexBlend))
			//Resize
			m_WeightList.Resize(maxVertexBlend);


		//Normalize
		float weightLenght = 0.f;
		for (S32 i = 0; i < m_WeightList.Size(); i++) weightLenght += m_WeightList[i].m_VertexWeight;
		
		for (S32 i = 0; i < m_WeightList.Size(); i++) m_WeightList[i].m_VertexWeight /= weightLenght;
	}
};

class C_InfluencePolygon
{
public:
	C_InfluencePolygon() {m_MatrixPaletteIndex = -1;};
	C_InfluencePolygon(S32 iTriangle) : m_iTriangle(iTriangle) { m_MatrixPaletteIndex = -1;};
	
	S32 m_iTriangle;
	Btl::Vector<U32> m_VertexList;
	S32 m_MatrixPaletteIndex;
	Btl::Vector<const Aftk::C_SceneNode *> m_PolyBoneVector;

	void AddVertex(U32 vertexIndex)
	{
		m_VertexList.PushBack(vertexIndex);
	}

	void AddInfluenceBone(const Aftk::C_SceneNode *pBone)
	{
		for (S32 i = 0; i < m_PolyBoneVector.Size(); i++)
			if (pBone == m_PolyBoneVector[i])
				return;
		//New bone
		m_PolyBoneVector.PushBack(pBone);
	}
};
class C_SrcInfluenceMaterial
{
public:
	Btl::MultiMap<S32,C_InfluencePolygon> m_PolygonMap;

	void AddPoly(const S32 iTriangle)
	{
		Btl::MultiMap<S32,C_InfluencePolygon>::Iterator it = m_PolygonMap.Find(iTriangle);

		m_PolygonMap.Insert(iTriangle, C_InfluencePolygon(iTriangle));
	}
};

class C_MatrixPalette
{
public:
	Btl::Vector<const Aftk::C_SceneNode *> m_BoneMatrixVector;

	S32 CalculateDistant(const C_InfluencePolygon& poly)
	{
		S32 distant = 0;//abs( m_BoneMatrixVector.Size() - poly.m_PolyBoneVector.Size());
		for (S32 i = 0; i < poly.m_PolyBoneVector.Size(); i++)
		{
			bool found = false;
			//Look for it in the matrix palette
			for (S32 j = 0; j < m_BoneMatrixVector.Size() && !found; j++)
			{
				if (m_BoneMatrixVector[j] == poly.m_PolyBoneVector[i])
					found = true;
			}
			if (!found)
				distant++;
		}

		return distant;
	}

	void AddPoly(const C_InfluencePolygon& poly)
	{
		for (S32 i = 0; i < poly.m_PolyBoneVector.Size(); i++)
		{
			bool found = false;
			//Look for it in the matrix palette
			for (S32 j = 0; j < m_BoneMatrixVector.Size() && !found; j++)
			{
				if (m_BoneMatrixVector[j] == poly.m_PolyBoneVector[i])
					found = true;
			}
			if (!found)
			{
				m_BoneMatrixVector.PushBack(poly.m_PolyBoneVector[i]);
			}
		}
	}
};


class C_SrcInfluenceModel
{
public:
	//Map with the source polygons
	Btl::MultiMap<U32, C_SrcInfluenceMaterial> m_SrcPolygonMap;
	
	//Map with the vertices
	Btl::MultiMap<U32, C_InfluenceVertex> m_InfluenceVertexMap;


	void AddPoly(const S32 iTriangle, const S32 iMaterialIndex )
	{
		Btl::MultiMap<U32, C_SrcInfluenceMaterial>::Iterator it = m_SrcPolygonMap.Find(iMaterialIndex);

		if (it != m_SrcPolygonMap.End())
			it->GetValue().AddPoly(iTriangle);		
		else
		{
			m_SrcPolygonMap.Insert(iMaterialIndex, C_SrcInfluenceMaterial())->GetValue().AddPoly(iTriangle);
		}	
	}
	void AddVertex(U32 iVertexIndex)
	{
		Btl::MultiMap<U32, C_InfluenceVertex>::Iterator it = m_InfluenceVertexMap.Find(iVertexIndex);

		if (it == m_InfluenceVertexMap.End())
		{
			m_InfluenceVertexMap.Insert(iVertexIndex, C_InfluenceVertex());
		}	
	}
	void AddVertexInfluence(U32 iVertexIndex, const Aftk::C_SceneNode * pBone, float weight)
	{
		Btl::MultiMap<U32, C_InfluenceVertex>::Iterator it = m_InfluenceVertexMap.Find(iVertexIndex);

		if (it != m_InfluenceVertexMap.End())
			it->GetValue().AddWeight(C_Weight(pBone, weight));		
		
		//If not, nothing to do
	}

	void Normalize(U32 maxVertexBlend)
	{
		Btl::MultiMap<U32, C_InfluenceVertex>::Iterator it = m_InfluenceVertexMap.Begin();

		while (it != m_InfluenceVertexMap.End())
		{
			it->GetValue().Normalize(maxVertexBlend);
			
			it++;
		}
	}

	void CalculateBoneVertexVector()
	{
		Btl::MultiMap<U32, C_SrcInfluenceMaterial>::Iterator it = m_SrcPolygonMap.Begin();

		while (it != m_SrcPolygonMap.End())
		{
			C_SrcInfluenceMaterial& influenceMaterial = it->GetValue();
			
			Btl::MultiMap<S32, C_InfluencePolygon>::Iterator itPoly = influenceMaterial.m_PolygonMap.Begin();
			while (itPoly != influenceMaterial.m_PolygonMap.End())
			{
				C_InfluencePolygon& poly = itPoly->GetValue();
				
				//Each vertex
				for (S32 j = 0; j < poly.m_VertexList.Size(); j++)
				{
					Btl::MultiMap<U32, C_InfluenceVertex>::Iterator itVertex = m_InfluenceVertexMap.Find(poly.m_VertexList[j]);
					if (itVertex != m_InfluenceVertexMap.End())
					{
						C_InfluenceVertex& vertex = itVertex->GetValue();
						//Get Bone list
						for (S32 k = 0; k < vertex.m_WeightList.Size(); k++)
						{
							poly.AddInfluenceBone(vertex.m_WeightList[k].m_pBone);
						}
					}
				}

				itPoly++;
			}
			it++;
		}
	
	}

	void ProcessMatrixPalette(Btl::Vector<C_MatrixPalette>& matrixPaletteVector, U32 maxMatrixPalettes);
};



class C_SkinningProcessor : public I_SkinningProcessor
{
public:
	C_SkinningProcessor(U32 maxMatrices, U32 maxVertexBlend) : m_MaxMatrices(maxMatrices), m_MaxVertexBlend(maxVertexBlend) {}
	virtual ~C_SkinningProcessor() {}

    // insert a polygon into the scene
    virtual void Insert(const Polygon& polygon, S32 iMaterialIndex);

    // do the skinning processing
    virtual void Process();

	 //Get Num of MatrixPalette
	virtual S32 GetNumMatrixPalette()
	{
		return m_MatrixPaletteVector.Size();
	}
	//Get Num of Bones in a MatrixPalette
	virtual S32 GetMatrixPaletteNumBones(S32 matrixPaletteIndex)
	{
		assert(matrixPaletteIndex<m_MatrixPaletteVector.Size());
		return m_MatrixPaletteVector[matrixPaletteIndex].m_BoneMatrixVector.Size();
	}
	//Get Bone
	virtual S32 GetMatrixPaletteBoneIndex(S32 matrixPaletteIndex, S32 boneIndex)
	{
		assert(matrixPaletteIndex < m_MatrixPaletteVector.Size());
		assert(boneIndex < m_MatrixPaletteVector[matrixPaletteIndex].m_BoneMatrixVector.Size());
		return GetBoneIndex(m_MatrixPaletteVector[matrixPaletteIndex].m_BoneMatrixVector[boneIndex]);
	}
	//Get Bone Name
	virtual const char * GetBoneName(S32 matrixPaletteIndex, S32 boneIndex)
	{
		if (m_JointList.Size() && boneIndex < m_JointList.Size())
		{
			return m_JointList[boneIndex].Chars();
		}
		else
		{
			assert(matrixPaletteIndex < m_MatrixPaletteVector.Size());
			assert(boneIndex < m_MatrixPaletteVector[matrixPaletteIndex].m_BoneMatrixVector.Size());

			return m_MatrixPaletteVector[matrixPaletteIndex].m_BoneMatrixVector[boneIndex]->GetName().c_str();
		}
	}

	//Get Num Bones
	virtual S32 GetNumBones()
	{
		return m_BoneVector.Size();
	};
	//Get Bone from Index
	virtual const Aftk::C_SceneNode * GetBone(S32 boneIndex)
	{
		assert(boneIndex < m_BoneVector.Size());
		return m_BoneVector[boneIndex];
	};
	
	//Get maximum number of bone palette matrices.
	virtual U32 GetMaxMatrices() const
	{
		return m_MaxMatrices;
	}
	
	//Get maximum number of bone influences per vertex.
	virtual U32 GetMaxVertexBlend() const
	{
		return m_MaxVertexBlend;
	}
		
	//Get MatrixPalette from polygon
	virtual S32 GetMatrixPaletteIndex(const Polygon& polygon)
	{
		//Look for model and iTriangle
		Btl::MultiMap<const Aftk::C_SceneMeshNode *, C_SrcInfluenceModel>::Iterator it = m_SrcModelMap.Find(polygon.pNode);
		if (it != m_SrcModelMap.End())
		{
			C_SrcInfluenceModel& influenceModel = it->GetValue();
			Btl::MultiMap<U32, C_SrcInfluenceMaterial>::Iterator itMaterial = influenceModel.m_SrcPolygonMap.Begin();

			while (itMaterial != influenceModel.m_SrcPolygonMap.End())
			{
				C_SrcInfluenceMaterial& influenceMaterial = itMaterial->GetValue();
				//Get Triangle
				Btl::MultiMap<S32, C_InfluencePolygon>::Iterator itPoly = influenceMaterial.m_PolygonMap.Find(polygon.iTriangle);
				if (itPoly != influenceMaterial.m_PolygonMap.End())
				{
					C_InfluencePolygon& poly = itPoly->GetValue();
					return poly.m_MatrixPaletteIndex;
				}

				itMaterial++;
			}
		}
		return -1;
	};

	// Get blend info
	virtual bool GetVertexInfo(const Aftk::C_SceneMeshNode *pNode, U32 iVertexIndex, U32* boneIndexArray, F32* boneWeightArray )
	{
		//Look for model and iTriangle
		Btl::MultiMap<const Aftk::C_SceneMeshNode *, C_SrcInfluenceModel>::Iterator it = m_SrcModelMap.Find(pNode);
		if (it != m_SrcModelMap.End())
		{
			C_SrcInfluenceModel& influenceModel = it->GetValue();
			Btl::MultiMap<U32, C_InfluenceVertex>::Iterator itVertex = influenceModel.m_InfluenceVertexMap.Find(iVertexIndex);

			if (itVertex != influenceModel.m_InfluenceVertexMap.End())
			{
				C_InfluenceVertex& influenceVertex = itVertex->GetValue();
				
				for (U32 i = 0; i < m_MaxVertexBlend; i++)
				{
					if ( i < U32(influenceVertex.m_WeightList.Size()))
					{
						boneWeightArray[i] = influenceVertex.m_WeightList[i].m_VertexWeight;
						boneIndexArray[i] = GetBoneIndex(influenceVertex.m_WeightList[i].m_pBone);
					}
					else
					{
						boneWeightArray[i] = 0.f;
						boneIndexArray[i] = 0;
					}				
				}
			}
		}
		return false;
	};

	virtual bool GetVertexInfo(const Polygon& polygon, U32 vertexPolygonIndex, U32* boneIndexArray, F32* boneWeightArray )
	{
		//Add vertices for every poly
		const Aftk::C_Polygon& atfkPolygon = polygon.pNode->GetPolygon(polygon.iTriangle);
		assert(vertexPolygonIndex <  atfkPolygon.GetNumPolygonVertices());
		U32 vertexIndex = atfkPolygon.GetPolygonVertex(vertexPolygonIndex).GetPositionIndex();
		
		return GetVertexInfo(polygon.pNode, vertexIndex, boneIndexArray, boneWeightArray);
	};

	virtual void PolygonListCallback(SkinningProcessorCallback pCallbackFunc, void* pData)
	{
		//List Polygons
		Btl::MultiMap<const Aftk::C_SceneMeshNode *, C_SrcInfluenceModel>::Iterator it = m_SrcModelMap.Begin();
		while (it != m_SrcModelMap.End())
		{
			C_SrcInfluenceModel& influenceModel = it->GetValue();
			Btl::MultiMap<U32, C_SrcInfluenceMaterial>::Iterator itMaterial = influenceModel.m_SrcPolygonMap.Begin();

			while (itMaterial != influenceModel.m_SrcPolygonMap.End())
			{
				C_SrcInfluenceMaterial& influenceMaterial = itMaterial->GetValue();
				//Get Triangle
				Btl::MultiMap<S32, C_InfluencePolygon>::Iterator itPoly = influenceMaterial.m_PolygonMap.Begin();
				while (itPoly != influenceMaterial.m_PolygonMap.End())
				{
					//Callback
					pCallbackFunc(Polygon(it->GetKey(), itPoly->GetKey()), itMaterial->GetKey(), pData);

					itPoly++;
				}

				itMaterial++;
			}
			it++;
		}
	}

	virtual void CreateJointList(const Btl::Vector<const Aftk::C_SceneNode*>& joints)
	{
		m_JointList.Clear();

		if (joints.Size() <= 0 || joints.Size() > static_cast<S32>(m_MaxMatrices))
			return;

		for (S32 i = 0, count = joints.Size(); i < count; ++i)
			m_JointList.PushBack(joints[i]->GetName().c_str());
	}

	virtual S32 GetNumHardMappedBones() const
	{
		return m_JointList.Size();
	}
protected:

	U32 m_MaxMatrices;
	U32 m_MaxVertexBlend;

	S32 AddBone(const Aftk::C_SceneNode * pBone)
	{
		for (S32 i = 0; i < m_BoneVector.Size(); i++)
			if (pBone == m_BoneVector[i])
				return i;

		m_BoneVector.PushBack(pBone);
		return m_BoneVector.Size() - 1;
	}

	S32 GetBoneIndex(const Aftk::C_SceneNode * pBone)
	{
		if (pBone)
		{
			for (S32 i = 0, count = m_JointList.Size(); i < count; ++i)
			{
				if (_stricmp(pBone->GetName().c_str(), m_JointList[i].Chars()) == 0)
					return i;
			}

			for (S32 i = 0; i < m_BoneVector.Size(); i++)
			{
				if (pBone == m_BoneVector[i])
					return i;
			}
		}

		return -1;
	}

	//Map with the source polygons
	Btl::MultiMap<const Aftk::C_SceneMeshNode *, C_SrcInfluenceModel> m_SrcModelMap;

	//Vector of Matrix palette
	Btl::Vector<C_MatrixPalette> m_MatrixPaletteVector;

	//Vector of Bones, Index access
	Btl::Vector<const Aftk::C_SceneNode *> m_BoneVector;

	/*Btl::Map<Btl::String, S32> m_JointNameMap;
	Btl::Map<Btl::String*/

	Btl::Vector<Btl::String>	m_JointList;
};

void C_SkinningProcessor::Insert(const Polygon& polygon, S32 iMaterialIndex)
{
	assert(polygon.pNode->GetNumInfluenceClusters() > 0);

	Btl::MultiMap<const Aftk::C_SceneMeshNode *, C_SrcInfluenceModel>::Iterator it = m_SrcModelMap.Find(polygon.pNode);

	if (it != m_SrcModelMap.End())
		it->GetValue().AddPoly(polygon.iTriangle, iMaterialIndex);		
	else
	{
		m_SrcModelMap.Insert(polygon.pNode, C_SrcInfluenceModel())->GetValue().AddPoly(polygon.iTriangle, iMaterialIndex);
	}
}

void C_SkinningProcessor::Process()
{

	//Calculate weight distribution
	Btl::MultiMap<const Aftk::C_SceneMeshNode *, C_SrcInfluenceModel>::Iterator itModel  = m_SrcModelMap.Begin();
	while (itModel != m_SrcModelMap.End())
	{
		//For each model
		const Aftk::C_SceneMeshNode *pModel = itModel->GetKey();
		C_SrcInfluenceModel& influenceModel = itModel->GetValue();
		
		Btl::MultiMap<U32, C_SrcInfluenceMaterial>::Iterator itMaterial = influenceModel.m_SrcPolygonMap.Begin();
		while (itMaterial != influenceModel.m_SrcPolygonMap.End())
		{
			C_SrcInfluenceMaterial& influenceMaterial = itMaterial->GetValue();
			//For each material
			S32 iMaterialIndex = itMaterial->GetKey();
			Btl::MultiMap<S32, C_InfluencePolygon>::Iterator itPoly = influenceMaterial.m_PolygonMap.Begin();
			while (itPoly != influenceMaterial.m_PolygonMap.End())
			{
				C_InfluencePolygon& influencePoly = itPoly->GetValue();

				//Add vertices for every poly
				const Aftk::C_Polygon& polygon = pModel->GetPolygon(influencePoly.m_iTriangle);
				U32 noPolyVertices = polygon.GetNumPolygonVertices();
				for(U32 polyTriNo = 0; polyTriNo < noPolyVertices; polyTriNo++)
				{
					U32 vertexIndex = polygon.GetPolygonVertex(polyTriNo).GetPositionIndex();
					influencePoly.AddVertex(vertexIndex);
					influenceModel.AddVertex(vertexIndex);
				}

				itPoly++;
			}

			itMaterial++;
		}
		//Apply weights to the list	
		for (U32 i = 0; i < pModel->GetNumInfluenceClusters(); i++)
		{
			const Aftk::C_InfluenceClusterNode* pAftkInfluenceCluster = pModel->GetInfluenceClusterPtr(i);	

			const S32 noEnvelopes = pAftkInfluenceCluster->GetNumEnvelopes();
			assert(noEnvelopes > 0);
			for(S32 j=0; j<noEnvelopes; j++)
			{
				const Aftk::C_WeightEnvelope* pEnvelope = &pAftkInfluenceCluster->GetEnvelope(j);	

				if (pEnvelope->GetInfluenceNodePtr())
				{
					//Create Bone Vector
					AddBone(pEnvelope->GetInfluenceNodePtr());

					for (U32 k = 0; k < pEnvelope->GetNumWeights(); k++)
					{
						Aftk::C_Weight<F32> weight;
						pEnvelope->GetWeight(k, weight);

						influenceModel.AddVertexInfluence(weight.GetIndex(), pEnvelope->GetInfluenceNodePtr(), weight.GetWeight());
					}
				}
			}
	
		}
		
		assert(m_BoneVector.Size() > 0);

		//Normalize vertex weights
		influenceModel.Normalize(m_MaxVertexBlend);

		//Calculate Bones per Poly
		influenceModel.CalculateBoneVertexVector();

		//Create matrix palettes
		influenceModel.ProcessMatrixPalette(m_MatrixPaletteVector, m_MaxMatrices);

		itModel++;
	}
}

//////////////////////////////////////////////////////////////////////////
// C_SrcInfluenceModel
//////////////////////////////////////////////////////////////////////////
void C_SrcInfluenceModel::ProcessMatrixPalette(Btl::Vector<C_MatrixPalette>& matrixPaletteVector,  U32 maxMatrixPalettes )
{
	
	//For each material
	Btl::MultiMap<U32, C_SrcInfluenceMaterial>::Iterator itMaterial = m_SrcPolygonMap.Begin();
	while (itMaterial != m_SrcPolygonMap.End())
	{
		C_SrcInfluenceMaterial& influenceMaterial = itMaterial->GetValue();
		
		Btl::List<S32> polygonItemList;

		//Add polys to process
		Btl::MultiMap<S32, C_InfluencePolygon>::ConstIterator itPoly = influenceMaterial.m_PolygonMap.Begin();
		while (itPoly != influenceMaterial.m_PolygonMap.End())
		{
			const C_InfluencePolygon& influencePoly = itPoly->GetValue();	
			polygonItemList.PushBack(influencePoly.m_iTriangle);
			itPoly++;
		}
		
		//Create a first palette with the first poly
		C_MatrixPalette matrixPalette;
		
		while (!polygonItemList.Empty())
		{
			//Calculate the poly with less distant
			S32 lessDistant = 0xFFFFFF;
			Btl::List<S32>::Iterator nextItemIt;
			Btl::List<S32>::Iterator it = polygonItemList.Begin();
			while (it != polygonItemList.End())
			{
				Btl::MultiMap<S32, C_InfluencePolygon>::Iterator itPoly = influenceMaterial.m_PolygonMap.Find(*it);
				assert(itPoly != influenceMaterial.m_PolygonMap.End());

				S32 distant = matrixPalette.CalculateDistant(itPoly->GetValue());
				if (distant < lessDistant )
				{
					nextItemIt = it;
					lessDistant = distant;
				}
				++it;
			}

			if ((matrixPalette.m_BoneMatrixVector.Size() + lessDistant ) < S32(maxMatrixPalettes) )
			{
				//Add to Matrix palette and remove to the list
				Btl::MultiMap<S32, C_InfluencePolygon>::Iterator itPoly = influenceMaterial.m_PolygonMap.Find(*nextItemIt);
				assert(itPoly != influenceMaterial.m_PolygonMap.End());

				//Add to matrix palette
				C_InfluencePolygon& influencePoly = itPoly->GetValue();
				influencePoly.m_MatrixPaletteIndex = matrixPaletteVector.Size();
				matrixPalette.AddPoly(influencePoly);
			
				//Remove from the list
				polygonItemList.Erase(nextItemIt);
			}
			else
			{
				//Need a new Matrix Palette
				matrixPaletteVector.PushBack(matrixPalette);
				matrixPalette.m_BoneMatrixVector.Clear();
			}
		}
		
		//Add MatrixPalette
		matrixPaletteVector.PushBack(matrixPalette);
		
		//Next material
		itMaterial++;
	}
}

I_SkinningProcessor* CreateSkinningProcessor(U32 maxMatrices, U32 maxVertexBlend)
{
	return new C_SkinningProcessor(maxMatrices, maxVertexBlend);
}
void DestroySkinningProcessor(I_SkinningProcessor* pScene)
{
	delete pScene;
}

}
