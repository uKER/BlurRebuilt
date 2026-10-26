// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdGraphics/bdGfx.h>
#include <bdGraphics/shaders-xenon.h>
#include <bdGraphics/bdMesh.h>
#include <bdGraphics/bdD3D.h>

#ifdef BD_PLATFORM_XENON

BD_REGISTER_SINGLETON(bdD3DDevWrapper)
BD_REGISTER_SINGLETON(bdGfxImpl)

bdGfxImpl::bdGfxImpl() :
m_deviceWidth(0),
m_deviceHeight(0)
{
}

bdGfxImpl::~bdGfxImpl()
{
}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	// Initialize Direct3D
	if (!initDirect3D())
	{
		return false;
	}

	// Initialize the text object
	if (!m_font.init(BD_ARIAL_FONT_FILE))
	{
		return false;
	}

	return true;
}

bdBool bdGfxImpl::runFrame() const
{
	render();

	return true;
}

void bdGfxImpl::cleanup()
{
}

D3DCOLOR bdGfxImpl::ConvertColor(bdColor color)
{
	return D3DCOLOR_ARGB(color.getAlpha(), color.getRed(), color.getGreen(), color.getBlue());
}

bdColor bdGfxImpl::ConvertColor(D3DCOLOR color)
{
	return bdColor(GetAlpha(color), GetRed(color), GetGreen(color), GetBlue(color));
}

bdUByte8 bdGfxImpl::GetAlpha(D3DCOLOR color)
{
	return static_cast<bdUByte8>((color >> 24) & 0xff);
}

bdUByte8 bdGfxImpl::GetRed(D3DCOLOR color)
{
	return static_cast<bdUByte8>((color >> 16) & 0xff);
}

bdUByte8 bdGfxImpl::GetGreen(D3DCOLOR color)
{
	return static_cast<bdUByte8>((color >> 8) & 0xff);
}

bdUByte8 bdGfxImpl::GetBlue(D3DCOLOR color)
{
	return static_cast<bdUByte8>(color & 0xff);
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
	// The first part of the vertex is 3 floats to denote the position
	// The second part is 4 unsigned bytes to denote the color of the vertex
	static D3DVERTEXELEMENT9 vertexElements[] =
	{
		{0,	0,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0},
		{1,	0,	D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0},
		D3DDECL_END()
	};

	// Pointer to a D3DX Buffer, used to hold the compiled shader code
	LPD3DXBUFFER shaderCode = NULL;

	// Used to hold any error messages from shader compilation
	LPD3DXBUFFER errorMsg = NULL;

	// Pointer to a vertex shader. Used to create and set the application's vertex shader
	LPDIRECT3DVERTEXSHADER9 vertexShader;

	// Pointer to a pixel shader. Used to create and set the application's pixel shader
	LPDIRECT3DPIXELSHADER9 pixelShader;

	// Structure containing information about the XBOX 360's video settings
	XVIDEO_MODE videoMode;

	// Get a pointer to the Direct3D interface
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)
	{
		OutputDebugString("Could not obtain Direct3D9 object");
		return false;
	}

	// Get the XBox360's current video mode
	XGetVideoMode(&videoMode);

	// store window dimensions for use in render()
	// 720p is maximum, even if the xbox 360 is set to 1080p, otherwise we'll run out of vid memory
	m_deviceHeight = BD_MIN(videoMode.dwDisplayHeight, 720);
	m_deviceWidth = BD_MIN(videoMode.dwDisplayWidth, 1280);

	// Fill in the present parameters for the Direct3D device
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferWidth        = m_deviceWidth;
	d3dpp.BackBufferHeight       = m_deviceHeight;
	d3dpp.BackBufferFormat       = D3DFMT_A8R8G8B8;
	d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality     = 0;
	d3dpp.BackBufferCount        = 1;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Create the D3D device
	if (FAILED(d3d->CreateDevice(	D3DADAPTER_DEFAULT,
									D3DDEVTYPE_HAL,
									NULL,
									D3DCREATE_HARDWARE_VERTEXPROCESSING,
									&d3dpp,
									&device)))
	{
		OutputDebugString("Could not create Direct3D Device");
		d3d->Release();
		d3d = NULL;
		return false;
	}

	bdD3D::getInstance()->setDevice(device);

	// Compile the vertex shader for the application
	if (FAILED(D3DXCompileShader(	vertexShaderProgram,
									strlen(vertexShaderProgram),
									NULL,
									NULL,
									"main",
									"vs_2_0",
									0,
									&shaderCode,
									&errorMsg,
									NULL)))
	{
		OutputDebugString(errorMsg ? (CHAR*)errorMsg->GetBufferPointer() : "");
		d3d->Release();
		d3d = NULL;
		errorMsg->Release();
		errorMsg = NULL;
		return false;
	}

	// Create the vertex shader for the application
	device->CreateVertexShader((DWORD*)shaderCode->GetBufferPointer(), &vertexShader);
	device->SetVertexShader(vertexShader);
//	vertexShader->Release();
//	vertexShader = NULL;
//	shaderCode->Release();
//	shaderCode = NULL;

	// Compile the pixel shader for the application
	if (FAILED(D3DXCompileShader(	pixelShaderProgram,
									strlen(pixelShaderProgram),
									NULL,
									NULL,
									"main",
									"ps_2_0",
									0,
									&shaderCode,
									&errorMsg,
									NULL)))
	{
		OutputDebugString(errorMsg ? (CHAR*)errorMsg->GetBufferPointer() : "");
		d3d->Release();
		d3d = NULL;
		errorMsg->Release();
		errorMsg = NULL;
		return false;
	}

	// Create the pixel shader for the application
	device->CreatePixelShader((DWORD*)shaderCode->GetBufferPointer(), &pixelShader);
	device->SetPixelShader(pixelShader);

	pixelShader = NULL;

	// Create and set the vertex declaration
	device->CreateVertexDeclaration(vertexElements, &vertexDecl);
	device->SetVertexDeclaration(vertexDecl);

	// Set the initial render states
	// Turn off back face culling
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// Turn on the Z-buffer
	device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

	vertexDecl = NULL;

	d3d->Release();
	d3d = NULL;

	return true;
}

void bdGfxImpl::render() const
{
	LPDIRECT3DDEVICE9 device = bdD3D::getInstance()->getDevice();
	D3DXMATRIX matView;
	D3DXMATRIX matProj;
	D3DXVECTOR3 position(0, 0, -15);
	D3DXVECTOR3 at(0, 0, 0);
	D3DXVECTOR3 up(0, 1, 0);

	device->Clear(	0L,
					NULL,
					D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,
					m_backgroundColor,
					1.0f,
					0L);

	D3DXMatrixLookAtLH(&matView, &position, &at, &up);
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, bdFloat32(m_deviceWidth) / m_deviceHeight, 1, 1000);

	device->SetVertexShaderConstantF(4, matView, 4);
	device->SetVertexShaderConstantF(8, matProj, 4);

	// Start drawing the scene
	device->BeginScene();

	// Draw each item in the items array
	for (bdUInt i = 0; i < m_meshes.getSize(); i++)
	{
		m_meshes[i]->draw();
	}

	// Draw text if there is any
	if (m_text[0] != '\0')
	{
		bdUInt16 *wideText;

		// We need to convert the text to Unicode
		wideText = new bdUInt16[bdStrlen(m_text) + 1];
		mbstowcs(reinterpret_cast<wchar_t*>(wideText), m_text, bdStrlen(m_text) + 1);

		// Draw the text
		m_font.drawText(m_textPositionX, m_textPositionY, m_textColor, wideText);

		delete [] wideText;
	}

	// Finish drawing the scene
	device->EndScene();

	// Display the next buffer in the chain on screen
	device->Present(NULL, NULL, NULL, NULL);
}

#endif // BD_PLATFORM_XENON

