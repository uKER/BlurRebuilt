
#ifndef CAMERA2_LOCKSCREEN_H
#define CAMERA2_LOCKSCREEN_H

namespace Cam2
{

class C_LockScreenInfo
{
public:
	const void *bits;
	S32 pitch, format, width, height;
	C_LockScreenInfo()
	{
		bits = 0;
		pitch = format = width = height = 0;
	}
};

S32 LockScreen(C_LockScreenInfo& info);
void UnlockScreen();

}

#endif
