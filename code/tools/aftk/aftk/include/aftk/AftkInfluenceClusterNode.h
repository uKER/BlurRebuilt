#ifndef AFTK_INFLUENCE_CLUSTER_H
#define AFTK_INFLUENCE_CLUSTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_SceneNode;
	class C_NativeSceneNode;
	class C_NativeInfluenceClusterNode;

	class C_InfluenceClusterNode : public C_Node
	{
	public:
		///\brief Constructor.
		///
		C_InfluenceClusterNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs The influence cluster to copy.
		///
		C_InfluenceClusterNode(const C_InfluenceClusterNode& rhs);

		///\brief Destructor.
		///
		~C_InfluenceClusterNode();

		///\brief Asignment operator.
		///
		///@param rhs The influence cluster to copy.
		///@return A reference to this influence cluster.
		const C_InfluenceClusterNode& operator=(const C_InfluenceClusterNode& rhs);

		///\brief Sets the scene node which this cluster influences.
		///
		///@param pSceneNode The scene node pointer.
		void SetSceneNodePtr(C_SceneNode* pSceneNode);

		///\brief Adds a new weighting envelope to this cluster.
		///
		///Influence clusters contain arrays of weighting envelopes. Each envelope contains a set of
		///weights that are associated with a fragment of a particular object of influence, such as a bone.
		///
		///@param envelope The weighting envelope to add
		///@return The index of this weighting envelope in the array of weight enveloeps.
		U32	AddEnvelope(const C_WeightEnvelope& envelope = C_WeightEnvelope());

		///\brief Set the initial size of the weighting envelope array
		///
		///Array presizing will increase performance.
		///
		///@param capacity The initial size of the weighting envelope array.
		void SetEnvelopeInitialCapacity(U32 capacity);

		///\brief Overwrites an existing weight envelope instance.
		///
		///@param instance The index of the weight envelope to overwrite.
		///@param envelope The new weight envelope.
		void SetEnvelope(U32 instance, const C_WeightEnvelope& envelope);

		///\brief Set the usage string cue for this influence cluster.
		///
		///This is used to describe to purpose of the influnce cluster.
		///Typically this will be set as AFTK_INFLUENCE_CLUSTER_USAGE_SKIN, indicating it can be interpreted as skinning information.
		///@param usage The usage string.
		void SetUsage(const std::string& usage);

		///\brief Return the number of weighting envelopes in this cluster.
		///
		///@return The number of weighting envelopes.
		U32	GetNumEnvelopes() const;

		///\brief Returns the weighting envelope at the given index.
		///
		///@param instance The index of the weighting envelope to return.
		///@return The weighting envelope.
		const C_WeightEnvelope&	GetEnvelope(U32 instance) const;

		///\brief Returns the weighting envelope at the given index.
		///
		///@param instance The index of the weighting envelope to return.
		///@return The weighting envelope.
		C_WeightEnvelope& GetEnvelope(U32 instance);

		///\brief Return a pointer to the scene node that this cluster influences.
		///
		///@return The scene node pointer
		C_SceneNode* GetSceneNodePtr();

		///\brief Return a pointer to the scene node that this cluster influences.
		///
		///@return The scene node pointer
		const C_SceneNode* GetSceneNodePtr() const;

		///\brief Returns the usage string cue for this influence cluster.
		///
		///This is used to describe to purpose of the influnce cluster.
		///Typically this will be set as AFTK_INFLUENCE_CLUSTER_USAGE_SKIN, indicating it can be interpreted as skinning information.
		///@return The usage string.
		const std::string& GetUsage() const;

		virtual bool IsUsed() const;

		virtual E_ObjType GetType() const;

		virtual bool HasType(E_ObjType type) const;

		///\brief Clears this influence cluster to their initial state.
		///
		virtual void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		Ut::UtVector< Ut::DynamicVector<C_WeightEnvelope> >		m_envelopes;

		std::string m_usage;

		C_SceneNode* m_pSceneNode;

		FRIEND_CLASS C_Scene;
		FRIEND_CLASS C_SceneNode;
		FRIEND_CLASS C_NativeFtk;
		FRIEND_CLASS C_NativeSceneNode;
		FRIEND_CLASS C_SceneMeshNode;
		FRIEND_CLASS C_NativeInfluenceClusterNode;
	#endif
	};

	typedef Ut::UtVector< Ut::DynamicVector<C_InfluenceClusterNode> >	InfluenceClusterArray;
	typedef Ut::UtVector< Ut::DynamicVector<C_InfluenceClusterNode*> >	InfluenceClusterPtrArray;
}

#endif
