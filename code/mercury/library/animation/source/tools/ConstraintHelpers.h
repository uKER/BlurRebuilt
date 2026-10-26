#ifndef ANIMATION_TOOLS_CONSTRAINTHELPERS_H
#define ANIMATION_TOOLS_CONSTRAINTHELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimationTools
{

	///
	/// Assists with importing aim constraints connected to a scene node.
	///
	/// Aim constraints have the following restrictions:
	///		- Only a single constraint may affect the rotation channels of a node.
	///		- Aim constraints must affect all rotation cahnnels.
	///
	class C_AimConstraintHelper
	{
	public:
		/// Construct helper for the specified scene node.
		C_AimConstraintHelper( const Aftk::C_SceneNode& sceneNode );
	
		/// Returns true if scene node is driven by a valid aim constraint.
		bool IsValid() const;
		
		/// Get scene node being driven by the aim constraint.
		const Aftk::C_SceneNode& GetSceneNode() const;
		
		/// Get a bitfield describing constrained axes.
		U32 GetConstrainedAxes() const;
				
		/// Get the target scene node used by an aim constraint. Returns null if no valid scene node found.
		const Aftk::C_SceneNode* GetTargetSceneNode() const;
		
		/// Get the up vector type of the aim constraint.
		Aftk::E_UpVectorType GetUpVectorType() const;
		
		/// Get up vector type name.
		const char* GetUpVectorTypeName() const;
				
		/// Get an aim constraint's up vector reference scene node. This only applies to aim constraints
		/// with "Object Up" or "Object Rotation Up" up vector types.
		const Aftk::C_SceneNode* GetUpVectorReferenceSceneNode() const;
		
		MathsStorage::Float3 GetRotationOffset() const;

		MathsStorage::Float3 GetAimVector() const;

		MathsStorage::Float3 GetLocalUpVector() const;

		MathsStorage::Float3 GetReferenceUpVector() const;
		
		Aftk::E_RotationOrder GetRotationOrder() const;

		const MathsStorage::Float3& GetMinLimits() const	{ return m_minLimits; }
		
		const MathsStorage::Float3& GetMaxLimits() const	{ return m_maxLimits; }
		
	private:
		bool Import( const Aftk::C_SceneNode& sceneNode );
		
		MathsStorage::Float3	m_minLimits;
		MathsStorage::Float3	m_maxLimits;
	
		const Aftk::C_SceneNode& m_sceneNode;
		const Aftk::C_SceneAimConstraintNode* m_aimConstraintNode;
		U32 m_constrainedAxes;
	};

	
	//
	// C_WeightedInputMixin
	//
	class C_WeightedInputMixin
	{
	public:
		C_WeightedInputMixin();
	
		/// Gets the number of input nodes to the orientation constriant.
		U32 GetNumInputs() const;
		
		/// Add an input node together with a weight.
		void AddInput( const Aftk::C_SceneNode* inputNode, F32 weight );
		
		/// Get an input node.
		const Aftk::C_SceneNode* GetInputNode( U32 index ) const;
		
		/// Gets the weight associated with an input node.
		F32 GetInputWeight( U32 index ) const;	
		
	private:
		static const U32 kMaxInputs = 2;
		U32 m_numInputs;
		F32 m_inputWeights[kMaxInputs];
		const Aftk::C_SceneNode* m_inputNodes[kMaxInputs];
	};
	
	///
	/// Assists with importing orientation constraints connected to a scene node.
	///
	class C_OrientationConstraintHelper: public C_WeightedInputMixin
	{
	public:
		C_OrientationConstraintHelper( const Aftk::C_SceneNode& sceneNode );

		const MathsStorage::Float3& GetMinLimits() const;
		
		const MathsStorage::Float3& GetMaxLimits() const;
		
		const MathsStorage::Float3& GetLocalRotationOffset() const;
		
		bool IsValid() const;
		
		/// Get scene node whose rotation is driven by the orientation constraint.
		const Aftk::C_SceneNode& GetSceneNode() const;
		
		Aftk::E_RotationOrder GetRotationOrder() const;
				
	private:
	
		static const Aftk::C_SceneOrientConstraintNode* C_OrientationConstraintHelper::GetOrientConstraintNode( const Aftk::C_SceneNode& sceneNode );
	
		const Aftk::C_SceneNode&	m_sceneNode;
		const Aftk::C_SceneOrientConstraintNode* const m_orientConstraintNode;
		MathsStorage::Float3		m_minLimits;
		MathsStorage::Float3		m_maxLimits;
		MathsStorage::Float3		m_localRotationOffset;
	};	
	
	///
	/// Assists with importing position constraints connected to a scene node.
	/// 
	class C_PositionConstraintHelper: public C_WeightedInputMixin
	{
	public:
		C_PositionConstraintHelper( const Aftk::C_SceneNode& sceneNode );
		
		/// Get minimium local space translation limits.
		const MathsStorage::Float3& GetMinLimits() const;
		
		/// Get maximum local space translation limits.
		const MathsStorage::Float3& GetMaxLimits() const;
		
		/// Returns true if scene node's rotation is driven by a valid position constraint.
		bool IsValid() const;
		
		/// Get scene node whose rotation is driven by the position constraint.
		const Aftk::C_SceneNode& GetSceneNode() const;
		
		/// Get local offset.
		const MathsStorage::Float3& GetLocalOffset() const;
				
	private:
		static const Aftk::C_ScenePointConstraintNode* GetPointConstraintNode( const Aftk::C_SceneNode& sceneNode );
	
		const Aftk::C_SceneNode&	m_sceneNode;
		const Aftk::C_ScenePointConstraintNode* const m_pointConstraintNode;
		MathsStorage::Float3		m_minLimits;
		MathsStorage::Float3		m_maxLimits;
		MathsStorage::Float3		m_localOffset;
	};
	
	enum E_AnimationInputType
	{
		E_AnimationInputType_ScaleX = 0,
		E_AnimationInputType_ScaleY,
		E_AnimationInputType_ScaleZ,
		E_AnimationInputType_RotationX,
		E_AnimationInputType_RotationY,
		E_AnimationInputType_RotationZ,
		E_AnimationInputType_TranslationX,
		E_AnimationInputType_TranslationY,
		E_AnimationInputType_TranslationZ,

		E_AnimationInputType_Count
	};
	
	enum E_Axis
	{
		E_Axis_X	= 0,
		E_Axis_Y,
		E_Axis_Z,
		E_Axis_Count
		
	};
	
	///
	/// Assists with importing driven key constraints.
	///	
	class C_DrivenKeyHelper
	{
	public:
		C_DrivenKeyHelper( const Aftk::C_SceneNode& sceneNode, const Aftk::I_Plug& plug );
		
		bool IsValid() const;
		
		const Aftk::C_SceneNode& GetSceneNode() const;
		
		bool AxisIsConstrained( E_Axis axis ) const;
		
		const Aftk::C_SceneNode* GetInputNode( E_Axis axis ) const;
		
		const Aftk::C_AnimationCurveNode* GetAnimationCurveNode( E_Axis axis ) const;
		
		E_AnimationInputType GetInputType( E_Axis axis ) const;
		
		F32 GetInputScaleFactor( E_Axis axis ) const;
		
		
	private:
		bool Import( const Aftk::C_SceneNode& sceneNode, const Aftk::I_Plug& plug );
		
		const Aftk::C_SceneNode& m_sceneNode;
		const Aftk::C_SceneNode* m_inputNodes[ E_Axis_Count ];
		F32 m_inputScaleFactors[ E_Axis_Count ];
		const Aftk::C_AnimationCurveNode* m_animationCurveNodes[ E_Axis_Count ];
		E_AnimationInputType m_inputTypes[ E_Axis_Count ];
		U32 m_constrainedAxes;
		bool m_isValid;
	};
		
	///
	/// Assists with importing driven rotation key constraints connected to a scene node.
	///
	class C_DrivenRotationKeyHelper: public C_DrivenKeyHelper
	{
	public:
		C_DrivenRotationKeyHelper( const Aftk::C_SceneNode& sceneNode );
	};

	///
	/// Assists with importing driven translation key constraints connected to a scene node.
	///
	class C_DrivenTranslationKeyHelper: public C_DrivenKeyHelper
	{
	public:
		C_DrivenTranslationKeyHelper( const Aftk::C_SceneNode& sceneNode );
	};


}

#endif
