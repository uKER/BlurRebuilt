#ifndef CROWD_ACTOR_UPDATE_TASK_H
#define CROWD_ACTOR_UPDATE_TASK_H

#include <crowd_system/CrowdSpectatorUpdateData.h>

namespace AnimatedCrowd
{

//! C_CrowdActorUpdateTask
class ALIGN16 C_CrowdActorUpdateTask
{
public:
	C_CrowdActorUpdateTask();

	static bool	Run(C_CrowdActorUpdateTask* pTask);
	static bool	Schedule(C_CrowdActorUpdateTask* pTask, S32 group, S32 policy);

private:
	static void	Fetch(C_SpectatorUpdateInput& input);
	static void	Fetch(C_SpectatorUpdateOutput& output, const U32 numBlocks);

public:
	C_SpectatorUpdateInput	m_SpectatorUpdateInput;
	C_SpectatorUpdateOutput	m_SpectatorUpdateOutput;
};

}; // namespace AnimatedCrowd

#endif // CROWD_ACTOR_UPDATE_TASK_H
