#ifndef AFTK_SCENE_SPLINE_NODE_H
#define AFTK_SCENE_SPLINE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
	class C_NativeSceneSplineNode;
	class C_EvaluateSpline;

	class C_SceneSplineNode : public C_SceneGeometryNode
	{
	public:
		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_SceneSplineNode(C_Scene* pScene);

		C_SceneSplineNode(const C_SceneSplineNode& rhs);

		const C_SceneSplineNode& operator=(const C_SceneSplineNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Destructor.
		///
		virtual ~C_SceneSplineNode();
		
		///\brief Sets the initial capacity of this splines control vertex array.
		///
		///@param capacity The control vertex capacity.
		void SetCVInitialCapacity(U32 capacity);

		///\brief Sets the initial capacity of this splines knot array.
		///
		///@param capacity The knot capacity.
		void SetKnotInitialCapacity(U32 capacity);

		///\brief Adds a control vertex to this spline.
		///
		///@param cv The new control vertex.
		///@return The index of the control vertex.
		U32 AddCV(const Position3& cv);

		///\brief Adds a knot to this spline.
		///
		///@param knot The new knot.
		///@return The index of the new knot.
		U32	AddKnot(F32 knot);

		///\brief Sets the value of an existing control vertex.
		///
		///@param instance The index of the control vertex.
		///@param cv The value of the control vertex.
		void SetCV(U32 instance, const Position3& cv);

		///\brief Sets the form of the spline.
		///
		///The form can be AFTK_SPLINE_FORM_OPEN, AFTK_SPLINE_FORM_CLOSED or AFTK_SPLINE_FORM_PERIODIC.
		///These are the NURBS curve forms supported by Maya.
		///
		///@param form The form to set.
		void SetSplineForm(E_SplineForm form);

		///\brief Sets the spline type. Currently only NURBS spline types are supported.
		///
		///@param splineType currently the only value for this is AFTK_SPLINE_TYPE_NURBS.
		void SetSplineType(E_SplineType splineType);

		///\brief Sets the degree of the curve.
		///
		///In other words, this sets whether the spline is linear, quadratic, cubic etc.
		///@param degree The spline degree.
		void SetDegree(U32 degree);

		///\brief Sets the value of a knot.
		///
		///@param instance The index of the knot to set.
		///@param knot The value of the knot.
		void SetKnot(U32 instance, F32 knot);

		///\brief Sets the rationality if the spline.
		///
		///@param flag True if rational, False if not.
		void SetIsRational(bool flag);

		///\brief Gets the number of control vertices in the spline.
		///
		///@return The number of control vertices.
		U32	GetNumCVs() const;

		///\brief Returns a given control vertex instance.
		///
		///@param instance The index of the control vertex to return.
		///@return The control vertex.
		Position3& GetCV(U32 instance);

		///\brief Returns a given control vertex instance.
		///
		///@param instance The index of the control vertex to return.
		///@return The control vertex.
		const Position3& GetCV(U32 instance) const;

		Position3Array& GetCVArray();

		const Position3Array& GetCVArray() const;

		///\brief Returns a given knot instance.
		///
		///@param instance The index of the knot to return.
		///@return The knot.
		F32	GetKnot(U32 instance) const;

		F32Array& GetKnotArray();

		const F32Array& GetKnotArray() const;

		///\brief Returns the form of this spline.
		///
		///The form can be AFTK_SPLINE_FORM_OPEN, AFTK_SPLINE_FORM_CLOSED or AFTK_SPLINE_FORM_PERIODIC.
		///These are the NURBS curve forms supported by Maya.
		///
		///@return The spline form.
		E_SplineForm GetSplineForm() const;

		///\brief Returns the spline type. Only NURBS splines are currently supported.
		///
		///Currently the only spline type value is AFTK_SPLINE_TYPE_NURBS.
		///@return The spline type.
		E_SplineType GetSplineType() const;

		///\brief Returns the degree of ths spline.
		///
		///In other words this shows whether the spline is linear, quadratic, cubic etc.
		///@return The spline degree.
		U32 GetDegree() const;

		///\brief Returns the number of knots in the spline.
		///
		U32	GetNumKnots() const;

		///\brief Returns the splines rationality.
		///
		///@return True if rational, False if not.
		bool IsRational() const;

		///\brief Removes the control vertex at the given index.
		///
		///@param instance The index of the control vertex to delete.
		void DeleteCV(U32 instance);

		///\brief Removes the last control vertex from the control vertex array.
		///
		void DeleteCV();

		///\brief Generates the required knots for a linear spline using the current control vertex array.
		///
		void GenerateLinearFromCVs();

		///\brief Evaluates the curve for the parameter x.
		///
		///@return result
		bool Evaluate(Position3& value, const F32 u) const;

		bool EvaluateTotalLength(F32& length) const;

		bool EvaluateLength(F32& u, const F32 length) const;

		///\brief Applys a matrix to the contents of this spline.
		///
		///@param matrix The tranform to apply.
		void ApplyMatrix(const Matrix44& matrix);

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		virtual E_ObjType GetType() const;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const;

		bool operator== (const C_SceneSplineNode &rhs) const;

		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Clears the contents of this node to their initial state.
		///
		virtual void Clear();

		///\brief Generate hash of data in current class
		///
		///@return 32b hash value
		U32 GetHashOfThis() const;

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		bool IsEqualToThis(const I_Object &rhs, U32 flags = E_Comparison_LocalTransform) const;

		virtual void DirtyGeometryDependantCaches() const;

		void SetNumSubDivisions(U32 numSubDivisions);

		U32 GetNumSubDivisions() const;

		bool GetEvaluationStepSize(F32& stepSize) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		virtual void CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const;

		bool BuildEvaluationHelpers() const;

		void ClearEvaluationHelpers() const;

		E_SplineForm									m_form;
		E_SplineType									m_splineType;
		U32												m_degree;
		Ut::UtVector< Ut::DynamicVector<Position3> >	m_cvs;
		Ut::UtVector< Ut::DynamicVector<F32> >			m_knots;
		bool											m_rational;
		mutable C_EvaluateSpline*						m_pEvaluationHelper;
		U32												m_numSubDivisions;

		FRIEND_CLASS C_NativeSceneSplineNode;

	#endif
	};
}
#endif
