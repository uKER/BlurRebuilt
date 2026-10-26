#ifndef AFTK_SCENE_JOINT_NODE_H
#define AFTK_SCENE_JOINT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;

	class C_SceneJointNode : public C_SceneNode, public C_LimitMixin
	{
	public:
		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_SceneJointNode(C_Scene* pScene);

		C_SceneJointNode(const C_SceneJointNode& rhs);

		const C_SceneJointNode& operator=(const C_SceneJointNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;
		
		///\brief Destructor.
		///
		virtual ~C_SceneJointNode();

		bool GetSegmentScaleCompensation() const;

#ifndef AFTK_LEAN_AND_MEAN

		const Vector3F32& GetStiffness() const;

		const Vector3F32& GetPreferredAngle() const;

		const Matrix44& GetScaleOrientation() const;

		const Matrix44& GetOrientation() const;

		const Vector3Bool& GetDegreesOfFreedom() const;

		F32 GetRadius() const;
		

		const Vector3F32& GetMinRotateDampRange() const;

		const Vector3F32& GetMaxRotateDampRange() const;

		const Vector3F32& GetMinRotateDampStrength() const;

		const Vector3F32& GetMaxRotateDampStrength() const;


		void SetStiffness(const Vector3F32& stiffness);

		void SetScaleOrientation(const Matrix44& matrix);

		void SetOrientation(const Matrix44& matrix);

		void SetPreferredAngle(const Vector3F32& preferredAngle);

		void SetDegreesOfFreedom(const Vector3Bool& degreesOfFreedom);
#endif
		void SetSegmentScaleCompensation(bool compensate);
#ifndef AFTK_LEAN_AND_MEAN

		void SetRadius(F32 radius);


		void SetMinRotateDampRange(const Vector3F32& range);

		void SetMaxRotateDampRange(const Vector3F32& range);

		void SetMinRotateDampStrength(const Vector3F32& strength);

		void SetMaxRotateDampStrength(const Vector3F32& strength);

#endif

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		E_ObjType GetType() const;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const;

		///\brief Clears the contents of this class to their initial state.
		///
		virtual void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		virtual void RecalculateStaticMatrixCache() const;

		virtual void RecalculateDynamicMatrixCache() const;

		bool m_segmentScaleCompensation;
#ifndef AFTK_LEAN_AND_MEAN
		Vector3F32 m_stiffness;

		Matrix44 m_scaleOrientation;
		Matrix44 m_orientation;

		Vector3F32 m_preferredAngle;
		Vector3Bool m_dof;


		F32 m_radius;

		Vector3F32 m_minDampRange;
		Vector3F32 m_maxDampRange;
		Vector3F32 m_minDampStrength;
		Vector3F32 m_maxDampStrength;

#endif
		friend class C_NativeSceneJointNode;

	#endif
	};
}
#endif
