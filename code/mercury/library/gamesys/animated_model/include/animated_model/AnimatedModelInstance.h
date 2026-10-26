#ifndef ANIMATEDMODEL_ANIMATEDMODELINSTANCE_H
#define ANIMATEDMODEL_ANIMATEDMODELINSTANCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <animation/pose/PoseFieldTypes.h>
#include <renderingdata/renderenums.h>

namespace Animation
{
	class C_Pose;
	class C_PoseFormat;
	class C_PoseMatrixCache;
	class C_Skeleton;
}

namespace GameResource
{
	class C_ModelResource;
	class C_Model;
}

namespace WorldRender
{
	class C_ModelInstance;
	class C_Model;
}

namespace SceneGraph
{
	class C_SceneGraph;
	class I_ModelInstanceCullNode;
}

namespace AnimatedModel
{

/**
An instance of an animated model.
*/
class C_AnimatedModelInstance
{
public:
	friend class C_UpdateTask;

	/// Default constructor.
	C_AnimatedModelInstance();

	/// Constructs an animated model instance from an animation pose format.
	C_AnimatedModelInstance(const GameResource::C_ModelResource* modelResource, U32 modelIndex, SceneGraph::C_SceneGraph* pSceneGraph, const Animation::C_PoseFormat& poseFormat);
	
	/// Destructor.
	~C_AnimatedModelInstance();
	
	/// Creates an animated model instance without any model parts. @see AddModel
	void Create(SceneGraph::C_SceneGraph* pSceneGraph, const Animation::C_PoseFormat& poseFormat);

	/// Creates an animated model instance with a single model part.
	void Create(const GameResource::C_ModelResource* modelResource, U32 modelIndex, SceneGraph::C_SceneGraph* pSceneGraph, const Animation::C_PoseFormat& poseFormat);

	/// Adds a model resource as a part of this animated model instance and returns its index (-1 if part could not be added).
	S32 AddPart(const GameResource::C_ModelResource* modelResource, U32 modelIndex);
		
	//
	// Update.
	// 
				
	/// Prepares internal data before an Update().
	/// Only needs to be called before Update() when this animated model is to be updated on PS3 SPU.
	/// Returns false if model is not in the scene graph and the following update could be skipped.
	bool PreUpdate();
	
private:
	/// Updates animated material parameters and element transforms of model instance.
	/// The @a poseMatrixCache parameter must have been updated from the specified pose.
	/// The pose format of @a pose must be the same as that passed into the constructor of this animated model instance.
	/// See #Animation::C_PoseMatrixCache 
	void Update(const Maths::HMat43& worldMatrix, const Animation::C_Pose& pose, const Animation::C_PoseMatrixCache& poseMatrixCache);
public:
	
	/// Performs a number of main-thread only tasks after update, e.g. setting per-element visibility and updating portal classification.
	void PostUpdate(const Animation::C_Pose& pose);
	
	/// Sets the world matrix of the the model instance. Useful if the animation of the model has not changed since the last update.
	void SetWorldMatrix(const Maths::HMat43& worldMatrix);
	
	/// Inserts the managed model instance into scene graph.
	void InsertIntoSceneGraph();
	
	/// Removes from the managed model instance from the scene graph.
	void RemoveFromSceneGraph();
	
	
	//
	// Multi-threaded update.
	//
	
	/// Updates animated material parameters and element transforms of model instance using a thread/SPU task.
	/// The @a poseMatrixCache parameter must have been updated from the specified pose.
	/// The pose format of @a pose must be the same as that passed into the constructor of this animated model instance.
	/// See #Animation::C_PoseMatrixCache 
	void ScheduleUpdateTask(const Maths::HMat43& worldMatrix, const Animation::C_Pose& Pose, const Animation::C_PoseMatrixCache& poseMatrixCache);
	
	void SpuStore(C_AnimatedModelInstance* ppuInstance) const;
	
	static void Lock(C_AnimatedModelInstance* ppuInstance);
	static void Unlock(C_AnimatedModelInstance* ppuInstance);
	static bool IsLocked(const C_AnimatedModelInstance* ppuInstance);
	
#if !defined(PLATFORM_SPU)
	bool IsLocked() const		{ return IsLocked(this); }
#endif
	
	//
	// Misc.
	//
		
	/// Returns a const version of a part's scene graph model instance.
	const WorldRender::C_ModelInstance* GetModelInstance(U32 partIndex = 0) const;
	
	/// Returns a part's scene graph model instance.
	WorldRender::C_ModelInstance* GetModelInstance(U32 partIndex = 0);
	
	/// Returns the number of parts in the model.
	U32 GetNumParts() const;
	
	/// Returns true if the animated model instance uses at least one animated parameter from its pose. Returns false
	/// if the model instance is effectively static.
	bool IsAnimated() const;

	/// Returns the number of animated parameters.
	U32 GetNumAnimatedParameters() const;

	// Model visibility is maintained to be the same for all component ModelInstances.
	
	void		SetVisibilityFlags(U32 visibilityFlags);
	const U32	GetVisibilityFlags() const  { return m_VisibilityFlags;}
	void		EnableVisibilityFlags(U32 visibilityFlags);
	void		DisableVisibilityFlags(U32 visibilityFlags);
	
	/// Sets the visibility of all the elements in all the parts of the animated model.
	void SetVisibility(bool visible);

	/// Sets the visibility of all the elements in a specific part of the animated model.
	void SetVisibility(U32 partIndex, bool visible);
	
	/// Sets the near/far draw distances for all parts of the animated model.
	void SetNearFarDistance(F32 nearDist, F32 farDist);
		
	/// Returns the most recently computed world space bounding sphere.
	const Maths::C_Sphere& GetWorldSphere() const		{ return m_WorldSphere; }

	/// Returns the local space bounding sphere of the model instance before animation.
	Maths::C_Sphere GetLocalSphereWithoutAnimation() const;
	
	/// Sets the rendering data render pass mapping for each part.
	void SetRenderingDataRenderPassMapping(RenderingData::RenderPass renderingDataPass, S32 renderPass);

	/// Returns the scene graph used to create this model instance.
	SceneGraph::C_SceneGraph* GetSceneGraph() const		{ return m_pSceneGraph; }
	
private:

	struct AnimatedParameter
	{
		U8*				data;					// Direct pointer to material parameter buffer.
		U16				poseFieldIndex;			// Index of corresponding pose field.
		U16				isElement		: 1;	// 1 if this parameter is an element's world matrix, 0 otherwise.
		U16				elementIndex	: 15;	// Relative to m_Elements array.
	};
	
	struct Element
	{
		Maths::Sphere				localSphere;	// Local bounding sphere of element.
		MathsStorage::MatrixRow44*	worldMatrix;	// Element's world matrix (duplicate of AnimatedParameter data field, if animated)
		S32							visFieldIndex;	// Index of scalar animated visibility field, or < 0 if not present.
	};

	struct Part
	{
		WorldRender::C_ModelInstance*			modelInstance;		// Pointer to model instance node for this model.
		Maths::Sphere							localSphere;		// Local bounding sphere for this sub-model.
		SceneGraph::I_ModelInstanceCullNode*	cullNode;			// Direct pointer to cull node containing bounding sphere.
		U32										firstElementIndex;	// Index of first element in m_Elements array.
	};
	
	void BindParameters(
		const GameResource::C_Model* gameModel,
		const WorldRender::C_Model* worldRenderModel,
		WorldRender::C_ModelInstance* modelInstance,
		U32 firstElementIndex);
			
	void SetJointTransformParameters(
		const AnimatedParameter* params,
		U32 count,
		const Element* elements,
		const Animation::C_Pose& pose,
		const Maths::Mat43& worldMatrix,
		const Maths::Mat43* skinningMatrices,
		Maths::Sphere& outBoundingSphere) const;

	void SetVector4Parameters(
		const AnimatedParameter* params,
		U32 count,
		const Animation::C_Pose& pose) const;
		
	void SetGenericParameters(
		const AnimatedParameter* params,
		U32 count,
		const Animation::C_Pose& pose,
		Animation::E_PoseFieldType poseFieldType) const;
		
	void Destroy();
	
	void FlushIfLocked() const;
	
	U32 GetParameterSize(Animation::E_PoseFieldType fieldType) const;
	
	enum E_Flag
	{
		E_Flag_HasMoved					= 1 << 0,
		E_Flag_FirstUpdate				= 1 << 1,
		E_Flag_HasAnimatedVisibility	= 1 << 2,
		E_Flag_AwaitingPostUpdate		= 1 << 3
	};
	
	
	// --- variables modified during Update
	Maths::Sphere						m_LocalBoundingSphere;
	Maths::C_Sphere						m_WorldSphere;
	U32									m_Flags;
	// --- end
	volatile U32						m_Locked;

	Maths::Sphere						m_OriginalLocalBoundingSphere;
	SceneGraph::C_SceneGraph*			m_pSceneGraph;
	const Animation::C_PoseFormat*		m_pPoseFormat;
	
	F32									m_NearDist;
	F32									m_FarDist;

	U32									m_VisibilityFlags;			// Visibility flags, which must be the same for all parts.
	Btl::Vector<Part>					m_Parts;
	Btl::Vector<Element>				m_Elements;
	Btl::Vector<AnimatedParameter>		m_AnimatedParameters;
	U16									m_AnimatedParameterCounts[Animation::E_PoseFieldType_Count];
	S16									m_FirstSkinnedJointIndex;
		
	bool								m_InSceneGraph;
};

}

#endif
