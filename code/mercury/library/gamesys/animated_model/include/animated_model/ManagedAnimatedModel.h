#ifndef ANIMATEDMODEL_MANAGEDANIMATEDMODEL_H
#define ANIMATEDMODEL_MANAGEDANIMATEDMODEL_H

#include <animated_model/TaskManager.h>

namespace Animation
{
	class C_CommandList;
	class C_Cycle;
	class C_CycleInstance;
	class C_Pose;
	class C_PoseFormat;
	class C_PoseMatrixCache;
	class C_Skeleton;
	class C_SkeletonInstance;
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
	Manages a C_AnimatedModelInstance together with an animation Pose, PoseMatrixCache,
	SkeletonInstance, CommandList and CycleInstances.
	
	Each C_ManagedAnimatedModel object can be safely destroyed at any point
	during a frame, as the data it manages is kept allocated until it
	can be safely deleted once other threads have finished with it.
	*/
	class C_ManagedAnimatedModel
	{
	public:
		C_ManagedAnimatedModel();
		
		C_ManagedAnimatedModel(
			const GameResource::C_ModelResource* modelResource,
			U32 modelIndex,
			SceneGraph::C_SceneGraph* pSceneGraph,
			const Animation::C_PoseFormat& poseFormat,
			const Animation::C_Skeleton* skeleton = NULL);
			
		~C_ManagedAnimatedModel();
	
		/// Creates a new managed animated model and initialises its
		/// animation objects.
		void Create(
			const GameResource::C_ModelResource* modelResource,
			U32 modelIndex,
			SceneGraph::C_SceneGraph* pSceneGraph,
			const Animation::C_PoseFormat& poseFormat,
			const Animation::C_Skeleton* skeleton = NULL);
			
		/// Creates a new managed animated model and initialises its
		/// animation objects.
		void Create(
			SceneGraph::C_SceneGraph* pSceneGraph,
			const Animation::C_PoseFormat& poseFormat,
			const Animation::C_Skeleton* skeleton = NULL);	
			
		/// Adds a part to the managed animated model.
		void AddPart(const GameResource::C_ModelResource* modelResource, U32 modelIndex);
		
		/// Returns the number of parts in the animated model.
		U32 GetNumParts() const;
		
		/// Sets the world matrix of all the parts.
		void SetWorldMatrix(const Maths::HMat43& worldMatrix);
		
		/// Inserts the animated model into the scene graph.
		void InsertIntoSceneGraph();
		
		/// Removes the animated model from the scene graph.
		void RemoveFromSceneGraph();
		
		/// Returns a const pointer to the model instance used for a particular part.
		const WorldRender::C_ModelInstance* GetModelInstance(U32 partIndex) const;

		/// Returns a pointer to the model instance used for a particular part.
		WorldRender::C_ModelInstance* GetModelInstance(U32 partIndex);
			
		/// Destroys the animated model at any point during a frame. 
		void Destroy();
		
		/// Creates a cycle instance to be managed by this model.
		Animation::C_CycleInstance* CreateCycleInstance(const Animation::C_Cycle* cycle);
		
		/// Returns number of cycle instances currently owned by this model.
		U32 GetNumCycleInstances() const;
		
		/// Returns pointer to a previously created cycle instance.
		Animation::C_CycleInstance* GetCycleInstance(U32 index);
		
		/// Asynchronously updates the animated model and evaluates its animation.
		void UpdateAsync(
			E_Sync syncPoint,
			const Maths::HMat43& worldMatrix,
			const Animation::C_Skeleton* skeleton = NULL,
			bool forceSchedule = false
		);
		
		/// Returns true if this model has been created.
		bool IsCreated() const		{ return m_impl != NULL; }
	
		/// Returns the animation pose.
		Animation::C_Pose&			GetPose();
		
		/// Returns a const version of the animation pose.
		const Animation::C_Pose&	GetPose() const;
		
		/// Returns the animation pose matrix cache.
		Animation::C_PoseMatrixCache&			GetPoseMatrixCache();
		
		/// Returns a const version of the animation pose matrix cache.
		const Animation::C_PoseMatrixCache&		GetPoseMatrixCache() const;

		/// Returns a the animation command list.
		Animation::C_CommandList&		GetCommandList();
		
		/// Returns a const version of the animation command list.
		const Animation::C_CommandList& GetCommandList() const;
		
		/// Returns the animated model instance.
		AnimatedModel::C_AnimatedModelInstance&			GetAnimatedModelInstance();
		
		/// Returns a const version of the animated model instance.
		const AnimatedModel::C_AnimatedModelInstance&	GetAnimatedModelInstance() const;	
		
		/// Returns a const version of the SkeletonInstance.
		const Animation::C_SkeletonInstance& GetSkeletonInstance() const;
		
		/// Returns the SkeletonInstance.
		Animation::C_SkeletonInstance& GetSkeletonInstance();
	
	private:
		struct C_Impl;
		C_Impl*	m_impl;
	};

}


#endif
