
#include "stdafx.h"
#include <camera2/debug/PositionLogger.h>
#include <camera2/Camera.h>
#pragma warning (push, 3)
#include <Framework/application/CoreSystems.h>
#include <System/Serialise/I_FileSystem.h>
#include <system/serialise/TextSerialise.h>
#pragma warning (pop)

namespace Cam2
{

static Debug::Logger& g_Log = Debug::GetLogger("mercury.camera2");

C_PositionLogger::C_PositionLogger()
{
}

void C_PositionLogger::Reset()
{
	m_frames.Clear();

	LOGINFO(g_Log, "Position Logger - Frames reset");
}

void C_PositionLogger::AddFrame(const C_Camera& camera)
{
	C_PosFrame frame;
	frame.m_position = camera.Position();
	frame.m_orient = camera.Shaken();
	frame.m_fov = camera.Fov();
	
	m_frames.PushBack(frame);
}

bool C_PositionLogger::WriteAfterEffects()
{
	const char* outName = "AfterEffects.txt";
	
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	Sys::I_File* file = fs.Open(outName, Sys::E_FileAccess_Write);
	if (file == 0)
		return false;

	Sys::TextSerialiser serial(*file);
	Util::C_String text;

	serial << "Adobe After Effects 8.0 Keyframe Data\n\n";
	serial << "\tUnits Per Second	30\n";
	serial << "\tSource Width	1280\n";
	serial << "\tSource Height	720\n";
	serial << "\tSource Pixel Aspect Ratio	1\n";
	serial << "\tComp Pixel Aspect Ratio	1\n";
	serial << "\n";

	serial << "Transform\tPoint of Interest\n";
	serial << "	Frame	X pixels	Y pixels	Z pixels\n";
	S32 i = 0;
	for (i = 0; i < m_frames.Size(); i++)
	{
		const C_PosFrame& pos = m_frames[i];

		Maths::HVec3 interest = pos.m_position + Maths::HMat33(pos.m_orient).AZ() * 10.f;

		text.Printf("	%d	%f	%f	%f\n",
			i, (F32)interest.X(), (F32)interest.Y(), (F32)interest.Z());
		serial << text;
	}
	serial << "\n";

	serial << "Transform\tPosition\n";
	serial << "	Frame	X pixels	Y pixels	Z pixels\n";
	for (i = 0; i < m_frames.Size(); i++)
	{
		const C_PosFrame& pos = m_frames[i];

		text.Printf("	%d	%f	%f	%f\n",
			i, (F32)pos.m_position.X(), (F32)pos.m_position.Y(), (F32)pos.m_position.Z());
		serial << text;
	}
	serial << "\n";

	serial << "Transform\tRotation\n";
	serial << "	Frame	degrees\n";
	for (i = 0; i < m_frames.Size(); i++)
	{
		F32 roll = 0.0f;
		QuatToEuler(m_frames[i].m_orient, 0, 0, &roll);
		roll = RadToDeg(roll);
	
		text.Printf("	%d	%f\n", i, roll);
		serial << text;
	}

	serial << "\n";
	serial << "End of Keyframe Data\n";
	
	delete file;
	file = 0;

	LOGINFO(g_Log, "Position Logger - wrote %d frames to %s", m_frames.Size(), outName);
	
	return true;
}

bool C_PositionLogger::WriteMaya()
{
	const char* outName = "GameCamera.ma";
	
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	Sys::I_File* file = fs.Open(outName, Sys::E_FileAccess_Write);
	if (file == 0)
		return false;

	Sys::TextSerialiser serial(*file);

	serial << "//Maya ASCII 2008 scene\n";
	serial << "requires maya 2008;\n";
	serial << "createNode transform -n GameCamera;\n";
	serial << "setAttr .ro 2;\n";
	serial << "createNode camera -p GameCamera -n GameCameraShape;\n";

	S32 axis = 0, i = 0;
	S32 count = m_frames.Size();
	Util::C_String text;

	const char* attribute[7] =
	{
		"GameCamera_translateX", "GameCamera_translateY", "GameCamera_translateZ",
		"GameCamera_rotateX", "GameCamera_rotateY", "GameCamera_rotateZ", "GameCameraShape_focalLength"
	};

	const char* shortName[7] =
	{
		"GameCamera.tx", "GameCamera.ty", "GameCamera.tz",
		"GameCamera.rx", "GameCamera.ry", "GameCamera.rz", "GameCameraShape.fl"
	};

	for (axis = 0; axis < 7; axis++)
	{
		// Get the correct curve type:
		const char* animCurve = 0;
		if (axis < 3)
			animCurve = "animCurveTL";
		else if (axis < 6)
			animCurve = "animCurveTA";
		else
			animCurve = "animCurveTU";
	
		serial << text.Printf("createNode %s -n %s;\n", animCurve, attribute[axis]);

		serial << text.Printf("setAttr -s %d .ktv[0:%d]\n", count, count - 1);
		
		for (i = 0; i < count; i++)
		{
			F32 val = 0.f;
			if (axis == 0) val =  (F32)m_frames[i].m_position.X();
			if (axis == 1) val =  (F32)m_frames[i].m_position.Y();
			if (axis == 2) val = -(F32)m_frames[i].m_position.Z();
			if (axis == 3) QuatToEuler(m_frames[i].m_orient, &val, 0, 0);
			if (axis == 4) QuatToEuler(m_frames[i].m_orient, 0, &val, 0);
			if (axis == 5) QuatToEuler(m_frames[i].m_orient, 0, 0, &val);
			if (axis == 6) val = C_Camera::FovToFocalLength(m_frames[i].m_fov) * 1000.0f;
			
			if (axis == 4 || axis == 5) val = -val;
			if (axis >= 3 && axis <= 5)
				val = RadToDeg(val);

			serial << text.Printf("\t%4d %12.6f", i, val);
			if ((i & 3) == 3)
				serial << "\n";
		}
		serial << ";\n";
	}

	for (axis = 0; axis < 7; axis++)
		serial << text.Printf("connectAttr %s.o %s;\n", attribute[axis], shortName[axis]);

	delete file;
	file = 0;

	LOGINFO(g_Log, "Position Logger - wrote %d frames to %s", count, outName);

	return true;
}

} // End of namespace Cam2
