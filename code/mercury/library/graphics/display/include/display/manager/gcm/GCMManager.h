#ifndef DISPLAY_MANAGER_GCM_GCM_MANAGER_H
#define DISPLAY_MANAGER_GCM_GCM_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Manager/ManagerBase.h"

namespace Display
{
namespace GCM
{

//! C_GCMManager

class C_GCMManager : public C_ManagerBase
{
public:

	C_GCMManager();
	~C_GCMManager();

	static U32 GetCBSize();
	static U32 GetIOSize();
	static void* GetIOAddress();

	bool IsInitialised();
	bool Initialise();
	void Shutdown();

	static bool GetVideoMode(C_VideoMode* pVideoMode);
	
private:

	#ifdef USE_GCM_REPLAY
	void SetCaptureBuffers();
	#endif

protected:

	static bool			s_Initialised;

private:

	static U32			s_cbSize;
	static U32			s_ioSize;
	static void*		s_ioAddress;
};

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_MANAGER_GCM_GCM_MANAGER_H
