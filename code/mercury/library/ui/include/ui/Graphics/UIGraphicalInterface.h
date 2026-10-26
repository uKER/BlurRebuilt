
#ifndef UIGRAPHICALINTERFACE_H
#define UIGRAPHICALINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace UI

{

/*
	This class is to abstract the different back-ends of different game-engines, where the actual meat and potato's has to be defined in the
	source of the game using.
*/
class I_UIGraphicalInterface
{
public:
	static bool					WideScreen();		// Is the UI running in Widescreen?
	static void					ClearSetResources(void);
};

}

#endif
