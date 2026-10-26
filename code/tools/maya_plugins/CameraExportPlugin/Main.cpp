
#include "stdafx.h"

#define NT_PLUGIN
#pragma warning (disable:4100)
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>

#pragma warning (push,3)
#include <aftk/Aftk.h>
#pragma warning (pop)

#include "CamCom.h"
#include "Xenon.h"
#include "SceneSend.h"
#include "AnimSend.h"

int MainVer = 0x1182;

void dprintf(char *format, ...)
{
	va_list val = 0;
	va_start(val, format);

	vfprintf(stdout, format, val);
	fflush(stdout);

	va_end(val);
}

// -----------------------------------------------------------------------------
class Biz_dh_c2export : public MPxCommand
{
public:
	static void *creator() { return new Biz_dh_c2export(); }
	virtual MStatus doIt(const MArgList&)
	{
		unsigned int ip = XenonAltAddress();

		dprintf("biz_dh_c2export v%.4x running\n", MainVer);
		g_CamCom.Connect(ip);
		SceneSend();
		g_CamCom.Disconnect();
		dprintf("\n");

		return MS::kSuccess;
	}
};

class Biz_dh_c2anim : public MPxCommand
{
public:
	static void *creator() { return new Biz_dh_c2anim(); }
	virtual MStatus doIt(const MArgList&)
	{
		unsigned int ip = XenonAltAddress();

		dprintf("biz_dh_c2anim v%.4x running\n", MainVer);
		g_CamCom.Connect(ip);
		AnimSend();
		g_CamCom.Disconnect();
		dprintf("\n");

		return MS::kSuccess;
	}
};

MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, "", "");
	
	// Camera commands.
	plugin.registerCommand("biz_dh_c2export", Biz_dh_c2export::creator);
	plugin.registerCommand("biz_dh_c2anim",   Biz_dh_c2anim::creator);

	XenonInit();
	Aftk::C_Global::Create();
	C_CamCom::NetworkInit();

	return MStatus::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	Aftk::C_Global::Destroy();

	plugin.deregisterCommand("biz_dh_c2export");
	plugin.deregisterCommand("biz_dh_c2anim");

	C_CamCom::NetworkExit();
	
	return MStatus::kSuccess;
}
