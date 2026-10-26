#include "stdafx.h"


using namespace SRD;

D3dWindow::D3dWindow()
{
	m_width = 640;
	m_height = 480;
	m_hWnd = 0;

	// Fill Out Window
	m_className = "DirectX9 Framework Code";
	m_hInstance = 0;
	m_width = 640;
	m_height = 480;
	m_bitsPerPixel = 32;
	m_depth = 32;
	m_fullScreenFlag = false;
	m_pD3d = 0;
	m_pD3dDevice = 0;
	m_d3dFormat = D3DFMT_UNKNOWN;

	ZeroMemory(&m_fonts, sizeof(m_fonts));
	m_pD3dxSprite = 0;
	ZeroMemory(&m_pp, sizeof (m_pp));
}

D3dWindow::~D3dWindow()
{
}

// Create the window and begin initialisation routines
bool D3dWindow::AttachToWindow(HINSTANCE instance, HWND hWnd)
{
	m_hInstance = instance;
	m_hWnd = hWnd;

	bool result = CreateD3dAndD3dDevice();

	if (!result) 
	{
		// for some reason we couldn't initialise d3d, destroy window and exit
		DestroyD3dAndD3dDevice();
		return false;
	}

	CreateFonts();

	F32 aspect=((F32)m_width / (F32)m_height);

	D3DXMATRIX projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, //Result Matrix
								D3DX_PI/4,          //Field of View, in radians.
								aspect,             //Aspect ratio
								1.0f,               //Near view plane
								1000.0f );           //Far view plane

	//Our Projection matrix won't change either, so we set it now and never touch
	//it again.
	m_pD3dDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

	m_pD3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	return true;
}


bool D3dWindow::CreateFonts()
{
	if (m_pD3dDevice == 0)
	{
		return false;
	}

    HRESULT hr = D3D_OK;
    S32 nHeight;

    HDC hDC = GetDC( NULL );
    S32 nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC( NULL, hDC );

    nHeight = -m_fontSize * nLogPixelsY / 72;


    nHeight = -9 * nLogPixelsY / 72;
    hr = D3DXCreateFont(m_pD3dDevice, 
						nHeight, 
						0, 
						FW_BOLD, 
						0, 
						FALSE, 
						DEFAULT_CHARSET, 
                        OUT_DEFAULT_PRECIS, 
						DEFAULT_QUALITY, 
						DEFAULT_PITCH | FF_DONTCARE, 
                        TEXT("Arial"), 
						&m_fonts[0]);

	hr = D3DXCreateFont(m_pD3dDevice,          // D3D device
						nHeight,               // Height
						0,                     // Width
						FW_BOLD,               // Weight
						0,                     // MipLevels, 0 = autogen mipmaps
						FALSE,                 // Italic
						DEFAULT_CHARSET,       // CharSet
						OUT_DEFAULT_PRECIS,    // OutputPrecision
						DEFAULT_QUALITY,       // Quality
						DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
						m_strFont,              // pFaceName
						&m_fonts[1]);              // ppFont

    nHeight = -9 * nLogPixelsY / 72;
	hr = D3DXCreateFont(m_pD3dDevice, 
						nHeight, 
						0, 
						FW_BOLD, 
						0, 
						FALSE, 
						DEFAULT_CHARSET, 
                        OUT_DEFAULT_PRECIS,
						DEFAULT_QUALITY, 
						DEFAULT_PITCH | FF_DONTCARE, 
                        TEXT("System"), 
						&m_fonts[2]);

	hr = D3DXCreateSprite(m_pD3dDevice, &m_pD3dxSprite);

	return true;
}

void D3dWindow::DestroyFonts()
{
	for (S32 i = 0; i < D3DWINDOW_NUM_FONTS; i++)
	{
		if (m_fonts[i] != 0)
		{
			m_fonts[i]->Release();
			m_fonts[i] = 0;
		}
		if (m_pD3dxSprite != 0)
		{
			m_pD3dxSprite->Release();
			m_pD3dxSprite = 0;
		}
	}
}


// Destroy window and shut everything down
void D3dWindow::Destroy()
{
	HRESULT hr = D3D_OK;

	DestroyFonts();

	// Get rid of d3d bits and pieces
	DestroyD3dAndD3dDevice();
	m_pD3dDevice = 0;
}

//******************************************************************************************
// Function:dhFind16BitMode
// Whazzit:Tests a couple of 16-bit modes to see if they are supported.  Virtually every
//         graphics card in existance will support one of these 2 formats:
//         D3DFMT_R5G6B5,D3DFMT_X1R5G5B5
//         Returns D3DFMT_UNKNOWN if it can't find a valid mode
//******************************************************************************************
D3DFORMAT Find16BitMode(IDirect3D9 *d3d){
const D3DFORMAT cBufferFormats[]={D3DFMT_R5G6B5,D3DFMT_X1R5G5B5};
const S32 cFormatCount=sizeof(cBufferFormats)/sizeof(D3DFORMAT);
HRESULT hr=D3D_OK;
S32 count;


   for(count=0;count<cFormatCount;count++){

      //CheckDeviceType() is used to verify that a Device can support a particular display mode.
      hr=d3d->CheckDeviceType(D3DADAPTER_DEFAULT, //Test the primary display device, this is
                                                 //necessary because systems can have add-on cards
                                                 //or multi-monitor setups
                                D3DDEVTYPE_HAL,  //This states that we want support for this mode
                                              //in hardware rather than emulated in software
                                cBufferFormats[count],   //The is the primary (viewable) buffer format
                                cBufferFormats[count],   //This is the back (drawable) buffer format
                                FALSE);   //Is this windowed mode?  Nope

      if(SUCCEEDED(hr)){
         return cBufferFormats[count];
      }

   }

   //dhFind16BitMode failed to find a usable mode
   return D3DFMT_UNKNOWN;
}

//******************************************************************************************
// Finds an appropriate display mode for our app
//******************************************************************************************
HRESULT D3dWindow::GetFormat(IDirect3D9 *d3d,bool fullscreen,UCHAR depth,D3DFORMAT *format)
{
HRESULT hr=D3D_OK;

   // 
   // TODO: Implement dhFind32BitMode

   if(fullscreen){
      switch(depth) {
         case 16:
            (*format)=Find16BitMode(d3d);
      	   break;
         case 32:
            //dhLog("dhGetFormat: 32-bit not implemented yet");
            (*format)=D3DFMT_UNKNOWN;
            hr=E_NOTIMPL;
      	   break;
         default:
            //dhGetFormat: called with invalid depth
            (*format)=D3DFMT_UNKNOWN;
            hr=E_FAIL;
            break;
      }
   }else{
      (*format)=D3DFMT_UNKNOWN; //D3D will automatically use the Desktop's format
   }

   return hr;
}


//******************************************************************************************
//Tests the Cooperative level and attempts to Reset the device if needed
//******************************************************************************************
HRESULT D3dWindow::HandleLostDevice()
{
	HRESULT hr;

	hr = m_pD3dDevice->TestCooperativeLevel();

	if(hr == D3DERR_DEVICELOST) 
	{ 
		//Device is lost and cannot be reset yet
		Sleep(100); 
		//Wait a bit so we don't burn through cycles for no reason
	}
	else if(hr == D3DERR_DEVICENOTRESET)
	{ 
		//Lost but we can reset it now
		hr = m_pD3dDevice->Reset(&m_pp);
	}

	return hr;
}


bool D3dWindow::CreateD3dAndD3dDevice()
{
   if(((m_pD3d) = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
   {
        return false;
   }

	D3DPRESENT_PARAMETERS d3dPresentParams;
	ZeroMemory(&d3dPresentParams, sizeof(D3DPRESENT_PARAMETERS));

	d3dPresentParams.Windowed				= true;
	d3dPresentParams.SwapEffect				= D3DSWAPEFFECT_DISCARD;	// ok
	d3dPresentParams.hDeviceWindow			= m_hWnd;
	d3dPresentParams.EnableAutoDepthStencil = true;
	d3dPresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;

	d3dPresentParams.BackBufferWidth		= m_width;
	d3dPresentParams.BackBufferHeight		= m_height;
	d3dPresentParams.BackBufferFormat		= D3DFMT_A8R8G8B8;
	d3dPresentParams.MultiSampleType		= D3DMULTISAMPLE_NONE;
	d3dPresentParams.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = m_pD3d->CreateDevice(
		D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		m_hWnd, 
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,
		&d3dPresentParams, 
		&m_pD3dDevice);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

void D3dWindow::DestroyD3dAndD3dDevice()
{
  
   if (m_pD3dDevice != 0)
   {
      m_pD3dDevice->Release();
      m_pD3dDevice = 0;
   }
   
   if (m_pD3d != 0)
   {
      m_pD3d->Release();
      m_pD3d = 0;
   }
  
}


// Get Direct3D S32erface
IDirect3D9 *D3dWindow::GetD3dInterfacePtr()
{
	return m_pD3d;
}


// Get Direct3D device
IDirect3DDevice9 *D3dWindow::GetD3dDevicePtr()
{
	return m_pD3dDevice;
}

void D3dWindow::Resize(U32 width, U32 height)
{
	if (m_hWnd == 0)
	{
		// we're not bound to a rendering surface!
		return;
	}

	DestroyFonts();

	m_width = width;
	m_height = height;

	D3DPRESENT_PARAMETERS d3dPresentParams;
    ZeroMemory(&d3dPresentParams, sizeof(D3DPRESENT_PARAMETERS));

    d3dPresentParams.Windowed				= true;
    d3dPresentParams.SwapEffect				= D3DSWAPEFFECT_DISCARD;
	d3dPresentParams.hDeviceWindow			= m_hWnd;
    d3dPresentParams.EnableAutoDepthStencil = true;
    d3dPresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;

	d3dPresentParams.BackBufferWidth		= m_width;
	d3dPresentParams.BackBufferHeight		= m_height;
    d3dPresentParams.BackBufferFormat		= D3DFMT_A8R8G8B8;
	d3dPresentParams.MultiSampleType		= D3DMULTISAMPLE_NONE;
	d3dPresentParams.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = m_pD3dDevice->Reset(&d3dPresentParams);

	m_pD3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
/*
	F32 aspect=((F32)m_width / (F32)m_height);

	D3DXMATRIX projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, //Result Matrix
								D3DX_PI/4,          //Field of View, in radians.
								aspect,             //Aspect ratio
								0.01f,               //Near view plane
								50000.0f );           //Far view plane

	//Our Projection matrix won't change either, so we set it now and never touch
	//it again.
	m_pD3dDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);
*/
	CreateFonts();
}

void D3dWindow::Print(S32 fontIndex, S32 x, S32 y, const std::string& text, const Colour4& colour)
{
	RECT r;
	SetRect(&r, x, y, 0, 0);
	m_fonts[fontIndex]->DrawText( 
		NULL, 
		text.c_str(), 
		-1, 
		&r, 
		DT_NOCLIP, 
		D3DXCOLOR( 
		colour[0], 
		colour[1],
		colour[2],
		colour[3]));
}

void D3dWindow::SetHInstance(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}

void D3dWindow::SetClassName(const std::string className)
{
	m_className = className;
}

void D3dWindow::SetHWnd(HWND hWnd)
{
	m_hWnd = hWnd;
}

void D3dWindow::SetHDC(HDC hDC)
{
	m_hDC = hDC;
}

void D3dWindow::SetHRC(HGLRC hRC)
{
	m_hRC = hRC;
}

void D3dWindow::SetIsVisibleFlag(bool flag)
{
	m_isVisibleFlag = flag;
}

void D3dWindow::SetD3dDevicePtr(IDirect3DDevice9* pD3dDevice)
{
	m_pD3dDevice = pD3dDevice;
}

HINSTANCE D3dWindow::GetHInstance() const
{
	return m_hInstance;
}

const std::string& D3dWindow::GetClassName() const
{
	return m_className;
}

HWND D3dWindow::GetHWnd() const
{
	return m_hWnd;
}

HDC	D3dWindow::GetHDC() const
{
	return m_hDC;
}

HGLRC D3dWindow::GetHRC() const
{
	return m_hRC;
}

bool D3dWindow::GetIsVisibleFlag() const
{
	return m_isVisibleFlag;
}

void D3dWindow::SetWidth(U32 width)
{
	m_width = width;
}

void D3dWindow::SetHeight(U32 height)
{
	m_height = height;
}

void D3dWindow::SetBitsPerPixel(U32 bitsPerPixel)
{
	m_bitsPerPixel = bitsPerPixel;
}

void D3dWindow::SetDepth(U32 depth)
{
	m_depth = depth;
}

void D3dWindow::SetFullScreenFlag(bool flag)
{
	m_fullScreenFlag = flag;
}

U32 D3dWindow::GetWidth() const
{
	return m_width;
}

U32 D3dWindow::GetHeight() const
{
	return m_height;
}

U32 D3dWindow::GetBitsPerPixel() const
{
	return m_bitsPerPixel;
}

U32 D3dWindow::GetDepth() const
{
	return m_depth;
}

bool D3dWindow::GetFullScreenFlag() const
{
	return m_fullScreenFlag;
}

LPD3DXFONT*	D3dWindow::GetFontPtr(S32 index)
{
	return &m_fonts[index];
}

S32	D3dWindow::GetNumFonts() const
{
	return D3DWINDOW_NUM_FONTS;
}