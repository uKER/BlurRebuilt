#ifndef _TRACKDATA__H
#define _TRACKDATA__H

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

#include <utilityai/KdTree.h>
#include <utilityai/utilityai.h>
#include <fuzzylib/fuzzylibone/fuzzyone.h>
#include <system/Perf/PerfEvent.h>

namespace airacingtech
{

	class JunctionRule
	{
	public:
		JunctionRule(){};
		~JunctionRule(){};

		enum OPENCLOSED_STATE
		{
			JUNCTION_OPEN = 0,
			JUNCTION_CLOSE, 
			JUNCTION_NOT_INTIALISED
		};

		int m_ruleNO;
	};

	class JunctionRule1 : public JunctionRule
	{
	public:
		JunctionRule1(){};
		~JunctionRule1(){};

		OPENCLOSED_STATE	m_state; 

		

	};

	class JunctionRule2 : public JunctionRule
	{
	public:
		JunctionRule2(){};
		~JunctionRule2(){};

		OPENCLOSED_STATE	m_state; 
		CHash				m_routeName;
		
		
	};

	class JunctionRule3 : public JunctionRule
	{
	public:
		JunctionRule3(){};
		~JunctionRule3(){};

		OPENCLOSED_STATE	m_state; 
		CHash				m_routeName;
		float				m_t;

		

	};

	class JunctionRule4 : public JunctionRule
	{
	public:
		JunctionRule4(){};
		~JunctionRule4(){};

		OPENCLOSED_STATE	m_state[2]; 
		CHash				m_routeName;
		float				m_t[2];

		

	};

	class JunctionRule5 : public JunctionRule
	{
	public:
		JunctionRule5(): m_currentState(JUNCTION_NOT_INTIALISED), m_lastTime(0.0f) {};
		~JunctionRule5(){};

		OPENCLOSED_STATE	m_state[2]; 
		float				m_t[2];

		OPENCLOSED_STATE	m_currentState;
		float				m_lastTime;
		int					m_helperState;


	};

	class JunctionRule6 : public JunctionRule
	{
	public:
		JunctionRule6(){};
		~JunctionRule6(){};

		OPENCLOSED_STATE	m_state; 
		int					m_lap;

		

	};

	class JunctionRule7 : public JunctionRule
	{
	public:
		JunctionRule7(){};
		~JunctionRule7(){};

		OPENCLOSED_STATE	m_state; 
		int					m_lap;

		

	};

	class JunctionRule8 : public JunctionRule
	{
	public:
		JunctionRule8(){};
		~JunctionRule8(){};

		OPENCLOSED_STATE	m_state; 
		int					m_lap[3];

		

	};

	class JunctionRule9 : public JunctionRule
	{
	public:
		JunctionRule9() : m_currentState(JUNCTION_NOT_INTIALISED), m_lastTime(0.0f), m_CarCount(0), m_startCounterReset(false) {};
		~JunctionRule9(){};

		OPENCLOSED_STATE	m_state; 
		int					m_Ncars;
		float				m_resetTime;

		OPENCLOSED_STATE	m_currentState;
		float				m_lastTime;
		int					m_CarCount;
		int					m_startCounterReset;

	};

	

	class AITrackDataLines
	{
	public:
		AITrackDataLines()
		{
			/*for (int i=0; i < m_Speeds.Size(); i++)
			{
				m_Speeds[i] = 0;
			}*/
		}

		~AITrackDataLines()
		{
		}

		float	m_curvature;
		float	m_offset;
		float	m_distance;			// forwards......
		float	m_height;			// height of the track.........................

		//float	m_yangle;
		//int		m_flags;			// looped //pits// etc,etc..... surface type.....
		//char	m_surfaceFlags;		// surface to have it`s own flags................
		
		void SetCurvature(const Maths::HF32 PCREF curvature)
		{
			Maths::Store(curvature, &m_curvature);
		}

		const Maths::HF32 GetCurvature() const
		{
			return Maths::HF32(m_curvature);
		}

		void SetOffset(const Maths::HF32 PCREF offset)
		{
			Maths::Store(offset, &m_offset);
		}

		const Maths::HF32 GetOffset() const
		{
			return Maths::HF32(m_offset);
		}

		void SetDistance(const Maths::HF32 PCREF distance)
		{
			Maths::Store(distance,  &m_distance);
		}

		const Maths::HF32 GetDistance() const
		{
			return Maths::HF32(m_distance);
		}

		void SetHeight(const Maths::HF32 PCREF height)
		{
			Maths::Store(height, &m_height);
		}

		const Maths::HF32 GetHeight() const
		{
			return Maths::HF32(m_height);
		}

		/*enum TrackDataLinesEnum
		{
			TRACK_DATA_LINE_RIGHT_5 = 0,
			TRACK_DATA_LINE_RIGHT_4,
			TRACK_DATA_LINE_RIGHT_3,
			TRACK_DATA_LINE_RIGHT_2,
			TRACK_DATA_LINE_RIGHT_1,
			TRACK_DATA_LINE_RACING,
			TRACK_DATA_LINE_LEFT_1,
			TRACK_DATA_LINE_LEFT_2,
			TRACK_DATA_LINE_LEFT_3,
			TRACK_DATA_LINE_LEFT_4,
			TRACK_DATA_LINE_LEFT_5,
			TRACK_DATA_NUM_LINES,
			TRACK_DATA_EXTREME_LEFT,
			TRACK_DATA_EXTREME_RIGHT
		};*/

		const static int TRACK_DATA_LINE_RIGHT_5;
		const static int TRACK_DATA_LINE_RIGHT_4;
		const static int TRACK_DATA_LINE_RIGHT_3;
		const static int TRACK_DATA_LINE_RIGHT_2;
		const static int TRACK_DATA_LINE_RIGHT_1;
		const static int TRACK_DATA_LINE_RACING;
		const static int TRACK_DATA_LINE_LEFT_1;
		const static int TRACK_DATA_LINE_LEFT_2;
		const static int TRACK_DATA_LINE_LEFT_3;
		const static int TRACK_DATA_LINE_LEFT_4;
		const static int TRACK_DATA_LINE_LEFT_5;
		const static int TRACK_DATA_NUM_LINES;
		const static int TRACK_DATA_EXTREME_LEFT;
		const static int TRACK_DATA_EXTREME_RIGHT;

		//Btl::Vector<U8, Btl::VectorParameters_Array<20 > >  m_Speeds;	// speeds of each vehicle..............
	};


	class AITrackDataSegments
	{
	public:
		AITrackDataSegments()
		{
			m_flags = SEGMENT_FLAGS_NONE;
		}

		AITrackDataSegments(int indexNo) //: m_segmentNo(indexNo)
		{
			m_flags = SEGMENT_FLAGS_NONE;
		}

		~AITrackDataSegments()
		{
		}

		enum SEGMENT_FLAGS
		{
			SEGMENT_FLAGS_NONE = 0,
			SEGMENT_FLAGS_TRIGGER = 1
		};


		//Maths::HVec2	m_position;
		//Maths::HVec2	m_tangent;
		//Maths::HVec3	m_tangentXYZ;

		MathsStorage::Float2 m_position;
		MathsStorage::Float2 m_tangent;

		float		m_width;
		float		m_minY;
		float		m_maxY;

		SEGMENT_FLAGS	m_flags;

		// are these needed....
		//int				m_segmentNo; // No.
		// end are these needed...

		Btl::Vector<AITrackDataLines>  m_Lines;

		void SetAsTriggerSegment()
		{
			m_flags = SEGMENT_FLAGS_TRIGGER;
		}

		inline bool IsTrigger()
		{
			return m_flags == SEGMENT_FLAGS_TRIGGER;
		}

		void SetWidth(const Maths::HF32 PCREF width)
		{
			Maths::Store(width, &m_width);
		}

		const Maths::HF32 GetWidth() const
		{
			return Maths::HF32(m_width);
		}

		void SetMinY(const Maths::HF32 PCREF minY)
		{
			Maths::Store(minY, &m_minY);
		}

		const Maths::HF32 GetMinY() const
		{
			return Maths::HF32(m_minY);
		}

		void SetMaxY(const Maths::HF32 PCREF maxY)
		{
			Maths::Store(maxY, &m_maxY);
		}

		const Maths::HF32 GetMaxY() const
		{
			return Maths::HF32(m_maxY);
		}

		inline void GetDirection(Maths::HVec3 &direction) const
		{
			direction.X(m_tangent.y);
			direction.Y(Maths::Constants<Maths::HF32>::Zero());
			direction.Z(-m_tangent.x);
		}

		inline Maths::HF32 GetOffset(int line) const
		{

			if (line < AITrackDataLines::TRACK_DATA_NUM_LINES)
			{
				return m_Lines[line].GetOffset();
			}
			else if (line == AITrackDataLines::TRACK_DATA_EXTREME_RIGHT)
			{
				return Maths::Constants<Maths::HF32>::Zero();
			}
			else if (line == AITrackDataLines::TRACK_DATA_EXTREME_LEFT)
			{
				return Maths::Constants<Maths::HF32>::One();
			}

			assert(1);
			return Maths::Constants<Maths::HF32>::Max(); // should not get here
		}


		//assumes racing line
		inline void CalculateXOffsetFromXDistance(int line, float &xdistance, float &xdistanceP) const
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			
			const AITrackDataLines *pLine = &m_Lines[line];

			if (xdistance > 0.0f)
			{
				// right of the racing line
				const AITrackDataLines *pLineRight = &m_Lines[AITrackDataLines::TRACK_DATA_LINE_RIGHT_5];
				float diff = pLine->GetOffset() - pLineRight->GetOffset();
				float diffT = diff * GetWidth();
				xdistanceP = fuzzy_lib_one::FuzzyRightShoulder(xdistance, 0.0f, diffT);
				xdistance = Maths::Clamp(xdistance, 0.0f, diffT);
			}
			else
			{
				// left of the racing line
				const AITrackDataLines *pLineLeft = &m_Lines[AITrackDataLines::TRACK_DATA_LINE_LEFT_5];
				float diff = pLineLeft->GetOffset() - pLine->GetOffset();
				float diffT = diff * GetWidth();
				float x = xdistance;
				xdistanceP = -fuzzy_lib_one::FuzzyRightShoulder(Maths::Abs(x), 0.0f, diffT);
				xdistance = Maths::Clamp(xdistance, -Maths::Abs(diffT), 0.0f);
			}
		}

		inline Maths::HF32 GetActualOffsetUsingParamAndLine(const Maths::HF32 PCREF offset, int line) const
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);

#if 1
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const AITrackDataLines *pLine = &m_Lines[line];
			
			const AITrackDataLines *pLineRight = &m_Lines[AITrackDataLines::TRACK_DATA_LINE_RIGHT_5];
			const AITrackDataLines *pLineLeft = &m_Lines[AITrackDataLines::TRACK_DATA_LINE_LEFT_5];
			return Maths::Select(offset.GE(zero), 
								pLine->GetOffset() - (offset * (pLine->GetOffset() - pLineRight->GetOffset())), 
								pLine->GetOffset() + (Maths::Abs(offset) * (pLineLeft->GetOffset() - pLine->GetOffset())));
#else

			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const AITrackDataLines *pLine = &m_Lines[line];
			Maths::HF32 actualOffset = zero;
			if (offset >= zero)
			{
				// right of the line
				const AITrackDataLines *pLineRight = &m_Lines[AITrackDataLines::TRACK_DATA_LINE_RIGHT_5];
				const Maths::HF32 diff = pLine->m_offset - pLineRight->m_offset;
				actualOffset = pLine->m_offset - (offset * diff);
			}
			else 
			{
				// left of the line
				const AITrackDataLines *pLineLeft = &m_Lines[AITrackDataLines::TRACK_DATA_LINE_LEFT_5];
				const Maths::HF32 diff = pLineLeft->m_offset - pLine->m_offset;
				actualOffset = pLine->m_offset + (Maths::Abs(offset) * diff);
			}

			return actualOffset;
#endif
		}

		inline Maths::HF32 GetDistanceUsingOffsetParamAndLine(Maths::HF32 PCREF offset, int line) const
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);

			const Maths::HF32 actualOffset = GetActualOffsetUsingParamAndLine(offset, AITrackDataLines::TRACK_DATA_LINE_RACING);
			
			const AITrackDataLines *pLine = &m_Lines[line];

			//float dist = 0.0f;
			//dist = (pLine->m_offset - actualOffset) * m_width;
			return (pLine->GetOffset() - actualOffset) * GetWidth();

//#if 0
//			
//			if (actualOffset <= pLine->m_offset)
//			{
//				// to the right
//				dist = (pLine->m_offset - actualOffset) * m_width;
//			}
//			else
//			{
//				// to the left
//				dist = -((actualOffset - pLine->m_offset) * m_width);
//			}
//#endif

			//return dist;
			
		}
		
		inline void GetPositionOffsetLine(const Maths::HF32 PCREF offset, Maths::HVec3 &position, int line) const
		{
			// Really we only use the racing line
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			
			Maths::HF32 actualOffset = GetActualOffsetUsingParamAndLine(offset, line);
			
			GetPositionFromOffset(actualOffset,position);
		}

		inline void GetPositionFromOffset(const Maths::HF32 PCREF offset, Maths::HVec3 &position) const
		{
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			Maths::HVec3 startPos(m_position.x , zero, m_position.y);
			Maths::HVec3 normal(m_tangent.x, zero, m_tangent.y);

			//float off = Maths::Clamped(offset, m_Lines[AITrackDataLines::TRACK_DATA_LINE_RIGHT_5]->m_offset, m_Lines[AITrackDataLines::TRACK_DATA_LINE_LEFT_5]->m_offset);
			//float off = offset;
			
			position = startPos + (normal * GetWidth() * offset);

			// assume the racing line is the y height, not always the case ?
			position.Y(m_Lines[AITrackDataLines::TRACK_DATA_LINE_RACING].GetHeight());
		}

		inline void GetPositionFromLine(const int& line, Maths::HVec3 &position) const
		{
			
			/*if (line < AITrackDataLines::TRACK_DATA_NUM_LINES)
			{
				const AITrackDataLines *pLine = &m_Lines[line];

				Maths::HVec3 startPos(m_position.X() , 0.0f, m_position.Y());
				Maths::HVec3 normal(m_tangent.X(), 0.0f, m_tangent.Y());

				position = startPos + (normal * m_width * pLine->m_offset);
				position.Y(pLine->m_height);
			}
			else
			{
				
				if (line == AITrackDataLines::TRACK_DATA_EXTREME_LEFT)
				{
					const AITrackDataLines *pLine = &m_Lines[AITrackDataLines::TRACK_DATA_LINE_RACING];

					Maths::HVec3 startPos(m_position.X() , 0.0f, m_position.Y());
					Maths::HVec3 normal(m_tangent.X(), 0.0f, m_tangent.Y());

					position = startPos + (normal * m_width * 1.0f);
					position.Y(pLine->m_height);
				}
				else if (line == AITrackDataLines::TRACK_DATA_EXTREME_RIGHT)
				{
					const AITrackDataLines *pLine = &m_Lines[AITrackDataLines::TRACK_DATA_LINE_RACING];

					Maths::HVec3 startPos(m_position.X() , 0.0f, m_position.Y());
					Maths::HVec3 normal(m_tangent.X(), 0.0f, m_tangent.Y());

					position = startPos + (normal * m_width * 0.0f);
					position.Y(pLine->m_height);
				}
				
			}*/

			/*const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			bool extremeLeft = (line == AITrackDataLines::TRACK_DATA_EXTREME_LEFT);
			bool extremeRight = (line == AITrackDataLines::TRACK_DATA_EXTREME_RIGHT);
			const Maths::Scalar_Bool vExtremeLeft(extremeLeft), vExtremeRight(extremeRight);

			U32 index = Maths::Select(extremeLeft || extremeRight, AITrackDataLines::TRACK_DATA_LINE_RACING,
				line);
			const AITrackDataLines *pLine = &m_Lines[index];
			const Maths::HF32 offset(Maths::Select(vExtremeLeft, Maths::Constants<Maths::HF32>::One(),
				Maths::Select(vExtremeRight, zero, pLine->GetOffset())));*/

			// Get the line.
			//int line = *pLine;
			int extremeLeft = (line == AITrackDataLines::TRACK_DATA_EXTREME_LEFT);
			int extremeRight = (line == AITrackDataLines::TRACK_DATA_EXTREME_RIGHT);
			U32 index = Maths::Select(extremeLeft | extremeRight, AITrackDataLines::TRACK_DATA_LINE_RACING, line);
			const AITrackDataLines *pTrackDataLines = &m_Lines[index];

			// Get the offset.
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const Maths::HF32 vLine(line);
			const Maths::HF32 vExtremeLeft(AITrackDataLines::TRACK_DATA_EXTREME_LEFT);
			const Maths::HF32 vExtremeRight(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT);
			const Maths::Scalar_Bool vLineIsExtremeLeft(vLine == vExtremeLeft);
			const Maths::Scalar_Bool vLineIsExtremeRight(vLine == vExtremeRight);
			const Maths::HF32 offset(Maths::Select(vLineIsExtremeLeft, 
													Maths::Constants<Maths::HF32>::One(), 
													Maths::Select(vLineIsExtremeRight, zero, pTrackDataLines->GetOffset())));

			const Maths::HVec3 startPos(m_position.x, zero, m_position.y);
			const Maths::HVec3 normal(m_tangent.x, zero, m_tangent.y);
			position = startPos + (normal * GetWidth() * offset);
			position.Y(pTrackDataLines->GetHeight());
		}


		//void AddLine(AITrackDataLines *pLine)
		//{
		//	assert(pLine);
		//	m_Lines.PushBack(pLine);
		//}

		inline Maths::HF32 GetDistance(int line) const
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			return m_Lines[line].GetDistance();
		}

		inline Maths::HF32 GetCurvature(int line) const
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			return m_Lines[line].GetCurvature();
		}

		void SetCurvature(int line, float c)
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			m_Lines[line].SetCurvature(c);
		}

		/*inline Maths::HF32 GetWidth() const
		{
			return m_width;
		}*/

		/*void SetSpeed(int line, U8 speed, int speedIndex)
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			AITrackDataLines *pLine = &m_Lines[line];
			pLine->m_Speeds[speedIndex] = speed;
		}

		void AddSpeed(int line)
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			AITrackDataLines *pLine = &m_Lines[line];
			//pLine->m_Speeds.PushBack(0);
		}

		inline U8 GetSpeed(int line, int speedIndex) const
		{
			assert(line >= AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 && line <= AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			const AITrackDataLines *pLine = &m_Lines[line];
			U8 speed = pLine->m_Speeds[speedIndex];
			return speed;
		}*/
	};



	
	// wastefull of memory, but quick for now
	
	// forward dec
	class AITrackDataRoutes;

	class AITrackDataCachedData
	{
	public:	
		AITrackDataCachedData()
		{
		}

		AITrackDataCachedData(AITrackDataRoutes *pRoute, int index) : m_pRoute(pRoute), m_index(index)
		{
		}
		
		~AITrackDataCachedData()
		{
		}
		
		AITrackDataRoutes	*m_pRoute;
		int					m_index;
	};

	class AITrackDataCache
	{
	public:

		AITrackDataCache(){};
		~AITrackDataCache()
		{
		}

		Btl::Vector<AITrackDataCachedData>  m_data;	

	};

	

	class AITrackDataRoutes
	{
	public:

		enum ROUTE_FLAGS
		{
			ROUTE_LOOPED = 1,
			ROUTE_START_ZONE = 2,
			ROUTE_FINISH_ZONE = 4,
			ROUTE_INTERMEDIATE_ZONE_1 = 8,
			ROUTE_INTERMEDIATE_ZONE_2 = 16,
			ROUTE_LOGIC_ZONE = 32,
			ROUTE_JUNCTION = 64,
			ROUTE_DYNAMIC_JUNCTION_ZONE = 128,
			ROUTE_VALID_RESET = 256,
			ROUTE_VALID_NITRO = 512,
			ROUTE_VALID_DROP_PRIORITY = 1024,
			ROUTE_END
		};

		AITrackDataRoutes() : m_pJunctionRule(0), m_Active(true), m_routeLength(0.f)
		{
		}
		
		~AITrackDataRoutes()
		{
			if (m_pJunctionRule)
			{
				delete m_pJunctionRule;
			}
		}

		Maths::HVec3		m_minBounds;
		Maths::HVec3		m_maxBounds;
		CHash				m_Name;
		int					m_RouteFlags;
		float				m_stepSize;
		float				m_routeLength; // number of segments * step size
		CHash				m_InputRoute;
		CHash				m_OutputRoute;
		bool				m_Active;

		JunctionRule		*m_pJunctionRule;

		KdTree	m_kdtree;
		Btl::Vector<AITrackDataSegments>		m_Segments;

		Btl::Vector<AITrackDataCache>			m_dataCache;

		void SetAsATriggerSegment(int index)
		{
			if (index >=0 && index < m_Segments.Size())
			{
				AITrackDataSegments *pSegment = &m_Segments[index];
				pSegment->SetAsTriggerSegment();
			}
		}

		inline bool IsTrigger(int index)
		{
			bool v = false;

			if (index >=0 && index < m_Segments.Size())
			{
				AITrackDataSegments *pSegment = &m_Segments[index];
				v = pSegment->IsTrigger();
			}

			return v;
		}



		void CalculateXOffsetFromXDistance(int index, int line, float &xdistance, float &xdistanceP)
		{
			AITrackDataSegments *pSegment = &m_Segments[index];
			pSegment->CalculateXOffsetFromXDistance(line, xdistance, xdistanceP);
		}

		inline float GetDistanceUsingOffsetParamAndLine(float offset, int line, int index) const
		{
			const AITrackDataSegments *pSegment = &m_Segments[index];

			return pSegment->GetDistanceUsingOffsetParamAndLine(Maths::HF32(offset), line);
		}


		int NearestLane(float offset)
		{
			// this is quite aggressive.........
			int lane = -1;

			if (offset >= -0.25f && offset <= 0.25f)
				lane = AITrackDataLines::TRACK_DATA_LINE_RACING;
			else if (offset < -0.25f)
			{
				if (offset <= -0.25f && offset >= -0.5f)
					lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_1;
				else if (offset <= -0.5f && offset >= -0.75f)
					lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_2;
				//else if (offset <= -0.75f && offset >= -0.9f)
				//	lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_3;
				else if (offset <= -0.75f)
					lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_2;
			}
			else
			{
				if (offset >= 0.25f && offset <= 0.5f)
					lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_1;
				else if (offset >= 0.5f && offset <= 0.75f)
					lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_2;
				//else if (offset >= 0.75f && offset <= 0.9f)
				//	lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_3;
				else if (offset >= 0.75f)
					lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_2;
			}




#if 0
			if (offset >= -0.1f && offset <= 0.1f)
				lane = AITrackDataLines::TRACK_DATA_LINE_RACING;
			else if (offset < -0.1f)
			{
				if (offset <= -0.1f && offset >= -0.3f)
					lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_1;
				else if (offset <= -0.3f && offset >= -0.5f)
					lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_2;
				else if (offset <= -0.5f && offset >= -0.7f)
					lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_3;
				else if (offset <= -0.7f && offset >= -0.9f)
					lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_4;
				else if (offset <= -0.9f)
					lane = AITrackDataLines::TRACK_DATA_LINE_LEFT_5;
			}
			else
			{
				if (offset >= 0.1f && offset <= 0.3f)
					lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_1;
				else if (offset >= 0.3f && offset <= 0.5f)
					lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_2;
				else if (offset >= 0.5f && offset <= 0.7f)
					lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_3;
				else if (offset >= 0.7f && offset <= 0.9f)
					lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_4;
				else if (offset >= 0.9f)
					lane = AITrackDataLines::TRACK_DATA_LINE_RIGHT_5;
			}
#endif


			return lane;
		}

		inline int IsStart() const { return (m_RouteFlags & ROUTE_START_ZONE); }
		inline int IsIntermediate(int no) const
		{
			if (no == 1)
				return (m_RouteFlags & ROUTE_INTERMEDIATE_ZONE_1);
			else
				return (m_RouteFlags & ROUTE_INTERMEDIATE_ZONE_2);
		}

		inline int IsIntermediate1() const
		{
			return (m_RouteFlags & ROUTE_INTERMEDIATE_ZONE_1);
		}

		inline int IsIntermediate2() const
		{
			return (m_RouteFlags & ROUTE_INTERMEDIATE_ZONE_2);
		}

		inline int IsFinish() const				{ return (m_RouteFlags & ROUTE_FINISH_ZONE); }
		inline int IsLogic() const				{ return (m_RouteFlags & ROUTE_LOGIC_ZONE); }
		inline int IsJunction() const			{ return (m_RouteFlags & ROUTE_JUNCTION); }
		inline int IsDynamicJunction() const	{ return (m_RouteFlags & ROUTE_DYNAMIC_JUNCTION_ZONE); }
		inline int IsDropPriority() const		{ return (m_RouteFlags & ROUTE_VALID_DROP_PRIORITY); }
		
		inline bool IsActive() const {return m_Active;}
		void SetActive(bool state){m_Active = state;}

		inline bool IsChangeableRoute() const
		{
			/*bool changeable = false;
			
			if (IsIntermediate(1) || IsIntermediate(2) || IsStart() || IsFinish() || IsLogic() || IsJunction() || IsDynamicJunction() )
				changeable = false;
			else
				changeable = true;
			
			return changeable;*/
			return !(m_RouteFlags & (ROUTE_INTERMEDIATE_ZONE_1 | ROUTE_INTERMEDIATE_ZONE_2 | ROUTE_START_ZONE | 
				ROUTE_FINISH_ZONE | ROUTE_LOGIC_ZONE | ROUTE_JUNCTION | ROUTE_DYNAMIC_JUNCTION_ZONE));
		}


		inline bool IsInBounds(Maths::HVec3 &pos)
		{
			//bool isIn = false;
			//
			//// 2d check only here.....
			//if ( (pos.X() > m_minBounds.X() && pos.Z() > m_minBounds.Z()) &&
			//	pos.X() < m_maxBounds.X() && pos.Z() < m_maxBounds.Z())
			//{
			//	isIn = true;
			//}

			//return isIn;
			const Maths::Vector3_Bool vBool(pos > m_minBounds & pos < m_maxBounds);
			return Maths::X(vBool) & Maths::Z(vBool);
		}


		void CalculateBounds()
		{
			/*m_minBounds.X(FLT_MAX); m_minBounds.Y(FLT_MAX); m_minBounds.Z(FLT_MAX);
			m_maxBounds.X(-FLT_MAX); m_maxBounds.Y(-FLT_MAX); m_maxBounds.Z(-FLT_MAX);*/
			m_minBounds = Maths::Constants<Maths::HVec3>::Max();
			m_maxBounds = -Maths::Constants<Maths::HVec3>::Max();
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
			const Maths::HVec3 five(Maths::HF32(5.0f));
			

			int size = m_Segments.Size();
			for(int i=0; i<size; i++)
			{
				const AITrackDataSegments *pSegment = &m_Segments[i];

				Maths::HVec3 positionLeft;
				pSegment->GetPositionFromOffset(one, positionLeft);

				if (positionLeft.X() > m_maxBounds.X())
					m_maxBounds.X(positionLeft.X());
				if (positionLeft.Y() > m_maxBounds.Y())
					m_maxBounds.Y(positionLeft.Y());
				if (positionLeft.Z() > m_maxBounds.Z())
					m_maxBounds.Z(positionLeft.Z());

				if (positionLeft.X() < m_minBounds.X())
					m_minBounds.X(positionLeft.X());
				if (positionLeft.Y() < m_minBounds.Y())
					m_minBounds.Y(positionLeft.Y());
				if (positionLeft.Z() < m_minBounds.Z())
					m_minBounds.Z(positionLeft.Z());

				Maths::HVec3 positionRight;
				pSegment->GetPositionFromOffset(zero, positionRight);

				if (positionRight.X() > m_maxBounds.X())
					m_maxBounds.X(positionRight.X());
				if (positionRight.Y() > m_maxBounds.Y())
					m_maxBounds.Y(positionRight.Y());
				if (positionRight.Z() > m_maxBounds.Z())
					m_maxBounds.Z(positionRight.Z());

				if (positionRight.X() < m_minBounds.X())
					m_minBounds.X(positionRight.X());
				if (positionRight.Y() < m_minBounds.Y())
					m_minBounds.Y(positionRight.Y());
				if (positionRight.Z() < m_minBounds.Z())
					m_minBounds.Z(positionRight.Z());				
			}
			m_minBounds -= five;
			m_maxBounds += five;
		}


		void PopulateKDTree()
		{
			// TODO : randomly populate this kdtree for optimal speed improvements..  takes more memory using this.
			const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
			int size = m_Segments.Size();
			for(int i=0; i<size; i++)
			{
				AITrackDataSegments *pSegment = &m_Segments[i];

				Maths::HVec3 position;
				pSegment->GetPositionFromOffset(half, position);

				m_kdtree.add(position.X(), position.Y(), position.Z(), 20.0f, pSegment);
			}

		}

		inline Maths::HF32 GetOffset(int index, int line) const
		{
			const AITrackDataSegments *pSegment = &m_Segments[index];
			return pSegment->GetOffset(line);
		}

		inline Maths::HF32 GetWidth(int index) const
		{
			const AITrackDataSegments *pSegment = &m_Segments[index];
			return pSegment->GetWidth();
		}

		inline bool IsLoopedTrack() const { return bool(m_RouteFlags & ROUTE_LOOPED); }

		
		inline void GetDirection(int index, Maths::HVec3 &direction) const
		{
			const AITrackDataSegments *pSegment = &m_Segments[index];
			pSegment->GetDirection(direction);
		}
		
		inline void GetPositionOffsetLine(const Maths::HF32 PCREF offset, int index, Maths::HVec3 &position, int line) const
		{
			const AITrackDataSegments *pSegment = &m_Segments[index];
			pSegment->GetPositionOffsetLine(offset, position, line);
		}

		inline void GetPositionFromOffset(const Maths::HF32 PCREF  offset, int index, Maths::HVec3 &position) const
		{
			const AITrackDataSegments *pSegment = &m_Segments[index];
			pSegment->GetPositionFromOffset(offset, position);
		}

		inline void GetPositionFromLine(const int& pLine, int index,  Maths::HVec3 &position) const
		{
			const AITrackDataSegments *pSegment = &m_Segments[index];
			pSegment->GetPositionFromLine(pLine, position);
		}

		inline void GetHeightBounds(Maths::HF32 &minY, Maths::HF32 &maxY, int index) const
		{
			assert(index >=0 && index < m_Segments.Size());
			const AITrackDataSegments *pSegment = &m_Segments[index];
			minY = pSegment->GetMinY();
			maxY = pSegment->GetMaxY();
		}

		//void AddSegment(AITrackDataSegments *pSegment)
		//{
		//	assert(pSegment);
		//	m_Segments.PushBack(pSegment);
		//}

		inline int GetNumSegments() const { return (int)m_Segments.Size(); }

		inline void SetStepSize(float size)
		{
			m_stepSize = size;
			m_routeLength = size * GetNumSegments();
		}

		inline float GetStepSize() const { return m_stepSize; }

		inline float GetRouteLength() const {return m_routeLength;}

		inline int GetIndexOffset(int index, int offset) const
		{
			assert(index >=0 && index < m_Segments.Size());
			
			int newIndex = index + offset;

			if (IsLoopedTrack())
			{
				if (newIndex >= m_Segments.Size())
					newIndex -= m_Segments.Size();
				else if (newIndex < 0)
					newIndex += m_Segments.Size();

				if (newIndex == 0)
					newIndex = 1;

			}
			else
			{
				if (newIndex >= m_Segments.Size())
					newIndex = m_Segments.Size() - 1;
				else if (newIndex <= 0)
					newIndex = 1;
			}

			return newIndex;
		}

		inline float CalculateDistance(int startIndex, int endIndex, int line) const
		{
			float currentDistance = 0.0f;
			for(int i=startIndex; i<=endIndex; i++)
			{
				const AITrackDataSegments *pSegment = &m_Segments[i];
				float dist = pSegment->GetDistance(line);
				currentDistance += dist;
			}

			return currentDistance;
		}


		inline void GetSegmentBounds(int index, Maths::HVec3 &BottomLeft, Maths::HVec3 &TopLeft, Maths::HVec3 &BottomRight, Maths::HVec3 &TopRight) const
		{
			assert(index >=0 && index < m_Segments.Size());

			GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, index , TopLeft);
			GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, index , TopRight);

			GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, index-1 , BottomRight);
			GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, index-1 , BottomLeft);
		}

		inline bool InfrontSegmentTopPlane(Maths::HVec3 &position, int index) const
		{
			bool infront = true;
			
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			Maths::HVec3 BottomLeft(Maths::UNDEFINED), TopLeft(Maths::UNDEFINED), BottomRight(Maths::UNDEFINED), TopRight(Maths::UNDEFINED);
			GetSegmentBounds(index,BottomLeft, TopLeft, BottomRight, TopRight);
			const Maths::HF32 topSide = airacingtech::SideOfLine(position, TopLeft, TopRight);

			return (topSide.LE(zero));
		}


		inline bool InfrontSegmentBottomPlane(Maths::HVec3 &position, int index) const
		{
			bool infront = true;

			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			Maths::HVec3 BottomLeft(Maths::UNDEFINED), TopLeft(Maths::UNDEFINED), BottomRight(Maths::UNDEFINED), TopRight(Maths::UNDEFINED);
			GetSegmentBounds(index,BottomLeft, TopLeft, BottomRight, TopRight);
			const Maths::HF32 topSide = airacingtech::SideOfLine(position, BottomLeft, BottomRight);

			return (topSide.LE(zero));
		}

		inline bool InSegmentBounds(Maths::HVec3 &position, int index) const
		{
			bool inSegment = false;
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			Maths::HVec3 BottomLeft(Maths::UNDEFINED), TopLeft(Maths::UNDEFINED), BottomRight(Maths::UNDEFINED), TopRight(Maths::UNDEFINED);
			
			GetSegmentBounds(index,BottomLeft, TopLeft, BottomRight, TopRight);
			
			const Maths::HF32 topSide = airacingtech::SideOfLine(position, TopLeft, TopRight);
			const Maths::HF32 bottomSide = airacingtech::SideOfLine(position, BottomRight, BottomLeft);
			const Maths::HF32 leftSide = airacingtech::SideOfLine(position, BottomLeft, TopLeft);
			const Maths::HF32 rightSide = airacingtech::SideOfLine(position, TopRight, BottomRight);
			return (topSide.GE(zero) && bottomSide.GE(zero) && leftSide.GE(zero) && rightSide.GE(zero));
			/*{
				inSegment = true;
			}
			
			return inSegment;*/
		}

		inline bool InSegmentWidths(Maths::HVec3 &position, int index) const
		{
			bool inSegment = false;
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			Maths::HVec3 BottomLeft(Maths::UNDEFINED), TopLeft(Maths::UNDEFINED), BottomRight(Maths::UNDEFINED), TopRight(Maths::UNDEFINED);

			GetSegmentBounds(index,BottomLeft, TopLeft, BottomRight, TopRight);

			const Maths::HF32 leftSide = airacingtech::SideOfLine(position, BottomLeft, TopLeft);
			const Maths::HF32 rightSide = airacingtech::SideOfLine(position, TopRight, BottomRight);
			return (leftSide.GE(zero) && rightSide.GE(zero));
		}


		inline bool IntersectSegmentBounds(Maths::HVec3 start, Maths::HVec3 end, int index)
		{
			bool intersect = false;
			Maths::HVec3 BottomLeft(Maths::UNDEFINED), TopLeft(Maths::UNDEFINED), BottomRight(Maths::UNDEFINED), TopRight(Maths::UNDEFINED), intersectPoint(Maths::UNDEFINED);

			GetSegmentBounds(index,BottomLeft, TopLeft, BottomRight, TopRight);

			intersect = LineIntersection(start.X(), start.Z(), end.X(), end.Z(), BottomLeft.X(), BottomLeft.Z(), TopLeft.X(), TopLeft.Z(), intersectPoint);			
			if (!intersect)
			{
				intersect = LineIntersection(start.X(), start.Z(), end.X(), end.Z(), TopLeft.X(), TopLeft.Z(), TopRight.X(), TopRight.Z(), intersectPoint);	
				if (!intersect)
				{
					intersect = LineIntersection(start.X(), start.Z(), end.X(), end.Z(), TopRight.X(), TopRight.Z(), BottomRight.X(), BottomRight.Z(), intersectPoint);	

					if (!intersect)
					{
						intersect = LineIntersection(start.X(), start.Z(), end.X(), end.Z(), BottomRight.X(), BottomRight.Z(), BottomLeft.X(), BottomLeft.Z(), intersectPoint);	
					}
				}

			}

			return intersect;
		}


		inline int FindNearest(int startIndex, int endIndex, Maths::HVec3 &position, float &dist, Maths::HF32 step)
		{
			PERF_EVENT("AI.FindNearest");

			int index = -1;

			if (IsInBounds(position))
			{
				bool heightTest = true;
				if (IsLogic() | IsStart() | IsFinish() | IsIntermediate1() | IsIntermediate2())
				{
					heightTest = false;
					index = FindNearestNoHeightTest(startIndex, endIndex, position, dist, step);
				}
				else
				{
					index = FindNearestHeightTest(startIndex, endIndex, position, dist, step);
				}
			}

			return index;
		}



		inline int FindNearestNoHeightTest(int startIndex, int endIndex, Maths::HVec3 &position, float &dist, float step)
		{
			PERF_EVENT("AI.FindNearestNoHeight");

#if 0
			int index = -1;
			for(int i=startIndex; i<=endIndex; i++)
			{
				Maths::HVec3 BottomLeft, TopLeft, BottomRight, TopRight;
				GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i , TopLeft);

				Maths::HVec3 PosTest(position.X(), 0.0f, position.Z());
				GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i , TopRight);
				float topSide = airacingtech::SideOfLine(PosTest, TopLeft, TopRight);

				if (topSide >= 0.0f)
				{
					GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i-1 , BottomRight);
					GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i-1 , BottomLeft);
					float bottomSide = airacingtech::SideOfLine(PosTest, BottomRight, BottomLeft);
					if (bottomSide >= 0.0f)
					{
						float leftSide = airacingtech::SideOfLine(PosTest, BottomLeft, TopLeft);
						if (leftSide >= 0.0f)
						{
							float rightSide = airacingtech::SideOfLine(PosTest, TopRight, BottomRight);
							if (rightSide >= 0.0f)
							{
								// we have a hit, store index and work out distance through segment as a paramatised variable...
								index= i;

								float dist1 = airacingtech::DistToLine(PosTest,BottomRight,BottomLeft);
								float dist2 =  Maths::Abs(airacingtech::DistToLine(PosTest,TopRight,TopLeft));
								float totalDist = dist1 + dist2;
								dist = dist1 / totalDist;
								break; 
							}
						}
					}
				}

			}
#else
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			int index = -1;
			for(int i=startIndex; i<=endIndex; i++)
			{
				Maths::HVec3 BottomLeft(Maths::UNDEFINED), TopLeft(Maths::UNDEFINED), BottomRight(Maths::UNDEFINED), TopRight(Maths::UNDEFINED);
				GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i , TopLeft);

				Maths::HVec3 PosTest(position.X(), zero, position.Z());
				GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i , TopRight);
				GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i-1 , BottomRight);
				GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i-1 , BottomLeft);
				const Maths::HF32 topSide = airacingtech::SideOfLine(PosTest, TopLeft, TopRight);
				const Maths::HF32 bottomSide = airacingtech::SideOfLine(PosTest, BottomRight, BottomLeft);
				const Maths::HF32 leftSide = airacingtech::SideOfLine(PosTest, BottomLeft, TopLeft);
				const Maths::HF32 rightSide = airacingtech::SideOfLine(PosTest, TopRight, BottomRight);

				if (topSide.GE(zero) & bottomSide.GE(zero) & leftSide.GE(zero) & rightSide.GE(zero))
				{
					// we have a hit, store index and work out distance through segment as a paramatised variable...
					index= i;
					const Maths::HF32 dist1 = airacingtech::DistToLine(PosTest,BottomRight,BottomLeft);
					const Maths::HF32 dist2 =  Maths::Abs(airacingtech::DistToLine(PosTest,TopRight,TopLeft));
					const Maths::HF32 totalDist = dist1 + dist2;
					dist = dist1 / totalDist;
					break; 
				}

			}
#endif

			return index;
		}

		inline int FindNearestHeightTest(int startIndex, int endIndex, Maths::HVec3 &position, float &dist, float step)
		{
			PERF_EVENT("AI.FindNearestHeight");
			int index = -1;

			for(int i=startIndex; i<=endIndex; i++)
			{

#if 0
				Maths::HVec3 BottomLeft, TopLeft, BottomRight, TopRight;
				GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i , TopLeft);

				float minY, maxY;
				GetHeightBounds(minY, maxY, i);

				// quick bounds check here
				if (position.Y() >= minY && position.Y() <= maxY)
				{
					Maths::HVec3 PosTest(position.X(), 0.0f, position.Z());
					GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i , TopRight);
					float topSide = airacingtech::SideOfLine(PosTest, TopLeft, TopRight);

					if (topSide >= 0.0f)
					{
						GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i-1 , BottomRight);
						GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i-1 , BottomLeft);
						float bottomSide = airacingtech::SideOfLine(PosTest, BottomRight, BottomLeft);
						if (bottomSide >= 0.0f)
						{
							float leftSide = airacingtech::SideOfLine(PosTest, BottomLeft, TopLeft);
							if (leftSide >= 0.0f)
							{
								float rightSide = airacingtech::SideOfLine(PosTest, TopRight, BottomRight);
								if (rightSide >= 0.0f)
								{
									// we have a hit, store index and work out distance through segment as a paramatised variable...
									index= i;

									float dist1 = airacingtech::DistToLine(PosTest,BottomRight,BottomLeft);
									float dist2 =  Maths::Abs(airacingtech::DistToLine(PosTest,TopRight,TopLeft));
									float totalDist = dist1 + dist2;
									dist = dist1 / totalDist;
									break; 
								}
							}
						}
					}
				}
#else
				
				Maths::HF32 minY(Maths::UNDEFINED), maxY(Maths::UNDEFINED);
				GetHeightBounds(minY, maxY, i);

				if (position.Y().GE(minY) & position.Y().LE(maxY))
				{
					const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
					Maths::HVec3 BottomLeft(Maths::UNDEFINED), TopLeft(Maths::UNDEFINED), BottomRight(Maths::UNDEFINED), TopRight(Maths::UNDEFINED);
					Maths::HVec3 PosTest(position.X(), zero, position.Z());

					GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i , TopLeft);
					GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i , TopRight);
					GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i-1 , BottomRight);
					GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i-1 , BottomLeft);
					
					const Maths::HF32 topSide = airacingtech::SideOfLine(PosTest, TopLeft, TopRight);
					const Maths::HF32 bottomSide = airacingtech::SideOfLine(PosTest, BottomRight, BottomLeft);
					const Maths::HF32 leftSide = airacingtech::SideOfLine(PosTest, BottomLeft, TopLeft);
					const Maths::HF32 rightSide = airacingtech::SideOfLine(PosTest, TopRight, BottomRight);

					if (topSide.GE(zero) & bottomSide.GE(zero) & leftSide.GE(zero) & rightSide.GE(zero))
					{
						index= i;

						const Maths::HF32 dist1 = airacingtech::DistToLine(PosTest,BottomRight,BottomLeft);
						const Maths::HF32 dist2 =  Maths::Abs(airacingtech::DistToLine(PosTest,TopRight,TopLeft));
						const Maths::HF32 totalDist = dist1 + dist2;
						dist = dist1 / totalDist;
						break; 
					}
				}


#endif
			}


			return index;
		}

		inline int FindNearestQuick(int startIndex, int endIndex, Maths::HVec3 &position, float &dist, int increment = 10, float step = 1.0f)
		{
			int index = -1;

			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			if (IsInBounds(position))
			{
				
				bool loop = true;
				bool found = false;

				int st = startIndex;
				int end = st - increment;


				bool heightTest = true;
				if (IsLogic() | IsStart() | IsFinish() | IsIntermediate1() || IsIntermediate2())
				{
					heightTest = false;
				}

				while(loop)
				{
					st += increment;
					if (st >= endIndex)
					{
						st = endIndex;
						loop = false;
					}
					end = st - increment;

					

					bool test = true;
					if (heightTest)
					{
						Maths::HF32 minY(Maths::UNDEFINED), maxY(Maths::UNDEFINED);
						GetHeightBounds(minY, maxY, st);

						//if (position.Y() > TopLeft.Y() - 2.0f && position.Y() < TopLeft.Y() + 4.0f)
						test = (position.Y().GE(minY) & position.Y().LE(maxY));
						/*{
							test = true;
						}
						else
						{
							test = false;
						}*/

					}

					// quick bounds check here
					if (test == true)
					{

						Maths::HVec3 BottomLeft(Maths::UNDEFINED), TopLeft(Maths::UNDEFINED), BottomRight(Maths::UNDEFINED), TopRight(Maths::UNDEFINED);
						GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, st , TopLeft);
						GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, st , TopRight);
						GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, end , BottomRight);
						GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, end , BottomLeft);
						const Maths::HF32 topSide = airacingtech::SideOfLine(position, TopLeft, TopRight);
						const Maths::HF32 bottomSide = airacingtech::SideOfLine(position, BottomRight, BottomLeft);
						const Maths::HF32 leftSide = airacingtech::SideOfLine(position, BottomLeft, TopLeft);
						const Maths::HF32 rightSide = airacingtech::SideOfLine(position, TopRight, BottomRight);

						if (topSide.GE(zero) & bottomSide.GE(zero) & 
							leftSide.GE(zero) & rightSide.GE(zero))
						{
							// we have a hit, store index and work out distance through segment as a paramatised variable...
							loop = false;
							found = true;
						}
					}
				}

				if (found == true)
				{
					index =  FindNearest(end, st, position,dist,step);
				}
			}

			return index;
		}
		

		


		inline float GetDistance(int startIndex, int endIndex, int line)
		{
			assert(startIndex >=0 && startIndex < m_Segments.Size());
			assert(endIndex >=0 && endIndex < m_Segments.Size());
			
			float currentDistance = 0.0f;
			if (IsLoopedTrack())
			{
				// +ve if endIndex is ahead of startIndex, -ve if endIndex behind startIndex
				//int infront = DistanceInfrontBehind(startIndex, endIndex, m_Segments.Size());
				int infront = DistanceInfrontBehind(endIndex, startIndex, m_Segments.Size());

				if (infront > 0)
				{
					// ok infront, but we are potentially crossing the end and start of the spline
					if (endIndex < startIndex)
					{
						currentDistance = CalculateDistance(0, startIndex, line);	
						currentDistance += CalculateDistance(endIndex, m_Segments.Size()-1, line);
					}
					else
					{
						currentDistance = CalculateDistance(startIndex, endIndex, line);	
					}
				}
				else if (infront < 0)
				{
					// ok, behind but potentiall crossing the end/start if the spline.
					if (startIndex < endIndex)
					{
						currentDistance = CalculateDistance(0, startIndex, line);	
						currentDistance += CalculateDistance(endIndex, m_Segments.Size()-1, line);
					}
					else
					{
						currentDistance = CalculateDistance(endIndex, startIndex, line);	
					}

					currentDistance = -Maths::Abs(currentDistance);	
				}
				else
				{
					currentDistance = 0.0f;
				}
			}
			else
			{
				if (endIndex < startIndex)
				{
					// negative distance.....
					currentDistance = -Maths::Abs(CalculateDistance(endIndex, startIndex, line));
				}
				else
				{
					currentDistance = CalculateDistance(startIndex, endIndex, line);
				}
			}

			return currentDistance;
		}


		inline int GetIndexOffsetByDistance(int startindex, float offsetDistance, int line, bool &status) const
		{
			// Only positve offsetDistance at present
			// TODO : negative off set distance....
			assert(startindex >=0 && startindex < m_Segments.Size());
			
			float currentDistance = 0;
			status = false;
			int currentIndex = startindex+1;

			if (IsLoopedTrack())
			{
				// the looped version could loop over itself on tracks where the offsetDistance > actual looped circuit
				do 
				{
					if (currentIndex >= m_Segments.Size())
						currentIndex -= m_Segments.Size();

					const AITrackDataSegments *pSegment = &m_Segments[currentIndex];
					float dist = pSegment->GetDistance(line);
					currentDistance += dist;
					currentIndex++;
				} while(currentDistance < offsetDistance);

				currentIndex--; // 
				currentIndex = Maths::Max(1, currentIndex);
				if (currentDistance >= offsetDistance)
					status = true;
				else
					status = false;
			}
			else
			{
				for(int i=currentIndex; i<m_Segments.Size(); i++)
				{
					const AITrackDataSegments *pSegment = &m_Segments[currentIndex];
					float dist = pSegment->GetDistance(line);
					currentDistance += dist;

					currentIndex++;
					if (currentDistance > offsetDistance)
					{
						break;
					}
				}

				currentIndex--; // 
				if (currentDistance >= offsetDistance)
					status = true;
				else
					status = false;
			}

			return currentIndex;
		}

		inline float GetCurvature(int index, int line) const
		{
			const AITrackDataSegments *pSegment = &m_Segments[index];
			float c = pSegment->GetCurvature(line);
			return c;
		}

		void SetCurvature(int index, int line, float c)
		{
			AITrackDataSegments *pSegment = &m_Segments[index];
			pSegment->SetCurvature(line, c);
		}

		struct SpeedData
		{
			struct CarSpeedData
			{
				struct LineSpeedData
				{
					Btl::Vector<U8> speeds;
				};

				Btl::Vector<LineSpeedData> lines;
			};

			Btl::Vector<CarSpeedData, Btl::VectorParameters_Array<20> > m_speeds; // per car

			void Initialise(int nSegments)
			{
				for (int i = 0; i < m_speeds.Size(); i++)
				{
					m_speeds[i].lines.Resize(AITrackDataLines::TRACK_DATA_NUM_LINES);

					for (int j = 0; j < m_speeds[i].lines.Size(); j++)
					{
						m_speeds[i].lines[j].speeds.Resize(nSegments);
						memset(&m_speeds[i].lines[j].speeds[0], 0, nSegments);
					}
				}
			}

			U8 GetSpeed(int index, int line, int speedIndex) const
			{
				return m_speeds[speedIndex].lines[line].speeds[index];
			}

			void SetSpeed(int index, int line, U8 speed, int speedIndex)
			{
				m_speeds[speedIndex].lines[line].speeds[index] = speed;
			}
		};

		SpeedData m_speedData;

		void SetSpeed(int index, int line, U8 speed, int speedIndex)
		{
			//AITrackDataSegments *pSegment = &m_Segments[index];
			//pSegment->SetSpeed(line, speed, speedIndex);

			m_speedData.SetSpeed(index, line, speed, speedIndex);
		}

		inline U8 GetSpeed(int index, int line, int speedIndex) const
		{
			//const AITrackDataSegments *pSegment = &m_Segments[index];
			//U8 speed = pSegment->GetSpeed(line, speedIndex);

			return m_speedData.GetSpeed(index, line, speedIndex);
		}

		inline U8 GetSpeedMin(int from, int dist, int line, int carNo) const	
		{
			U8 speed = 255;
			int to = Maths::Min( from + dist, m_Segments.Size() );
	
			for(int i=from; i<to; i++)
			{
				//const AITrackDataSegments *pSegment = &m_Segments[i];
				//U8 speedCurrent = pSegment->GetSpeed(line, carNo);

				U8 speedCurrent = GetSpeed(i, line, carNo);

				speed = Maths::Min(speed, speedCurrent);
			}

			return speed;
		}



		/*void AddSpeed(int index, int line)
		{
			AITrackDataSegments *pSegment = &m_Segments[index];
			pSegment->AddSpeed(line);
		}*/

		inline float GetMaxCurvature(int from, int to, int line, int &index, bool &status) const
		{
			// TODO : Non looped tracks.....
			assert(from >=0 && from < m_Segments.Size() && to >=0 && to < m_Segments.Size());

			float curvature = -FLT_MAX;
			if (from <= to)
			{

				for(int i=from; i<=to; i++)
				{
					float value = Maths::Abs(GetCurvature(i, line));

					if (value > curvature)
					{	
						curvature = value;
						index = i;
					}	
				}
			}
			else
			{

				for(int i=from; i<m_Segments.Size(); i++)
				{
					float value = Maths::Abs(GetCurvature(i, line));

					if (value > curvature)
					{	
						curvature = value;
						index = i;
					}
				}


				for(int i=0; i<=to; i++)
				{
					float value = Maths::Abs(GetCurvature(i, line));

					if (value > curvature)
					{	
						curvature = value;
						index = i;
					}	
				}

			}

			return curvature;
		}

		

	};

	// not sure this should be here
	class TrackLogicData
	{
	public:
		TrackLogicData() : m_beenCalculated(false), m_pLogicRoute(0), m_startSamplePoint(-1), m_finishSamplePoint(-1),
							m_inter1SamplePoint(-1),m_inter2SamplePoint(-1), m_PointToPoint(false) {};
		~TrackLogicData(){};

		int		m_startSamplePoint;
		float	m_startSamplePos;

		int		m_finishSamplePoint;
		float	m_finishSamplePos;

		int		m_inter1SamplePoint;
		float	m_inter1SamplePos;

		int		m_inter2SamplePoint;
		float	m_inter2SamplePos;

		bool	m_beenCalculated;
		
		float	m_invSize;
		float	m_noSamples;
		float	m_stepSize;
		
		// point to point race variables.....
		bool	m_PointToPoint;
		float	m_invSize_PointToPoint;

	

		AITrackDataRoutes	*m_pLogicRoute;

	};

	class RouteInfo
	{
	public:

		enum VOLUME_SIDE
		{
			ENTERED_FRONT = 0,
			ENTERED_REAR = 1,
			ENTERED_SIDE = 2,
			EXIT_FRONT = 4,
			EXIT_REAR = 8,
			EXIT_SIDE = 16,
			NUM_THINGS
		};

		RouteInfo() {}

		RouteInfo(int index, AITrackDataRoutes *pRoute, bool in, int routeIndex) :	m_pRoute(pRoute),
			m_index(index), 
			m_in(in),
			m_dist(-1.0f), 
			m_width(0.0f),  
			m_ZDist(0.0f),
			m_offset(0.0f),
			m_offsetCentre(0.0f),
			m_pos(0.0f),
			m_justEntered(false),
			m_enteredExitedFrom(ENTERED_FRONT),
			m_RouteIndex(routeIndex){};
		~RouteInfo(){};

		AITrackDataRoutes	*m_pRoute;
		
		float				m_dist;
		float				m_width;
		float				m_ZDist;
		float				m_offset;			// offset from centre of racing line
		float				m_offsetCentre;		// offset from centre of route
		float				m_pos;

		S16					m_RouteIndex;		// for quick look up.
		S16					m_enteredExitedFrom;

		S16					m_index;			// spline position index
		bool				m_justEntered;
		bool				m_in;

		int NearestLane()
		{
			if (m_pRoute)
				return m_pRoute->NearestLane(m_offset);
			else
				return -1;
		}

		/*void *operator new (size_t size)
		{
			return BizAlignedMalloc(size, 16);
		}

		void *operator new [](size_t size)
		{
			return BizAlignedMalloc(size, 16);
		}

		void operator delete(void *p)
		{
			return BizAlignedFree(p);
		}

		void operator delete[](void *p)
		{
			return BizAlignedFree(p);
		}*/
	};


	class AITrackData
	{
	public:
		AITrackData(){};
		~AITrackData()
		{
			for(int i=0; i<m_Routes.Size(); i++)
			{
				AITrackDataRoutes *pRoute = m_Routes[i];
				delete pRoute;
			}

			m_Routes.Clear();

		};

		// list of routes.......
		Btl::Vector<AITrackDataRoutes *>	m_Routes;
		
		TrackLogicData						m_TrackLogicData;


		// Helper functions..............................

		inline void AddRoute(AITrackDataRoutes *pRoute)
		{
			assert(pRoute);
			m_Routes.PushBack(pRoute);
		}


		inline AITrackDataRoutes *GetRoute(int index) const 
		{
			assert(index >=0 && index < m_Routes.Size());

			AITrackDataRoutes *pRoute = m_Routes[index];

			return pRoute;
		}

		inline AITrackDataRoutes *GetRoute(CHash name) const 
		{
			for(int i=0; i<m_Routes.Size(); i++)
			{
				AITrackDataRoutes *pRoute = m_Routes[i];
				if (pRoute->m_Name == name)
				{
					return pRoute;
				}
			}

			return 0;
		}

		inline int GetNumRoutes() const {return m_Routes.Size();}

		
	};


	//struct ResetInfo
	//{
	//	RouteInfo		*m_pResetRoute;
	//	Maths::HVec3	m_position;
	//	Maths::HVec3	m_SplineDirection;
	//	float			m_speed; //mps
	//	int				m_ResetRouteIndex;
	//	int				m_LogicIndex;
	//};

	class ResetInfo
	{
	public:
		ResetInfo(): m_pResetRoute(NULL), m_speed(-1.f), m_ResetRouteIndex(-1), m_LogicIndex(-1)
		{
		}

		~ResetInfo()
		{
		}

		RouteInfo		*m_pResetRoute;
		Maths::HVec3	m_position;
		Maths::HVec3	m_SplineDirection;
		float			m_speed; //mps
		int				m_ResetRouteIndex;
		int				m_LogicIndex;
	};


}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif



#endif
