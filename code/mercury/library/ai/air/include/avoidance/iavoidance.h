#ifndef _AVOIDANCE_I_H
#define _AVOIDANCE_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>


namespace airacingtech
{


	class IData;
	class RouteChangeMSG;
	class IGameServerTime;
	class RouteInfo;
	class IVehicleState;
	class VehicleState;
	class ISplineTracker;
	class ISplineFollower;
	

	class AvoidCar
	{
	public:

		AvoidCar(){Reset();};
		~AvoidCar(){};

		enum AVOIDCARWHERE
		{
			NoValue = 0,
			Ahead,
			Behind,
			Alongside
		};


		IData			*m_pIData;
		IVehicleState	*m_pIVehicleState;
		VehicleState	*m_pVehicleState;
		ISplineTracker	*m_pSplineTracker;
		ISplineFollower *m_pISplineFollower;
		const RouteInfo	*m_pRouteInfo;

		int				m_AheadBehindAlongside;
		int				m_RouteIndex;
		float			m_splineZDist;	
		float			m_splineXDist;	
		float			m_worldZDist;	
		float			m_worldXDist;
		float			m_relativeSpeed;
		float			m_splinePredition;
		float			m_linearPrediction;
		float			m_vehicleLength;
		float			m_vehicleWidth;
		float			m_actualSplineXDist;
		float			m_actualSplineZDist;
		bool			m_leftConstraintSet;
		bool			m_rightConstraintSet;
		float			m_leftDistance;
		float			m_rightDistance;
		float			m_trackSpeedPercentage;

		inline bool IsValid(){return (m_pIData != 0);}

		void Reset()
		{
			m_pIData = 0; 
			m_pIVehicleState = 0; 
			m_pVehicleState = 0;
			m_pSplineTracker = 0;
			m_pISplineFollower = 0;
			m_AheadBehindAlongside = NoValue;
			m_pRouteInfo = 0;

			// do we really need to clear all these variables ???
			m_splineZDist = FLT_MAX;	
			m_splineXDist = FLT_MAX;	
			m_worldZDist = FLT_MAX;	
			m_worldXDist = FLT_MAX;	

			m_relativeSpeed = FLT_MAX;
			m_splinePredition = FLT_MAX;
			m_linearPrediction = FLT_MAX;

			m_vehicleLength = FLT_MAX;
			m_vehicleWidth = FLT_MAX;

			m_RouteIndex = -1;

			m_actualSplineXDist = FLT_MAX;
			m_actualSplineZDist = FLT_MAX;

			m_leftConstraintSet = false;
			m_rightConstraintSet = false;

			m_leftDistance = FLT_MAX;
			m_rightDistance = FLT_MAX;
			m_trackSpeedPercentage = 0.0f;
		}

	};

	BEGIN_INTERFACE_DEC(IAvoidance)

	public:
		
		virtual void Update() = 0;
		virtual void PreUpdate() = 0;

		virtual int GetOverallSituation() const = 0;
		virtual AvoidCar *GetAvoidCar(int whichOne) = 0;
	
	END_INTERFACE_DEC

}

#endif //_AVOIDANCE_I_H
