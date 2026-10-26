
#include "stdafx.h"
#include "AnimSend.h"

#pragma warning (push,3)
#include <aftk/Aftk.h>
#pragma warning (pop)
#include <camera2/export/AftkToAnimData.h>
#include <camera2/tv/AnimData.h>
#include <camera2/tv/AnimDataSerialise.h>
#include <camera2/tv/CameraScene.h>
#include <system/serialise/C_MemoryStream.h>
#include "CamCom.h"

// Get the selected camera, and read the fcurves from it
S32 AnimSend()
{
	Aftk::C_Scene scene;
	scene.ReadExternalScene(Aftk::E_ExternalScene_Maya, "exportSelection=1");

	Aftk::C_SceneNodeIterator it(&scene);

	Sys::C_MemoryStream buffer(128);

	for ( ; it; it++)
	{
		Aftk::C_SceneNode* node = it;
		if (node->GetType() == Aftk::E_ObjType_SceneCamera)
		{
			Cam2::C_AnimData anim;
			Cam2::AftkToAnimData(&scene, node, node, &anim);

			buffer.Write("CANI", 4);
			Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(buffer);
			S32 pad = 0;
			archive << Cam2::C_CameraScene::Version << pad << anim;
			break;
		}
	}

	S32 bsize = (S32)buffer.GetBufferSize();
	if (bsize <= 0)
		return 1;

	dprintf("Sending animation...\n");

	S32 confirmsBefore = g_CamCom.Confirms();
	g_CamCom.Send(buffer.GetBufferPtr(), bsize);

	S32 start = (S32)Time::GetCurrentTime();
	for (;;)
	{
		g_CamCom.Update();
		S32 rel = g_CamCom.Confirms() - confirmsBefore;
		if (rel > 0)
		{
			dprintf("Sent ok!\n");
			break;
		}
		
		if ((S32)Time::GetCurrentTime() - start >= 3000000)
			break; // Time out
		MT::Thread::Sleep(1);
	}

	return 0;
}
