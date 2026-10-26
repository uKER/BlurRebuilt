#ifndef AFTK_ENUM_H
#define AFTK_ENUM_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	enum  E_Axis
	{
		E_Axis_X = 0,
		E_Axis_Y = 1,
		E_Axis_Z = 2,
		E_Axis_W = 3
	};

	enum E_AnimHelperState
	{
		E_AnimHelperState_NotBuilt,
		E_AnimHelperState_WeightBuilt,
		E_AnimHelperState_GeoCacheWeightBuilt,
		E_AnimHelperState_NotNeeded
	};

	enum E_SplineType
	{
		E_SplineType_Nurbs = 0,

		E_SplineType_Count
	};

	enum E_SplineForm
	{
		E_SplineForm_Open = 0,
		E_SplineForm_Closed = 1,
		E_SplineForm_Periodic = 2,

		E_SplineForm_Count
	};

	enum E_FileType
	{
		E_FileType_BestGuess,
		E_FileType_Xsi,
		E_FileType_A3d,
		E_FileType_Ma,
		E_FileType_Mb,
		E_FileType_Fbx,

		E_FileType_Count,
		E_FileType_Unknown,
	};

	enum E_ExternalScene
	{
		E_ExternalScene_Maya,

		E_ExternalScene_Unknown
	};

	enum E_MaterialOrigin
	{
		E_MaterialOrigin_Editor			= 0,
		E_MaterialOrigin_Maya			= 1,
		E_MaterialOrigin_Maya_CgFX		= 2,
		E_MaterialOrigin_XSI			= 3,
		E_MaterialOrigin_SI				= 4,
		E_MaterialOrigin_Maya_CgFX7		= 5,
		E_MaterialOrigin_Maya_Hlsl_2008	= 6,

		E_MaterialOrigin_Count
	};

	enum E_LightDecay
	{
		E_LightDecay_None		= 0,
		E_LightDecay_Linear		= 1,
		E_LightDecay_Quadratic	= 2,
		E_LightDecay_Cubic		= 3,

		E_LightDecay_Count
	};

	enum E_ObjType
	{
		E_ObjType_Node						= 0,
		E_ObjType_SceneNode					= 1,
		E_ObjType_SceneMesh					= 3,
		E_ObjType_SceneSpline				= 4,
		E_ObjType_ScenePointLight			= 5,
		E_ObjType_SceneXRef					= 6,
		E_ObjType_SceneInstance				= 7,
		E_ObjType_SceneCamera				= 8,
		E_ObjType_Material					= 10,
		E_ObjType_BlindData					= 11,
		E_ObjType_Shader					= 14,
		E_ObjType_ShaderImage				= 15,
		E_ObjType_SceneDirectionalLight		= 16,
		E_ObjType_SceneSpotLight			= 17,
		E_ObjType_SceneJoint				= 18,
		E_ObjType_SceneMeshBonedInstance	= 19,
		E_ObjType_SceneMeshRemappedInstance	= 20,
		E_ObjType_SceneLodGroup				= 21,
		E_ObjType_SceneAreaLight			= 22,
		E_ObjType_SceneGeometry				= 23,
		E_ObjType_SceneLight				= 24,
		E_ObjType_SceneNonAmbientLight		= 25,
		E_ObjType_ISceneMesh				= 26,
		E_ObjType_SceneMeshRemappable		= 27,
		E_ObjType_SceneHemisphereLight		= 28,
		E_ObjType_IPlug						= 30,
		E_ObjType_NullPlug					= 31,
		E_ObjType_StaticPlug				= 32,
		E_ObjType_DynamicPlug				= 33,
		E_ObjType_IObject					= 34,
		E_ObjType_BlendWeighted				= 35,
		E_ObjType_InfluenceCluster			= 36,
		E_ObjType_ClipMixer					= 37,
		E_ObjType_Clip						= 38,
		E_ObjType_ClipSource				= 39,
		E_ObjType_Layer						= 40,
		E_ObjType_AnimationCurve			= 41,
		E_ObjType_IClip						= 42,
		E_ObjType_IEvaluatable				= 43,
		E_ObjType_UnitConversion			= 44,
		E_ObjType_Track						= 45,
		E_ObjType_ISceneConstraint			= 46,
		E_ObjType_SceneAimConstraint		= 47,
		E_ObjType_SceneLookAtConstraint		= 48,
		E_ObjType_ProxyPlug					= 49,
		E_ObjType_DistanceDim				= 50,
		E_ObjType_ScenePointConstraint		= 51,
		E_ObjType_SceneOrientConstraint		= 52,
		E_ObjType_SceneConstraint			= 53,

		E_ObjType_Count,
		E_ObjType_Unknown,
	};

	#define DEF(e, id, str) e = id,
	enum E_GenericAttributeType
	{
		#include "AftkAttributeTypeMapping.h"
		E_GenericAttributeType_Count
	};
	#undef DEF

	enum E_AnimAttribute
	{
		E_AnimAttribute_TranslateX		= 0,
		E_AnimAttribute_TranslateY		= 1,
		E_AnimAttribute_TranslateZ		= 2,
		E_AnimAttribute_RotateX			= 3,
		E_AnimAttribute_RotateY			= 4,
		E_AnimAttribute_RotateZ			= 5,
		E_AnimAttribute_ScaleX			= 6,
		E_AnimAttribute_ScaleY			= 7,
		E_AnimAttribute_ScaleZ			= 8,
		E_AnimAttribute_ShearX			= 9,
		E_AnimAttribute_ShearY			= 10,
		E_AnimAttribute_ShearZ			= 11,

		E_AnimAttribute_Count,
		E_AnimAttribute_Unknown,
	};

	enum E_ComponentClusterType
	{
		E_ComponentClusterType_Polygon = 0,

		E_ComponentClusterType_Count
	};

	enum E_CurveTangentType
	{
		E_CurveTangentType_Unknown = 0,
		E_CurveTangentType_Stepped = 1,
		E_CurveTangentType_Clamped = 2,
		E_CurveTangentType_SteppedNext = 3
	};

	enum E_CurveInfinityType
	{
		E_CurveInfinityType_Constant		= 0,
		E_CurveInfinityType_Linear			= 1,
		E_CurveInfinityType_Cycle			= 2,
		E_CurveInfinityType_CycleWithOffset	= 3,
		E_CurveInfinityType_Oscillate		= 4,
	};

	enum E_AnimCurveType
	{
		E_AnimCurveType_Linear,
		E_AnimCurveType_Hermite, //Valid for non-weighted tangents
		
		E_AnimCurveType_Count,
		E_AnimCurveType_Unknown,
	};

	enum E_OutputChannel
	{
		E_OutputChannel_Debug,
		E_OutputChannel_Info,
		E_OutputChannel_Warning,
		E_OutputChannel_Error,
		E_OutputChannel_Fatal,

		E_OutputChannel_Count
	};

	enum E_LimitType
	{
		E_LimitType_ScaleMinX,
		E_LimitType_ScaleMinY,
		E_LimitType_ScaleMinZ,
		E_LimitType_ScaleMaxX,
		E_LimitType_ScaleMaxY,
		E_LimitType_ScaleMaxZ,

		E_LimitType_RotateMinX,
		E_LimitType_RotateMinY,
		E_LimitType_RotateMinZ,
		E_LimitType_RotateMaxX,
		E_LimitType_RotateMaxY,
		E_LimitType_RotateMaxZ,

		E_LimitType_TranslateMinX,
		E_LimitType_TranslateMinY,
		E_LimitType_TranslateMinZ,
		E_LimitType_TranslateMaxX,
		E_LimitType_TranslateMaxY,
		E_LimitType_TranslateMaxZ,

		E_LimitType_Count
	};

	enum E_RotationOrder
	{
		E_RotationOrder_XYZ,
		E_RotationOrder_YZX,
		E_RotationOrder_ZXY,
		E_RotationOrder_XZY,
		E_RotationOrder_YXZ,
		E_RotationOrder_ZYX,
	};

	enum E_UpVectorType
	{
		E_UpVectorType_None,
		E_UpVectorType_SceneUp,
		E_UpVectorType_ObjectUp,
		E_UpVectorType_ObjectRotationUp,
		E_UpVectorType_Vector,

		E_UpVectorType_Count
	};

	enum E_ComparisonFlags
	{
		E_Comparison_LocalTransform = 1,
		E_Comparison_WorldTransform = 2
	};

	enum E_RotationInterpolation
	{
		E_RotationInterpolation_Independent,
		E_RotationInterpolation_Quaternion
	};

	typedef Ut::UtVector< Ut::DynamicVector<E_ObjType> > ObjTypeArray;
}

#endif
