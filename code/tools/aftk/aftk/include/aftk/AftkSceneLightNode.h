#ifndef AFTK_SCENE_LIGHT_NODE_H
#define AFTK_SCENE_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_Scene;

	class C_SceneLightNode : public C_SceneNode
	{
	public:
		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_SceneLightNode(C_Scene* pScene);

		C_SceneLightNode(const C_SceneLightNode& rhs);

		///\brief Destructor.
		///
		virtual ~C_SceneLightNode();

		const C_SceneLightNode& operator=(const C_SceneLightNode& rhs);

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		virtual E_ObjType GetType() const
		{
			return E_ObjType_SceneLight;
		}

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const
		{
			if (C_SceneNode::HasType(type))
				return true;
			if (type == C_SceneLightNode::GetType()) 
				return true;

			return false;
		}

		///\brief Sets the intensity of the light.
		///
		///@param intensity The light intensity.
		void SetIntensity(F32 intensity);

		///\brief Sets the colour of the light.
		///
		///The colour is stored as RGBA.
		///
		///@param colour The colour of the light.
		void SetColour(const Vector4F32& colour);

		///\brief Returns the light intensity.
		///
		///@return The light intensity.
		F32	GetIntensity() const;

		///\brief Returns the light colour.
		///
		///@return The light colour.
		const Vector4F32&	GetColour() const;

		///\brief Returns the light colour.
		///
		///@return The light colour.
		Vector4F32&		GetColour();

		bool operator== (const C_SceneLightNode &rhs) const;

		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Clears the contents of this node to their initial state.
		///
		virtual void Clear();

		bool IsEqualToThis(const I_Object& rhs, U32 flags = E_Comparison_LocalTransform) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		F32			m_intensity;
		Vector4F32	m_colour;

	#endif
	};
}
#endif
