#ifndef _AI_SIDESWIPE_MESSAGE_H
#define _AI_SIDESWIPE_MESSAGE_H

#include <ComponentSystem/CObjectId.h>

#ifdef _MSC_VER
#pragma once
#endif

namespace airacingtech
{
	class AISideSwipeMSG
	{
	public:

		AISideSwipeMSG(int aiDoingIndex, int aiAgainstIndex, float approxdistfromPlayer) :  m_indexAIDoingSideSwipe(aiDoingIndex), 
																							m_indexAIDoingSideSwipeAgainst(aiAgainstIndex),
																							m_approxDistFromPlayerAlongTrack(approxdistfromPlayer)
		{};
		~AISideSwipeMSG(){};

		int			m_indexAIDoingSideSwipe;
		int			m_indexAIDoingSideSwipeAgainst;
		float		m_approxDistFromPlayerAlongTrack;


	};
}



#endif //_AI_SIDESWIPE_MESSAGE_H