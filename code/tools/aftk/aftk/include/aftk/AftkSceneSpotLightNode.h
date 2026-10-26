#ifndef AFTK_SCENE_SPOT_LIGHT_NODE_H
#define AFTK_SCENE_SPOT_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
	class C_NativeSceneSpotLightNode;

	class C_SceneSpotLightNode : public C_SceneNonAmbientLightNode
	{
	public:
		///\brief Constructor.
		///
		///In order for a scene node to be valid you must supply a valid scene pointer.
		///
		///@param pScene C_Scene pointer.
		C_SceneSpotLightNode(C_Scene* pScene);

		C_SceneSpotLightNode(const C_SceneSpotLightNode& rhs);

		const C_SceneSpotLightNode& operator=(const C_SceneSpotLightNode& rhs);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Destructor.
		///
		virtual ~C_SceneSpotLightNode();

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

		///\brief Sets the cone angle for this light.
		///
		///The cone angle is the angle of the spot lights exterior cone in degrees.
		///@param coneAngle The cone angle.
		void SetConeAngle(F32 coneAngle);

		///\brief Sets the penumbra angle of this light.
		///
		///The penumbra angle is the angle between the outer and inner spot light cone in degrees.
		///@param penumbraAngle The penumbra angle.
		void SetPenumbraAngle(F32 penumbraAngle);

		///\brief Sets the light drop off.
		///
		///@param dropOff The drop off.
		void SetDropOff(F32 dropOff);

		///\brief Sets the cut-off radius of this light.
		///
		///This is a visual prop used by the artists to define the intensity of the light within a 3D package.
		///It has no basis in reality.
		///@param cutOffRadius
		void SetCutOffRadius(F32 cutOffRadius);

		///\brief Returns the cone angle of this light.
		///
		///The cone angle is the angle of the spot lights exterior cone in degrees.
		///@return The cone angle.
		F32 GetConeAngle() const;
		
		///\brief Returns the penumbra angle.
		///
		///The penumbra angle is the angle between the outer and inner spot light cone in degrees.
		///@return the penumbra angle.
		F32 GetPenumbraAngle() const;

		///\brief Returns the drop off.
		///
		///@return The drop off.
		F32 GetDropOff() const;

		///\brief Returns the cut-off radius of this light.
		///
		///This is a visual prop used by the artists to define the intensity of the light within a 3D package.
		///It has no basis in reality.
		///@return The cut-off radius.
		F32 GetCutOffRadius() const;

		bool operator== (const C_SceneSpotLightNode &rhs) const;

		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Clears the contents of this node to their initial state.
		///
		virtual void Clear();

		bool IsEqualToThis(const I_Object &_rhs, U32 flags = E_Comparison_LocalTransform) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		F32 m_coneAngle;
		F32 m_penumbraAngle;
		F32 m_dropOff;
		F32 m_cutOffRadius;

		FRIEND_CLASS C_NativeSceneSpotLightNode;
	#endif
	};
}
#endif
