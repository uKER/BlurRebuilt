#ifndef _VEHICLELOGIC_AI_C_H
#define _VEHICLELOGIC_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <vehiclelogic/ivehiclelogic.h>


namespace airacingtech
{
	class IGameServerTime;

	class LapTime
	{
		public:

			LapTime(){Reset();};
			~LapTime(){};

			inline float GetStartTime(){return m_startLap;}

			inline void Reset()
			{
				m_intermediate1 = -1.0f;
				m_intermediate2 = -1.0f;
				m_intermediate3	= -1.0f;
				m_illegal = false;
				m_Laptime = -1.0f;
				m_startLap = -1.0f;
				m_realLapTime = -1.0f;
			}

			inline void SetRealLapTime(float laptime){m_realLapTime = laptime;}
			inline float GetRealLapTime(float laptime){return m_realLapTime;}

			inline void CalculateRealLapTime(float serverTime)
			{
				SetRealLapTime(-1.0f);
				if (m_startLap != -1.0f)
				{
					SetRealLapTime(serverTime - m_startLap);
				}			
			}

			inline bool IsComplete() const
			{
				return (m_Laptime != -1.0f);
			}

			inline bool IsValid() const
			{
				return !m_illegal;
			}
			

			inline void Invalidate()
			{
				m_illegal = true;
			}

			inline bool IsValidAndComplete() const
			{
				return IsValid() && IsComplete();
			}


			inline void StoreStartTime(float timeNow)
			{
				if (m_startLap == -1.0f)
					m_startLap = timeNow;
			}

			inline void StoreIntermediate1(float timeNow)
			{
				if (m_startLap != -1.0f)
					m_intermediate1 =  timeNow - m_startLap;
			}

			inline void StoreIntermediate2(float timeNow)
			{
				if (m_intermediate1 != -1.0f)
					m_intermediate2 = (timeNow - m_startLap) - m_intermediate1;  
			}

			inline void StoreIntermediate3(float timeNow)
			{
				if (m_intermediate2 != -1.0f)
				{
					m_intermediate3 = (timeNow - m_startLap) - (m_intermediate1 + m_intermediate2);
					m_Laptime = m_intermediate1 + m_intermediate2 + m_intermediate3;
				}
			}

			float	m_intermediate1;
			float	m_intermediate2;
			float	m_intermediate3;
			float	m_Laptime;

			float	m_startLap;
			bool	m_illegal;			// illegal lap due to rules violation
			float	m_realLapTime;		// this is the counting lap time

	};


	class LogicState
	{
	public:

		LogicState()
		{
			Reset();
		};

		~LogicState(){};

		void Reset()
		{
			m_zone = FINAL_THIRD;
			m_lap = 0;
			m_current.Reset();
			m_last.Reset();
			m_logicPosition = 0.0f;
			m_logicPositionFurthest = 0.0f;
			m_Rank = -1;
			m_FastestLap.Reset();

			m_reversedDistance = 0.0f;
			m_prevpos = 0.0f;
			m_wrongway = false;
			m_racePercentage = 0.0f;
			m_logicPositionCombined = 0.0f;
			m_LapIncMessage = false;
			m_LapDecMessage = false;
			m_SplitOneMessage = false;
			m_SplitTwoMessage = false;
			m_positionIndex = 0;

		}

		enum LOGIC_ZONE
		{
			FIRST_THIRD = 0,
			SECOND_THIRD,
			FINAL_THIRD,
			NO_ZONE
		};

		int m_zone;
		int m_lap;
		int m_Maxlaps;

		inline void SetRaceLaps(int lap){m_Maxlaps = lap;}
		inline int GetRaceLaps(){return m_Maxlaps;}
		inline void SetLap(int lap){m_lap = lap;}
		inline int GetLap(){return m_lap;}

		inline LapTime *GetCurrentLapTime() {return &m_current;}
		inline LapTime *GetLastLapTime() {return &m_last;}
		inline LapTime *GetFastestLapTime() {return &m_FastestLap;}

		inline float	GetLogicPosition() const  {return m_logicPosition;}
		inline float	GetLogicPositionCombined() const {return m_logicPositionCombined;}
		inline float	GetLogicPositionFurthest() const {return m_logicPositionFurthest;}
		inline int		GetRank() const {return m_Rank;}
		inline float	GetDistanceFromStart() const {return m_DistanceFromStart;}
		inline void		SetRank(int rank){m_Rank = rank;}
		inline void		SetRacePercenatge(float percentage){m_racePercentage = percentage;}
		inline float    GetRacePercentage(){return m_racePercentage;}
		
		

		LapTime	m_current;
		LapTime m_last;
		LapTime	m_FastestLap;

		float	m_logicPosition;
		float	m_logicPositionFurthest;
		float	m_DistanceFromStart;
		float	m_racePercentage;

		int		m_Rank;
		int		m_positionIndex;

		float	m_reversedDistance;
		float	m_prevpos;
		bool	m_wrongway;
		float	m_logicPositionCombined;
		bool	m_InStartZone;
		bool	m_InFinishZone;
		bool	m_InInter1Zone;
		bool	m_InInter2Zone;



		bool	m_LapDecMessage;
		bool	m_LapIncMessage;
		bool	m_SplitOneMessage;
		bool	m_SplitTwoMessage;
	};

	BEGIN_COMPONENT_DEC(CVehicleLogic, IVehicleLogic)

	public:

		inline LogicState *GetLogicState() {return &m_state;}
		inline LogicState *GetLogicStateBuffered() {return &m_stateBuffered;}
		inline void StoreResetLogicState(){m_stateReset = m_state;}
		inline LogicState *GetResetLogicState() {return &m_stateReset;}
		inline void ResetResetLogicState()
		{
			// if we are on any start, intermediate for finish zone, then do not reset
			
			//if (!m_state.m_InStartZone && !m_state.m_InFinishZone && !m_state.m_InInter1Zone && !m_state.m_InInter2Zone)
			{
				m_state = m_stateReset;
			}
			
		}

	private:

		void StartZone();
		void FinishZone();
		void IntermediateZoneOne();
		void IntermediateZoneTwo();
		void LogicZone();
		void CachedInterfaces();
		void InitRace();
		void Update();
		int CalculateZone(float logicPos);
		void CalculateRacePercenatge();

		void ResetWrongWayTracking();
		void CheckWrongWay();
		void InitialiseRace();
		void ValidateZone();
		bool DebugValidateZone();
		void ValidateInter1();
		void ValidateInter2();
		float CheckIfInIntermediates();
		
		ITrackDataAI		*m_pTrackData;
		IData				*m_pIData;
		IVehicleState		*m_pIVehicleState;
		VehicleState		*m_pVehState;
		ISplineTracker		*m_pSplineTracker;
		IGameServerTime		*m_pIGameServerTime;

		LogicState			m_state;
		LogicState			m_stateReset;
		LogicState			m_stateBuffered;

		bool				m_SendLapIncMessagePointToPointHack;
		

	END_COMPONENT_DEC



}

#endif //_VEHICLELOGIC_AI_C_H
