#ifndef RLDEBUGLINEDATA_H
#define RLDEBUGLINEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include "RLDefinitions.h"

namespace Game
{
namespace Actors
{

class RLDebugLineData
{
//Functions
public:
	~RLDebugLineData()
	{
		Destroy();
	}

	void										Initialise()
	{
		Destroy();
	}

	void										Destroy()
	{
		for (int iLine = 0; iLine < mLines.Size(); iLine++)
			SAFE_DELETE_PTR(mLines[iLine]);
		mLines.Clear();
	}

//Variables
public:
	Btl::Vector<RLDebugLine*, Btl::VectorParameters_Static<MAX_DEBUG_LINES> > mLines;
};

};
};

#endif //RLDEBUGLINEDATA_H
