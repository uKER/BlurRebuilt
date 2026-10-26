// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdGraphics/bdGfx.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

#ifdef BD_PLATFORM_PSP

BD_REGISTER_SINGLETON(bdGfxImpl)

#include <kernel.h>
#include <displaysvc.h>
#include <ctrlsvc.h>
#include <libgu.h>
#include <libgum.h>

SCE_MODULE_INFO(bitdemon, 0, 1, 1);
SceInt32 sce_newlib_heap_kb_size = 1000;

bdGfxImpl::bdGfxImpl()
{
}

bdGfxImpl::~bdGfxImpl()
{
}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	// Initialize the graphics library
	sceGuInit();

	// Create the display buffer.
	sceGuStart(SCEGU_IMMEDIATE, m_displayList, sizeof(m_displayList));

	// This works. Why is unknown. But it does, so it's here.
	bdPlatformTiming::sleep(1);

	// Turn on the display
	sceGuDisplay(SCEGU_DISPLAY_ON);

	// Set up the draw, display and depth buffers.
	sceGuDrawBuffer(SCEGU_PF8888, SCEGU_VRAM_BP32_0, SCEGU_VRAM_WIDTH);
	sceGuDispBuffer(SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT, SCEGU_VRAM_BP32_1, SCEGU_VRAM_WIDTH);

	// Set up the depth buffer. Set the clear value for the depth buffer to 1.0 (for glClear()), enable depth testing
	// and let pixels pass the test if their depth is <= the current value in the depth buffer.
	sceGuDepthBuffer(SCEGU_VRAM_BP32_2, SCEGU_VRAM_WIDTH);
	sceGuDepthRange(65535, 0);
	sceGuDepthFunc(SCEGU_GEQUAL);
	sceGuEnable(SCEGU_DEPTH_TEST);
	sceGuClearDepth(0);

	// Set up the viewport coordinate transformations.
	sceGuOffset(SCEGU_SCR_OFFSETX, SCEGU_SCR_OFFSETY);
	sceGuViewport(2048, 2048, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT);
//	sceGuDepthRange(10000, 50000);

	// Enable the scissor test to clip drawing to the screen.
	sceGuScissor(0, 0, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT);
	sceGuEnable(SCEGU_SCISSOR_TEST);

	// Create the matrix stacks
	sceGumSetMatrixStack(m_matrixStack, projMatrixStackSize, viewMatrixStackSize,
		worldMatrixStackSize, texMatrixStackSize);
	sceGumMatrixMode(SCEGU_MATRIX_WORLD);

	// Set the OpenGL clear color
	setBackgroundColor(m_backgroundColor);

	// Set the shading model to smooth (as opposed to flat)
	sceGuShadeModel(SCEGU_SMOOTH);

	// Clear and swap the buffers now to fill in the window ASAP
	sceGuClear(SCEGU_CLEAR_COLOR|SCEGU_CLEAR_DEPTH);

	// Turn off lighting.
	sceGuDisable(SCEGU_LIGHTING);

	// Turn off back face culling because direct x and OpenGL use different (left- and right-handed) coordinate systems
	sceGuDisable(SCEGU_CULL_FACE);

	// Finish the display list
	sceGuFinish();

	// Sync to the screen refresh
	sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);
	sceDisplayWaitVblankStart();
//	sceGuSwapBuffers();

	// Set the sampling mode for the controls to digital only (ignore the analog pad)
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALONLY);

	return true;
}

bdBool bdGfxImpl::runFrame() const
{
	render();
	
	return true;
}

void bdGfxImpl::cleanup()
{
	// Shutdown graphics
	sceGuTerm();
}

void bdGfxImpl::setText(const bdNChar8 *const str)
{
	const bdInt top = 0;
	const bdInt left = 0;
	const bdUInt color = 0xffffffff;	// White text

	sceGuDebugPrint(top, left, color, str);
}

void bdGfxImpl::setBackgroundColor(const bdColor &color)
{
	bdGfxBase::setBackgroundColor(color);

	sceGuClearColor(m_backgroundColor);
}

void bdGfxImpl::render() const
{
	ScePspFVector3 eyePosition = {0, 0, 15};
	ScePspFVector3 targetPosition = {0, 0, 0};
	ScePspFVector3 upVector = {0, 1, 0};

	// Start a display list
	sceGuStart(SCEGU_IMMEDIATE, m_displayList, sizeof(m_displayList));

	// Clear the screen
	sceGuClear(SCEGU_CLEAR_ALL);

	// Set up the projection matrix
	sceGumMatrixMode(SCEGU_MATRIX_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(SCEGU_RAD(45.0f), SCEGU_SCR_ASPECT,
		1.0f, 1000.0f);

	sceGumMatrixMode(SCEGU_MATRIX_VIEW);
	sceGumLoadIdentity();
	sceGumLookAt(&eyePosition, &targetPosition, &upVector);
	
	for (bdUInt i = 0; i < m_meshes.getSize(); i++)
	{
		// Store the modelview matrix
		sceGumMatrixMode(SCEGU_MATRIX_WORLD);
		sceGumPushMatrix();

		// Draw the mesh
		m_meshes[i]->draw();

		// Restore the modelview matrix
		sceGumPopMatrix();
	}

	// Finish the drawing list
	sceGuFinish();

	// Wait for drawing to finish, the vsync to happen and then swap the buffers
	sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);

	sceDisplayWaitVblankStart();

	sceGuSwapBuffers();
}

#endif // BD_PLATFORM_PSP
