#ifndef AVATARAWARD_SYSTEM_H
#define AVATARAWARD_SYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\Achievements\AchievementEnums.h>
#include <Core/Threads/Thread.h>

namespace Game
{

	//these will get mapped to the relevant platform
class C_AvatarAwardSystem
{
public:

	C_AvatarAwardSystem()
	{
	};

	void Init();
	void Update();
	void GiveAvatarAward(int user_index, BLUR_AVATAR_AWARD avatar_award);

	static C_AvatarAwardSystem& Instance(){return s_instance;};

private:
	static C_AvatarAwardSystem s_instance;

	int mAvatarAwardMapping[BLUR_AVATAR_AWARD_MAX];

};

} // namespace Game

#endif // AVATARAWARD_SYSTEM_H