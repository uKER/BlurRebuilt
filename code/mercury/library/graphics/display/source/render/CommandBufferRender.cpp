#include "stdafx.h"
#include "Display/Render/CommandBufferRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Render/FontRender.h"
#include "Display/Device/Device.h"
#include <Display2/Debug/Debug.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

Debug::Int	g_debugDisplayCommandBufferRenderSize("Graphics.Display.Render.CommandBufferRender.Size", 14, Debug::Range(0, 20));
Debug::Int	g_debugDisplayCommandBufferRenderTransparency("Graphics.Display.Render.CommandBufferRender.Transparency", 200, Debug::Range(0, 255));

namespace Display
{

C_CommandBufferRender::C_CommandBufferRender()
{
	assert(g_pCommandBufferRender == NULL);
	g_pCommandBufferRender = this;
}

C_CommandBufferRender::~C_CommandBufferRender()
{
	assert(g_pCommandBufferRender == this);
	g_pCommandBufferRender = NULL; 
}

void C_CommandBufferRender::Draw()
{
#if TRACK_DISPLAY2_COMMAND_BUFFERS

	BasicRender()->DrawColoured2DRect(20.f, 20.f, 984.f, 728.f, Maths::RGBA(0, 0, 0, g_debugDisplayCommandBufferRenderTransparency));

	F32 x = 50.0f;
	F32 y = 50.0f;
	F32 s = (F32)g_debugDisplayCommandBufferRenderSize;
	char str[1024];

	static U32 prevInstance = 0;
	U32 currInstance = Display2::g_CommandBufferTracker.m_CurInstance;

	for(U32 i=0; i<Display2::g_CommandBufferTracker.m_MaxInstances; i++)
	{
		Display2::C_CommandBufferInstance& instance = Display2::g_CommandBufferTracker.m_Instances[i];

		Maths::RGBA colour;

		if(currInstance >= prevInstance ? (i >= prevInstance && i < currInstance) : (i >= prevInstance || i < currInstance))
			colour = Maths::RGBA(255, 255, 255, 255);
		else
			colour = Maths::RGBA(128, 128, 128, 255);

#if 1
		U32 headOffset, tailOffset;
		cellGcmAddressToOffset(instance.m_pHeadAddress, &headOffset);
		cellGcmAddressToOffset(instance.m_pTailAddress, &tailOffset);
		sprintf_s(str, sizeof(str), "%2i : %.8x %.8x %s %s", i,
			headOffset,
			tailOffset,
			i == currInstance ? "c" : i == prevInstance ? "p" : " ",
			instance.m_Name);
#else
		sprintf_s(str, sizeof(str), "%2i : 0x%.8x 0x%.8x %s %s", i,
			instance.m_pHeadAddress,
			instance.m_pTailAddress,
			i == currInstance ? "c" : i == prevInstance ? "p" : " ",
			instance.m_Name);
#endif

		FontRender()->Draw(str, x, y, s, s, colour, E_FontRenderAlign_Lt, true);

		y += s*1.1f;
	}

	prevInstance = currInstance;

#endif
}

C_CommandBufferRender* g_pCommandBufferRender = NULL;

void CommandBufferRender(C_CommandBufferRender* pCommandBufferRender)
{
	g_pCommandBufferRender = pCommandBufferRender;
}

}; // namespace Display
