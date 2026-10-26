#ifndef ANIMATEDMODEL_UPDATETASK_H
#define ANIMATEDMODEL_UPDATETASK_H

#include <animation/task/EvaluationTask.h>

namespace Animation
{
	class C_Skeleton;
	class C_Pose;
	class C_PoseMatrixCache;
	class C_CommandList;
}

namespace AnimatedModel
{

	class C_AnimatedModelInstance;
	

	class ALIGN16 C_UpdateTask
	{
	public:
		
		Maths::Mat43					worldMatrix;
		C_AnimatedModelInstance*		animatedModelInstance;
		const Animation::C_Skeleton*	skeleton;
		Animation::C_Pose*				pose;
		Animation::C_PoseMatrixCache*	poseMatrixCache;
		Animation::C_CommandList*		commandList;	
		
		C_UpdateTask();
		
		bool IsValid() const;
		
		void Invalidate();
		
		bool Schedule(S32 group, S32 policy);
		
		static bool Schedule(const C_UpdateTask* tasks, U32 numTasks, S32 group, S32 policy);
		
		void Run() const;
		
		static void Run(const C_UpdateTask* tasks, U32 numTasks);

		void PostUpdate() const;

	private:
		void ReleaseLocks() const;
	};


} // end AnimatedModel



#endif
