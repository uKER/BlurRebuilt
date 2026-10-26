#ifndef _TRACKDATA_AI_C_H
#define _TRACKDATA_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <trackdata/itrackdata.h>
#include <trackdata/trackdata.h>

namespace airacingtech
{

	BEGIN_COMPONENT_DEC(CTrackDataAI, ITrackDataAI)

	public:

		AITrackData *GetTrackData(){return &m_TrackData;}
		
		void CalculateTrackData();
		float WorkOutOffsetToRacingSpline(AITrackDataRoutes *pRoute, Maths::HVec3 pos, int index);
		float CalculateLogicPositionFromIndex(int index);		

	private:
				
		void BuildTrackDataCache();
		bool PushDataBack(AITrackDataCache *pDataCache, AITrackDataRoutes *pTestRoute, AITrackDataRoutes *pLogicRoute, float offset, int segments, int i, int maxInt);
		
		void CalculateCurvatureNew(AITrackDataRoutes *pRoute);
		
		AITrackData	m_TrackData;

	
	END_COMPONENT_DEC



}

#endif //_TRACKDATA_AI_C_H
