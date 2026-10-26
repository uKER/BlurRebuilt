
#include "stdafx.h"
#include <camera2/Camera.h>
#include "CamSync.h"
#include "Follow.h"
#include "Xenon.h"

#include <maya/MDagPath.h>
#include <maya/MEulerRotation.h>
#include <maya/MFnCamera.h>
#include <maya/MFnTransform.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MVector.h>

C_CameraFollow g_CameraFollow;

C_CameraFollow::C_CameraFollow()
{
	m_quitEvent = m_doneEvent = 0;
}

void C_CameraFollow::Start()
{
	// Make sure thread is stopped:
	Stop();
	
	// Create events to signal to quit the thread, and that the thread is done:
	m_quitEvent = CreateEvent(0, 0, 0, 0);
	m_doneEvent = CreateEvent(0, 0, 0, 0);

	// Create the thread:
	DWORD tid = 0;
	CreateThread(0, 0, StaticThreadCode, this, 0, &tid);
}

void C_CameraFollow::Stop()
{
	// Signal the thread to quit:
	if (m_quitEvent)
		SetEvent(m_quitEvent);

	if (m_doneEvent)
	{
		// Wait for the thread to exit:
		WaitForSingleObject(m_doneEvent, 1000);
		CloseHandle(m_doneEvent);
	}

	// Tidy up:
	if (m_quitEvent)
		CloseHandle(m_quitEvent);
	
	m_doneEvent = m_quitEvent = 0;
}

// Convert a Quaternion to Euler angles:
static void QuatToEuler(F32 *pitch, F32 *yaw, F32 *roll, Maths::C_Quaternion quat)
{
	Maths::HMat43 mat(quat);
	*pitch=(F32)asin (mat[2][1]);
	*yaw  =(F32)atan2(mat[2][0], mat[2][2]);
	*roll =(F32)atan2(mat[0][1], mat[1][1]);
}

DWORD WINAPI C_CameraFollow::StaticThreadCode(LPVOID self)
{
	((C_CameraFollow *)self)->ThreadCode();
	return 0;
}

void C_CameraFollow::ThreadCode()
{
	g_CamSync.Connect(XenonAltAddress());
	for (;;)
	{
		// Check for Quit event, and quit the thread if so:
		if (WaitForSingleObject(m_quitEvent, 16) == WAIT_OBJECT_0)
			break;
		
		g_CamSync.Update();
		UpdateMayaCamera();
	}
	g_CamSync.Disconnect();

	SetEvent(m_doneEvent);
}

void C_CameraFollow::UpdateMayaCamera()
{
	const Cam2::C_Camera& camera = g_CamSync.GameCam();
	Maths::HVec3 pos = camera.Position();

	MSelectionList selection;
	MGlobal::getSelectionListByName("GameCam", selection);
	MItSelectionList iter(selection);
	MObject object;
	iter.getDependNode(object);
	MFnTransform transform = MFnTransform(object);

	MVector vec((F32)pos.X(), (F32)pos.Y(), -(F32)pos.Z());
	transform.setTranslation(vec, MSpace::kTransform);

	F32 pitch = 0.f, yaw = 0.f, roll = 0.f;
	QuatToEuler(&pitch, &yaw, &roll, camera.Shaken());
	transform.setRotationOrder(MTransformationMatrix::kZXY, false);
	transform.setRotation(MEulerRotation(pitch, -yaw, roll, MEulerRotation::kZXY));

	MDagPath cameraPath;
	iter.getDagPath(cameraPath);
	cameraPath.extendToShape();

	object = cameraPath.node();
	MFnCamera fncam(object);
	fncam.setVerticalFieldOfView(camera.Fov());
	fncam.setAspectRatio(16.f / 9.f);
	fncam.setCenterOfInterest(10.f);
}
