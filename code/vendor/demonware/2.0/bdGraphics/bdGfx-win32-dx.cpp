// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdGraphics/bdGfx.h>
#include <bdGraphics/bdMesh.h>

#if defined BD_PLATFORM_WIN32 && defined BD_WIN32_DIRECTX

#include <bdGraphics/bdD3D.h>

#include <d3d9.h>
#include <d3dx9.h>

// Make the linker reference the Direct3D and Direct3DX libraries.
#pragma comment (lib, "d3d9")
#pragma comment (lib, "d3dx9")

BD_REGISTER_SINGLETON(bdD3DDevWrapper)
BD_REGISTER_SINGLETON(bdGfxImpl)

bdGfxImpl::bdGfxImpl()
{
	m_mainWindowHandle = BD_NULL;
	m_initialized = false;
	m_font = BD_NULL;
	m_textHeight = 20;
	m_textWidth = 10;
	m_textWeight = 5;
	m_textRight = 635;
	m_textBottom = 475;
}

bdGfxImpl::~bdGfxImpl()
{
	DestroyWindow(m_mainWindowHandle);
	UnregisterClass(BD_MAIN_WINDOW_CLASS_NAME, GetModuleHandle(NULL));
}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	// Initialize win32, create the window to draw to, etc.
	if (!initWindows())
	{
		return false;
	}

	// Initialize Direct3D, create the render context, etc.
	if (!initDirect3D())
	{
		return false;
	}

	// Create the font to use for drawing text
	D3DXCreateFont(	bdD3D::getInstance()->getDevice(),	// Pointer to the D3D device
					m_textHeight, m_textWidth,			// Desired dimensions of the font
					m_textWeight,						// Desired weight of font
					1,									// Use 1 Mip-level, to map it to the screen space
					false,								// Do not use italics
					DEFAULT_CHARSET,					// Use default character set
					OUT_DEFAULT_PRECIS,					// Use default precision
					DEFAULT_QUALITY,					// Use default quality
					FIXED_PITCH|FF_MODERN,				// Fixed-pitch modern font
					NULL,								// Choose any matching font (not a specific font)
					&m_font);

	// Success
	m_initialized = true;
	return true;
}

bdBool bdGfxImpl::runFrame() const
{
	MSG msg;

	// Check the message queue for messages
	while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE) != 0)
	{

		// Dispatch the message to the window procedure
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
	if (m_font)
	{
		m_font->Release();
	}
}

void bdGfxImpl::setText(const bdNChar8 *const str)
{
	bdGfxBase::setText(str);

	// Load the text into the font object, to help performance
	m_font->PreloadText(m_text, static_cast<INT>(bdStrlen(m_text)));
}

DWORD BD_CALL bdGfxImpl::ConvertColor(bdColor color)
{
	return D3DCOLOR_ARGB(color.getAlpha(), color.getRed(), color.getGreen(), color.getBlue());
}

bdColor BD_CALL bdGfxImpl::ConvertColor(DWORD color)
{
	return bdColor(GetAlpha(color), GetRed(color), GetGreen(color), GetBlue(color));
}

bdUByte8 BD_CALL bdGfxImpl::GetAlpha(DWORD color)
{
	return static_cast<bdUByte8>((color >> 24) & 0xff);
}

bdUByte8 BD_CALL bdGfxImpl::GetRed(DWORD color)
{
	return static_cast<bdUByte8>((color >> 16) & 0xff);
}

bdUByte8 BD_CALL bdGfxImpl::GetGreen(DWORD color)
{
	return static_cast<bdUByte8>((color >> 8) & 0xff);
}

bdUByte8 BD_CALL bdGfxImpl::GetBlue(DWORD color)
{
	return static_cast<bdUByte8>(color & 0xff);
}

HWND bdGfxImpl::getWindowHandle() const
{
	return m_mainWindowHandle;
}

bdBool bdGfxImpl::initWindows()
{
	WNDCLASSEX wc;

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
	m_mainWindowHandle = CreateWindow(BD_MAIN_WINDOW_CLASS_NAME,
										"DemonWare", 
										WS_OVERLAPPEDWINDOW,
										100, 100, 640, 480,
										GetDesktopWindow(),
										NULL,
										wc.hInstance,
										NULL );

	// Test for failure to create the main window
	if (!m_mainWindowHandle)
	{
		MessageBox(NULL, "Failed to Create Window", "Error", MB_OK);
		return false;
	}

	// Display the newly created window
	ShowWindow(m_mainWindowHandle, SW_NORMAL);

	// Success
	return true;
}

bdBool bdGfxImpl::initDirect3D()
{
	// Pointer to Direct3D object. Used to create the Direct3D Device
	LPDIRECT3D9 d3d;

	// Pointer to Direct3D device. Used for almost all Direct3D calls, including drawing
	LPDIRECT3DDEVICE9 device;

	// Presentation parameters. Sets up the buffers for the D3D device
	D3DPRESENT_PARAMETERS d3dpp;

	// Vertex Declaration. Specifies the format for the vertex buffer
	LPDIRECT3DVERTEXDECLARATION9 vertexDecl;

	// Vertex Elements. Defines the vertex Declaration
	static D3DVERTEXELEMENT9 vertexElements[] =
	{
		// The first part of the vertex is 3 floats to denote the position
		{0,	0,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0},

		// The second part is 4 unsigned bytes to denote the color of the vertex
		{1,	0,	D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0},

		// End of vertex declaration
		D3DDECL_END()
	};

	// Get a pointer the direct3D interface
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)
	{
		MessageBox(NULL, "Could not Access Direct3D", "Error", MB_OK);
		return false;
	}

	// Fill in the present parameters for the Direct3d device
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.hDeviceWindow = m_mainWindowHandle;
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.PresentationInterval = D3DPRESENT_DONOTWAIT;

	// Create the direct 3d device
	if (FAILED(d3d->CreateDevice(	D3DADAPTER_DEFAULT,
									D3DDEVTYPE_HAL,
									m_mainWindowHandle,
									D3DCREATE_HARDWARE_VERTEXPROCESSING,
									&d3dpp,
									&device)))
	{
		MessageBox(NULL, "Could not create the Direct 3D device", "Error", MB_OK);
		d3d->Release();
		d3d = NULL;

		return false;
	}

	// Store the pointer to the Direct3D Device in the global singleton class for it
	bdD3D::getInstance()->setDevice(device);

	// Create and set the vertex declaration
	device->SetVertexShader(NULL);
	device->CreateVertexDeclaration(vertexElements, &vertexDecl);
	device->SetVertexDeclaration(vertexDecl);

	// Turn off lighting
	device->SetRenderState(D3DRS_LIGHTING, FALSE);

	// Turn off back face culling because direct x and OpenGL use different (left- and right-handed) coordinate systems
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

	// Release the Direct3D object and the vertex declaration
	d3d->Release();
	vertexDecl->Release();

	// Clear the target buffer immediately, to give the window background an initial fill
	device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0, 0);
	device->Present(NULL, NULL, NULL, NULL);

	return true;
}

void bdGfxImpl::render() const
{
	LPDIRECT3DDEVICE9 device;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;
	D3DXVECTOR3 position(0, 0, -15);
	D3DXVECTOR3 at(0, 0, 0);
	D3DXVECTOR3 up(0, 1, 0);

	// Only render the scene if the graphics object is fully initialized
	if (m_initialized)
	{
		// Get the pointer to the Direct3D device
		device = bdD3D::getInstance()->getDevice();

		// Clear the buffers to their initial values for the beginning of the frame
		device->Clear(	0L,									// Clear entire screen
						NULL,								// Clear entire screen
						D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,	// Clear the target and depth buffers
						ConvertColor(m_backgroundColor),	// Use background color for screen
						1.0f,								// Clear z-buffer to 1
						0L);								// Unused: No stencil buffer

		// Create the view and projection matrices, using some default values
		D3DXMatrixLookAtLH(&matView, &position, &at, &up);
		D3DXMatrixPerspectiveFovLH(&matProj, BD_PI / 4.0f, 640.0f / 480.0f, 1, 1000);

		// Set up the view and projection matrices on the device
		device->SetTransform(D3DTS_VIEW, &matView);
		device->SetTransform(D3DTS_PROJECTION, &matProj);
		
		// Start drawing the scene
		device->BeginScene();

		for (bdUInt i = 0; i < m_meshes.getSize(); i++)
		{
			m_meshes[i]->draw();
		}

		if (m_text[0] != '\0')
		{
			// Create the rectangle for the text to fit into
			RECT textRect = 
			{
				static_cast<LONG>(m_textPositionX),
				static_cast<LONG>(m_textPositionY),
				static_cast<LONG>(m_textRight),
				static_cast<LONG>(m_textBottom)
			};

			// Draw the text to the screen
			m_font->DrawTextA(	NULL,
								m_text,
								static_cast<INT>(bdStrlen(m_text)),
								&textRect,
								DT_TOP|DT_LEFT|DT_WORDBREAK|DT_EXPANDTABS,
								m_textColor);
		}

		// Finish drawing the scene
		device->EndScene();

		// Rotate the back buffer chain to present the scene to the screen
		device->Present(NULL, NULL, NULL, NULL);
	}
}

LRESULT WINAPI bdGfxImpl::MessageProc(HWND hWnd,
									  UINT msg,
									  WPARAM wParam,
									  LPARAM lParam)
{
	LRESULT res;

	switch (msg)
	{
	case WM_PAINT:
		// Validate the client rectangle and draw the scene.
		ValidateRect(hWnd, NULL);
		bdGfx::getInstance()->render();

		// Return zero to say we processed this message
		res = 0;
		break;
	case WM_DESTROY:
		// Handle the destroy message by telling the application to quit
		UnregisterClass(BD_MAIN_WINDOW_CLASS_NAME, GetModuleHandle(NULL));
		PostQuitMessage(0);
		res = 0;
		break;
	default:
		// Use default behavior for all other messages
		res = DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return res;
}

#endif // BD_PLATFORM_WIN32 && BD_WIN32_DIRECTX
