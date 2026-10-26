#ifndef AFTK_SCENE_H
#define AFTK_SCENE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <System/Serialise/FilePath.h>

/*! \mainpage "Abstract File Transfer Kit (AFTK)"
Author: Ken MacLeod\n
(c) 2004 Bizarre Creations
 *
 * \section intro Introduction
 *
 * The AFTK provides in memory data structures for storing a 3D scene. These structures are such that
 * they should provide a superset of the commonly used features of the various 3D file formats out there.
 * The AFTK provides a native file format for lossless reading and writing of scene data, as well as providing
 * an interface for integrating readers for other in-house and third party file formats.
 *
 */

namespace Aftk
{
	// Forward declarations
	class C_Node;
	class C_SceneNode;
	class C_XRefScene;
	class C_InfluenceClusterNode;
	class C_SceneXRefNode;
	class C_Progress;
	class C_LayerNode;
	class C_ClipSourceNode;
	class C_ClipNode;
	class C_ClipMixerNode;
	class C_ShaderNode;
	class C_ShaderImageNode;
	class C_MaterialNode;
	class C_BlindDataNode;
	class C_AttachData;
	class I_Plug;
	class C_GenericAttribute;

	template<class T>
	class C_SharedResource;

	typedef C_SharedResource<C_MaterialNode>	C_MaterialResource;
	typedef C_SharedResource<C_BlindDataNode>	C_BlindDataResource;

	class C_Scene
	{
	public:
		struct ActiveXRefEntry
		{
			C_XRefScene* pXRefScene;
			//Maintain a list of the scenes that use this x-ref.
			Ut::UtVector< Ut::DynamicVector< C_Scene*> > usageList;
		};

		C_Scene();

		///\brief Constructor.
		///
		C_Scene(C_Scene* pRootScene);

		///\brief Destructor.
		///
		~C_Scene();

		///\brief Erase the contents of the scene.
		///
		void Clear();

		///\brief Check whether the current scene has any problems.
		///
		///Once implemented this function will check the current scene for any problems, such as isolated scene
		///nodes and unused resources.
		///@return Returns the number of errore encountered.
		U32 Validate(I_Progress* pProgressCallback = 0) const;

		///\brief Repairs problems in the scene.
		///
		///@return The number of remaining fatal errors in the scene.
		U32 Repair(I_Progress* pProgressCallback = 0);

		///\brief Enfore the constraint that all scene node names must be unique.
		///
		///@return The number of renaming operations executed.
		U32 MakeUniqueSceneNodeNames();

		///\brief Get the number of scene nodes.
		///
		///@return The number of scene nodes.
		U32	GetNumSceneNodes() const;

		///
		///\brief Get the number of blind data nodes.
		///
		///@return The number of blind data nodes in the scene.
		U32	GetNumBlindDataNodes() const;

		///\brief Get the number of materials in the scene.
		///
		///@return The number of material nodes.
		U32	GetNumMaterialNodes() const;

		///\brief Get the number of mesh vertex positions in the scene.
		///
		///@return The number of mesh vertex positions.
		U32	GetNumMeshVertexPositions() const;

		///\brief Get the number of mesh vertex normals in the scene.
		///
		///@return The number of mesh vertex normals.
		U32	GetNumMeshVertexNormals() const;

		///\brief Get the number of mesh vertex tangents in the scene.
		///
		///@return The number of vertex tangents.
		U32	GetNumMeshVertexTangents() const;

		///\brief Get the number of vertex binormals in the scene.
		///
		///@return The number of vertex binormals.
		U32	GetNumMeshVertexBinormals() const;

		///\brief Get the number of mesh texture coordinates in the scene.
		///
		///@return The number of texture coordinates.
		U32	GetNumMeshTextureCoordinates() const;
		
		///\brief Get the number of mesh polygon vertices in the scene.
		///
		///@return The number of mesh polygon vertices in the scene.
		U32	GetNumMeshPolygonVertices() const;

		///\brief Get the number of mesh polygons in the scene.
		///
		///@return The number of mesh polygons in the scene.
		U32	GetNumMeshPolygons() const;

		///
		///\brief Get the number of unique texture maps in the scene.
		///
		///@return The number of unique texture maps in the scene.
		U32	GetNumImageFileNames() const;

		///\brief Get the number of layers in the scene.
		///
		///@return The number of layers in the scene.
		U32	GetNumLayers() const;

		///\brief Get a pointer to the scene graph root node.
		///
		///@return A pointer to the scene graph root node.
		C_SceneNode* GetRootSceneNodePtr();

		///\brief Get a const pointer to the scene graph root node.
		///
		///@return A const pointer to the scene graph root node.
		const C_SceneNode* GetRootSceneNodePtr() const;

		///\brief Get a pointer to a scene node by using it's unique instance number.
		///
		///@param id The unique ID number of the node to retrieve.
		///@return A pointer to the desired scene graph node. This is equal to zero if the desired node cannot be found.
		C_SceneNode* GetSceneNodePtrByUniqueID(S32 id);

		///\brief Get a pointer to a scene node by using it's unique instance number.
		///
		///@param id The unique ID number of the node to retrieve.
		///@return A pointer to the desired scene graph node. This is equal to zero if the desired node cannot be found.
		const C_SceneNode* GetSceneNodePtrByUniqueID(S32 id) const;

		const C_MaterialResource* GetMaterialResourcePtrBySubString(const std::string& name) const;

		const C_MaterialResource* GetMaterialResourcePtrByName(const std::string& name) const;

		///\brief Fetch a shared resource pointer for a particular material node instance.
		///
		///@param instance The material node instance to fetch.
		///@return A const shared resource pointer.
		const C_MaterialResource*	GetMaterialResourcePtr(U32 instance) const;

		///\brief Fetch a shared resource pointer for a particular blind data node instance.
		///
		///@param instance The blind data node instance to fetch.
		///@return A const shared resource pointer.
		const C_BlindDataResource* GetIthBlindDataResourcePtr(U32 instance) const;

		///\brief Fetch a shared resource pointer for a particular blind data node instance.
		///
		///@param instance The blind data node instance to fetch.
		///@return A const shared resource pointer.
		C_BlindDataResource* GetIthBlindDataResourcePtr(U32 instance);

		///\brief Fetch a given layer instance from the scene.
		///
		///This function fetches the layer at the given location in the scene's array of layers.
		///@param instance The layer instance to fetch.
		///@return The desired layer.
		C_LayerNode* GetLayerPtr(U32 instance);

		///\brief Fetch a given layer instance from the scene.
		///
		///This function fetches the layer at the given location in the scene's array of layers.
		///@param instance The layer instance to fetch.
		///@return The desired layer.
		const C_LayerNode* GetLayerPtr(U32 instance) const;

		///\brief Set the scene graph root root.
		///
		///@param pNode A pointer to the new scene root node.
		void SetRootSceneNodePtr(C_SceneNode* pNode);

		///\brief Sets the origin string for this scene's blind data
		///
		///This is a cue used to determine how the blind data is to be processed when performing
		///game preprocessing. For example, it might be set to "PGR2", or "TreasurePlanet".
		void SetOrigin(const std::string& origin);

		///\brief Attaches user-data to a given material node.
		///
		///@param pMaterialResource The material resource holding the desired material node
		///@param attachData The user data to attach.
		void SetMaterialAttachData(
			const C_MaterialResource* pMaterialResource,
			const C_AttachData& attachData);

		///\brief Attaches user-data to a given material node.
		///
		///@param instance The index of the material to which the data will be attached.
		///@param attachData The user data to attach.
		void SetMaterialAttachData(U32 instance, const C_AttachData& attachData);

		///\brief Returns the blind data origin string.
		///
		///@return The blind data origin string.
		const std::string& GetOrigin() const;

		///\brief Add a new material node resource to the scene.
		///
		///If an identical material node already exists, then a handle to this existing node is returned.
		///@param materialNode The material node to add.
		///@return A pointer to a shared resource containing the new material node.
		const C_MaterialResource* AddMaterialNode(
			const C_MaterialNode& materialNode,
			bool enforceUnique = true);

		///\brief Add a new blind data node resource to the scene.
		///
		///If an identical blind data node already exists, then a handle to this existing node is returned.
		///@param blindDataNode The blind data node to add.
		///@return A pointer to a shared resource containing the new blind data node.
		const C_BlindDataResource* AddBlindDataNode(
			const C_BlindDataNode& blindDataNode);

		void SetSceneBlindDataResource(const C_BlindDataResource* pRes);

		const C_BlindDataResource* GetSceneBlindDataResourcePtr() const;

		///\brief Gets the index of the given layer in the scene's array of layers.
		///
		///Returns -1 if the layer is not found.
		///@param pLayer the layer to fetch the index for.
		///@return C_LayerNode index.
		S32	GetLayerIndex(const C_LayerNode* pLayer) const;

		///\brief Removes a node from the hierarchy, and reparents any children it might have.
		///
		///Note that pNode is NOT deleted.
		///
		///@param pNode The node to extract.
		///@param updateChildMatrices If true, children get their parents matrix.
		void ExtractSceneNodeAndReparentChildren(C_SceneNode* pNode, bool updateChildMatrices);

		///\brief Deletes the given node and reparents its children.
		///
		///@param pNode The node to delete.
		///@param updateChildMatrices If true, children get their parents matrix.
		void DeleteSceneNodeAndReparentChildren(C_SceneNode* pNode, bool updateChildMatrices);

		///\brief Delete the given node, and all nodes further down the hierarchy.
		///
		///@param pNode The node to begin the deletion from.
		void DeleteSceneNodeAndChildren(C_SceneNode* pNode);

		///\brief Delete all nodes further down the hierarchy.
		///
		///@param pNode The node to begin the deletion from.
		void DeleteChildren(C_SceneNode* pNode);

		///\brief Isolates a scene node such that it is removed from the hierachy, but keeps its children.
		///
		///@param pNode the node to isolate.
		void ExtractSceneNodeAndChildren(C_SceneNode* pNode);

		///\brief Displays the scene graph structure.
		///
		///Contents are displayed via the output manager. A callback to this can be registered in by a
		///call to 'SetOutputNotificationCallback'.
		///
		void OutputSceneGraph();

		/***************************** Animation related functions *****************************/

		///\brief Sets the global animation frame rate.
		///
		///@param frameRate The frame rate.
		void SetFrameRate(F32 frameRate);

		///\brief Sets the global animation start time (in frames).
		///
		///@param startTime The start time.
		void SetStartTime(F32 startTime);

		///\brief Sets the global animation end time (in frames).
		///
		///@param endTime The end time.
		void SetEndTime(F32 endTime);

		///\brief Gets the global animation frame rate.
		///
		///@return The frame rate.
		F32 GetFrameRate() const;

		///\brief Gets the global animation start time (in frames).
		///
		///@return The start time.
		F32 GetStartTime() const;

		///\brief Gets the global animation end time (in frames).
		///
		///@return The end time.
		F32 GetEndTime() const;

		///\brief Gets the duration of the scene's animation (in frames).
		///
		///@return The duration.
		F32 GetDuration() const;

		/***************************** End animation related functions *****************************/

		///\brief Removes unused materials from the scene.
		///
		///@return The number of unused materials removed.
		U32 DeleteUnusedMaterialNodes();

		///\brief Removes unused blind data from the scene.
		///
		///@return The number of blind data nodes removed.
		U32 DeleteUnusedBlindDataNodes();

		///\brief Removes unused material palette entries on mesh nodes.
		///
		///@return The number of palette entries removed.
		U32 DeleteUnusedMeshPolygonMaterialMappings();

		///\brief Removes unused blind data palette entries on mesh nodes.
		///
		///@return The number of blind data pelette entries removed.
		U32 DeleteUnusedMeshPolygonBlindDataMappings();

		///\brief Optimises the scene by removing unused information.
		///
		///This function removes unused resources and resource mappings. It is basically a shortcut to the 
		///DeleteUnusedMaterialsNodes, DeleteUnusedBlindDataNodes, DeleteUnusedMeshPolygonMaterialMappings and
		///DeleteUnusedMeshPolygonBlindDataMappings functions.
		void Optimise();

		const C_SceneNode* GetSceneNodePtrByPartialPath(const std::string& path) const;

		C_SceneNode* GetSceneNodePtrByPartialPath(const std::string& path);

		const C_SceneNode* GetSceneNodePtrByFullPath(const std::string& path) const;

		C_SceneNode* GetSceneNodePtrByPathByFullPath(const std::string& path);

		C_SceneNode* GetSceneNodePtr(const std::string& name);

		const C_SceneNode* GetSceneNodePtr(const std::string& name) const;

		C_SceneXRefNode* GetSceneXRefNodePtr(const std::string& name);

		const C_SceneXRefNode* GetSceneXRefNodePtr(const std::string& name) const;

		/***************************** X-Ref related functions *****************************/

		S32 AddXRefScene(const std::string& fileName, C_Status& status);

		S32 AddXRefScene(const std::string& fileName, U32 crc, C_Status& status);

		void DeleteXRefScene(const std::string& fileName);

		void DeleteXRefScene(C_XRefScene* pXRefScene);

		U32 GetNumXRefScenes() const;

		///The active X-Ref list contins all of the X-Refs that we are currently aware of.
		U32 GetNumActiveXRefScenes() const;

		C_XRefScene* GetXRefScenePtr(U32 instance);

		const C_XRefScene* GetXRefScenePtr(U32 instance) const;

		const ActiveXRefEntry* GetActiveXRefEntryPtr(U32 instance) const;

		C_Scene* GetXRefRootScenePtr();

		const C_Scene* GetXRefRootScenePtr() const;

		C_Status LoadAllXRefScenes();

		void UnloadAllXRefScenes();

		/***************************** End X-Ref related functions *************************/

		/***************************** File related functions *************************/

		///\brief Reads a scene from disk.
		///
		///@param fileName The name of the file to load.
		///@param format The type of file to load. This is optional, by default the AFTK uses the file extension to determine the correct format.
		///@param importParameters Any parameters which are to be passed to the importer.
		///@param pProgressCallback Callback structure for file loading progress.
		///@return C_Status class, indicating the result of the operation.
		C_Status Read(
			const std::string &fileName, 
			E_FileType format = E_FileType_BestGuess,
			const char* pOptions = 0,
			I_Progress* pProgressCallback = 0);

		C_Status ReadExternalScene(
			E_ExternalScene source,
			const char* pOptions = 0,
			I_Progress* pProgressCallback = 0);

		///\brief Writes this scene to disk.
		///
		///@param fileName The name of the file to create or overwrite.
		///@param format The type of file to write. This is must be specified.
		///@param version The version of the file to write. This is optional, by default the AFTK writes using the latest version of the specified file format.
		///@param exportParameters Any parameters which are to be passed to the exporter.
		///@param pProgressCallback Callback structure for file saving progress.
		///@return C_Status class, indicating the result of the operation.
		C_Status Write(
			const std::string &fileName, 
			E_FileType format = E_FileType_BestGuess,
			U32 version = 0,
			const char* pOptions = 0,
			I_Progress* pProgressCallback = 0) const;

		C_Status WriteExternalScene(
			E_ExternalScene target,
			const char* pOptions = 0,
			I_Progress* pProgressCallback = 0) const;

		///\brief Makes 'instances of an instance' instace what their instance is pointing to.
		///
		U32 ResourceIndirectInstances();

		void DeleteDgNode(C_Node* pNode);

		U32 DeleteUnusedDgNodes(E_ObjType nodeType = E_ObjType_Unknown);

		U32 GetNumDgNodes(E_ObjType nodeType = E_ObjType_Unknown) const;

		const Ut::UtSet<C_Node*>& GetDgNodeSet() const;

		void SetCurrentTime(F32 currentTime, bool invalidateCache = true);

		F32 GetCurrentTime() const;

		C_Bounds GetBounds(const ObjTypeArray* pFilter = 0) const;

		C_Bounds GetEvaluatedBounds(const ObjTypeArray* pFilter = 0) const;

		void SetRootFilePath(const Sys::C_FilePath& filePath);

		Sys::C_FilePath GetRootFilePath();

		/***************************** End File related functions *********************/

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		///
		///\brief Need to remove mappings that are not valid
		///
		void SyncMaterials(
			const C_MaterialResource* pMaterialRes,
			C_ShaderNode* pShaderToReplace,
			C_ShaderImageNode* pShaderToUse = NULL);

		void OneTimeInit(C_Scene* pRootScene);

		void ClearThis();

		///
		///\brief Copy constructor. Not yet implemented
		///
		C_Scene(const C_Scene& rhs);

		U32 ValidateLayers() const;

		U32 ProcessProblems(bool fix, I_Progress* pProgressCallback = 0);

		///
		///\brief Asignment operator. Not yet implemented.
		///
		const C_Scene& operator=(const C_Scene& rhs);

		void OutputSceneGraph(const C_SceneNode* pSceneNode, U32 level);

		///
		/// Called during the C_SceneNode constructor. This stores a pointer to the new scene node within
		/// an array held by this scene class. This is used to fetch nodes for instances and groups. In other
		/// words, it makes it possible to make queries such as "Give me a pointer to scene node 5".
		/// The function returns the index of the newly inserted pointer.
		S32	OnCreateSceneNode(C_SceneNode* pSceneNode);

		void OnCreateUnmanagedDgNode(C_Node* pNode);
		
		///
		///\brief Adds a new layer to the scene and returns its index in the scene's array of layers.
		///
		///@return C_LayerNode index.
		U32	OnCreateLayer(C_LayerNode* pLayer);

		///
		///\brief Deletes the layer residing at the given address.
		///
		///@param pLayer The layer to delete.
		void OnDestroyLayer(C_LayerNode* pLayer);

		void OnDestroyUnmanagedDgNode(C_Node* pNode);
		///
		/// Deletes the given node from the scene's map of scene nodes. 
		/// This is called by the scene node destructor
		void OnDestroySceneNode(C_SceneNode* pSceneNode);

		void IncrementResourceReferenceCount(
			const C_MaterialResource* pRes);

		void DecrementResourceReferenceCount(
			const C_MaterialResource* pRes);

		void IncrementResourceReferenceCount(
			const C_BlindDataResource* pRes);

		void DecrementResourceReferenceCount(
			const C_BlindDataResource* pRes);

		U32 ValidateBlindDataReferenceCounts() const;

		U32 ValidateMaterialReferenceCounts() const;

		C_Status ReadFile(
			const std::string &fileName, 
			E_FileType format, 
			const char* pOptions,
			I_Progress* pProgressCallback);

		C_Status WriteFile(
			const std::string &fileName, 
			E_FileType format, 
			U32 version, 
			const char* pOptions,
			I_Progress* pProgressCallback) const;

		S32 AddActiveXRefScene(C_XRefScene* pNewXRefScene, C_Status& status);

		void DeleteActiveXRefScene(C_XRefScene* pNewXRefScene);

		C_Status OnLoadXRefScene(C_XRefScene* pXRefScene);

		void OnUnloadXRefScene(C_XRefScene* pXRefScene);

		ActiveXRefEntry* GetActiveXRefEntryPtr(U32 instance);

		const C_SceneNode* GetSceneNodePtrByFullPath(const std::string& path, const C_SceneNode* pRoot) const;

		C_SceneNode* GetSceneNodePtrByFullPath(const std::string& path, const C_SceneNode* pRoot);

		const C_SceneNode* GetSceneNodePtrByPartialPath(const std::string& path, const C_SceneNode* pRoot) const;

		C_SceneNode* GetSceneNodePtrByPartialPath(const std::string& path, const C_SceneNode* pRoot);

		void AddMaterialNodeNoTest(C_MaterialResource* pResource);

		void AddBlindDataNodeNoTest(C_BlindDataResource* pResource);

		// The root node
		C_SceneNode*										m_pRootSceneNode;
		// Material nodes for the entire scene
		C_NamedResourceManager<C_MaterialNode>				m_materialNodeManager;

		// Blind data nodes for the entire scene
		C_BlindDataResourceManager							m_blindDataNodeManager;

		//Managed DG nodes can be iterated over view the manager classes.
	#ifdef AFTK_USE_MEMORY_POOL
		Ut::UtSet<C_Node*>	m_unmanagedDgNodes;

		Ut::UtMap<
			S32, 
			C_SceneNode*>	m_sceneNodes;
	#else
		Ut::UtSet<C_Node*>	m_unmanagedDgNodes;

		Ut::UtMap<
			S32, 
			C_SceneNode*>	m_sceneNodes;
	#endif

		// A vector of layers, used to organise parts of the scene graph.
		Ut::UtVector< Ut::DynamicVector<C_LayerNode*> >	m_layers;

		std::string		m_origin;

		// Stores the next node instance number to be allocated
		S32				m_nextSceneNodeUniqueId;

		F32				m_startTime;
		F32				m_endTime;
		F32				m_frameRate;
		F32				m_currentTime;

		const C_BlindDataResource* m_pSceneBlindDataResource;

		/***************************** X-Ref related variables *****************************/
		//The root scene is the scene which contains our x-ref table.
		C_Scene*						m_pRootScene;
		//X-ref scenes used either directly or indirectly by this scene. C_XRefScene memory is
		//deallocated by the root scene.
		Ut::UtVector< Ut::DynamicVector<
			ActiveXRefEntry > >*		m_pActiveXRefScenes;
		//X-ref scenes used by this scene only - reference counted.
		Ut::UtVector< Ut::DynamicVector<
			C_XRefScene* > >			m_xRefScenes;
		/***************************** End X-Ref related variables *************************/

		friend class C_Polygon;
		friend class C_SceneNode;
		friend class C_LayerNode;
		friend class I_SceneMeshNode;
		friend class C_SceneMeshRemappableNode;
		friend class C_SceneMeshNode;
		friend class C_MaterialNode;

		friend class C_ClipNode;
		friend class C_ClipSourceNode;
		friend class C_ClipMixerNode;
		friend class C_InfluenceClusterNode;
		friend class C_XRefScene;
		friend class C_Node;

		friend class C_NativeSceneNode;
		friend class C_NativePolygon;
		friend class C_NativeScene;
		friend class C_NativeFtk;
		friend class C_NativeSceneMeshRemappableNode;
	#endif
	};
}

#endif
