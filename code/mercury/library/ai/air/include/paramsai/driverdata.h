#ifndef _DRIVERDATA_H
#define _DRIVERDATA_H

#ifdef _MSC_VER
#pragma once
#endif




namespace airacingtech
{

	struct OldAISpeed
	{
		float	m_brakingCoefficient;
		float	m_frictionCoefficient;
	};
	
	class DriverData
	{
	public:
		DriverData()
		{
			m_speed.m_brakingCoefficient = 1.0f;
			m_speed.m_frictionCoefficient = 1.0f;
			m_difficulty = 1.0f;
		};

		~DriverData(){};

		

		CObjectId	m_name;
		OldAISpeed		m_speed;
		
		// this is never exposed....
		float			m_difficulty;

	};
}




#endif // _DRIVERDATA_H
