// [Blur2PC] T272: use the tested InputFusion implementation as an explicit
// game dependency, without replacing Windows' XInput DLL by filename.
#pragma once
#include <Windows.h>
#include <Xinput.h>

namespace Input
{
DWORD ReadPadState(DWORD userIndex, XINPUT_STATE* state);
DWORD WritePadRumble(DWORD userIndex, XINPUT_VIBRATION* vibration);
}
