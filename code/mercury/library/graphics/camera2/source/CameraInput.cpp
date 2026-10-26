
#include "stdafx.h"
#include "camera2/CameraInput.h"

namespace Cam2
{

CamInput::CamInput()
{
	S32 i = 0;
	for (i = 0; i < E_Input_Count; i++)
		val[i] = 0;
}

} // End of namespace Cam2
