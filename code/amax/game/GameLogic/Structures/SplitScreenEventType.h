#ifndef SPLITSCREEN_EVENT_TYPE_H
#define SPLITSCREEN_EVENT_TYPE_H

namespace UI
{
	enum E_SPLITSCREEN_EVENT_TYPE
	{
		E_SPLITSCREEN_EVENT_TYPE_NONE = -1,		//Error

		E_SPLITSCREEN_EVENT_TYPE_CLEAN,			//Racing with no power ups
		E_SPLITSCREEN_EVENT_TYPE_ELIMINATOR,	//Racing with power ups, every n seconds the person in last place is removed
		E_SPLITSCREEN_EVENT_TYPE_NITRO,			//Racing with only nitrous power ups
		E_SPLITSCREEN_EVENT_TYPE_RACE,			//Racing with power ups, vanilla Blur
		E_SPLITSCREEN_EVENT_TYPE_SURVIVOR,		//Destruction derby in an arena
		E_SPLITSCREEN_EVENT_TYPE_TARGET,		//A confused scoring mode where you win by scoring more points

		E_SPLITSCREEN_EVENT_TYPE_WORLD_TOUR,	//A string of different events

		E_SPLITSCREEN_EVENT_TYPE_MAX
	};
};

#endif //SPLITSCREEN_EVENT_TYPE_H