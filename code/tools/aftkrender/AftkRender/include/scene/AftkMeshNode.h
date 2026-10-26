#ifndef AFTK_RENDER_SCENE_MESH_NODE_H
#define AFTK_RENDER_SCENE_MESH_NODE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{

	//Classes to generate GPU vertex/index buffers directly from Aftk data,
	//therefore bypassing AftkRenders own data structures

	class C_AftkMeshNode;

	class C_AftkMeshCluster : public C_GeometryCluster
	{
	public:
		struct Flags
		{
			unsigned isStatic:1;
			unsigned isSkinned:1;
			unsigned isSkinAnimated:1;
		};

	public:
		C_AftkMeshCluster(C_AftkMeshNode* pNode, U8 lod);
		~C_AftkMeshCluster();

		void Clear();
		bool Build();

		U8 GetLodLevel() const;
		const Flags& GetFlags() const;

	private:
		bool CreateVertexBuffers();
		bool PopulateVertexBuffers();

		bool CreateIndexBuffers();
		bool PopulateIndexBuffers();

		//map aftk polygon vertex indices to vertex buffer indices
		Ut::UtVector< Ut::DynamicVector<U32> >	m_PolygonVertexIndexMap;

		//map aftk polygon indices to batches
		Ut::UtVector< Ut::DynamicVector<U32> >	m_PolygonIndexBatchMap;


		Flags	m_Flags;
		U8		m_Lod;
	};



	class C_AftkMeshNode : public C_GeometryNode
	{
	public:
		C_AftkMeshNode(Aftk::C_SceneMeshNode *pNode);
		~C_AftkMeshNode();

		void Clear();
		bool Build();

		Aftk::C_SceneMeshNode* GetAftkSceneMeshNodePtr();
		const Aftk::C_SceneMeshNode* GetAftkSceneMeshNodePtr() const;

	private:
		bool CreateGeometryClusters();

		Aftk::C_SceneMeshNode		*m_pAftkSceneMeshNode;
	};



	//Helper functions for populating vertex buffers from aftk mesh data

	template <typename T>
	static void SetVertexPosition(
		T* vertex,
		const Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		const Aftk::C_PolygonVertex *pAftkPolygonVertex,
		bool isSkinned,
		U32 polygonVertexIndex)
	{
		Aftk::Position3 vertexPosition;
		if (isSkinned)
			vertexPosition = pAftkSceneMeshNode->GetEvaluatedPolygonVertexPosition(polygonVertexIndex);
		else
			vertexPosition = pAftkSceneMeshNode->GetVertexPosition(pAftkPolygonVertex->GetPositionIndex());

		vertex->x0 = vertexPosition.X();
		vertex->y0 = vertexPosition.Y();
		vertex->z0 = vertexPosition.Z();
	}

	template <typename T>
	static void SetVertexNormal(
		T* vertex,
		const Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		const Aftk::C_PolygonVertex *pAftkPolygonVertex,
		bool isSkinned,
		U32 polygonVertexIndex)
	{
		Aftk::Normal3 vertexNormal;
		if (isSkinned)
			vertexNormal = pAftkSceneMeshNode->GetEvaluatedPolygonVertexNormal(polygonVertexIndex);
		else
		{
			if (pAftkPolygonVertex->GetNormalIndex() != -1)
				vertexNormal = pAftkSceneMeshNode->GetVertexNormal(pAftkPolygonVertex->GetNormalIndex());
			else
			{
				vertexNormal.x = 1.0f;
				vertexNormal.y = vertexNormal.z = 0.0f;
			}
		}
		vertex->x1 = vertexNormal.X();
		vertex->y1 = vertexNormal.Y();
		vertex->z1 = vertexNormal.Z();
	}

	template <typename T>
	static void SetVertexColour(
		T* vertex,
		S32 vertexColourChannel,
		const Aftk::E_GenericAttributeType vertexColourType,
		const Aftk::C_PolygonVertex *pAftkPolygonVertex)
	{
		if (vertexColourChannel != -1)
		{
			switch (vertexColourType)
			{
			case Aftk::E_GenericAttributeType_Vector4F32:
			{
				Aftk::Vector4F32 vertexColour;
				pAftkPolygonVertex->GetData(vertexColour, vertexColourChannel);
				vertex->r = (U8)(vertexColour.X()*255.0f);
				vertex->g = (U8)(vertexColour.Y()*255.0f);
				vertex->b = (U8)(vertexColour.Z()*255.0f);
				vertex->a = (U8)(vertexColour.W()*255.0f);
			}
			break;
			case Aftk::E_GenericAttributeType_U32:
			{
				U32 vertexColourInt;
				Aftk::Vector4F32 vertexColour;

				pAftkPolygonVertex->GetData(vertexColourInt, vertexColourChannel);
				vertexColour = Aftk::ColourRGBAFromU32(vertexColourInt);

				vertex->r = (U8)(vertexColour.X()*255.0f);
				vertex->g = (U8)(vertexColour.Y()*255.0f);
				vertex->b = (U8)(vertexColour.Z()*255.0f);
				vertex->a = (U8)(vertexColour.W()*255.0f);
			}
			default:
			{
				vertex->r = 255;
				vertex->g = 255;
				vertex->b = 255;
				vertex->a = 255;
			}
			break;
			}
		}
		else
		{
			vertex->r = 255;
			vertex->g = 255;
			vertex->b = 255;
			vertex->a = 255;
		}
	}

	template <typename T>
	static void SetVertexTextureCoordinates(
		T* vertex,
		U32 uvSet,
		const  Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		const Aftk::C_PolygonVertex *pAftkPolygonVertex)
	{
		U32 numUVSets = pAftkSceneMeshNode->GetNumTextureCoordinateSets();

		S32 uvIndex = pAftkPolygonVertex->GetTextureCoordinateIndex(uvSet);
		if (uvIndex < 0)
		{
			vertex->u = 0.0f;
			vertex->v = 0.0f;
		}
		else
		{
			const Aftk::TextureCoordinate4* pAftkUv =
				&pAftkSceneMeshNode->GetTextureCoordinate(uvSet, uvIndex);

			vertex->u = pAftkUv->X();
			vertex->v = pAftkUv->Y();
		}
	}

	template <typename T>
	static void SetVertexWeightsAndIndices(
		T* vertex,
		const Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		const Aftk::C_PolygonVertex *pAftkPolygonVertex)
	{
		U32 vertexPosIndex = pAftkPolygonVertex->GetPositionIndex();
		const Aftk::C_VertexWeight* pAftkWeight = &pAftkSceneMeshNode->GetVertexWeight(vertexPosIndex);

		// max number of influences = 4 in Aftk::C_VertexWeight
		for (U32 iInf=0; iInf<pAftkWeight->GetNumWeights(); ++iInf)
		{
			vertex->w[iInf] = pAftkWeight->GetWeight(iInf);
			vertex->idx[iInf] = pAftkWeight->GetInfluenceNodeIndex(iInf);
		}
		for (U32 iInf=pAftkWeight->GetNumWeights(); iInf<4; ++iInf)
		{
			vertex->w[iInf] = 0;
			vertex->idx[iInf] = 0;
		}
	}

	// Currently only uses the vectors for the 1st texture coordinate set.
	template <typename T>
	static void SetVertexTangentAndBinormal(
		T* vertex,
		const Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		const Aftk::C_PolygonVertex *pAftkPolygonVertex,
		bool isSkinned,
		U32 polygonVertexIndex)
	{
		
		Aftk::Vector3F32 vertexTangent, vertexBinormal;
		if (isSkinned)
		{
			vertexTangent = pAftkSceneMeshNode->GetEvaluatedPolygonVertexTangent(0, polygonVertexIndex);
			vertexBinormal = pAftkSceneMeshNode->GetEvaluatedPolygonVertexBinormal(0, polygonVertexIndex);
		}
		else
		{
			if (pAftkPolygonVertex->GetTangentIndex(0) != -1)
				vertexTangent = pAftkSceneMeshNode->GetVertexTangent(0, pAftkPolygonVertex->GetTangentIndex(0));
			else
			{
				vertexTangent.x = 1.0f;
				vertexTangent.y = vertexTangent.z = 0.0f;
			}

			if (pAftkPolygonVertex->GetBinormalIndex(0) != -1)
				vertexBinormal = pAftkSceneMeshNode->GetVertexBinormal(0, pAftkPolygonVertex->GetBinormalIndex(0));
			else
			{
				vertexBinormal.x = 1.0f;
				vertexBinormal.y = vertexBinormal.z = 0.0f;
			}
		}
		vertex->x0 = vertexTangent.x;
		vertex->y0 = vertexTangent.y;
		vertex->z0 = vertexTangent.z;
		vertex->x1 = vertexBinormal.x;
		vertex->y1 = vertexBinormal.y;
		vertex->z1 = vertexBinormal.z;
	}

}

#endif
