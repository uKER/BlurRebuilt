#ifndef _PERK_NITRO_C_H
#define _PERK_NITRO_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <perks/perknitro/iperknitro.h>

namespace airacingtech
{

	class IData;
	class IGameServerTime;
	class IAvoidance;


	BEGIN_COMPONENT_DEC(CPerkNitro, IPerkNitro)


		void Evaluate();

	private:

		void CachedInterfaces();

		IData				*m_pIData;
		IGameServerTime		*m_pIGameServerTime;
		IAvoidance			*m_pIAvoidance;

	END_COMPONENT_DEC
}




#endif //_PERK_NITRO_C_H
