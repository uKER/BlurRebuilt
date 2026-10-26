#ifndef AFTK_RENDER_MESH_NODE_INCLUDE
#define AFTK_RENDER_MESH_NODE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Status;
	class C_SceneNode;
}

namespace Display
{
	namespace D3D9
	{
		class C_VertexDeclaration;
	}
}

namespace AftkRender
{
	class C_RenderMeshNode : public C_RenderMeshRemappableNode
	{
	public:

		enum E_ReservedClusterType
		{
			//E_ReservedClusterType_Entire_Mesh,
			E_ReservedClusterType_No_Material = 0,
			E_ReservedClusterType_Count
		};

		struct VBStream
		{
			enum E_VertexStreamUsage
			{
				E_VertexStreamUsage_Position_Normals,
				E_VertexStreamUsage_Position_Normals_Colour,
				E_VertexStreamUsage_UVs,
				E_VertexStreamUsage_Weights_PaletteIndices,
				E_VertexStreamUsage_Tangents_Binormals
			};

			Display::C_VertexBuffer	m_VertexBuffer;
			U8							m_StreamIndex;
		};

		C_RenderMeshNode(
			Aftk::C_SceneNode *pAftkNode,
			Aftk::C_SceneMeshRemappableNode *pAftkMaterialMappingSouce,
			C_SceneRenderer *pRenderer);

		~C_RenderMeshNode();

		void Clear();

		Aftk::C_Status Rebuild();

		Aftk::C_Status Render(U32 lodLevel=0);

		Aftk::C_Status Render(U32 lodLevel, U32 clusterIndex);

		U32 GetNumClusters(U32 lod) const;

		const C_RenderMeshCluster* GetMeshClusterPtr(U32 lodLevel, U32 clusterIndex) const;

		C_RenderMeshCluster* GetMeshClusterPtr(U32 lodLevel, U32 clusterIndex);

		Display::C_VertexDeclaration* GetVertexDeclaration() const;

		I_RenderMeshNode::E_EffectMode GetEffectMode() const;

		U8 GetNumUVSets() const;

		U32 GetNumVertexBuffers();

		E_RenderNodeType GetType() const;

		bool HasType(E_RenderNodeType type) const;

		U32 GetVBIndexFromAftkPolyIndex(U32 lod, U32 polyIndex);

		const Ut::UtVector< Ut::DynamicVector<U32> >& GetVBPVIndicesFromAftkPolyIndex(U32 lod, U32 polyIndex);

		U32 GetAftkPVIndexFromVBPVIndex(U32 vbIndex, U32 vbPvIndex);

		const Ut::UtVector< Ut::DynamicVector<VBStream> >& GetVertexBufferStreams(U32 vbIndex) const;

		VBStream::E_VertexStreamUsage GetVertexStreamUsage(U32 index) const ;

	private:

		// Defines a polygon:- which VB & trilist indices
		struct PolygonVertexRecord
		{	
			U32 m_VBIndex;
			Ut::UtVector< Ut::DynamicVector<U32> > m_TriVtxIndices;
		};

		// Defines a mesh cluster & the polygons in contains
		struct MeshClusterRecord
		{
			C_RenderMeshCluster* pCluster;
			Ut::UtVector< Ut::DynamicVector<U32> > m_polyIndices;
		};

		C_RenderMeshNode();
		C_RenderMeshNode(const C_RenderMeshNode&);
		const C_RenderMeshNode& operator=(const C_RenderMeshNode&);

		Aftk::C_Status CreateVertexBuffers();
		void DestroyVertexBuffers();

		Aftk::C_Status FillVertexBuffers();

		Aftk::C_Status BuildMeshClusters();
		void DestroyMeshClusters();

		Ut::UtVector< Ut::DynamicVector<
			Ut::UtVector< Ut::DynamicVector<PolygonVertexRecord> > > > m_LodPolyToVBMap;

		Ut::UtVector< Ut::DynamicVector<
			Ut::UtVector< Ut::DynamicVector<U32> > > > m_VBToPolyVertexMap;


		//Per LOD array of mesh clusters
		Ut::UtVector<Ut::DynamicVector<
			Ut::UtVector< Ut::DynamicVector<MeshClusterRecord> > > > m_LodMeshClusters;

		//Collection of VBs organised into "packages" of streams
		// At the moment, streams are as follows:-
		// 0 - Position, Normal, (and maybe vertex colour),
		// 1 - Texture coordinates
		// 2 - Weights & matrix palette indices (if skinned)
		// 3 - Tangents & binormals
		Ut::UtVector<Ut::DynamicVector<
			Ut::UtVector< Ut::DynamicVector<VBStream> > > > m_VBStreamPackages;

		Ut::UtVector< Ut::DynamicVector<VBStream::E_VertexStreamUsage> > m_VBStreamUsage;
		U8 m_nUVSets;

		I_RenderMeshNode::E_EffectMode m_EffectMode;

		Display::C_VertexFormat m_VertexFormat;
		Display::C_VertexDeclaration	*m_pVertexDeclaration;
	};


	template <typename T>
	static void SetVertexPosition(
		T* vertex,
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
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
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
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
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
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
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
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