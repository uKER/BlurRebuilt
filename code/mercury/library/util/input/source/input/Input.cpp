#include "stdafx.h"

#include <input\Input.h>

namespace Input
{

Debug::Logger &g_Log = Debug::GetLogger("mercury.util.input");

void ApplyCircularDeadzone(F32& x, F32& y, const F32 deadzone)
{
	const F32 len2 = x * x + y * y;
	const F32 deadzone2 = deadzone * deadzone;

	if (len2 <= deadzone2)
	{
		x = y = 0.0f;
	}
	else
	{
		const F32 len = sqrtf(len2);
		F32 newLen = (len - deadzone) / (1.0f - deadzone);

		if (newLen > 1.0f)
			newLen = 1.0f;

		F32 ratio = newLen / len;

		x *= ratio;
		y *= ratio;
	}
}


} // namespace Input
