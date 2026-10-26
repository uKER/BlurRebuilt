// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdGraphics/bdGfx.h>

#if defined BD_PLATFORM_WIN32 && defined BD_WIN32_GL

#include <GL/gl.h>
#include <GL/glu.h>

#pragma comment (lib, "opengl32")
#pragma comment (lib, "glu32")

BD_REGISTER_SINGLETON(bdGfxImpl)

bdGfxImpl::bdGfxImpl()
{
}

bdGfxImpl::~bdGfxImpl()
{
}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	if (!initWin32())
	{
		return false;
	}

	if (!initGL())
	{
		return false;
	}

	ShowWindow(m_mainWindowHandle, SW_NORMAL);
	SetForegroundWindow(m_mainWindowHandle);
	SetFocus(m_mainWindowHandle);
	UpdateWindow(m_mainWindowHandle);

	return true;
}

bdBool bdGfxImpl::runFrame() const
{
	MSG msg;

	// Check the message queue for messages
	while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE) != 0)
	{
		// Dispatch the message to the window proc
		DispatchMessage(&msg);

		// If it was a quit message, the program should exit
		if (msg.message == WM_QUIT)
		{
			return false;
		}
	}

	// Once the message queue is empty, draw the frame
	render();

	return true;
}

void bdGfxImpl::cleanup()
{
}

void bdGfxImpl::setText(const bdNChar8 *const)
{
	/// \todo: Implement this function
}

void bdGfxImpl::setBackgroundColor(const bdColor &color)
{
	bdFloat32 col[4];

	// Store the background color
	bdGfxBase::setBackgroundColor(color);

	// Convert the color to a floating point representation of the components
	color.asFloat(col);

	// Set the OpenGL clear color
	glClearColor(col[0], col[1], col[2], col[3]);
}

HWND bdGfxImpl::getWindowHandle() const
{
	return m_mainWindowHandle;
}

bdBool bdGfxImpl::initWin32()
{
	PIXELFORMATDESCRIPTOR pfd;	// Describes our parameters for a pixel format
	bdUInt pixelFormat;			// Unique number identifying the selected pixel format
	WNDCLASSEX wc;				// Structure describing our window class

	// Clear the window class structure and then fill it as required
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);

	// Give each window its own DC and redraw on movement/resize
	wc.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;

	// Set the Window Procedure for this window class
	wc.lpfnWndProc = MessageProc;

	// Handle to the current instance
	wc.hInstance = GetModuleHandle(NULL);

	// Use the standard cursor as the window's cursor
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	// Give the window class a name
	wc.lpszClassName = BD_MAIN_WINDOW_CLASS_NAME;

	// Register the window class with Windows
	if (RegisterClassEx(&wc) == 0)
	{
		MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
		return false;
	}

	// Create a window to draw to
	m_mainWindowHandle = CreateWindow(	BD_MAIN_WINDOW_CLASS_NAME,	// Use the class we just created
										"DemonWare",			// Window caption
										WS_OVERLAPPEDWINDOW,	// Standard window style
										100, 100, 640, 480,		// Make window 640x480 at (100, 100)
										GetDesktopWindow(),		// Parent window is desktop
										NULL,					// No menu
										wc.hInstance,			// Instance handle
										NULL );					// No extra parameter

	// Test for failure to create the main window
	if (!m_mainWindowHandle)
	{
		MessageBox(NULL, "Failed to Create Window", "Error", MB_OK);
		return false;
	}

	// Store the pointer to this object with the window data, so we can retrieve it in the Window procedure
	SetWindowLongPtr(m_mainWindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	// Set up a PIXELFORMAT with the kind of pixel format that we want
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;					// Version 1 of the PIXELFORMATDESCRIPTOR struct
    pfd.dwFlags =	PFD_DRAW_TO_WINDOW|	// Drawing is to a Window (not bitmap)
					PFD_SUPPORT_OPENGL|	// Need OpenGL support
					PFD_DOUBLEBUFFER;	// Need double-buffering
    pfd.iPixelType = PFD_TYPE_RGBA;		// Pixels are RGBA color components
    pfd.cColorBits = 32;				// Using 32-bit color
    pfd.cDepthBits = 16;				// 16-bit depth buffer

	// Get a device context for the main window
	m_deviceContext = GetDC(m_mainWindowHandle);
	if (!m_deviceContext)
	{
		MessageBox(NULL, "Could not obtain a device context for the main window", "Error", MB_OK);
		return false;
	}

	// Try to find a matching (or close) pixel format
	pixelFormat = ChoosePixelFormat(m_deviceContext, &pfd);
	if (!pixelFormat)
	{
		MessageBox(NULL, "Could not obtain a suitable pixel format", "Error", MB_OK);
		return false;
	}

	// Set the pixel format of the device context to the once we have just selected
	if (!SetPixelFormat(m_deviceContext, pixelFormat, &pfd))
	{
		MessageBox(NULL, "Could not set the pixel format", "Error", MB_OK);
		return false;
	}

	// Now create an OpenGL rendering context for this device context
	m_renderContext = wglCreateContext(m_deviceContext);
	if (!m_renderContext)
	{
		MessageBox(NULL, "Could not create an OpenGL rendering context", "Error", MB_OK);
		return false;
	}

	// Set the new render context to be this thread's rendering context
	if (!wglMakeCurrent(m_deviceContext, m_renderContext))
	{
		MessageBox(NULL, "Could not activate the OpenGL rendering context", "Error", MB_OK);
		return false;
	}

	// Display the newly created window
	ShowWindow(m_mainWindowHandle, SW_NORMAL);

	// Success
	return true;
}

bdBool bdGfxImpl::initGL()
{
	bdFloat32 color[4];

	// Convert the background color to a floating-point representation
	m_backgroundColor.asFloat(color);

	// Set the OpenGL clear color
	glClearColor(color[0], color[1], color[2], color[3]);

	// Set the shading model to smooth (as opposed to flat)
	glShadeModel(GL_SMOOTH);

	// Set up the depth buffer. Set the clear value for the depth buffer to 1.0 (for glClear()), enable depth testing
	// and let pixels pass the test if their depth is <= the current value in the depth buffer.
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Clear and swap the buffers now to fill in the window ASAP
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SwapBuffers(m_deviceContext);

	// Turn off lighting.
	glDisable(GL_LIGHTING);

	// Turn off back face culling because direct x and OpenGL use different (left- and right-handed) coordinate systems
	glDisable(GL_CULL_FACE);

	// Use the nicest looking perspective correction
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set up the viewport
	glViewport(0, 0, 640, 480);

	// Enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);

	return true;
}

void bdGfxImpl::render() const
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, 640.0f / 480.0f, 1, 1000);

	// Set up the viewpoint using the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	0, 0, 15,	// The position of the camera
				0, 0, 0,	// The target point of the camera
				0, 1, 0);	// The up vector

	for (bdUInt i = 0; i < m_meshes.getSize(); i++)
	{
		// Store the modelview matrix
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		// Draw the mesh
		m_meshes[i]->draw();

		// Restore the modelview matrix
		glPopMatrix();
	}

	glFlush();

	SwapBuffers(m_deviceContext);
}

LRESULT WINAPI bdGfxImpl::MessageProc(HWND hWnd,
									  UINT msg,
									  WPARAM wParam,
									  LPARAM lParam)
{
	// Retrieve the pointer to the graphics object from the window
	bdGfxImpl *gfx = reinterpret_cast<bdGfxImpl*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (msg)
	{
	case WM_DESTROY:
		// Deselect the rendering context from the device context
		wglMakeCurrent(gfx->m_deviceContext, NULL);

		// Delete the rendering context
		wglDeleteContext(gfx->m_renderContext);

		// Release the device context
		ReleaseDC(gfx->m_mainWindowHandle, gfx->m_deviceContext);

		gfx->m_mainWindowHandle = NULL;
		gfx->m_deviceContext = NULL;
		gfx->m_renderContext = NULL;

		// Handle the destroy message by telling the application to quit
		PostQuitMessage(0);
		return 0;
	default:
		// Use default behavior for all other messages
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

#endif // BD_PLATFORM_WIN32 && BD_WIN32_GL
