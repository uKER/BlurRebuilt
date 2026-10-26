#pragma once

namespace NexusRender
{
	class SceneRenderer;
	class RenderInstanceNode;

	class RenderNode
	{
	public:
		enum ERenderNodeType
		{
			RENDER_NODE_TYPE_NODE,
			RENDER_NODE_TYPE_MESH,
			RENDER_NODE_TYPE_IMESH,
			RENDER_NODE_TYPE_MESH_REMAPPABLE,
			RENDER_NODE_TYPE_MESH_INSTANCE,
			RENDER_NODE_TYPE_MESH_BONED_INSTANCE,
			RENDER_NODE_TYPE_NON_AMBIENT_LIGHT,
			RENDER_NODE_TYPE_POINT_LIGHT,
			RENDER_NODE_TYPE_SPOT_LIGHT,
			RENDER_NODE_TYPE_DIRECTIONAL_LIGHT,
			RENDER_NODE_TYPE_SPLINE,
			RENDER_NODE_TYPE_INSTANCE,

			RENDER_NODE_TYPE_COUNT
		};

		struct Flags
		{
			unsigned isBuilt:1;
			///Indicates whether we have been directly selected.
			unsigned isSelected:1;
			unsigned isStatic:1;
			unsigned isCulled:1;
			unsigned isHiddenByLodGroup:1;
			//If uniform scaling we can perform cheap matrix inversion on this node. Note that we determine
			//if scaling is uniform or not by looking at the nodes world matrix.
			unsigned isUniformlyScaled:1;
			unsigned hasInitialUpdate:1;
			unsigned isHidden:1;
			unsigned hasLodGroup:1;
			unsigned isHiddenByLayer:1;
			unsigned isHiddenExpandedSource:1;
		};

		RenderNode(
			SceneRenderer* pSceneRenderer, 
			Aftk::C_SceneNode* pAftkSceneNode);

		virtual ~RenderNode();

		virtual ERenderNodeType GetType() const;

		virtual  bool HasType(ERenderNodeType type) const;

		///
		///\brief Makes the given node a child of this node.
		///
		///This internally makes sure that the relationship pointers between the child and parent are correct.
		///
		///@param pNode The child node to parent.
		void AddChild(RenderNode* pNode);	

		///
		///\brief Inserts a sibling before the current next sibling of this node.
		///
		///This internally makes sure that the relationship pointers between the siblings are correct.
		///
		///@param pNode The sibling node to insert.
		void InsertNextPtr(RenderNode* pNode);

		FORCE_INLINE void SetLocalMatrix(const Maths::HMat44& matrix);

		FORCE_INLINE void SetWorldMatrix(const Maths::HMat44& matrix);

		FORCE_INLINE void SetWorldViewProjectionMatrix(const Maths::HMat44& matrix);

		void SetHiddenLod(bool flag);

		///
		///\brief Inserts a sibling before the current previous sibling of this node.
		///
		///This internally makes sure that the relationship pointers between the siblings are correct.
		///
		///@param pNode The sibling node to insert.
		void InsertPreviousPtr(RenderNode* pNode);

		///
		///\brief Get a pointer to the scene this node belongs to.
		///
		///@return The scene pointer.
		const SceneRenderer* GetScenePtr() const;

		///
		///\brief Get a pointer to the scene this node belongs to.
		///
		///@return The scene pointer.
		SceneRenderer* GetScenePtr();

		///
		///\brief Get a pointer to the parent of this node.
		///
		///@return Parent pointer.
		const RenderNode* GetParentPtr() const;

		///
		///\brief Get a pointer to the parent of this node.
		///
		///@return Parent pointer.
		RenderNode* GetParentPtr();

		FORCE_INLINE const RenderNode* GetChildPtr(U32 index) const;

		FORCE_INLINE RenderNode* GetChildPtr(U32 index);

		///
		///\brief Returns the transformation matrix for this node in object space.
		///
		///@return Transformation matrix.
		FORCE_INLINE const Maths::HMat44& GetLocalMatrix() const;

		///
		///\brief Returns the transformation matrix for this node in world space.
		///
		///Note that this information is not cached. Matrix multiplication is performed for each call
		///of this function.
		///
		///@return Transformation matrix.
		FORCE_INLINE const Maths::HMat44& GetWorldMatrix() const;

		FORCE_INLINE const Maths::HMat44& GetWorldMatrixTransposedInverse() const;
		///
		///brief Returns the world-view transformation matrix for this node.
		FORCE_INLINE const Maths::HMat44& GetWorldViewProjectionMatrix() const;

		///
		///\brief Returns the number of children for this node.
		///
		///@return The number of children.
		FORCE_INLINE U32 GetNumChildren() const;

		FORCE_INLINE const Aftk::C_SceneNode* GetAftkSceneNodePtr() const;

		FORCE_INLINE Aftk::C_SceneNode* GetAftkSceneNodePtr();

		virtual Tu::Status Rebuild();

		U32 AddInstance(RenderInstanceNode* pInstance);

		Maths::HMat44 GetGizmoMatrix(const Maths::HMat44& worldMatrix, F32 distanceClamp = -1.f);

		Maths::HMat44 GetGizmoMatrixBillboard();

		Maths::HVec3 GetGizmoScale() const;
/*
		const Maths::C_MatrixSRT& GetLocalSRT() const;

		const Maths::C_MatrixSRT& GetWorldSRT() const;
*/
		RenderInstanceNode* GetInstance(U32 index);

		const RenderInstanceNode* GetInstance(U32 index) const;

		U32 GetNumInstances() const;

		bool HiddenLod() const;

		virtual Tu::Status Update(
			bool timeChanged,
			bool scaleChanged,
			bool skinningModeChanged);

		virtual Tu::Status OnPreResetDevice() {return Tu::Status::SUCCESS;}

		virtual Tu::Status OnPostResetDevice() {return Tu::Status::SUCCESS;}

		U32 GetActiveLodLevel() const;

		void SetActiveLodLevel(U32 lodLevel);

		virtual Tu::Status OnSelected();

		virtual Tu::Status OnDeselected();

		FORCE_INLINE const Flags& GetNodeFlags() const;

		FORCE_INLINE Flags& GetNodeFlags();

		bool IsSelected() const;

		///True: Not animated directly or indirectly.
		///False: Directly or indirectly connected to animation driver.
		FORCE_INLINE bool IsStatic() const;

		virtual bool IsVisible() const;

		void Clear();

		void* operator new (size_t size) 
		{
			return AllocAligned(size);
		}

		void operator delete (void *p)
		{	
			FreeAligned(p);	
		}

	protected:
		RenderNode();
		RenderNode(const RenderNode& rhs);
		const RenderNode& operator=(const RenderNode& rhs);

		///
		///\brief Evaluate the current local matrix, world matrix and world-view-projection matrix.
		virtual Tu::Status Evaluate();

		void FrustumCull(const Camera& camera);

		void HideByLodGroup();

		virtual void SetActiveLodLevel();

		Maths::HMat44	m_localMatrix;
		Maths::HMat44	m_worldMatrix;
		Maths::HMat44	m_worldMatrixTransposedInverse;
		Maths::HMat44	m_worldViewProjectionMatrix;

		SceneRenderer* m_pSceneRenderer;
		Aftk::C_SceneNode* m_pAftkSceneNode;

		RenderNode* m_pParent;

/*
		Maths::C_MatrixSRT	m_localSRT;
		Maths::C_MatrixSRT	m_worldSRT;
		Maths::HVec3			m_worldNonCompensatedScaleExclusive;
*/
		U8					m_activeLod;
		Flags				m_nodeFlags;
		///A total count of how many times this node is directly and indirectly selected.
		///An example of indirect selection is where a mesh instance is selected. Here the source
		///mesh must be indirectly selected

		Ut::UtVector< Ut::DynamicVector<RenderInstanceNode*> > m_instances;
		Ut::UtVector< Ut::DynamicVector<RenderNode*> > m_children;
		Ut::UtVector< Ut::DynamicVector<U32> > m_layerIndices;
	};

	FORCE_INLINE void RenderNode::SetWorldViewProjectionMatrix(const Maths::HMat44& matrix)
	{
		m_worldViewProjectionMatrix = matrix;
	}

	FORCE_INLINE const Aftk::C_SceneNode* RenderNode::GetAftkSceneNodePtr() const
	{
		return m_pAftkSceneNode;
	}

	FORCE_INLINE Aftk::C_SceneNode* RenderNode::GetAftkSceneNodePtr()
	{
		return m_pAftkSceneNode;
	}

	FORCE_INLINE const Maths::HMat44& RenderNode::GetWorldMatrix() const
	{
		return m_worldMatrix;
	}

	FORCE_INLINE const Maths::HMat44& RenderNode::GetWorldMatrixTransposedInverse() const
	{
		return m_worldMatrix;
	}

	FORCE_INLINE const Maths::HMat44& RenderNode::GetWorldViewProjectionMatrix() const
	{
		return m_worldViewProjectionMatrix;
	}

	FORCE_INLINE bool RenderNode::IsStatic() const
	{
		return m_nodeFlags.isStatic;
	}

	FORCE_INLINE const RenderNode::Flags& RenderNode::GetNodeFlags() const
	{
		return m_nodeFlags;
	}

	FORCE_INLINE RenderNode::Flags& RenderNode::GetNodeFlags()
	{
		return m_nodeFlags;
	}

	FORCE_INLINE const RenderNode* RenderNode::GetChildPtr(U32 index) const
	{
		return m_children[index];
	}

	FORCE_INLINE RenderNode* RenderNode::GetChildPtr(U32 index)
	{
		return m_children[index];
	}

	FORCE_INLINE U32 RenderNode::GetNumChildren() const
	{
		return (U32)m_children.Size();
	}
}
