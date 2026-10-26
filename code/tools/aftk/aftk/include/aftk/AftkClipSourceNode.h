#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_CLIP_SOURCE_NODE_H
#define AFTK_CLIP_SOURCE_NODE_H

namespace Aftk
{
	class C_ClipNode;

	///Description: A container for animation curves. Seperate curves are used 
	///for the individial scalar components of an animations scale, rotation and 
	///translation values.
	class C_ClipSourceNode : public I_ClipNode
	{
	public:
		///\brief Constructor.
		///
		///@param pScene The scene that this clip is attached to.
		C_ClipSourceNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs The clip source to copy.
		C_ClipSourceNode(const C_ClipSourceNode& rhs);

		///\brief Destructor.
		///
		virtual ~C_ClipSourceNode();

		///\brief Asignment operator.
		///
		///@param rhs The clip source to copy.
		///@return A reference to this clip source.
		const C_ClipSourceNode& operator=(const C_ClipSourceNode& rhs);

		///\brief Adds an animation curve to this clip source.
		///
		///@param animCurve The animation curve to add.
		///@return The index of the animation curve.
		U32 SetCurvePtr(E_AnimAttribute attr, C_AnimationCurveNode* pCurve);

		///\brief Returns an animation curve.
		///
		///@param instance The index of the animation curve to return.
		///@return The animation curve.
		const C_AnimationCurveNode* GetCurvePtr(E_AnimAttribute attr) const;

		///\brief Returns an animation curve.
		///
		///@param instance The index of the animation curve to return.
		///@return The animation curve.
		C_AnimationCurveNode* GetCurvePtr(E_AnimAttribute attr);

		///\brief Returns the number of animation curve in this clip source.
		///
		///@return The number of animation curves.
		U32 GetNumCurves() const;

		///\brief Find the highest value of x for which there is a key in this clip.
		///
		///@return The highest value of x.
		F32	GetStartX() const;

		///\brief Find the highest value of x for which there is a key in this clip.
		///
		///@return The highest value of x.
		F32 GetEndX() const;

		///\brief Return the duration of this clip in frames.
		///
		///@return The clip duration.
		F32 GetLength() const;

		///\brief Return the number of clips that are instancing this source.
		///
		///@return The number of instance of this source.
		U32 GetNumInstances() const;

		///\brief Return an instance of this clip source.
		///
		///@param instance The index of the clip to return.
		///@return The clip.
		C_ClipNode* GetInstancePtr(U32 instance);

		///\brief Return an instance of this clip source.
		///
		///@param instance The index of the clip to return.
		///@return The clip.
		const C_ClipNode* GetInstancePtr(U32 instance) const;

		///\brief Queries whether a specific attribute type is animated by any curves in this clip at x.
		///
		///@param attribute The attribute type for which to query.
		///@param x The value of x at which to query.
		bool IsAnimated(E_AnimAttribute attr) const;

		///\brief Evaluates the curves of this clip at x to create a transformation matrix.
		///
		///@param x The value of x at which to evaluate.
		///@return The result.
		bool Evaluate(F32& value, E_AnimAttribute attr, F32 x) const;

		C_NullPlug& GetCurvePlug(E_AnimAttribute attr);

		const C_NullPlug& GetCurvePlug(E_AnimAttribute attr) const;

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual I_Plug* GetStaticPlugPtr(U32 instance);
		
		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual const I_Plug* GetStaticPlugPtr(U32 instance) const;
		
		///\brief Returns the number of static plugs on this node.
		///
		///@return The number of dynamic plugs.
		virtual U32	GetNumStaticPlugs() const;

		virtual bool IsUsed() const;

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

		///\brief Clears the contents of this clip to their initial state.
		///
		virtual void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void OneTimeInit();

		void ClearThis();

		///
		///\brief When an instance is created, this function is invoked upon the source node.
		///
		///This allows the source of an instance to keep track of how many xs it is being instanced.
		///@param pInstanceNode The instance that is using this node as its source.
		void AddInstanceOfThisNode(C_ClipNode* pInstanceNode);

		void DeleteInstanceOfThisNode(C_ClipNode* pInstanceNode);

		static void C_ClipSourceNode::OnCurvePlugChanged(void* _pThis);

        Ut::UtSet<C_ClipNode*> m_pInstancesOfThisNode;

		//Our animation curves plugs
		C_NullPlug* m_curvePlugs[E_AnimAttribute_Count];

		friend class C_ClipNode;
		friend class C_NativeClipSourceNode;
	#endif
	};
}

#endif
