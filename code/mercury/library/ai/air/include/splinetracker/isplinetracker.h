#ifndef _SPLINETRACKER_AI_I_H
#define _SPLINETRACKER_AI_I_H

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
	class RouteInfo;
	class ResetInfo;

	BEGIN_INTERFACE_DEC(ISplineTracker)

	public:
		
		virtual void Update() = 0;
		virtual const RouteInfo *GetRouteInfo(int index) const = 0;
		virtual const Btl::Vector<RouteInfo> &GetRouteInfo() const = 0;
		virtual const RouteInfo *GetCurrentRoute() const  = 0;
		virtual const int GetCurrentLane() const = 0;
		virtual bool IsInRoute(const RouteInfo *pRouteInfo, int &RouteIndex) = 0;
		virtual bool IsInRoute(int RouteIndex) = 0;
		virtual const RouteInfo *GetLogicRoute() const = 0;
		virtual void UpdateMultiRoutes() = 0;
		virtual const float GetCurrentOffset() const = 0;
		virtual void GetValidPosDir(Maths::HVec3 &pos, Maths::HVec3 &dir, float &speed) = 0;
		virtual ResetInfo *GetResetInfo() = 0;
		virtual bool InStartZone() = 0;
		virtual bool InInter1() = 0;
		virtual bool InInter2() = 0;
		virtual bool InFinish() = 0;
		
		virtual const RouteInfo *GetLogicInter1() const = 0;
		virtual const RouteInfo *GetLogicInter2() const = 0;
		virtual const RouteInfo *GetLogicStart() const = 0;
		virtual const RouteInfo *GetLogicFinish() const = 0;
	

	END_INTERFACE_DEC


	

}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

#endif //_SPLINETRACKER_AI_I_H
