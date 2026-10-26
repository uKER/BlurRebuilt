#ifndef PICTUREAWARD_SYSTEM_H
#define PICTUREAWARD_SYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\Achievements\AchievementEnums.h>
#include <Core/Threads/Thread.h>

namespace Game
{

	//these will get mapped to the relevant platform
class C_PictureAwardSystem
{
public:

	C_PictureAwardSystem()
	{
	};

	void Init();
	void Update();
	void GivePictureAward(int user_index, BLUR_PICTURE_AWARD picture_award);

	static C_PictureAwardSystem& Instance(){return s_instance;};

private:
	static C_PictureAwardSystem s_instance;

	int mPictureAwardMapping[BLUR_PICTURE_AWARD_MAX];

};

} // namespace Game

#endif // PICTUREAWARD_SYSTEM_H