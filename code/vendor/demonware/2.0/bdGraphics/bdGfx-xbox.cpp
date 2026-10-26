// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdGfx.h>

#include <bdCore/bdCore.h>
#include <bdGraphics/bdD3D.h>
#include <bdGraphics/bdMesh.h>

#ifdef BD_PLATFORM_XBOX

BD_REGISTER_SINGLETON(bdD3DDevWrapper)
BD_REGISTER_SINGLETON(bdGfxImpl)

bdGfxImpl::bdGfxImpl()
{
}

bdGfxImpl::~bdGfxImpl()
{
}

bdBool bdGfxImpl::init(bdInt32 *, bdNChar8 *[])
{
	if (!initDirect3D())
	{
		return false;
	}

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
	LPDIRECT3D8 d3d;
	LPDIRECT3DDEVICE8 device;
	D3DPRESENT_PARAMETERS d3dpp;

	XInitDevices(0, NULL);

	d3d = Direct3DCreate8(D3D_SDK_VERSION);
	if (!d3d)
	{
		OutputDebugString("Could not create Direct3D object");
		return false;
	}

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = 640;
	d3dpp.BackBufferHeight = 480;
	d3dpp.BackBufferFormat = D3DFMT_LIN_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;

	if (FAILED(d3d->CreateDevice(	0,
									D3DDEVTYPE_HAL,
									NULL,
									D3DCREATE_HARDWARE_VERTEXPROCESSING,
									&d3dpp,
									&device)))
	{
		OutputDebugString("Failed to create Direct3D device");
		return false;
	}

	DWORD shaderHandle;
	DWORD vertexDeclaration[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
		D3DVSD_STREAM(1),
		D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
		D3DVSD_END()
	};

	bdD3D::getInstance()->setDevice(device);

	device->CreateVertexShader(vertexDeclaration, NULL, &shaderHandle, 0);
	device->SetVertexShader(shaderHandle);
//	device->SetVertexShader(D3DFVF_XYZ|D3DFVF_DIFFUSE);

	device->SetRenderState(D3DRS_LIGHTING, FALSE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	d3d->Release();

	return true;
}

void bdGfxImpl::render() const
{
	LPDIRECT3DDEVICE8 device;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;
	D3DXVECTOR3 position(0, 0, -15);
	D3DXVECTOR3 at(0, 0, 0);
	D3DXVECTOR3 up(0, 1, 0);

	device = bdD3D::getInstance()->getDevice();

	device->Clear(	0L,
					NULL,
					D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
					ConvertColor(m_backgroundColor),
					1.0f,
					0L);

	D3DXMatrixLookAtLH(&matView, &position, &at, &up);
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 640.0f / 480.0f, 1, 1000);

	device->SetTransform(D3DTS_VIEW, &matView);
	device->SetTransform(D3DTS_PROJECTION, &matProj);

	device->BeginScene();

	for (bdUInt i = 0; i < m_meshes.getSize(); i++)
	{
		m_meshes[i]->draw();
	}

	if (m_text[0] != '\0')
	{
		bdUInt16 *wideText;

		// We need to convert the text to Unicode
		wideText = new bdUInt16[bdStrlen(m_text) + 1];
		mbstowcs(wideText, m_text, bdStrlen(m_text) + 1);

		// Draw the text
		m_font.drawText(m_textPositionX, m_textPositionY, m_textColor, wideText);

		delete [] wideText;
	}

	device->EndScene();

	device->Present(NULL, NULL, NULL, NULL);
}

#endif // BD_PLATFORM_XBOX

