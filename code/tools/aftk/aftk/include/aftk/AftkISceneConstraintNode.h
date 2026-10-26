#ifndef AFTK_I_SCENE_CONSTRAINT_NODE_H
#define AFTK_I_SCENE_CONSTRAINT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class I_SceneConstraintNode : public C_SceneNode
	{
	public:
		virtual C_Node* Clone(C_Scene* pScene) const = 0;

		virtual ~I_SceneConstraintNode() {}

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const
		{
			return C_SceneNode::GetHash(classToAbortTest);
		}

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		virtual E_ObjType GetType() const = 0;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const
		{
			if (C_SceneNode::HasType(type))
				return true;
			if (type == E_ObjType_ISceneConstraint) 
				return true;

			return false;
		}

		virtual bool IsUsed() const 
		{
			return C_SceneNode::IsUsed();
		}

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(
			const I_Object &rhs,
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const
		{
			return IsEqualToAll(rhs, classToAbortTest, flags);
		}

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object &rhs, U32 flags = E_Comparison_LocalTransform) const
		{
			return true;
		}

		virtual void Clear()
		{
			C_SceneNode::Clear();
		}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const
		{
			return C_SceneNode::IsEqualToAll(rhs, classToAbortTest, flags);
		}

		///\brief Default constructor.
		///
		I_SceneConstraintNode(C_Scene* pScene) : C_SceneNode(pScene) {}
	#endif
	};
}

#endif

/* XSI constraint types
	enum EConstraintType
	{
		SI_SCALING,			//!<Scaling constraint
		SI_ORIENTATION,		//!<Orientation constraint
		SI_DIRECTION,		//!<Direction constraint
		SI_POSITION,		//!<Position constraint
		SI_INTEREST,		//!<Interest constraint
		SI_UP_VECTOR,		//!<Up Vector constraint
		SI_PREFERED_AXIS,	//!<Prefered Axis constraint
	};

	Maya constraint types
	geometryConstraint,
	normalConstraint,
	orientConstraint, 
	parentConstraint,
	pointConstraint,
	poleVectorConstraint,
	tangentConstraint, 
	scaleConstraint,
	lookAt 
*/
