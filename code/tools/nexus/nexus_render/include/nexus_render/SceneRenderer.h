#ifndef GSCENE_RENDERER_H
#define GSCENE_RENDERER_H

namespace NexusRender
{
	class MeshCluster;
	class ViewGrid;
	class D3DShaderManager;
	class D3DTextureManager;

	class SceneRenderer
	{
	public:
		struct CropSheetImageRecord
		{
			U32 atlasIndex;
			std::string fileName;
			S32 textureIndex;
			F32 uOffset;
			F32 vOffset;
			F32 uScale;
			F32 vScale;
			bool rotated;
		};

		struct CropSheetAtlasRecord
		{
			U32 width;
			U32 height;
			std::string fileName;
			IDirect3DTexture9* pTexture;
		};

		struct LayerInfo
		{
			const Aftk::C_LayerNode* pLayer;

			unsigned isVisible:1;
		};

		//Create and associate with a window and scene.
		SceneRenderer();
		
		~SceneRenderer();

		Tu::Status Create(
			IDirect3DDevice9* pDevice,
			Aftk::C_Scene* pScene,
			Aftk::ObjTypeArray* pBoundsFilter,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text));
		
		void Destroy();

		Tu::Status PreResetDevice();

		Tu::Status PostResetDevice(Nexus::Progress* pProgress);

		///Render the current scene to the display device
		Tu::Status OnDraw();

		void OnCameraZoom();

		void OnGizmoRenderingChanged();

		void OnSkinningModeChanged();

		Tu::Status OnUpdateGeometry(Nexus::Progress* pProgress);

		Tu::Status OnUpdateTextures(Nexus::Progress* pProgress);

		Tu::Status OnUpdateCropSheetDatabase(
			const std::string& csDBFileName);

		Tu::Status OnUpdateScene(Nexus::Progress* pProgress);

		Tu::Status OnLoadTextures(Nexus::Progress* pProgress);

		Tu::Status OnLoadCropSheetTextures(Nexus::Progress* pProgress);

        void SetCameraPtr(Camera* pCamera);

		bool TexturesLoaded() const;

		bool CropSheetTexturesLoaded() const;

		void SetSettingsPtr(const SceneRendererSettings* pSettings);

		const SceneRendererSettings* GetSettingsPtr() const;

		RenderNode* GetRootRenderNodePtr();

		const RenderNode* GetRootRenderNodePtr() const;

		void UpdateTime(F32 dT);

		F32 GetCurrentTime() const;

		void SetTime(F32 time);

		void Clear();

		F32 GetZoomScale() const;

		bool UseMaterialClusters() const;

		const Camera* GetCameraPtr() const;

		Camera* GetCameraPtr();

		void SetDoWeRenderThisNodeCallback(
			bool (*pDoWeRenderThisNodeCallback)(const Aftk::C_SceneNode* pAftkSceneNode));

		void SetDoWeRenderThisPolygonCallback(
			bool (*pDoWeRenderThisPolygonCallback)(const Aftk::C_Polygon* pAftkPolygon));

		bool DoWeRenderThisPolygon(const Aftk::C_Polygon* pPolygon) const;

		bool DoWeRenderThisNode(const Aftk::C_SceneNode* pAftkSceneNode) const;

		bool IsBuilt() const;

		D3DShaderManager* GetShaderManagerPtr();

		D3DTextureManager* GetTextureManagerPtr();

		void SetSelected(E_SelectionMode selectionMode, const Nexus::AftkSceneNodeMap& aftkSceneNodeMap);

		const CropSheetAtlasRecord& GetCropSheetAtlasRecord(U32 index) const;

		U32 GetNumCropSheetAtlasRecords(U32 index) const;

		const CropSheetImageRecord& GetCropSheetImageRecord(U32 index) const;

		S32 GetCropSheetImageRecord(const std::string& fileName) const;

		U32 GetNumCropSheetImageRecords(U32 index) const;

		Aftk::C_Scene* GetAftkScenePtr();

		const Aftk::C_Scene* GetAftkScenePtr() const;

		void Pick(
			Nexus::C_PickResult& result, 
			E_SelectionMode selectionMode,
			const Aftk::C_Ray& ray);

		void DoMessage(const std::string& message);

		void DoWarning(const std::string& message);

		void DoError(const std::string& message);

		RenderNode* GetRenderNodeFromAftkNode(const Aftk::C_SceneNode* pAftkSceneNode);

		const LayerInfo* GetLayerInfoPtr(U32 index) const;

		LayerInfo* GetLayerInfoPtr(U32 index);

		S32 GetLayerIndex(const Aftk::C_LayerNode* pLayer) const;

		void* operator new (size_t size) 
		{
			return AllocAligned(size);
		}

		void operator delete (void *p)
		{	
			FreeAligned(p);	
		}

	protected:
		struct MeshClusterBatch;

		class HashKeyOpSceneNode
		{
		public:
			//Hashing function.
			static unsigned int GetHash(const Aftk::C_SceneNode* pKey)
			{
				return pKey->GetUniqueId();
			}

			static bool Equal(const Aftk::C_SceneNode* pKey1, const Aftk::C_SceneNode* pKey2)
			{
				if (pKey1 == pKey2)
					return true;

				return false;
			}
		};

		SceneRenderer(const SceneRenderer& rhs);

		const SceneRenderer& operator=(const SceneRenderer& rhs);

		void ComputeSceneBounds();

		Tu::Status UpdateRenderNodes();

		Tu::Status UpdateRenderNodeSources(
			bool updateScale,
			bool timeChanged,
			RenderNode* pRenderNode);

		Tu::Status UpdateRenderNodeInstances(
			bool updateScale,
			bool timeChanged);
        
		void LoadTexturesIntoMaterials();

		Tu::Status LoadCropSheetTexturesIntoMaterials();

		void ClearSelection();

		void ClearDynamicMeshEffectConstantsList();

		void ClearSceneHierachy();

		Tu::Status BuildDynamicMeshEffectConstantsList();

		Tu::Status BuildMeshBatches();

		Tu::Status DrawMeshComponents();

		Tu::Status DrawMesh(
			EffectConstants* pEffectConstants,
			IRenderMeshNode* pMesh,
			S32 cluster);

		Tu::Status DrawMeshBatches();

		Tu::Status DrawMeshBatches(
			D3DManager::EVertexDecl decl,
			RenderMeshNode::EEffectMode mode);

		Tu::Status DrawMeshBatch(
			MeshClusterBatch* pMeshClusterBatch,
			D3DManager::EVertexDecl decl,
			RenderMeshNode::EEffectMode mode,
			EffectWrapper** ppLastEffectWrapperUsed);

		Tu::Status DrawMeshes(
			Ut::UtVector< Ut::DynamicVector<IRenderMeshNode*> >
				meshes[D3DManager::VERTEX_DECL_COUNT][IRenderMeshNode::EFFECT_MODE_COUNT],
			EffectConstants* pEffectConstants);

		Tu::Status DrawMeshes(
			Ut::UtVector< Ut::DynamicVector<IRenderMeshNode*> >
				meshes[D3DManager::VERTEX_DECL_COUNT][IRenderMeshNode::EFFECT_MODE_COUNT],
			D3DManager::EVertexDecl decl,
			RenderMeshNode::EEffectMode mode,
			EffectConstants* pEffectConstants);

		Tu::Status DrawSplines();

		Tu::Status DrawSelectedNonMesh();

		Tu::Status DrawSceneBoundingBox();

		Tu::Status DrawGizmos();

		void SetCropSheetTextureIDs();

		///////////////////////////////////////////////////////////////////////////////
		//////// POINTERS TO EXTERNAL STATE INFORMATION ////////
			Aftk::C_Scene*					m_pAftkScene;

			const SceneRendererSettings*	m_pSettings;
			SceneRendererSettings			m_lastSettings;
		//////// END POINTERS TO EXTERNAL STATE INFORMATION
		///////////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////////////
		//////// STATE INFORMATION ////////
			F32 m_time;
			F32 m_lastTime;

			F32	m_zoomScale;
			F32 m_oldZoomScale;

			bool m_isBuilt;
			bool m_texturesLoaded;
			bool m_cropSheetTexturesLoaded;
			bool m_skinningModeChanged;
			bool m_timeChanged;

			U32 m_numProcessingStagesCompleted;
			U32 m_numProcessingStages;
		//////// END STATE INFORMATION ////////
		///////////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////////////
		//////// D3D ////////
			BoundingBox*		m_pRenderBoundingBox;
			D3DShaderManager*	m_pShaderManager;
			D3DTextureManager*	m_pTextureManager;

			//The home grid
			ViewGrid*			m_pViewGrid;

			Camera*				m_pCamera;
		//////// END D3D ////////
		///////////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////////////
		//////// CROP SHEET VARS ////////
			bool														m_cropSheetTextureIDsSet;
			Ut::UtVector<Ut::DynamicVector<CropSheetAtlasRecord> >		m_cropSheetAtlasDB;
			Ut::UtVector<Ut::DynamicVector<CropSheetImageRecord> >		m_cropSheetImageDB;
			//Map from texture filename to crop sheet entry
			std::map<std::string, U32>									m_cropSheetImageDBMapping;
		//////// END CROP SHEET VARS ////////
		///////////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////////////
		//////// MATERIAL VARS ////////
			EffectWrapper*					m_effectWrappers[EffectWrapper::EFFECT_TYPE_COUNT];
			MeshBasicEffectConstants*		m_pMeshBasicEffectConstants;

			MeshSelectionEffectConstants*	m_pMeshSelectionEffectConstants;

			//Mesh components each have different colour values, stored in a Gizmo 
			//constants instance.
			GizmoEffectConstants*		m_pGizmoEffectConstantsMeshNormal;
			GizmoEffectConstants*		m_pGizmoEffectConstantsMeshTangent;
			GizmoEffectConstants*		m_pGizmoEffectConstantsMeshBinormal;

			//The constants for the lightmap and texture effects are allocated as needed from the
			//given Aftk scene. They are then stored in a list, and deallocated from that
			//list when the renderer is cleared.

		//////// END MATERIAL VARS ////
		///////////////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////////////
		//////// RENDER NODE LISTS ////////

			//Contains a list of mesh cluster ptrs to be rendered with the given effect
			//constants.
			struct MeshClusterBatch
			{
				//The effect constants that we'll use for the list of clusters. From the
				//effect constants class we can also find out which effect we should upload
				//the constants into.
				EffectConstants* pEffectConstants;
				//A cluster is a fragment of a mesh that uses only one material.
				Ut::UtVector< Ut::DynamicVector<
					Ut::Pair<MeshCluster*, IRenderMeshNode*> > > meshClusters;
			};

			//The root of the scene graph;
			RenderNode* m_pRootRenderNode;

			Ut::UtVector<Ut::DynamicVector<RenderNode*> > m_renderNodes;
			Ut::UtVector<Ut::DynamicVector<RenderNode*> > m_renderDirectionalLightNodes;
			Ut::UtVector<Ut::DynamicVector<RenderNode*> > m_renderPointLightNodes;
			Ut::UtVector<Ut::DynamicVector<RenderNode*> > m_renderSpotLightNodes;
			Ut::UtVector<Ut::DynamicVector<RenderNode*> > m_renderAreaLightNodes;
			Ut::UtVector<Ut::DynamicVector<RenderNode*> > m_renderInstanceNodes;
			Ut::UtVector<Ut::DynamicVector<RenderNode*> > m_renderSplineNodes;
			Ut::UtVector<Ut::DynamicVector<RenderNode*> > m_renderMeshNodes;
			//For each vertex declaration we store a list of meshes per mesh _mode_.
			//The mode can be STATIC, SKINNED, or BONED INSTANCE.
			//The idea of this batching is that we can set one technique for each array of meshes to be 
			//renderered as each array will be of the same vertex type and mesh effect mode.
			Ut::UtVector< Ut::DynamicVector<IRenderMeshNode*> >	
				m_meshBatchesNoMaterial[D3DManager::VERTEX_DECL_COUNT][IRenderMeshNode::EFFECT_MODE_COUNT];
			//Batch selected meshes by declaration and mode.
			Ut::UtVector< Ut::DynamicVector<IRenderMeshNode*> >	
				m_meshBatchesNoMaterialSelected[D3DManager::VERTEX_DECL_COUNT][IRenderMeshNode::EFFECT_MODE_COUNT];

			//Render meshes sorting by vertex decl, then by mode, then by effect constants. The effect
			//constants should be sorted such that groups of constants that use the same effect should be
			//together to minimise shader switching.
			Ut::UtVector< Ut::DynamicVector<MeshClusterBatch*> >	
				m_meshClusterBatches[D3DManager::VERTEX_DECL_COUNT][IRenderMeshNode::EFFECT_MODE_COUNT];

			//List of selected render nodes - excludes mesh nodes.
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >	m_selectedRenderNodes;
		//////// END RENDER NODE LISTS ////////
		///////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////
		//////// CALLBACKS ////////
		void (*m_pMessageCallback)(const std::string& text);
		void (*m_pWarningCallback)(const std::string& text);
		void (*m_pErrorCallback)(const std::string& text);
		bool (*m_pDoWeRenderThisNodeCallback)(const Aftk::C_SceneNode* pAftkSceneNode);
		bool (*m_pDoWeRenderThisPolygonCallback)(const Aftk::C_Polygon* pAftkPolygon);
		//////// END CALLBACKS ////////
		///////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////
		//////// MISC /////////
		Ut::OpenHashMap<
			const void*, 
			RenderNode*, 
			Ut::HashKeyOpPtr > m_aftkNodeToRenderNodeMapping;

		///Maps between an Aftk material pointer and a set of dynamic mesh effect
		///constants. Effect constant pointers may be duplicated. This array is accessed by Aftk
		///material index.
		Ut::UtVector<Ut::DynamicVector<EffectConstants*> > m_aftkMaterialToEffectConstantsMapping;

		//A list of unique effect constants.
		Ut::UtVector<Ut::DynamicVector<EffectConstants*> >	m_dynamicMeshEffectConstantsList;

		Ut::UtVector<Ut::DynamicVector<LayerInfo> > m_layerInfo;

		AxisRenderer					m_axisRenderer;
		DirectionalLightGizmoRenderer	m_directionalLightGizmoRenderer;
		SpotLightGizmoRenderer			m_spotLightGizmoRenderer;
		PointLightGizmoRenderer			m_pointLightGizmoRenderer;
		AreaLightGizmoRenderer			m_areaLightGizmoRenderer;

		Aftk::ObjTypeArray*				m_pBoundsFilter;
		//////// END MISC ////////
		///////////////////////////////////////////////////////////////////////////////

		friend class RenderNode;
	};
}

#endif
