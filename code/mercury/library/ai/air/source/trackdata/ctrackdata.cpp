#include "stdafx.h"
#include <trackdata/ctrackdata.h>
#include <fuzzylib/fuzzylibone/fuzzyone.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


namespace airacingtech
{

	using namespace fuzzy_lib_one;

	CTrackDataAI::CTrackDataAI() 
	{
		
	}

	CTrackDataAI::~CTrackDataAI()
	{

	}

	void CTrackDataAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_TRACK_DATA_AI, CID_TRACKDATA_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_TRACKDATA_AI, CTrackDataAI::CreateMe, CTrackDataAI::DestroyMe, CHash("TrackData"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_TRACKDATA_AI, MT_POST_INIT_AI_SYSTEM); 
		
	}

	IComponent *CTrackDataAI::CreateMe()
	{
		return new CTrackDataAI;
	}

	bool CTrackDataAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CTrackDataAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}


	void CTrackDataAI::Deinit(void)
	{
		AITrackData *pAITrackData = GetTrackData();

		if (pAITrackData)
		{
			AITrackDataRoutes *pLogicRoute = pAITrackData->m_TrackLogicData.m_pLogicRoute;
			if (pLogicRoute)
			{
				pLogicRoute->m_dataCache.Destroy();
			}
		}
	}

	EMessageResult CTrackDataAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch(msg.mType)
		{
			case MT_POST_INIT_AI_SYSTEM:
				{
					CalculateTrackData();

					// This adds 15 seconds approx to the loading times on the la river track...who cares for this prototype...
					// TODO : move this into the the ai data file......
					BuildTrackDataCache();
				}
				break;
	
			default:
				break;
		}

		return msgRet;
	}


	EComponentTypeId CTrackDataAI::GetComponentTypeId(void)
	{
		return CID_TRACKDATA_AI;
	}

	bool CTrackDataAI::PushDataBack(AITrackDataCache *pDataCache, AITrackDataRoutes *pTestRoute, AITrackDataRoutes *pLogicRoute, float offset, int segments, int i, int maxInt)
	{
		bool result = false;
		Maths::HVec3 position(Maths::UNDEFINED);
		pTestRoute->GetPositionFromOffset(Maths::HF32(offset),segments, position);
		bool inSegment = pLogicRoute->InSegmentBounds(position, Maths::Min(i+1, maxInt) );
		if (inSegment == true)
		{
			// we are in the segment, no need to keep checking this segment.....
			AITrackDataCachedData aiTDCD(pTestRoute, segments);

			pDataCache->m_data.PushBack(aiTDCD);

			result = true;
		}

		return result;
	}



	void CTrackDataAI::BuildTrackDataCache()
	{
		// This may take some time.......here we go
		AITrackData *pAITrackData = GetTrackData();

		if (pAITrackData)
		{
			AITrackDataRoutes *pLogicRoute = pAITrackData->m_TrackLogicData.m_pLogicRoute;
			if (pLogicRoute && (pLogicRoute->m_dataCache.Size() == 0) )
			{
				int noRoutes = pAITrackData->GetNumRoutes();

				// Everything will hang off the logic route, we should never be outside this zone.....
				int maxInt = pLogicRoute->GetNumSegments()-1;

				pLogicRoute->m_dataCache.Reserve(pLogicRoute->GetNumSegments());

				for(int i=0; i<pLogicRoute->GetNumSegments(); i++)
				{
					pLogicRoute->m_dataCache.PushBack();
					AITrackDataCache *pDataCache = &pLogicRoute->m_dataCache.Back();

					for(int routes=0; routes < noRoutes; routes++)
					{
						AITrackDataRoutes *pTestRoute = pAITrackData->GetRoute(routes);
						if (pTestRoute != pLogicRoute) // don`t test against ourselves....
						{
							for(int segments=0; segments < pTestRoute->GetNumSegments(); segments++)
							{			
								Maths::HF32 offsetLeftExtreme = pTestRoute->GetOffset(segments, AITrackDataLines::TRACK_DATA_EXTREME_LEFT);
								if (!PushDataBack(pDataCache, pTestRoute, pLogicRoute, offsetLeftExtreme, segments, i, maxInt))
								{
									Maths::HF32 offsetRightExtreme = pTestRoute->GetOffset(segments, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT);
									if (!PushDataBack(pDataCache, pTestRoute, pLogicRoute, offsetRightExtreme, segments, i, maxInt))
									{
										// check for intersection

										Maths::HVec3 start;
										pTestRoute->GetPositionFromOffset(offsetLeftExtreme,segments, start);
										Maths::HVec3 end;
										pTestRoute->GetPositionFromOffset(offsetRightExtreme,segments, end);

										bool intersects = pLogicRoute->IntersectSegmentBounds(start,end, Maths::Min(i+1, maxInt) );
										if (intersects == true)
										{
											// we are in the segment, no need to keep checking this segment.....
											AITrackDataCachedData aiTDCD(pTestRoute, segments);
											pDataCache->m_data.PushBack(aiTDCD);
										}
									}
								}
							}
						}
					}
				}

				// TODO : looped track
				//if (pLogicRoute->m_RouteFlags & ROUTE_LOOPED)
				//{

				//}
			}
		}
	}


	void CTrackDataAI::CalculateTrackData()
	{
		AITrackData *pAITrackData = GetTrackData();

		AITrackDataRoutes *pLogicRoute = 0;	

		int noRoutes = pAITrackData->GetNumRoutes();
		for(int i=0; i<noRoutes; i++)
		{
			AITrackDataRoutes *pRoute = pAITrackData->GetRoute(i);
			pRoute->CalculateBounds();
			pRoute->SetStepSize(4.0f);

			// Calculate a new curvature, as exported data is not accurate
			// Should move into the tools really, this is just temporary, depending upon how long it takes...
			//CalculateCurvatureNew(pRoute);


			if (pRoute->IsLogic())
			{
				pLogicRoute = pRoute;
			}

			if (pRoute->IsDynamicJunction())
			{
				pRoute->m_Active = false;
			}
			else
			{
				pRoute->m_Active = true;
			}

			// not using the kdtree at this moment in time...
			//pRoute->PopulateKDTree();
		}


		// Calculate the logic needed for the ai, not used by old
		if (pLogicRoute) 
		{
			pLogicRoute->SetStepSize(4.0f);
			pAITrackData->m_TrackLogicData.m_beenCalculated = true;
			pAITrackData->m_TrackLogicData.m_invSize = 1.0f / pLogicRoute->GetNumSegments();
			pAITrackData->m_TrackLogicData.m_noSamples = (float)pLogicRoute->GetNumSegments();
			pAITrackData->m_TrackLogicData.m_stepSize = pLogicRoute->GetStepSize();
			pAITrackData->m_TrackLogicData.m_pLogicRoute = pLogicRoute;
			
			for(int i=0; i<noRoutes; i++)
			{
				AITrackDataRoutes *pRoute = pAITrackData->GetRoute(i);
				Maths::HVec3 testPoint;
				int index = -1;
				float paramDist;

				Maths::HF32 pointFive(0.5f);

				if (pRoute->IsStart())
				{
					

					int endSegment = 2;
					pRoute->GetPositionFromOffset(pointFive,endSegment,testPoint);
					index = pLogicRoute->FindNearest(1, pLogicRoute->GetNumSegments()-1, testPoint, paramDist, pLogicRoute->GetStepSize());
										
					if (index != -1)
					{
						pAITrackData->m_TrackLogicData.m_startSamplePoint = index;	
					}
					else
					{
						BIZ_ASSERT(index >= 0);
					}
				}

				if (pRoute->IsFinish())
				{
					pAITrackData->m_TrackLogicData.m_PointToPoint = true;
					//int endSegment = pRoute->GetNumSegments() - 1;
					//endSegment /= 2;

					int endSegment = 2;
					pRoute->GetPositionFromOffset(pointFive,endSegment,testPoint);
					index = -1;
					index = pLogicRoute->FindNearest(1, pLogicRoute->GetNumSegments()-1, testPoint, paramDist, pLogicRoute->GetStepSize());
					if (index != -1)
					{
						pAITrackData->m_TrackLogicData.m_finishSamplePoint= index;	
					}
					else
					{
						BIZ_ASSERT(index >= 0);
					}
				}

				if (pRoute->IsIntermediate(1))
				{
					//int endSegment = pRoute->GetNumSegments() - 1;
					//endSegment /= 2;

					int endSegment = 2;
					pRoute->GetPositionFromOffset(pointFive,endSegment,testPoint);
					index = -1;
					index = pLogicRoute->FindNearest(1, pLogicRoute->GetNumSegments()-1, testPoint, paramDist, pLogicRoute->GetStepSize());
					if (index != -1)
					{
						pAITrackData->m_TrackLogicData.m_inter1SamplePoint= index;	
					}
					else
					{
						BIZ_ASSERT(index >= 0);
					}
				}

				if (pRoute->IsIntermediate(2))
				{

					//int endSegment = pRoute->GetNumSegments() - 1;
					//endSegment /= 2;
					int endSegment = 2;
					pRoute->GetPositionFromOffset(pointFive,endSegment,testPoint);
					index = -1;
					index = pLogicRoute->FindNearest(1, pLogicRoute->GetNumSegments()-1, testPoint, paramDist, pLogicRoute->GetStepSize());
					if (index != -1)
					{
						pAITrackData->m_TrackLogicData.m_inter2SamplePoint= index;	
					}
					else
					{
						BIZ_ASSERT(index >= 0);
					}
				}

			}


			if (pAITrackData->m_TrackLogicData.m_PointToPoint)
			{
				pAITrackData->m_TrackLogicData.m_invSize_PointToPoint = 1.0f / (pAITrackData->m_TrackLogicData.m_finishSamplePoint - pAITrackData->m_TrackLogicData.m_startSamplePoint);
			}
		}
		



	}

	void CTrackDataAI::CalculateCurvatureNew(AITrackDataRoutes *pRoute)
	{
		
		if (pRoute->IsChangeableRoute())
		{
			// ok here we go........

			for(int i=0; i<pRoute->m_Segments.Size(); i++)
			{
				for(int line=0; line<AITrackDataLines::TRACK_DATA_NUM_LINES; line++)
				{
					float exportedCurvature = pRoute->GetCurvature(i, line);
					
					// we now need to build a spline
					if (pRoute->IsLoopedTrack())
					{
						int prev = i-1;
						if (prev < 0)
							prev += pRoute->m_Segments.Size();
						
						int next1 = i+1;
						if (next1 >= pRoute->m_Segments.Size())
							next1 -= pRoute->m_Segments.Size();
						
						int next2 = i+2;
						if (next2 >= pRoute->m_Segments.Size())
							next2 -= pRoute->m_Segments.Size();

						// now get the position 
						Maths::HVec3 pos1;
						pRoute->GetPositionFromLine(line, prev, pos1);

						Maths::HVec3 pos2;
						pRoute->GetPositionFromLine(line, i, pos2);

						Maths::HVec3 pos3;
						pRoute->GetPositionFromLine(line, next1, pos3);

						Maths::HVec3 pos4;
						pRoute->GetPositionFromLine(line, next2, pos4);


						// now build a spline....
						Btl::Vector<Maths::HVec3>  ctrlPoints;
						Btl::Vector<Maths::HVec3>  outputPoints;
						ctrlPoints.PushBack(pos1);
						ctrlPoints.PushBack(pos2);
						ctrlPoints.PushBack(pos3);
						ctrlPoints.PushBack(pos4);

						B_Spline_ApproxNonLooped(ctrlPoints, outputPoints, 0.1f, 0.0025f); //  allows us to get to 1m samples
						
						float maxCurature = FLT_MIN;
						float minCurvature = FLT_MAX;
						if (outputPoints.Size() >=3)
						{
							for(int op=2; op<outputPoints.Size(); op++)
							{
								Maths::HVec3 pos0 = outputPoints[op-2];
								Maths::HVec3 pos1 = outputPoints[op-1];
								Maths::HVec3 pos2 = outputPoints[op];

								float c = CalculateCurvature2(pos0, pos1, pos2);
								float cc = CalculateCurvature(pos0, pos1, pos2);

								if (Maths::Abs(c) > maxCurature)
									maxCurature = Maths::Abs(c);

								if (Maths::Abs(c) < minCurvature)
									minCurvature = Maths::Abs(c);

							}
						}
						else
						{
							minCurvature = exportedCurvature;
						}

						if (exportedCurvature < 0.0)
						{
							minCurvature = -Maths::Abs(minCurvature);

							minCurvature = Maths::Min(exportedCurvature, minCurvature);
						}
						else
						{
							minCurvature = Maths::Abs(minCurvature);
							minCurvature = Maths::Min(exportedCurvature, minCurvature);
						}
										
						float speedExported  = CalculateSpeedMPS(exportedCurvature);
						float speed= CalculateSpeedMPS(maxCurature);
						float speed2= CalculateSpeedMPS(minCurvature);

						
						pRoute->SetCurvature(i, line, minCurvature);


						ctrlPoints.Clear();
						outputPoints.Clear();
					}
					else
					{
						if (i==0)
						{
				
						}
						else
						{
							int prev = i-1;
							int next1 = i+1;
							int next2 = i+2;

							int nosamples = pRoute->m_Segments.Size();

							if (next2 < pRoute->m_Segments.Size())
							{
								// now get the position 
								Maths::HVec3 pos1;
								pRoute->GetPositionFromLine(line, prev, pos1);

								Maths::HVec3 pos2;
								pRoute->GetPositionFromLine(line, i, pos2);

								Maths::HVec3 pos3;
								pRoute->GetPositionFromLine(line, next1, pos3);

								Maths::HVec3 pos4;
								pRoute->GetPositionFromLine(line, next2, pos4);


								// now build a spline....
								Btl::Vector<Maths::HVec3>  ctrlPoints;
								Btl::Vector<Maths::HVec3>  outputPoints;
								ctrlPoints.PushBack(pos1);
								ctrlPoints.PushBack(pos2);
								ctrlPoints.PushBack(pos3);
								ctrlPoints.PushBack(pos4);

								B_Spline_ApproxNonLooped(ctrlPoints, outputPoints, 0.5f, 0.025f); //  allows us to get to 1m samples

								float maxCurature = FLT_MIN;
								float minCurvature = FLT_MAX;
								if (outputPoints.Size() >=3)
								{
									for(int op=2; op<outputPoints.Size(); op++)
									{
										Maths::HVec3 pos0 = outputPoints[op-2];
										Maths::HVec3 pos1 = outputPoints[op-1];
										Maths::HVec3 pos2 = outputPoints[op];

										float c = CalculateCurvature2(pos0, pos1, pos2);

										float cc = CalculateCurvature(pos0, pos1, pos2);

										if (Maths::Abs(c) > maxCurature)
											maxCurature = Maths::Abs(c);

										if (Maths::Abs(c) < minCurvature)
											minCurvature = Maths::Abs(c);
									}
								}
								else
								{
									minCurvature = exportedCurvature;
								}

								if (exportedCurvature < 0.0)
								{
									minCurvature = -Maths::Abs(minCurvature);

									minCurvature = Maths::Min(exportedCurvature, minCurvature);
								}
								else
								{
									minCurvature = Maths::Abs(minCurvature);
									minCurvature = Maths::Min(exportedCurvature, minCurvature);
								}

								float speedExported  = CalculateSpeedMPS(exportedCurvature);
								float speed = CalculateSpeedMPS(maxCurature);
								float speed2 = CalculateSpeedMPS(minCurvature);


								pRoute->SetCurvature(i, line, minCurvature);


								ctrlPoints.Clear();
								outputPoints.Clear();	
							}
							else
							{
								float c = pRoute->GetCurvature(i-1, line);
								pRoute->SetCurvature(i, line, c);
								if (i+1 < nosamples)
									pRoute->SetCurvature(i+1, line, c);
								break;
							}
								
							if (i == 1)
							{
								float c = pRoute->GetCurvature(i, line);
								pRoute->SetCurvature(i-1, line, c);
							}
						}
					}

				}
			}
		}
	}


	float CTrackDataAI::CalculateLogicPositionFromIndex(int index)
	{
		float logicPosition = 0.0f;

		AITrackData *pAITrackData = GetTrackData();
		if (pAITrackData)
		{
			AITrackDataRoutes *pLogicRoute = pAITrackData->m_TrackLogicData.m_pLogicRoute;
			if (pLogicRoute)
			{
				if (pAITrackData->m_TrackLogicData.m_PointToPoint == true)
				{
					logicPosition = (index - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
					if (logicPosition < 0.0f)
						logicPosition += 1.0f;
				}
				else
				{
					logicPosition = (index - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
					if (logicPosition < 0.0f)
						logicPosition += 1.0f;
				}
			}
		}

		return logicPosition;
	}
	
	float CTrackDataAI::WorkOutOffsetToRacingSpline(AITrackDataRoutes *pRoute, Maths::HVec3 pos, int index)
	{
		Maths::HVec3 position = pos;
		Maths::HVec3 startPos;
		Maths::HVec3 endPos;
		float fzOffset = 0.0f;

		// work out the distance to the lines...

		int startIndex = -1;
		int endIndex = -1;
		// distance to the lines.......
		if (pRoute->IsLoopedTrack())
		{
			startIndex = index; 
			endIndex = pRoute->GetIndexOffset(index, 1);	
		}
		else
		{
			// assuming that the spline is longer than 4 samples in length
			startIndex = index; 
			endIndex = pRoute->GetIndexOffset(index, 1);
			if (endIndex <= startIndex)
			{
				endIndex = startIndex;
				startIndex--;
			}
		}

		pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, startIndex , startPos);
		pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, endIndex , endPos);	

		// side distance is ? this has to be positive...due to using the extreme right position in the calculation.
		float d = Maths::Abs(airacingtech::DistToLine(position, startPos, endPos));

		// paramatise the distance
		float trackWidth = pRoute->GetWidth(startIndex);
		float paramD = d / trackWidth;
		paramD = Maths::Clamped(paramD, 0.0f, 1.0f );
		

		float offsetRacing = pRoute->GetOffset(startIndex, AITrackDataLines::TRACK_DATA_LINE_RACING);

		if (paramD > offsetRacing)
		{
			// on the left of the racing line
			float offsetLeft = pRoute->GetOffset(startIndex, AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
			fzOffset = -Maths::Abs(FuzzyRightShoulder(paramD, offsetRacing, offsetLeft ));
		}
		else
		{
			// on the right of the racing line

			// right of the line
			float offsetRight = pRoute->GetOffset(startIndex, AITrackDataLines::TRACK_DATA_LINE_RIGHT_5);
			fzOffset = 1.0f - FuzzyRightShoulder(paramD, offsetRight, offsetRacing );
			
			
		}

		return fzOffset;
	}

	
}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


