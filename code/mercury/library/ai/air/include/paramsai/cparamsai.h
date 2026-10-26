#ifndef _PARAMSAI_C_H
#define _PARAMSAI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <paramsai/iparamsai.h>

namespace airacingtech
{

	BEGIN_COMPONENT_DEC(CParamsAI, IParamsAI)
	
	public:

		void CopyDriverData(DriverData *pNew);

		const DriverData *GetDriverData() const {return &m_Params;}
	
	private:
		
		DriverData m_Params;

	END_COMPONENT_DEC
}







#endif //_PARAMSAI_C_H
