#ifndef _PARAMSAI_I_H
#define _PARAMSAI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>
#include <paramsai/driverdata.h>

namespace airacingtech
{

	BEGIN_INTERFACE_DEC(IParamsAI)

	public:
		
	virtual const DriverData *GetDriverData() const = 0;

	END_INTERFACE_DEC
}



#endif //_PARAMSAI_I_H
