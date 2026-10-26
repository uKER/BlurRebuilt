#ifndef AFTK_SCENE_CAMERA_NODE_H
#define AFTK_SCENE_CAMERA_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
	class C_NativeSceneCameraNode;

	class C_SceneCameraNode : public C_SceneNode
	{
	public:

		///\brief Constructor.
		///
		///@param pScene The scene to which this node is attached.
		C_SceneCameraNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs The node to copy.
		C_SceneCameraNode(const C_SceneCameraNode &rhs);

		///\brief Destructor.
		///
		virtual ~C_SceneCameraNode();

		///\brief Asignment operator.
		///
		///@param rhs The node to copy.
		///@return reference to this node.
		const C_SceneCameraNode& operator=(const C_SceneCameraNode &rhs);

		void SetVerticalFieldOfView(F32 fov);

		F32 GetVerticalFieldOfView() const;

		void SetHorizontalFieldOfView(F32 fov);

		F32 GetHorizontalFieldOfView() const;

		C_StaticPlug<F32>& GetFocalLengthPlug();
		
		const C_StaticPlug<F32>& GetFocalLengthPlug() const;

		//Connect this to a constraint node for target camera support.
		C_StaticPlug<F32>& GetCentreOfInterestPlug();

		const C_StaticPlug<F32>& GetCentreOfInterestPlug() const;

		C_StaticPlug<F32>& GetHorizontalFilmAperturePlug();
		
		const C_StaticPlug<F32>& GetHorizontalFilmAperturePlug() const;
		
		C_StaticPlug<F32>& GetVerticalFilmAperturePlug();

		const C_StaticPlug<F32>& GetVerticalFilmAperturePlug() const;

		C_StaticPlug<F32>& GetShutterAnglePlug();

		const C_StaticPlug<F32>& GetShutterAnglePlug() const;

		C_StaticPlug<F32>& GetNearClipPlanePlug();

		const C_StaticPlug<F32>& GetNearClipPlanePlug() const;

		C_StaticPlug<F32>& GetFarClipPlanePlug();

		const C_StaticPlug<F32>& GetFarClipPlanePlug() const;

		C_StaticPlug<F32>& GetOrthographicWidthPlug();

		const C_StaticPlug<F32>& GetOrthographicWidthPlug() const;

		C_StaticPlug<F32>& GetFStopPlug();

		const C_StaticPlug<F32>& GetFStopPlug() const;

		C_StaticPlug<F32>& GetFocusDistancePlug();

		const C_StaticPlug<F32>& GetFocusDistancePlug() const;

		void SetIsOrthographic(bool isOrthographic);

		bool IsOrthographic() const;

		void SetDepthOfFieldEnabled(bool depthOfFieldEnabled);

		bool GetDepthOfFieldEnabled() const;

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

		bool operator== (const C_SceneCameraNode &rhs) const;

		virtual bool IsEqualTo(
			const I_Object &rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown,
			U32 flags = E_Comparison_LocalTransform) const;

		///\brief Clears the contents of this class to their initial state.
		///
		virtual void Clear();

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		I_Plug* GetStaticPlugPtr(U32 instance)
		{
			return const_cast<I_Plug*>(((const C_Node*)this)->GetStaticPlugPtr(instance));
		}

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual const I_Plug* GetStaticPlugPtr(U32 instance) const;

		///\brief Returns the number of static plugs on this node.
		///
		///@return The number of dynamic plugs.
		virtual U32	GetNumStaticPlugs() const;

		bool IsEqualToThis(const I_Object& rhs, U32 flags = E_Comparison_LocalTransform) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(
			const I_Object& rhs, 
			E_ObjType classToAbortTest = E_ObjType_Unknown, 
			U32 flags = E_Comparison_LocalTransform) const;

		C_StaticPlug<F32>	m_focalLength;
		//Connect this to a constraint node for target camera support.
		C_StaticPlug<F32>	m_centreOfInterest; 
		C_StaticPlug<F32>	m_horizontalFilmAperture;
		C_StaticPlug<F32>	m_verticalFilmAperture;
		C_StaticPlug<F32>	m_shutterAngle;
		C_StaticPlug<F32>	m_nearClipPlane;
		C_StaticPlug<F32>	m_farClipPlane;
		C_StaticPlug<F32>	m_orthographicWidth;
		C_StaticPlug<F32>	m_focusDistance;
		C_StaticPlug<F32>	m_fStop;
		bool m_isOrthographic;
		bool m_depthOfFieldEnabled;

		FRIEND_CLASS C_NativeSceneCameraNode;

	#endif
	};
}

#endif
