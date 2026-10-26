#ifndef VEHICLEFEEDPARAM_H
#define VEHICLEFEEDPARAM_H

#include <Xt/XtGame.h>
#include "AmaxChaseParam.h"
#include "BoltedParam.h"

namespace AmaxCam
{

class C_VehicleFeedParam
{
public:
	C_VehicleFeedParam();

	XtHandle<C_AmaxChaseParam> m_Chase;
	C_BoltedParam m_Bonnet;
};

} // End of namespace

#endif
