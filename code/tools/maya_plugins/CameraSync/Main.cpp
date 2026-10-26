
#include "stdafx.h"
#include <camera2/Camera.h>
#include "CamSync.h"
#include "Xenon.h"
#include "MayaCameraCopy.h"
#include "Follow.h"

using namespace Maths;

int MainVer = 0x0026;

void dprintf(char *format, ...)
{
	va_list val=NULL;
	va_start(val,format);

	vfprintf(stdout,format,val);
	fflush(stdout);

	va_end(val);
}

static C_MayaCameraCopy g_MayaCameraCopy;

// Get the camera position from the game:
class CameraGetPos : public MPxCommand
{
public:
	static void *creator() { return new CameraGetPos(); }
	
	virtual MStatus doIt(const MArgList&)
	{
		g_CamSync.Connect(XenonAltAddress());
		g_CamSync.WaitForPosition();
		g_CamSync.Disconnect();
		
		g_MayaCameraCopy.SetCamera( g_CamSync.GameCam() );

		clearResult();
		setResult(g_MayaCameraCopy.GetResult());

		return MS::kSuccess;
	}
};

// Send the camera position to the game:
class CameraSetPos : public MPxCommand
{
public:
	static void *creator() { return new CameraSetPos(); }
	
	virtual MStatus doIt(const MArgList& argList)
	{
		unsigned index = 0;
		MDoubleArray da = argList.asDoubleArray(index);

		Cam2::C_Camera camera;
		HMat33 mat;
		mat.AX(HVec3( da[ 0], da[ 1],-da[ 2]));
		mat.AY(HVec3( da[ 4], da[ 5],-da[ 6]));
		mat.AZ(HVec3(-da[ 8],-da[ 9], da[10]));
		camera.SetOrient(Maths::C_Quaternion(mat));
		camera.SetPosition(Maths::HVec3(da[12], da[13], -da[14]));

		F32 fov = (F32)argList.asDouble(1);
		camera.SetFov(Maths::DegToRad(fov));
		
		g_CamSync.Connect(XenonAltAddress());
		
		g_CamSync.SendPosition(camera);
		g_CamSync.WaitForConfirm();
		g_CamSync.Disconnect();

		return MS::kSuccess;
	}
};


class CameraTrackStart : public MPxCommand
{
public:
	static void *creator() { return new CameraTrackStart(); }
	
	virtual MStatus doIt(const MArgList&)
	{
		g_CameraFollow.Start();
		return MS::kSuccess;
	}
};

class CameraTrackStop : public MPxCommand
{
public:
	static void *creator() { return new CameraTrackStop(); }
	
	virtual MStatus doIt(const MArgList&)
	{
		g_CameraFollow.Stop();
		return MS::kSuccess;
	}
};

MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	
	XenonInit();
	C_CamSync::NetworkInit();

	plugin.registerCommand("biz_dh_cameraGetPos", CameraGetPos::creator);
	plugin.registerCommand("biz_dh_cameraSetPos", CameraSetPos::creator);
	plugin.registerCommand("biz_dh_cameraTrackStart", CameraTrackStart::creator);
	plugin.registerCommand("biz_dh_cameraTrackStop",  CameraTrackStop::creator);

	return MStatus::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	g_CameraFollow.Stop();
	C_CamSync::NetworkExit();

	plugin.deregisterCommand("biz_dh_cameraGetPos");
	plugin.deregisterCommand("biz_dh_cameraSetPos");
	plugin.deregisterCommand("biz_dh_cameraTrackStart");
	plugin.deregisterCommand("biz_dh_cameraTrackStop");

	return MStatus::kSuccess;
}
