#ifndef _DEBUG_AI_C_H
#define _DEBUG_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <debugtools/idebugai.h>


namespace airacingtech
{
	
	class IData;
	class IVehicleState;
	class VehicleState;
	class IGameServerTime;
	class IParamsAI;


	class CapturedData
	{
	public:
		CapturedData(){};
		~CapturedData(){};

		Maths::HVec3 m_Position;
		Maths::HVec3 m_Zaxis;
		Maths::HVec3 m_angularVelocity;
		Maths::HVec3 m_Velocity;

		float m_Speed;
		float m_accel;
		float m_brake;
		float m_steering;

	};


	BEGIN_COMPONENT_DEC(CDebugAI, IDebugAI)

	public:
		
		void Update();
		
	private:
		
		IData				*m_pIData;
		IVehicleState		*m_pIVehicleState;
		VehicleState		*m_pVehState;
		IGameServerTime		*m_pIGameServerTime;
		IParamsAI			*m_IParams;
		
		std::string			m_filename;
		bool				m_ok;
		float				m_lastTime;

		CapturedData		m_currentData;

		void Configure();
		void CachedInterfaces();
		void CaptureData();


		
		
	END_COMPONENT_DEC

}




#endif //_DEBUG_AI_C_H
