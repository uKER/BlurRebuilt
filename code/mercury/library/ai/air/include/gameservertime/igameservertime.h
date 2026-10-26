#ifndef _GAMESERVERTIME_AI_I_H
#define _GAMESERVERTIME_AI_I_H

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

	BEGIN_INTERFACE_DEC(IGameServerTime)

	public:

		virtual float GetTime() const = 0;
		virtual void SetTime( float t ) = 0;
		virtual void IncrementTime() = 0;
		virtual void SetFrameRate(int rate) = 0;
		virtual void SetTimeAccum(float t) = 0;
	
	END_INTERFACE_DEC

}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

#endif //_GAMESERVERTIME_AI_I_H
