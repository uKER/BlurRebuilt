#ifndef AFTK_SCENE_NODE_H
#define AFTK_SCENE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_Scene;
	class C_SceneNode;
	class C_SceneNodeIterator;
	class C_LayerNode;
	class C_InfluenceClusterNode;
	class C_SceneInstanceNode;
	class C_SceneMeshBonedInstanceNode;
	class C_SceneMeshRemappedInstanceNode;
	class C_SceneJointNode;
	class C_NativeSceneMeshBonedInstanceNode;
	class C_NativeSceneInstanceNode;
	class C_NativeSceneMeshRemappedInstanceNode;

	class C_SceneNode : public C_Node
	{
	public:

		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_SceneNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs The node to copy.
		C_SceneNode(const C_SceneNode& rhs);

		///\brief Destructor.
		///
		virtual	~C_SceneNode();

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Generate hash of data in current class
		///
		///@return 32b hash value
		U32 GetHashOfThis() const;

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		///\brief Asignment operator.
		///
		///@param rhs The node to copy.
		///@return A reference to this node.
		const C_SceneNode& operator=(const C_SceneNode& rhs);

		///\brief Equality operator
		///
		///@param rhs The node to test against
		///@return Boolean equality
		bool operator==(const C_SceneNode &rhs) const;
		
		///\brief Set the blind data resource pointer for this node.
		///
		///If overwriting an existing resource pointer, the reference count for the old resource is decremented.
		///As a consequence, the old resource may be deleted by the blind data resource manager. 
		///
		///@param pBlindDataResource The blind data resource pointer to set.
		void SetBlindDataResourcePtr(const C_BlindDataResource* pBlindDataResource);
		
		///\brief Removes blind data from this node.
		///
		///This may not physically delete the blind data from the scene. The blind data pointer for this node is
		///set to null, and the old resources reference count is decrmented. /As a consequence, the old resource may 
		///be deleted by the blind data resource manager. 
		void DeleteBlindDataResourcePtr();

		///\brief Makes the given node a child of this node.
		///
		///This internally makes sure that the relationship pointers between the child and parent are correct.
		///
		///@param pChild The child node to parent.
		void AddChildPtr(C_SceneNode* pChild);	

		///\brief Inserts a new child node after the given child.
		///
		///This internally makes sure that the relationship pointers between the child and parent are correct.
		///
		///@param afterThis The child after which to insert.
		///@param pChild The child node to parent.
		void AddChildPtrAfter(C_SceneNode* afterThis, C_SceneNode* pChild);

		///\brief Inserts a new child node after the given child index.
		///
		///This internally makes sure that the relationship pointers between the child and parent are correct.
		///
		///@param afterThisChild The child index after which to insert.
		///@param pChild The child node to parent.
		void AddChildPtrAfter(U32 afterThisChild, C_SceneNode* pChild);

		///\brief Inserts a new child node before the given child.
		///
		///This internally makes sure that the relationship pointers between the child and parent are correct.
		///
		///@param beforeThis The child before which to insert.
		///@param pChild The child node to parent.
		void AddChildPtrBefore(C_SceneNode* beforeThis, C_SceneNode* pChild);

		///\brief Inserts a new child node before the given child index.
		///
		///This internally makes sure that the relationship pointers between the child and parent are correct.
		///
		///@param beforeThisChild The child index before which to insert.
		///@param pChild The child node to parent.
		void AddChildPtrBefore(U32 beforeThisChild, C_SceneNode* pChild);

		///\brief Sets the pointer for the first child node.
		///
		///This internally makes sure that the relationship pointers between the child and parent are correct.
		///
		///@param pChild The child node to parent.
		void SetChildPtr(C_SceneNode* pChild);	

		///\brief Removes the given child, and all its children.
		///
		///@param pChild The child to delete.
		void DeleteChildPtr(C_SceneNode* pChild);

		///\brief Sets the base pose (bind pose) matrix for this scene node.
		///
		///This is the world space matrix for this scene node at the time the relevant skin was bound.
		///@param matrix The pose matrix.
		void SetBasePose(const Matrix44& matrix);

		///\brief Set the local transformation matrix of this node.
		///
		///@param matrix New transformation matrix.
		void SetLocalMatrix(const Matrix44& matrix, bool compensateChildren = false);

		void SetWorldMatrix(const Matrix44& matrix, bool compensateChildren = false);

		///\brief Get a pointer to the parent of this node.
		///
		///@return Parent pointer.
		const C_SceneNode* GetParentPtr() const;

		///\brief Get a pointer to the nth child of this node.
		///
		///@param instance The child instance to fetch.
		///@return The child node.
		const C_SceneNode* GetChildPtr(U32 instance = 0) const;

		///\brief Get a pointer to this node's next sibling.
		///
		///@return The next sibling.
		const C_SceneNode* GetNextPtr() const;

		///\brief Get a pointer to this node's previous sibling.
		///
		///@return The previous sibling.
		const C_SceneNode* GetPreviousPtr() const;

		///\brief Get a pointer to the parent of this node.
		///
		///@return Parent pointer.
		C_SceneNode* GetParentPtr();

		///\brief Get a pointer to the nth child of this node.
		///
		///@param instance The child instance to fetch.
		///@return The child node.
		C_SceneNode* GetChildPtr(U32 instance = 0);

		///\brief Get a pointer to this node's next sibling.
		///
		///@return The next sibling.
		C_SceneNode* GetNextPtr();

		///\brief Get a pointer to this node's previous sibling.
		///
		///@return The previous sibling.
		C_SceneNode* GetPreviousPtr();

		///\brief Returns the base pose matrix for this node.
		///
		///This is the world space matrix for this scene node at the time the relevant skin was bound.
		///@return The pose matrix.
		INLINE FORCE_INLINE const Matrix44& GetBasePose() const;

		///\brief Returns the inverse base pose matrix for this node.
		///
		///This is the inverse world space matrix for this scene node at the time the relevant skin was bound.
		///@return The inverse pose matrix.
		INLINE Matrix44 GetBasePoseInverse() const;

		///\brief Returns the local transformation matrix for this node.
		///
		///@return Object space transformation matrix.
		const Matrix44& GetLocalMatrix(bool evaluate = true) const;

		///\brief Returns the inverse local transformation matrix for this node.
		///
		///The results of this function are cached instead of being re-evaluated for each call.
		///
		///@return Object space transformation matrix.
		Matrix44 GetLocalMatrixInverse(bool evaluate = true) const;

		///\brief Returns the transposed inverse of the local transformation matrix for this node.
		///
		///The results of this function are cached instead of being re-evaluated for each call.
		///
		///@return Object space transformation matrix.
		Matrix44 GetLocalMatrixInverseTranspose(bool evaluate = true) const;

		///\brief Returns the transformation matrix for this node in world space.
		///
		///The results of this function are cached instead of being re-evaluated for each call.
		///
		///@return World space transformation matrix.
		const Matrix44& GetWorldMatrix(bool evaluate = true) const;

		///\brief Returns the inverse of the transformation matrix for this node in world space.
		///
		///The results of this function are cached instead of being re-evaluated for each call.
		///
		///@return World space transformation matrix.
		Matrix44 GetWorldMatrixInverse(bool evaluate = true) const;

		///\brief Returns the inversed transpose of the transformation matrix for this node in world space.
		///
		///The world matrix is cached instead of being re-evaluated for each call of this function in
		///order to increase performance.
		///
		///@return World space transformation matrix.
		Matrix44 GetWorldMatrixInverseTranspose(bool evaluate = true) const;

		///\brief Returns the number of children for this node.
		///
		///@return The number of children.
		U32	GetNumChildren() const;

		///\brief The number of layers that we are a member of.
		///
		///@return The number of layers.
		U32 GetNumLayers() const;

		///\brief Return the number of instances that use this node as their source.
		///
		///@return The number of instances using this as their source.
		U32	GetNumInstancesOfThisNode() const;

		///\brief Returns a particular instance of this node.
		///
		///@param instance The index of the instance to retrieve.
		///@return The instance ndoe.
		const C_SceneNode* GetInstanceOfThisNode(U32 instance) const;

		///\brief Returns a particular instance of this node.
		///
		///@param instance The index of the instance to retrieve.
		///@return The instance ndoe.
		C_SceneNode* GetInstanceOfThisNode(U32 instance);
		
		///\brief Return a pointer to the layer which this node is a member of.
		///
		///@return C_LayerNode pointer.
		C_LayerNode* GetLayerPtr(U32 layer);

		///\brief Return a pointer to the layer which this node is a member of.
		///
		///@return C_LayerNode pointer.
		const C_LayerNode* GetLayerPtr(U32 layer) const;

		///\brief Return the blind data resource pointer for this node.
		///
		///@return The blind data resource pointer for this node.
		const C_BlindDataResource* GetBlindDataResourcePtr() const;
		
		///\brief Returns an ID number which uniquely identifies this node.
		///
		///@return Unique ID number for this node. All valid IDs are >= 0.
		S32	GetUniqueId() const;

		//\brief Returns an ID number which uniquely identifies the group that this node belongs to.
		///
		///@return Unique group ID number. All valid IDs are >= 0. An ID of -1 indicates that this node is not a member of a group.
		S32	GetGroupNodeInstance() const;

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///
		///@return C_Node type enum.
		virtual E_ObjType GetType() const;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const;

		///@return The number of errors remaining.
		virtual U32 Repair();

		///\brief Validate that the contents of this node are correct.
		///
		///@return The number of errors found.
		virtual U32	Validate() const;

		///\brief Replace the given node in the scene graph.
		///
		///@param pNode The node to replace.
		///@return SUCCESS if replace was successful, otherwise FAILURE.
		C_Status Replace(C_SceneNode* pNode, bool assumeUniqueId = true);

		///\brief Returns a pointer to the influence cluster for this scene node.
		///
		///@param instance The influence cluster index.
		///@return The influence cluster pointer.
		C_InfluenceClusterNode* GetInfluenceClusterPtr(U32 instance);

		///\brief Returns a pointer to the influence cluster for this scene node.
		///
		///@param instance The influence cluster index.
		///@return The influence cluster pointer.
		const C_InfluenceClusterNode* GetInfluenceClusterPtr(U32 instance) const;

		///\brief Returns the number of influence clusters for this scene node.
		///
		///@return The number of influence clusters.
		U32 GetNumInfluenceClusters() const;

		virtual C_Bounds GetEvaluatedLocalBounds() const;

		///\brief Returns the bounding box of this node in world space.
		///
		///@return The bounds information.
		virtual C_Bounds GetEvaluatedWorldBounds(bool evaluateTransform = true) const;

		///\brief Returns the bounding box of this node in object space.
		///
		///@return The bounds information.
		C_Bounds GetLocalBounds() const;

		///\brief Returns the bounding box of this node in world space.
		///
		///@return The bounds information.
		C_Bounds GetWorldBounds(bool evaluateTransform = true) const;

		///\brief Returns the path to this node.
		///
		///Each node has a unique path. The path functions like a directory path and is delimited by '|' characters.
		///
		///@return The path.
		std::string GetPath() const;

		virtual bool IsUsed() const;

		///\brief Clear the contents of this node to their initial state.
		///
		virtual void Clear();

		bool IsTransformAnimated() const;

		bool IsTransformDirectlyAnimated() const;

		bool IsTransformIndirectlyAnimated() const;

		C_NullPlug* GetTransformPlugPtr(E_AnimAttribute attr);

		C_NullPlug& GetTranslatePlug();

		C_NullPlug& GetRotatePlug();
		
		C_NullPlug& GetScalePlug();
		
		C_NullPlug& GetShearPlug();

		C_StaticPlug<bool>& GetVisibilityPlug();

		const C_NullPlug* GetTransformPlugPtr(E_AnimAttribute attr) const;

		const C_NullPlug& GetTranslatePlug() const;

		const C_NullPlug& GetRotatePlug() const;
		
		const C_NullPlug& GetScalePlug() const;
		
		const C_NullPlug& GetShearPlug() const;

		const C_StaticPlug<bool>& GetVisibilityPlug() const;

		bool GetTranslation(Vector3F32& translation, bool evaluate = true) const;

		bool GetRotation(Vector3F32& rotation, bool evaluate = true) const;

		bool GetRotation(Matrix44& rotation, bool evaluate = true) const;

		bool GetScale(Vector3F32& scale, bool evaluate = true) const;

		bool GetShear(Vector3F32& shear, bool evaluate = true) const;

		///\brief Sp
		void SetScalePivot(const Vector3F32& pivot);

		///\brief St
		void SetScalePivotTranslation(const Vector3F32& translate);

		///\brief Rp
		void SetRotatePivot(const Vector3F32& pivot);

		///\brief Rt
		void SetRotatePivotTranslation(const Vector3F32& translate);

		///\brief Ro
		void SetRotateOrientation(const Matrix44& m);


		///\brief Sp
		const Vector3F32& GetScalePivot() const;

		///\brief St
		const Vector3F32& GetScalePivotTranslation() const;

		///\brief Rp
		const Vector3F32& GetRotatePivot() const;

		///\brief Rt
		const Vector3F32& GetRotatePivotTranslation() const;

		///\brief Ro
		const Matrix44& GetRotateOrientation() const;

		E_RotationInterpolation GetRotationInterpolation() const;

		void SetRotationInterpolation(E_RotationInterpolation interp);

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		I_Plug* GetStaticPlugPtr(U32 instance)
		{
			return const_cast<I_Plug*>(((const C_Node*)this)->GetStaticPlugPtr(instance));
		}

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual const I_Plug* GetStaticPlugPtr(U32 instance) const;
		
		///\brief Returns the number of static plugs on this node.
		///
		///@return The number of dynamic plugs.
		virtual U32	GetNumStaticPlugs() const;

		///\brief Adds a new dynamic plug to this node.
		///
		///@param newPlug The plug to add.
		///@return The plug index.
		S32	AddPlugPtr(C_DynamicPlug* pNewPlug);

		///\brief Returns the dynamic plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		const C_DynamicPlug* GetDynamicPlugPtr(U32 instance) const;

		///\brief Returns the dynamic plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		C_DynamicPlug* GetDynamicPlugPtr(U32 instance);
		
		///\brief Returns the number of dynamic plugs on this node.
		///
		///Dynamic plugs are plugs that can be deleted and manipulated. Furthermore they are not
		///interpreted by the AFTK.
		///@return The number of dynamic plugs.
		U32	GetNumDynamicPlugs() const;

		virtual bool DeletePlug(U32 plugIndex);

		void SetRotationOrder(E_RotationOrder order);

		E_RotationOrder GetRotationOrder() const;

		C_ProxyPlug<Matrix44>& GetWorldMatrixPlug();

		const C_ProxyPlug<Matrix44>& GetWorldMatrixPlug() const;

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object& rhs, U32 flags = E_Comparison_LocalTransform) const;

		U32 GetDepth() const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void SetNullHierachyPtrs();

		void OneTimeInit(C_Scene* pScene);

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs,
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		virtual void RecalculateStaticMatrixCache() const;

		virtual void RecalculateDynamicMatrixCache() const;

		void RecalculateLocalBoundsCache() const;

		void RecalculateWorldBoundsCache() const;

		virtual void CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const;

		void AttachToLayer(C_LayerNode* pLayer);

		void DetachFromLayer(C_LayerNode* pLayer);

		///\brief Adds a sibling to the end of the siblings array for this node.
		///
		///This internally makes sure that the relationship pointers between the siblings are correct.
		///
		///@param pNode The sibling node to add.
		void AddNextPtr(C_SceneNode* pNode);

		///
		///\brief Adds an influence cluster pointer for this scene node
		///
		///@param pInfluenceCluster The influence cluster pointer.
		///@return The index at which the new cluster was added.
		S32 AddInfluenceClusterPtr(C_InfluenceClusterNode* pInfluenceCluster);

		void DeleteInfluenceClusterPtr(C_InfluenceClusterNode* pInfluenceCluster);

		void DeleteInfluenceClusterPtr(U32 instance);

		///
		///\brief When an instance is created, this function is invoked upon the source node.
		///
		///This allows the source of an instance to keep track of how many times it is being instanced.
		///@param pInstanceNode The instance that is using this node as its source.
		void AddInstanceOfThisNode(C_SceneNode* pInstanceNode);

		void DeleteInstanceOfThisNode(C_SceneNode* pInstanceNode);

		void InsertNextPtr(C_SceneNode* pNode);

		void InsertPreviousPtr(C_SceneNode* pNode);

		U32 ProcessProblems(bool fix);

		static void OnTransformPlugChanged(void* pThis);

		///\brief Evaluates the local transformation matrix for this animated node.
		///
		///@param x The variable that the animation is a function of.
		///@return Transformation matrix.
		const Matrix44& EvaluateLocalMatrix() const;

		///\brief Evaluates the inverse local transformation matrix for this animated node.
		///
		///@param x The variable that the animation is a function of.
		///@return Transformation matrix.
		Matrix44 EvaluateLocalMatrixInverse() const;

		///\brief Evaluates the inverse of the transpose of the local transformation matrix for this animated node.
		///
		///@param x The variable that the animation is a function of.
		///@return Transformation matrix.
		Matrix44 EvaluateLocalMatrixInverseTranspose() const;

		///\brief Evaluates the world transformation matrix for this animated node.
		///
		///@param x The variable that the animation is a function of.
		///@return Transformation matrix.
		const Matrix44& EvaluateWorldMatrix() const;

		///\brief Evaluates the inverse world transformation matrix for this animated node.
		///
		///@param x The variable that the animation is a function of.
		///@return Transformation matrix.
		Matrix44 EvaluateWorldMatrixInverse() const;

		///\brief Evaluates the inverse of the transpose of the world transformation matrix for this animated node.
		///
		///@param x The variable that the animation is a function of.
		///@return Transformation matrix.
		Matrix44 EvaluateWorldMatrixInverseTranspose() const;

		void DirtyAllTransformDependantCaches() const;

		void DirtyStaticTransformDependantCaches() const;

		void DirtyDynamicTransformDependantCaches() const;

		void EvaluateRotation(
			Matrix44& rotation) const;

		static Matrix44 OnWorldMatrixProxyQuery(const void* _pPlug, bool evaluate);

		///The function below is invoked when an instance source enters its destructor.
		virtual void DeleteInstanceSource(C_SceneNode* pSource);

		C_SceneNode*		m_pParent;
		C_SceneNode*		m_pChild;
		C_SceneNode*		m_pNext;
		C_SceneNode*		m_pPrevious;
		//The above four pointers can only be touched by the following functions:
		//C_SceneNode::SetNullHierachyPtrs()
		//C_SceneNode::SetChildPtr(C_SceneNode* pNode)
		//C_SceneNode::AddNextPtr(C_SceneNode* pNode)
		//C_SceneNode::Replace(C_SceneNode* pNode, bool assumeUniqueId)
		//C_SceneNode::DeleteChildPtr(C_SceneNode* pNode)
		//C_SceneNode::InsertPreviousPtr(C_SceneNode* pNode)
		//C_SceneNode::InsertNextPtr(C_SceneNode* pNode)
		//It's important that we know this as we cache the number of children.
		//Code-analyst showed that during large scene manipulation the uncached GetNumChildren()
		//function was taking 10% of the time for our process!
		//A cache value of -1 indicates that the cache is dirty and requires refreshing.
		mutable	S32			m_nChildrenCache;

		//Local transform information
		C_NullPlug			m_translatePlug;
		C_NullPlug			m_rotatePlug;
		C_NullPlug			m_scalePlug;
		C_NullPlug			m_shearPlug;
		C_ProxyPlug<Matrix44> m_worldMatrixProxyPlug;
		E_RotationOrder		m_rotationOrder;

		C_StaticPlug<bool>	m_visibilityPlug;

		Vector3F32			m_scalePivot;
		Vector3F32			m_scalePivotTranslation;
		Vector3F32			m_rotatePivot;
		Vector3F32			m_rotatePivotTranslation;
		Matrix44			m_rotateOrientation;
		E_RotationInterpolation 
							m_rotationInterpolation;

		Matrix44			m_basePose;

		//When set, this flag indicates that we need to recalculate our static matrices.
		union
		{
			U32 m_flags;
			struct
			{
				mutable U32		m_staticMatrixCacheDirtyFlag : 1;
				mutable U32		m_localBoundsCacheDirtyFlag : 1;
				//Last evaluation time - used by dynamic matrix cache system. Setting this to a -ve value will
				//trigger the re-evaluation of all dynamic matrices and bounds.
				mutable U32		m_dynamicMatrixCacheDirtyFlag : 1;
				mutable U32		m_worldBoundsCacheDirtyFlag : 1;

				U32				m_hierarchyValid : 1;
				U32				m_normalInverseTranspose : 1;
			};
		};

		//Local matrix cache.
		mutable Matrix44	m_staticLocalMatrixCache;
		mutable Matrix44	m_staticWorldMatrixCache;

		mutable Matrix44	m_dynamicLocalMatrixCache;
		mutable Matrix44	m_dynamicWorldMatrixCache;

#ifndef AFTK_LEAN_AND_MEAN
		Matrix44			m_basePoseInverse;

		mutable Matrix44	m_staticLocalMatrixInverseCache;
		mutable Matrix44	m_staticLocalMatrixInverseTransposeCache;

		//This is the result of our current matrix stack.
		mutable Matrix44	m_staticWorldMatrixInverseCache;
		mutable Matrix44	m_staticWorldMatrixInverseTransposeCache;

		//This is the evaluated local matrix for a given time.
		
		mutable Matrix44	m_dynamicLocalMatrixInverseCache;
		mutable Matrix44	m_dynamicLocalMatrixInverseTransposeCache;
		//This is the evaluated world matrix, using the hierachy at a given time.
		mutable Matrix44	m_dynamicWorldMatrixInverseCache;
		mutable Matrix44	m_dynamicWorldMatrixInverseTransposeCache;
		
		mutable C_Bounds	m_localBoundsCache;
		mutable C_Bounds	m_localEvaluatedBoundsCache;

		mutable C_Bounds	m_staticWorldBoundsCache;
		mutable C_Bounds	m_staticWorldEvaluatedBoundsCache;
		mutable C_Bounds	m_dynamicWorldBoundsCache;
		mutable C_Bounds	m_dynamicWorldEvaluatedBoundsCache;
#endif

		S32					m_uniqueId;
		const C_BlindDataResource* m_pBlindDataResource;

		Ut::UtVector<Ut::DynamicVector<C_LayerNode*> > 
			m_layers;
		Ut::UtVector<Ut::DynamicVector<C_SceneNode*> > 
			m_instancesOfThisNode;
		Ut::UtVector<Ut::DynamicVector<C_InfluenceClusterNode*> > 
			m_influenceClusters;
		Ut::UtVector< Ut::DynamicVector<C_DynamicPlug*> >	
			m_dynamicPlugs;

		FRIEND_CLASS C_Scene;
		FRIEND_CLASS C_LayerNode;
		FRIEND_CLASS C_SceneInstanceNode;
		FRIEND_CLASS C_SceneMeshBonedInstanceNode;
		FRIEND_CLASS C_SceneMeshRemappedInstanceNode;
		FRIEND_CLASS C_InfluenceClusterNode;
		FRIEND_CLASS C_SceneJointNode;

		FRIEND_CLASS C_NativeFtk;
		FRIEND_CLASS C_NativeSceneNode;
		FRIEND_CLASS C_NativeSceneMeshBonedInstanceNode;
		FRIEND_CLASS C_NativeSceneMeshRemappedInstanceNode;
		FRIEND_CLASS C_NativeSceneInstanceNode;
	#endif
	};

	INLINE const Matrix44& C_SceneNode::GetBasePose() const
	{
		return m_basePose;
	}

	INLINE Matrix44 C_SceneNode::GetBasePoseInverse() const
	{
#ifdef AFTK_LEAN_AND_MEAN
		Matrix44 tmp;
		UtMaths::Invert4x4(&tmp, m_basePose);
		return tmp;
#else
		return m_basePoseInverse;
#endif
	}

	typedef Ut::UtVector< Ut::DynamicVector<C_SceneNode*> >				SceneNodePtrArray;
	typedef Ut::UtVector< Ut::DynamicVector<C_SceneMeshNode*> >			SceneMeshNodePtrArray;
}

#endif
