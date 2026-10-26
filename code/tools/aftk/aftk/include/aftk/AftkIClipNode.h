#ifndef AFTK_I_CLIP_NODE_H
#define AFTK_I_CLIP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class I_ClipNode : public C_Node
	{
	public:
		///\brief Constructor.
		///
		I_ClipNode(C_Scene* pScene) : C_Node(pScene) {}

		///\brief Copy constructor.
		///
		///@param rhs The clip node to copy.
		I_ClipNode(const I_ClipNode& rhs) : C_Node(rhs) {}

		///\brief Destructor.
		///
		virtual ~I_ClipNode() {}

		///\brief Find the lowest value of x for which there is a key in this clip.
		///
		///@return The lowest value of x.
		virtual F32	GetStartX() const = 0;

		///\brief Find the highest value of x for which there is a key in this clip.
		///
		///@return The highest value of x.
		virtual F32 GetEndX() const = 0;

		///\brief Return the duration of this clip in frames.
		///
		///@return The clip duration.
		virtual F32 GetLength() const = 0;

		///\brief Queries whether a specific attribute type is animated by any curves in this clip at x.
		///
		///@param attribute The attribute type for which to query.
		///@param x The value of x at which to query.
		virtual bool IsAnimated(E_AnimAttribute attribute) const = 0;

		///\brief Evaluates the curves of this clip at x to create a transformation matrix.
		///
		///@param x The value of x at which to evaluate.
		///@return The result.
		virtual bool Evaluate(F32& value, E_AnimAttribute attr, F32 x) const = 0;

		///\brief Clears the contents of this clip to their initial state.
		///
		virtual void Clear() = 0;

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///
		///@return C_Node type enum.
		virtual E_ObjType GetType() const = 0;

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const
		{
			if (C_Node::HasType(type))
				return true;
			if (type == E_ObjType_IClip) 
				return true;

			return false;
		}

	protected:
	};
}

#endif
