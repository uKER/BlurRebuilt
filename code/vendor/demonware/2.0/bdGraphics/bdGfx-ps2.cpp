// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>

#ifdef BD_PLATFORM_PS2

#include <bdGraphics/bdGfx.h>
BD_REGISTER_SINGLETON(bdGfxImpl)

#include <libvu0.h>
#include <libdev.h>

inline void bdPerspectiveMatrix(ShellMatrix *mat,
								bdFloat32 fovy,
								bdFloat32 aspect,
								bdFloat32 near,
								bdFloat32 far)
{
	bdFloat32 yScale = 1.0f / tanf(fovy / 2);
	bdFloat32 xScale = yScale/ aspect;
	asm __volatile__
	(
		"
		sq		$0, 0x00(%0)
		sq		$0, 0x10(%0)
		sq		$0, 0x20(%0)
		sq		$0, 0x30(%0)
		"
		:
		:
		"r" (mat->m)
	);

    mat->m[0][0] = xScale;
	mat->m[1][1] = yScale;
	mat->m[2][2] = far / (far - near);
	mat->m[2][3] = 1.0f;
	mat->m[3][2] = -near * mat->m[2][2];
}

bdGfxImpl::bdGfxImpl()
:	m_screenWidth(640),
	m_screenHeight(448)
{
	m_pConsolePacket = reinterpret_cast<u_char*>(UncachePointer(
		bdMemory::alignedAllocate(0x10000, 128)));
}

bdGfxImpl::~bdGfxImpl()
{
	bdMemory::alignedDeallocate(CachePointer(m_pConsolePacket));
}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	shellDmaInit();

	shellDispInit(	SHELLDISP_INTERLACE_FIELD_AA,	// Interlace mode: Field Interlace with Anti-Aliasing
					SHELLDISP_NTSC,					// Use PAL TV mode (Use SHELLDISP_NTSC for NTSC)
					m_screenWidth, m_screenHeight, SCE_GS_PSMCT32,	// Display buffer dimensions/pixel format
					m_screenWidth, m_screenHeight, SCE_GS_PSMCT32,	// Draw buffer dimensions/pixel format
					SCE_GS_PSMZ24);

	shellDebugInit(500*8192, 500*8192+4096);
	shellDebugFontInit(&m_font,
						(m_screenWidth - 20),
						(m_screenHeight - 20),
						2048 - (m_screenWidth / 2) + 10,
						2048 - (m_screenHeight / 2) + 10,
						65530);

	shellDebugFontSetColour(&m_font, SCE_GS_SET_RGBAQ(0xff, 0xff, 0xff, 0xff, 0));
	shellDebugFontSetSize(&m_font, 1.5f, 1.5f);
	shellDebugConsoleInit(&m_console, &m_font);
	shellDebugConsoleClear(&m_console);

	shellDebugConsolePrintf(&m_console, "Demonware");
    shellPacketInit();

	setBackgroundColor(m_backgroundColor);


	// Allocate 3 megs of memory for meshes to use to create their GS Packets
	m_packetMem = reinterpret_cast<u_long*>(UncachePointer(
		bdMemory::alignedAllocate(3 * 1024 * 1024, 128)));

	return true;
}

bdBool bdGfxImpl::runFrame() const
{
	render();

	return true;
}

void bdGfxImpl::cleanup()
{
	bdMemory::alignedDeallocate(CachePointer(m_packetMem));
}

void bdGfxImpl::setBackgroundColor(const bdColor &color)
{
	bdGfxBase::setBackgroundColor(color);

	// Set the background clear color for the PS2 Shell
	shellDispSetClearColour(color.getRed(),
							color.getGreen(),
							color.getBlue(),
							color.getAlpha());
}

void bdGfxImpl::render() const
{
	ShellVector eyePosition = {{0.0f, 0.0f, -15.0f, 1.0f}};
	ShellVector targetPosition = {{0.0f, 0.0f, 0.0f, 1.0f}};
	ShellVector upVector = {{0.0f, 1.0f, 0.0f, 0.0f}};
	ShellMatrix viewportToPrimitiveSpace =
	{{
		{350.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, -270.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 100.0f, 0.0f},
		{2048.0f, 2048.0f, 0.0f, 1.0f}
	}};

	ShellVector lineOfSight;
	ShellMatrix cameraMatrix;
	ShellMatrix projectionMatrix;
	ShellMatrix viewProjMatrix;
	ShellMatrix worldToScreenMatrix;

	// Obtain the lineOfSight vector, i.e. the vector from the eyePosition to the target position
	sceVu0SubVector(lineOfSight.v, targetPosition.v, eyePosition.v);
	sceVu0Normalize(lineOfSight.v, lineOfSight.v);

	// Create the camera matrix
	sceVu0CameraMatrix(cameraMatrix.m, eyePosition.v, lineOfSight.v, upVector.v);

	// Create the projection matrix
	bdPerspectiveMatrix(&projectionMatrix, BD_PI / 4.0f, 640.0f / 480.0f, 1, 1000);

	// Load the projection and camera matrices into VU0
	sceVu0MulMatrix(viewProjMatrix.m, projectionMatrix.m, cameraMatrix.m);
	sceVu0MulMatrix(worldToScreenMatrix.m, viewportToPrimitiveSpace.m, viewProjMatrix.m);

	if (shellDmaSync(SHELLDMA_CHANNEL_VIF1, 1000000) == SHELL_FAIL)
	{
		sceGsResetPath();
	}

	// Sync to the vertical refresh
	shellDispSyncV();
	
	// Swap the buffers
	shellDispSwapDblBuffer();
	
	for (bdUInt i = 0; i < m_meshes.getSize(); i++)
	{
		// Re-load the viewProjectionMatrix into Matrix1 in VU0
		shellVU0MathMatrixLoad1(&worldToScreenMatrix);

		// Draw the matrix
		m_meshes[i]->draw();
	}

	// Create the packet for the text
	shellPacketStart(m_pConsolePacket, 1);
	shellPacketHandleVifUnpack(1,0);
	shellPacketAddDmaTag(SHELLDMA_TAG_ID_CNT, BD_NULL, 0, 0, 0);
	shellPacketAddVifCode(SCE_VIF1_SET_DIRECT(0,0));
	shellPacketAddGifTag(1, 0, 0, SHELLPACKET_GIFTAG_TYPE_PACKED, 1, 0xe);

	// Clear the console and add the text to it
	shellDebugConsoleClear(&m_console);
	shellDebugConsolePrintf(&m_console, m_text);

	// Select the font and draw the console
	shellDebugSelectFontTexture(1);
	shellDebugConsoleDraw(&m_console);

	// End the packet and add to the list to be sent to VIF1 by shellDispSwapDblBuffer()
	shellPacketEndGifTag();
	shellPacketEndVifCode();
	shellPacketEndDmaTag();

	ShellDmaTag *endTag;
	endTag = reinterpret_cast<ShellDmaTag*>(shellPacketAddDmaTag(SHELLDMA_TAG_ID_END, 0, 0, 0, 0));
	shellPacketEndDmaTag();

	FlushCache(0);

	shellDmaAppendVif1List(reinterpret_cast<ShellDmaTag*>(m_pConsolePacket), endTag);

}

#endif // BD_PLATFORM_PS2
