#ifndef PREVIEWSTATE_H
#define PREVIEWSTATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/skeleton/Skeleton.h>
#include <animation/cycle/Cycle.h>
#include <animation/cycle/CycleInstance.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseFormat.h>
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/tools/ExportConstraints.h>
#include <animation/tools/Status.h>
#include <core/maths/hvec/new/IncludeMinimal.h>
#include <btl/containers/bitvector.h>
#include <Aftk/Aftk.h>
#include <sigslot/sigslot.h>
#include "SkinnedMesh.h"
#include "ResourceCache.h"
#include "AnimationGraph.h"
#include "DynamicsContext.h"
#include "Camera.h"

namespace Animation
{
	class C_Pose;
	class C_JointTransform;
	class C_ObjectTransform;
}

namespace AnimationTools
{
	class C_SceneConfig;
}

class C_AnimationGraphEditor;
class C_XtDatabase;

enum E_CameraMode
{
	E_CameraMode_OrbitCharacter	 =0,
	E_CameraMode_OrbitSelected,
	E_CameraMode_AttachToJoint,
	E_CameraMode_Count
};

#define USE_SPRING_JOINT

//typedef Animation::C_SpringJoint TDynamicJoint;
//typedef Animation::C_SliderJoint TDynamicJoint;
//typedef TDynamicJoint Animation::C_DynamicJoint;


class C_ClipPreviewState: public sigslot::has_slots<>
{
public:
	C_ClipPreviewState( C_XtDatabase& db, C_AnimationGraphEditor& animGraphEditor );
	~C_ClipPreviewState();

	bool IsValid() const;
	
	bool HasValidGlobalSkeleton() const;
	
	void Reset();
	
	void InitialiseDynamicJoints();
	void UpdateDynamicJoints( F32 deltaTime );
	
	void Update(F32 dt);
	void UpdateCamera( F32 dt );
	void SetCameraMode( E_CameraMode mode );
	E_CameraMode GetCameraMode() const		{ return m_cameraMode; }
	
	F32 GetGlobalTime() const	{ return m_globalTime / 1000.f; }

	F32 GetTime() const	{ return 0.f; } // gLayerStack.GetFirstActiveLayer() ? gLayerStack.GetFirstActiveLayer()->m_time : m_animTime; }

	U32 GetNumJoints() const;

	Animation::C_JointTransform GetModelJointTransform(U32 jointIndex) const;
	Animation::C_ObjectTransform GetModelObjectTransform(U32 jointIndex) const;

	const Animation::C_Skeleton& GetGlobalSkeleton() const;
	
	const Animation::C_Pose& GetGlobalPose() const		{ return m_pose; }
	
	AnimationTools::C_Status LoadGlobalSkeleton( const char* filename, const AnimationTools::C_SceneConfig& sceneConfig );
	
	const C_SkinnedMesh& GetSkinnedMesh() const	{ return *C_ResourceCache::Instance().GetSkinnedMesh(); }
	C_SkinnedMesh& GetSkinnedMesh() { return *C_ResourceCache::Instance().GetSkinnedMesh(); }
		
	void SetSelectedJointIndex( S32 jointIndex );
	S32 GetSelectedJointIndex() const				{ return m_selectedJointIndex; }
	
	void SetHideUnselectedJoints( bool hide )		{ m_hideUnselectedJoints = hide; }
	bool GetHideUnselectedJoints() const			{ return m_hideUnselectedJoints; }
	
	void SetTimelineValue(F32 value);
	
	void SetSceneScale( F32 scale );
	F32 GetSceneScale() const;
	
	void DestroyGraphicsResources();
	
	void SetLoading( bool loading = true )	{ m_loading = loading; }
	bool IsLoading() const	{ return m_loading; }
	
	void UpdateFrameRate(F32 deltaTime);
	F32 GetFrameRate() const	{ return m_frameRate; }
	
	Animation::C_JointTransform	motionDelta;
	Animation::C_JointTransform	rootTransform;
	
	Maths::Point3				centre;							// Centre relative to root.
	F32							radius;							// Radius of base pose.
	
	F32							playbackSpeed;
	
	
	Btl::BitVector				filteredJoints;
	
	bool						paused;
	bool						poseInWorldSpace;
	bool						freezeRoot;
	bool						absoluteRootMotion;
	bool						enableCommandListPushPopElimination;
	bool						commandListOk;
	
	bool						hudShowCommandList;
	bool						hudShowCustomCurves;
	bool						hudShowSelectedJointTransform;
	bool						showSkeleton;
	bool						showSkinnedMesh;
	bool						showWireframe;
	bool						showJointNames;
	bool						showOnlySelectedJointName;
	bool						showWorldAxes;
	bool						hideUnselectedJoints;
	bool						showConstraints;
	bool						requestReloadAll;
	
	C_Camera					camera;
	
	AnimationTools::C_ExportConstraints constraints;
	Btl::Vector<U32>					selectedConstraints;
	
	

public:
	
	AnimationGraph::C_ControllerState	controllerState;
	

private:
	E_CameraMode						m_cameraMode;
	C_XtDatabase&						m_db;
	C_AnimationGraphEditor&				m_animGraphEditor;
	Aftk::C_Scene						m_scene;
	const Animation::C_Skeleton	*		m_globalSkeleton;	// Global skeleton.
	S32	 m_selectedJointIndex;
	F32									m_animTime;
	F32									m_spinAngle;
	
	
	// Main interface classes.
public:
	U16									m_customCurveChecksum;
	Animation::C_PoseFormat				m_poseFormat;
	Animation::C_Pose					m_pose;					// Joint transforms.
	Animation::C_SkeletonInstance		m_skeletonInstance;		// Per-instance skeleton data.
	
	C_DynamicsContext					m_dynamicsContext;		// Controls forces applied to dynamic joints in SkeletonInstance.
	
private:	
	Btl::Vector< Maths::Mat43 >			m_worldJointMatrices;
	Btl::Vector<Animation::C_JointTransform>	m_modelJointTransforms;
	
	F32 m_deltaTime;
	U32 m_globalTime;
	F32 m_smoothedDeltaTime;
	F32 m_frameRate;

	bool m_isValid;
	bool m_focusOnSelectedJoint;
	bool m_cameraOnSelectedJoint;
	bool m_hideUnselectedJoints;
	bool m_loading;
};

inline bool C_ClipPreviewState::HasValidGlobalSkeleton() const
{
	return m_globalSkeleton ? m_globalSkeleton->GetNumJoints() > 0 : false;
}

extern C_ClipPreviewState* gClipPreviewState;


#endif



