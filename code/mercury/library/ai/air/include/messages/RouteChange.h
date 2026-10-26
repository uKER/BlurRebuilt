#ifndef _ROUTE_CHANGE_H
#define _ROUTE_CHANGE_H

#ifdef _MSC_VER
#pragma once
#endif


namespace airacingtech
{
	class RouteChangeMSG
	{
		public:
			RouteChangeMSG(float offset) :m_offset(offset) {};
			~RouteChangeMSG(){};

			float	m_offset;

	};
}



#endif //_ROUTE_CHANGE_H


