#include "stdafx.h"
#include <trackdata/trackdata.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{

#ifdef PLATFORM_PS3
#define BRANCH_HINT_LIKELY( condition )		__builtin_expect( condition, 1 )
#define BRANCH_HINT_UNLIKELY( condition )	__builtin_expect( condition, 0 )
#else
#define BRANCH_HINT_LIKELY( condition )		condition
#define BRANCH_HINT_UNLIKELY( condition )	condition
#endif

const int AITrackDataLines::TRACK_DATA_LINE_RIGHT_5 = 0;
const int AITrackDataLines::TRACK_DATA_LINE_RIGHT_4 = 1;
const int AITrackDataLines::TRACK_DATA_LINE_RIGHT_3 = 2;
const int AITrackDataLines::TRACK_DATA_LINE_RIGHT_2 = 3;
const int AITrackDataLines::TRACK_DATA_LINE_RIGHT_1 = 4;
const int AITrackDataLines::TRACK_DATA_LINE_RACING = 5;
const int AITrackDataLines::TRACK_DATA_LINE_LEFT_1 = 6;
const int AITrackDataLines::TRACK_DATA_LINE_LEFT_2 = 7;
const int AITrackDataLines::TRACK_DATA_LINE_LEFT_3 = 8;
const int AITrackDataLines::TRACK_DATA_LINE_LEFT_4 = 9;
const int AITrackDataLines::TRACK_DATA_LINE_LEFT_5 = 10;
const int AITrackDataLines::TRACK_DATA_NUM_LINES = 11;
const int AITrackDataLines::TRACK_DATA_EXTREME_LEFT = 12;
const int AITrackDataLines::TRACK_DATA_EXTREME_RIGHT = 13;

#if 0


#if 0

	
	int AITrackDataRoutes::FindNearest(int startIndex, int endIndex, Maths::HVec3 &position, float &dist, float step)
	{
		PERF_EVENT("AI.FindNearest");

		int index = -1;

		if (BRANCH_HINT_LIKELY( IsInBounds(position) ) )
		{
			float positionX = position.X();
			float positionY = position.Y();
			float positionZ = position.Z();

			bool heightTest = true;
			if (BRANCH_HINT_UNLIKELY(IsLogic() || IsStart() || IsFinish() || IsIntermediate(1) || IsIntermediate(2)))
			{
				heightTest = false;
			}

			for(int i=startIndex; BRANCH_HINT_LIKELY( i<=endIndex ); i++)
			{
				Maths::HVec3 BottomLeft, TopLeft, BottomRight, TopRight;
				GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i , TopLeft);

				bool test = true;
				if (BRANCH_HINT_LIKELY(heightTest))
				{
					float minY, maxY;
					GetHeightBounds(minY, maxY, i);

					//if (position.Y() > TopLeft.Y() - 2.0f && position.Y() < TopLeft.Y() + 4.0f)
					if (BRANCH_HINT_LIKELY(positionY >= minY && positionY <= maxY))
					{
						test = true;
					}
					else
					{
						test = false;
					}

				}

				// quick bounds check here
				if (BRANCH_HINT_LIKELY(test == true))
				{
					Maths::HVec3 PosTest(positionX, 0.0f, positionZ);
					GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i , TopRight);
					float topSide = airacingtech::SideOfLine(PosTest, TopLeft, TopRight);

					if (BRANCH_HINT_UNLIKELY(topSide >= 0.0f))
					{
						GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i-1 , BottomRight);
						GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i-1 , BottomLeft);
						float bottomSide = airacingtech::SideOfLine(PosTest, BottomRight, BottomLeft);
						if (BRANCH_HINT_UNLIKELY(bottomSide >= 0.0f))
						{
							float leftSide = airacingtech::SideOfLine(PosTest, BottomLeft, TopLeft);
							if (BRANCH_HINT_LIKELY(leftSide >= 0.0f))
							{
								float rightSide = airacingtech::SideOfLine(PosTest, TopRight, BottomRight);
								if (BRANCH_HINT_LIKELY(rightSide >= 0.0f))
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
			}
		}

		return index;
	}

#else


	int AITrackDataRoutes::FindNearest(int startIndex, int endIndex, Maths::HVec3 &position, float &dist, float step)
	{
		PERF_EVENT("AI.FindNearest");

		int index = -1;

		if (IsInBounds(position))
		{
			bool heightTest = true;
			if (IsLogic() || IsStart() || IsFinish() || IsIntermediate(1) || IsIntermediate(2))
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



	int AITrackDataRoutes::FindNearestNoHeightTest(int startIndex, int endIndex, Maths::HVec3 &position, float &dist, float step)
	{
		PERF_EVENT("AI.FindNearestNoHeight");

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

		return index;
	}

	int AITrackDataRoutes::FindNearestHeightTest(int startIndex, int endIndex, Maths::HVec3 &position, float &dist, float step)
	{
		PERF_EVENT("AI.FindNearestHeight");
		int index = -1;

		for(int i=startIndex; i<=endIndex; i++)
		{

#if 1
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
			Maths::HVec3 BottomLeft, TopLeft, BottomRight, TopRight;
			Maths::HVec3 PosTest(position.X(), 0.0f, position.Z());

			GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i , TopLeft);
			GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i , TopRight);
			GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i-1 , BottomRight);
			GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i-1 , BottomLeft);
			float minY, maxY;
			GetHeightBounds(minY, maxY, i);

			if (position.Y() >= minY && position.Y() <= maxY)
			{
				float topSide = airacingtech::SideOfLine(PosTest, TopLeft, TopRight);
				float bottomSide = airacingtech::SideOfLine(PosTest, BottomRight, BottomLeft);
				float leftSide = airacingtech::SideOfLine(PosTest, BottomLeft, TopLeft);
				float rightSide = airacingtech::SideOfLine(PosTest, TopRight, BottomRight);

				if (topSide >= 0.0f && bottomSide >= 0.0f && leftSide >= 0.0f && rightSide >=0.0f)
				{
					index= i;

					float dist1 = airacingtech::DistToLine(PosTest,BottomRight,BottomLeft);
					float dist2 =  Maths::Abs(airacingtech::DistToLine(PosTest,TopRight,TopLeft));
					float totalDist = dist1 + dist2;
					dist = dist1 / totalDist;
					break; 
				}
			}
		

#endif
		}
		

		return index;
	}

#endif



#endif
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
