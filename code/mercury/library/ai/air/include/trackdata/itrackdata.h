#ifndef _TRACKDATA_AI_I_H
#define _TRACKDATA_AI_I_H

#ifdef _MSC_VER
#pragma once
#define NONINITVTABLE __declspec(novtable)
#endif

#include <ComponentSystem/componentlib.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{
	class AITrackData;
	class AITrackDataRoutes;

	BEGIN_INTERFACE_DEC(ITrackDataAI)
	
		virtual AITrackData *GetTrackData() = 0;
		virtual void CalculateTrackData() = 0;
		virtual float WorkOutOffsetToRacingSpline(AITrackDataRoutes *pRoute, Maths::HVec3 pos, int index) = 0;
		virtual float CalculateLogicPositionFromIndex(int index) = 0;


	END_INTERFACE_DEC
		
	
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

#endif //_TRACKDATA_AI_I_H
