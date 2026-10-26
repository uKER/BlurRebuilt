#ifndef AFTK_SCENE_NON_AMBIENT_LIGHT_NODE_H
#define AFTK_SCENE_NON_AMBIENT_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_Scene;

	class C_SceneNonAmbientLightNode : public C_SceneLightNode
	{
	public:
		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_SceneNonAmbientLightNode(C_Scene* pScene);

		C_SceneNonAmbientLightNode(const C_SceneNonAmbientLightNode& rhs);

		const C_SceneNonAmbientLightNode& operator=(const C_SceneNonAmbientLightNode& rhs);
		
		///\brief Destructor.
		///
		virtual ~C_SceneNonAmbientLightNode();

		///\brief Get this node's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this node. For example, spline, camera actor etc.
		///@return C_Node type enum.
		virtual E_ObjType GetType() const
		{
			return E_ObjType_SceneNonAmbientLight;
		}

		///\brief Returns true if this node implements the given type.
		///
		///@param The node type to test against.
		///@return true if we implement the given node type.
		virtual bool HasType(E_ObjType type) const
		{
			if (C_SceneLightNode::HasType(type))
				return true;
			if (type == C_SceneNonAmbientLightNode::GetType()) 
				return true;

			return false;
		}

		///\brief Sets the attenuation proporties for this light.
		///
		///The decay can be one of the following enum values:
		///AFTK_LIGHT_DECAY_NONE, AFTK_LIGHT_DECAY_LINEAR, 
		///AFTK_LIGHT_DECAY_QUADRATIC, AFTK_LIGHT_DECAY_CUBIC.
		///@param decayType The attenuation type enum.
		void SetDecayType(E_LightDecay decayType);

		///\brief Returns the attenuation proporties for this light.
		///
		///The decay can be one of the following enum values:
		///AFTK_LIGHT_DECAY_NONE, AFTK_LIGHT_DECAY_LINEAR, 
		///AFTK_LIGHT_DECAY_QUADRATIC, AFTK_LIGHT_DECAY_CUBIC.
		E_LightDecay GetDecayType() const;

		bool operator== (const C_SceneNonAmbientLightNode &rhs) const;

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

		E_LightDecay m_decayType;
	#endif
	};
}
#endif
