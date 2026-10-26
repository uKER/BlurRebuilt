#ifndef MAYA_IO_IMPORTER_H
#define MAYA_IO_IMPORTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class JenkinsHashKeyOpSetU32
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const Ut::UtSet<U32>& key)
		{
			unsigned int hash = 0;
			for (Ut::UtSet<U32>::ConstIterator it = key.Begin(); it != key.End(); ++it)
				hash += JenkinsHash(it->GetKey());

			return hash;
		}

		static bool Equal(const Ut::UtSet<U32>& key1, const Ut::UtSet<U32>& key2)
		{
#ifdef BUILD_CONFIG_DEBUG
			//Validate set operator== incase it goes missing again!
			if (key1.Size() != key2.Size())
			{
				assert(key1 != key2);
				return false;
			}

			for (Ut::UtSet<U32>::ConstIterator it = key1.Begin(); it != key1.End(); ++it)
				if (key2.Find(it->GetKey()) == key2.End())
				{
					assert(key1 != key2);
					return false;
				}

			assert(key1 == key2);
			return true;
#else
			//This rather depends upon utset having operator== overloaded.
			return key1 == key2;
#endif
		}

	private:
		//Jenkins' hash
		static U32 JenkinsHash(U32 a)
		{
		   a = (a+0x7ed55d16) + (a<<12);
		   a = (a^0xc761c23c) ^ (a>>19);
		   a = (a+0x165667b1) + (a<<5);
		   a = (a+0xd3a2646c) ^ (a<<9);
		   a = (a+0xfd7046c5) + (a<<3);
		   a = (a^0xb55a4f09) ^ (a>>16);
		   return a;
		}
	};


	class C_Importer
	{
	public:
		///
		C_Importer();
		
		///
		~C_Importer();

		///\brief Writes the current Maya scene to disk
		///
		///@param file A maya file object.
		///@param options A maya options string.
		///@param mode File access mode.
		///@param pluginName The name of this plugin, as provided by Maya.
		///@param pOptions A structure containing any custom options for the importer.
		///@return Returns true upon success, and false upon failure. This function succeeds or fails atomically.
		Aftk::C_Status Import(
			Aftk::C_Scene* pAftkScene,
			const char* pOptions,
			Aftk::I_Progress* pProgress);

		const C_MaterialImportFilter&
				GetMaterialFilter() const;

		const C_PlugImportFilter&
				GetPlugFilter() const;

		Aftk::C_Status AddToImportedMaterialMap(
			const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pMaterialResource);

		const Aftk::C_SharedResource<Aftk::C_MaterialNode>* 
			GetImportedMaterialByName(const std::string& name);

		Aftk::C_Scene* GetAftkScenePtr();

		const Aftk::C_Scene* GetAftkScenePtr() const;

		void DoWeImportThisNode(
			const MDagPath& dagPath, 
			bool& doWeImportThisNode,
			bool& doWeImportItsChildren,
			Aftk::C_Status& status);

		bool IsBonedInstanceJoint(const MDagPath& path);

		bool IsBonedInstance(const MFnDagNode& fnTransformDagNode);

		bool IsBonedInstanceParent(const MFnDagNode& fnTransformDagNode);

		bool HasSpeedTreeNode(const MObject& fnNodeObject, MObject& speedTreeNode) const;

		bool HasSpeedTreeNode(MFnDagNode& mFnShape, MObject& speedTreeNode) const;

		C_AftkMayaMapping& GetAftkMayaMapping();

		const C_AftkMayaMapping& GetAftkMayaMapping() const;

		void SetNonFatalErrorsEncountered(bool flag);

		bool NonFatalErrorsEncountered() const;

		const C_ImportOptions* GetImportOptionsPtr() const;

		void AddDebris(Aftk::C_SceneNode* pSceneNode);

		bool IsDebris(const Aftk::C_SceneNode* pSceneNode) const;

		const Ut::UtVector<Ut::DynamicVector<MObject> >& 
			GetPolySetArray() const;

		const Ut::OpenHashMap<Ut::UtSet<U32>, const Aftk::C_BlindDataResource*, JenkinsHashKeyOpSetU32 >& 
			GetPolySetCombinationTable() const;

		///Acquires blind data version of the given poly set combination. If no blind data already exists
		///for this combination then it is first created.
		const Aftk::C_BlindDataResource* GetPolySetCombinationBlindData(
			Ut::UtSet<U32>& polySetCombinationIndices);

		bool IsExcludedByImportFilters(
			const MDagPath& dagPath,
			Aftk::C_Status&	status);

	private:
		Aftk::C_Status GetParameters(
			C_ImportOptions& options,
			const std::string& arguments);

		Aftk::C_Status DGDirty();

		Aftk::C_Status ReloadIRM();

		Aftk::C_Status PreProcessScene();

		Aftk::C_Status PreProcessDagNode(
			Ut::UtSet<std::string>& bonedInstanceJoints,
			const MDagPath& transformDagPath,
			const MSelectionList& list);

		Aftk::C_Status PostProcessScene();

		Aftk::C_Status ImportBonedInstances();

		Aftk::C_Status ReplaceDebris();
	    
		Aftk::C_Status CheckForInvertedMatrices();

		void	Clear();
		
		void	CleanUpAfterFailure();

		Aftk::C_Status	TraverseDagTopLevel(
			Aftk::C_Scene *pAftkScene);

		Aftk::C_Status	ImportTransformAndShapeIfPresent(
			const MDagPath&	transformDagPath, 
			Ut::UtVector< Ut::DynamicVector<Aftk::Matrix44> >& matrixStack,
			Aftk::C_SceneNode*	pLastGoodAncestor,
			Aftk::C_Scene*		pAftkScene); 

		Aftk::C_SceneNode*	ImportTransformWithoutShape(
			const MDagPath&	transformDagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene*		pAftkScene,
			Aftk::C_Status&		status);

		Aftk::C_SceneNode*	ImportTransformWithShape(
			const MDagPath&	transformDagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene*		pAftkScene,
			Aftk::C_Status&		status);

		Aftk::C_SceneNode*	ImportXRefNode(
			const MDagPath&	transformDagPath, 
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene*		pAftkScene,
			Aftk::C_Status&		status);

		Aftk::C_Status GetLayerIndices(
			const		MFnDagNode& dagNodeFn, 
			bool&		gameLayerMember,
			MIntArray&	layerIndices);

		Aftk::C_Status GetGameLayerName(
			int			layerIndex,
			MString&	layerName);

		Aftk::C_Status GetLayerName(
			const		MFnDagNode& shapeDagNode, 
			bool&		layerMember,
			MString&	layerName);
		
		Aftk::C_Status AddToMayaLayerIfNeeded(
			const MFnDagNode&		fnTransformDagNode, 
			Aftk::C_SceneNode*		pAftkSceneNode);

		Aftk::C_Status AddToGameLayerIfNeeded(
			const MFnDagNode&		fnTransformDagNode, 
			Aftk::C_SceneNode*		pAftkSceneNode);

		Aftk::C_Status AddToLayerIfNeeded(
			const MFnDagNode&		fnTransformDagNode, 
			Aftk::C_SceneNode*		pAftkSceneNode);

		Aftk::C_SceneNode* ResolveAndImportShapeOrShapeInstance(
			const MDagPath&	transformDagPath, 
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene*		pAftkScene,
			Aftk::C_Status&		status);

		bool IsVisible(
			MFnDagNode&			dagNode, 
			Aftk::C_Status&		status);

		bool IsActor(
			const MDagPath&	transformDagPath);

		Aftk::C_Status GetClusterMembership(
			MObject& obCluster, 
			MSelectionList& obMembers);

		Aftk::C_Status ImportSkinClusters();

		Aftk::C_Status ImportSkinCluster(
			const MDagPath& mayaDagPath, 
			Aftk::I_SceneMeshNode* pAftkMeshNode);

		Aftk::C_Status ImportDg();

		Aftk::C_Status ConvertBasePoseToLocalSpace();

		Aftk::C_Status ConvertBasePoseToLocalSpace(Aftk::C_SceneNode* pAftkSceneNode, const Aftk::Matrix44& worldParentBasePose);

		Aftk::C_Status BuildSetInfo();

		C_ImportOptions m_options;

		// Stores all of the dag paths processed, along with pointers to the AftkSceneNodes which were added
		Ut::UtVector< Ut::DynamicVector<C_InstanceTracker> > m_instanceTracker;

		// We need to store a stack for the transforms that we are processing. The reason for this is that in cases
		// where multiple transforms are connected to each other (and have no shapes), we want to to collapse these
		// transforms instead of adding null nodes.
		Ut::UtVector< Ut::DynamicVector<MDagPath> >	m_shapelessTransformStack;

		//This mapping table allows us to select DAG objects by object name without having to exploit the
		//MGlobal::select API.
		Ut::UtMap<std::string, MDagPath> m_nameStringMayaSceneNodeMapping;

		Ut::UtMap<std::string, Aftk::C_LayerNode*> m_layerMap;

		Ut::UtMap<std::string, const Aftk::C_SharedResource<Aftk::C_MaterialNode>*> m_materialMap;

		Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneNode*> > m_debrisList;

		U32	m_interestingNodeCount;
		U32	m_interestingNodeImportedCount;

		Aftk::C_Scene* m_pAftkScene;

		C_BonedInstanceJointList	m_bonedInstanceJointList;

		C_MaterialImportFilter		m_materialFilter;
		C_PlugImportFilter			m_plugFilter;
		C_SceneNodeImportFilter		m_sceneNodeFilter;
		C_LayerImportFilter			m_layerFilter;

		C_BindPoseHelper			m_bindPoseHelper;
		C_AftkMayaMapping			m_aftkMayaMapping;

		Aftk::I_Progress*			m_pProgressNotification;

		//Abstract game import interface.
		I_GameImport*				m_pIGameImport;
		bool						m_nonFatalErrorsEncountered;

		//Used for poly sets.
		Ut::UtVector<Ut::DynamicVector<MObject> > m_polySetArray;
		Ut::OpenHashMap<Ut::UtSet<U32>, const Aftk::C_BlindDataResource*, JenkinsHashKeyOpSetU32 > 
			m_polySetCombinationTable;
	};
}

#endif
