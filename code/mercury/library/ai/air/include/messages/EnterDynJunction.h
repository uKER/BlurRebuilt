#ifndef _ENTER_DYNAMIC_JUNCTION_H
#define _ENTER_DYNAMIC_JUNCTION_H

#include <ComponentSystem/CObjectId.h>

#ifdef _MSC_VER
#pragma once
#endif


namespace airacingtech
{
	class AITrackDataRoutes;

	class EnterDynamicJunctionMSG
	{
	public:
		EnterDynamicJunctionMSG(AITrackDataRoutes *pRoute) : m_pRoute(pRoute) {};
		~EnterDynamicJunctionMSG(){};

		AITrackDataRoutes *m_pRoute;

	};

	class SetRouteMSG
	{
	public:

		SetRouteMSG(bool active, CObjectId	routeID) : m_active(active), m_routeID(routeID) {};
		~SetRouteMSG(){};

		bool	m_active;
		CObjectId	m_routeID;

	};
}



#endif //_ENTER_DYNAMIC_JUNCTION_H