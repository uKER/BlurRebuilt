#pragma once

namespace NexusRender
{
	class SceneRenderer;

	class RenderMeshNode : public RenderMeshRemappableNode
	{
	public:
		enum ESpecialCluster
		{
			SPECIAL_CLUSTER_ENTIRE_MESH = 0,
			SPECIAL_CLUSTER_NO_MATERIAL = 1,

			SPECIAL_CLUSTER_COUNT
		};

		RenderMeshNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkNode,
			Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource);
		
		~RenderMeshNode();

		virtual RenderNode::ERenderNodeType GetType() const;

		virtual bool HasType(RenderNode::ERenderNodeType type) const;

		U32 GetNumClusters(U32 lod) const;

		const MeshCluster* GetClusterPtr(U32 lod, U32 iMeshCluster) const;

		MeshCluster* GetClusterPtr(U32 lodLevel, U32 iMeshCluster);

		void Clear();

		Tu::Status Rebuild();

		Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged);

		virtual Tu::Status DrawNormals(U32 lodLevel);

		virtual Tu::Status DrawTangentSpace(U32 lodLevel, U16 set);

		virtual Tu::Status DrawSelectedComponents(U32 lodLevel);

		virtual Tu::Status Draw();

		virtual Tu::Status Draw(U32 lodLevel, U32 clusterIndex);

		const DynamicArrayU32& GetClusterPolyIndexList(U32 clusterIndex);

		VertexBuffer* GetVertexBufferPtr(U32 index);

		virtual bool IsSkinned() const;

		virtual bool IsSkinAnimated() const;

		S32 GetVBPVIndexFromAftkPVIndex(U32 vbIndex, U32 aftkPVIndex) const;

		U32	GetAftkPVIndexFromVBPVIndex(U32 vbIndex, U32 vbPVIndex) const;

		U32	GetVBIndexFromAftkPolyIndex(U32 lodLevel, U32 aftkPolyIndex) const;

		U32 GetNumMeshVertexBuffers() const;

		Tu::Status OnPreResetDevice();

		Tu::Status OnPostResetDevice();

		///\brief Returns an enum indicating which vertex declaration should be used to render this mesh.
		///
		///@return The vertex declaration enum.
		virtual D3DManager::EVertexDecl GetVertexDecl() const;

		virtual IRenderMeshNode::EEffectMode GetEffectMode() const;

		Tu::Status OnSelected();

		Tu::Status OnDeselected();

		U8 GetNumUVSets() const;

		virtual void SelectFaces(const Ut::UtSet<Nexus::C_ComponentSelection>& selection);

		virtual void SelectPoints(const Ut::UtSet<Nexus::C_ComponentSelection>& selection);

	private:
		//We only ever want to construct this node from the AFTK.
		RenderMeshNode(const RenderMeshNode& rhs);

		const RenderMeshNode& operator=(const RenderMeshNode& rhs);

		void OnAftkMeshUpdate(Aftk::C_SceneMeshNode* pAftkMesh);

		void ClearMeshClusters();

		Tu::Status ResetMeshVertexBuffers();

		Tu::Status RebuildClusterRecordLists();

		Tu::Status RebuildPolygonIndexLists();

		Tu::Status CreateVertexBuffers();

		void DestroyVertexBuffers();

		Tu::Status LockMeshVertexBuffer(U32 index, void*& pVertexBufferContents);

		Tu::Status UnlockMeshVertexBuffer(U32 index);

		Tu::Status EvaluateMeshVertexBuffers();

		Tu::Status Evaluate();

		void GetVertexColourChannelAndType(S32& channel, Aftk::E_GenericAttributeType& type);

		void SetActiveLodLevel();

		template<class T>
		Tu::Status CopyPolygonVertex(
			T* pD3DVertex,
			S32 vertexColourChannel,
			Aftk::E_GenericAttributeType vertexColourType,
			S32 lightMapUVSet,
			U32 polygonVertexIndex);

		template<class T>
		Tu::Status CopyEvaluatedPositionAndNormal(
			T* pD3DVertex,
			U32 polygonVertexIndex);

		struct Flags
		{
			unsigned isSkinned:1;
			unsigned isSkinAnimated:1;

			unsigned reserved1:1;
			unsigned reserved2:1;
			unsigned reserved3:1;
			unsigned reserved4:1;
			unsigned reserved5:1;
			unsigned reserved6:1;
		};

		struct ClusterRecord
		{
			MeshCluster* pCluster;
			DynamicArrayU32 polyIndexList;
		};

		//Array of mesh clusters. [lod][cluster]
		Ut::UtVector< Ut::DynamicVector<
			Ut::UtVector< Ut::DynamicVector<ClusterRecord> > > >
										m_meshClusterLists;

		VertexBuffer*					m_pMeshVBList;
		U8								m_nMeshVBs;
		U8								m_nUVSets;
		Flags							m_meshFlags;

		//Map from Aftk Polygon indices -> VB indices
		Ut::UtVector<Ut::DynamicVector<
			Ut::UtVector<Ut::DynamicVector<U32> > > >				
										m_aftkPolyVBMappingLists;
		//Map from AFTK PV index -> VB PV indices [VB][PV]
		Ut::UtVector<Ut::DynamicVector<
			Ut::UtVector<Ut::DynamicVector<S32> > > >				
										m_aftkVBPVMappingArrayLists;
		//Map from VB PV indices -> Aftk PV index
		Ut::UtVector<Ut::DynamicVector<
			Ut::UtVector<Ut::DynamicVector<U32> > > >			
										m_vbAftkPVMappingList;

		MeshComponentDisplay*			m_pComponentDisplay;

		D3DManager::EVertexDecl			m_vertexDecl;
		IRenderMeshNode::EEffectMode	m_effectMode;

		friend class RenderMeshInstanceNode;
		friend class RenderMeshBonedInstanceNode;
	};

	template<class T>
	Tu::Status RenderMeshNode::CopyPolygonVertex(
		T* pD3DVertex,
		S32 vertexColourChannel,
		Aftk::E_GenericAttributeType vertexColourType,
		S32 lightMapUVSet,
		U32 polygonVertexIndex)
	{
		const Aftk::C_SceneMeshNode* pAftkSceneMeshNode = 
			(Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
		const Aftk::C_PolygonVertex* pAftkPolygonVertex = 
			&pAftkSceneMeshNode->GetPolygonVertex(polygonVertexIndex);

		U32 numUVSets = pAftkSceneMeshNode->GetNumTextureCoordinateSets();

		//Keep vectors in local coordinates
		const Aftk::Position3* pVertexPos = 
			&pAftkSceneMeshNode->GetVertexPosition(pAftkPolygonVertex->GetPositionIndex());
		
		// Copy data into vertex array
		pD3DVertex->SetPosition(pVertexPos->X(), pVertexPos->Y(), pVertexPos->Z());

		if (pAftkPolygonVertex->GetNormalIndex() != -1)
		{
			const Aftk::Normal3* pVertexNorm = 
				&pAftkSceneMeshNode->GetVertexNormal(pAftkPolygonVertex->GetNormalIndex());

			pD3DVertex->SetNormal(pVertexNorm->X(), pVertexNorm->Y(), pVertexNorm->Z());
		}
		else
		{
			pD3DVertex->SetNormal(1,0,0);
		}

		if (vertexColourChannel != -1)
		{
			switch (vertexColourType)
			{
			case Aftk::E_GenericAttributeType_Vector4F32:
			{
				Aftk::Vector4F32 aftkColour;
				pAftkPolygonVertex->GetData(aftkColour, vertexColourChannel);

				pD3DVertex->SetColour(
					(U8)(aftkColour.X()*255.0f), 
					(U8)(aftkColour.Y()*255.0f), 
					(U8)(aftkColour.Z()*255.0f), 
					(U8)(aftkColour.W()*255.0f)
					);
			}
			break;
			case Aftk::E_GenericAttributeType_U32:
			{
				U32 aftkColourInt;
				pAftkPolygonVertex->GetData(aftkColourInt, vertexColourChannel);
				Aftk::Vector4F32 aftkColour = Aftk::ColourRGBAFromU32(aftkColourInt);

				pD3DVertex->SetColour(
					(U8)(aftkColour.X()*255.0f), 
					(U8)(aftkColour.Y()*255.0f), 
					(U8)(aftkColour.Z()*255.0f), 
					(U8)(aftkColour.W()*255.0f)
					);
			}
			break;
			default:
				pD3DVertex->SetColour(255,255,255,255);
			}
		}
		else
		{
			//We don't have any vertex colours
			pD3DVertex->SetColour((U8)(255), (U8)(255), (U8)(255), (U8)(255) );
		}
		

		// handle the case of one set of UVs - output main UV set
		if (numUVSets >= 1)
		{
			if (pAftkPolygonVertex->GetNumTextureCoordinateIndices() == 0)
			{
				pD3DVertex->SetTextureCoordinate(0, 0, 0);
				#ifdef _DEBUG
				g_debugManager << "Error: pAftkPolygonVertex->GetNumTextureCoordinateIndices() <= numUVSets\n";
				g_debugManager.Clear();
				#endif
			}
			else
			{
				S32 uvIndex = pAftkPolygonVertex->GetTextureCoordinateIndex(0);

				if (uvIndex < 0)
				{
					pD3DVertex->SetTextureCoordinate(0, 0, 0);
				}
				else
				{
					const Aftk::TextureCoordinate4* pAftkUv = 
						&pAftkSceneMeshNode->GetTextureCoordinate(0, uvIndex);
					pD3DVertex->SetTextureCoordinate(0, pAftkUv->X(), pAftkUv->Y());
				}
			}
		}

		// if there is a lightmap UV set, output that as well
		if (lightMapUVSet != -1)
		{
			if ((S32)pAftkPolygonVertex->GetNumTextureCoordinateIndices() <= lightMapUVSet)
			{
				pD3DVertex->SetTextureCoordinate(0, 0, 0);
			}
			else
			{
				// if we have multiple UV channels
				S32 uvIndex = pAftkPolygonVertex->GetTextureCoordinateIndex(lightMapUVSet);

				if (uvIndex < 0)
				{
					pD3DVertex->SetTextureCoordinate(1, 0, 0);
				}
				else
				{
					const Aftk::TextureCoordinate4* pAftkUv = 
						&pAftkSceneMeshNode->GetTextureCoordinate(lightMapUVSet, uvIndex);
					pD3DVertex->SetTextureCoordinate(1, pAftkUv->X(), pAftkUv->Y());
				}
			}
		}

		//Copy skinning information if we're skinned
		if (m_meshFlags.isSkinned)
		{
			U32 vPosIndex = pAftkPolygonVertex->GetPositionIndex();
			const Aftk::C_VertexWeight* pVertexWeight = 
				&pAftkSceneMeshNode->GetVertexWeight(vPosIndex);

			//Copy weights and bone indices
			for (U32 i = 0; i < pVertexWeight->GetNumWeights(); i++)
			{
				pD3DVertex->SetWeight(i, pVertexWeight->GetWeight(i));
				pD3DVertex->SetPaletteIndex(i, pVertexWeight->GetInfluenceNodeIndex(i));
			}
			//Fill any unused bone index and weighting storage with zeros
			for (U32 i = pVertexWeight->GetNumWeights(); i < 3; i++)
			{
				pD3DVertex->SetWeight(i, 0);
				pD3DVertex->SetPaletteIndex(i, 0);
			}

			//Store the number of vertex weights in the 4th element of the weight array.
			pD3DVertex->SetPaletteIndex(3, (U8)pVertexWeight->GetNumWeights());
		}

		return Tu::Status::SUCCESS;
	}

	template<class T>
	Tu::Status RenderMeshNode::CopyEvaluatedPositionAndNormal(
		T* pD3DVertex,
		U32 polygonVertexIndex)
	{
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
		const Aftk::C_PolygonVertex* pAftkPolygonVertex = 
			&pAftkSceneMeshNode->GetPolygonVertex(polygonVertexIndex);

		Aftk::Position3 vertexPos;
		Aftk::Normal3 vertexNorm;
		if (m_meshFlags.isSkinned)
		{
			vertexPos = pAftkSceneMeshNode->GetEvaluatedPolygonVertexPosition(polygonVertexIndex);
			vertexNorm = pAftkSceneMeshNode->GetEvaluatedPolygonVertexNormal(polygonVertexIndex);
		}
		else
		{
			vertexPos = pAftkSceneMeshNode->GetVertexPosition(
				pAftkPolygonVertex->GetPositionIndex());
			if (pAftkPolygonVertex->GetNormalIndex() != -1)
			{
				vertexNorm = pAftkSceneMeshNode->GetVertexNormal(
					pAftkPolygonVertex->GetNormalIndex());
			}
			else
			{
				vertexNorm[0] = 1;
				vertexNorm[1] = 0;
				vertexNorm[2] = 0;
			}
		}
		// Copy data into vertex array
		pD3DVertex->SetPosition(vertexPos.X(), vertexPos.Y(), vertexPos.Z());
		pD3DVertex->SetNormal(vertexNorm.X(), vertexNorm.Y(), vertexNorm.Z());
		
		return Tu::Status::SUCCESS;
	}
}