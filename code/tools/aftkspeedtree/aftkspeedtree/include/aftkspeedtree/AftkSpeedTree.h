#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_SPEEDTREE_H
#define AFTK_SPEEDTREE_H

#include <AftkSpeedTree\AftkSpeedTreeSetup.h>

namespace AftkSpeedTree
{
	class SpeedTree : public SpeedTreeSetup
	{
	public:
		SpeedTree();
		~SpeedTree() {};

		void ProcessAftk(Aftk::C_Scene&	aftk);
	};
}

#endif


