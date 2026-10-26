
#ifndef CAMERA2_CAMERAINPUT_H
#define CAMERA2_CAMERAINPUT_H

namespace Cam2
{

enum E_Input
{
	E_Input_FlyMoveX = 0,
	E_Input_FlyMoveY,
	E_Input_FlyMoveZ,
	E_Input_FlyRotateX,
	E_Input_FlyRotateY,
	E_Input_FlyRoll,
	E_Input_FlyZoom,
	E_Input_Keyframe,
	E_Input_FlySlow,
	E_Input_FlyFast,
	E_Input_Count
};

// Input to send to Fly camera
class CamInput
{
public:
	S32 pad;
	F32 val[E_Input_Count];

	CamInput()
	{
		pad = 0;
		S32 i = 0;
		for (i = 0; i < E_Input_Count; i++)
			val[i] = 0;
	}
};
}

#endif
