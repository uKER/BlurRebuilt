#ifndef DISPLAY_GLMANAGER_H
#define DISPLAY_GLMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Manager/ManagerInterface.h"

namespace NewDisplay
{
namespace GL
{

class C_GLManager : public I_Manager
{
public:

	C_GLManager();
	~C_GLManager();

protected:

	bool Create(C_DeviceParams& deviceParams);
};

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLMANAGER_H
