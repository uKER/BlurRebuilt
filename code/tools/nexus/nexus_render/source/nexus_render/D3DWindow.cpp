#include <nexus_render/stdafx.h>

using namespace NexusRender;

C_D3DWindow::C_D3DWindow()
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
	m_pD3D = 0;
	m_pD3DDevice = 0;
	m_d3dFormat = D3DFMT_UNKNOWN;

	ZeroMemory(&m_fonts, sizeof(m_fonts));
	m_pD3DxSprite = 0;
	ZeroMemory(&m_pp, sizeof (m_pp));
}

C_D3DWindow::~C_D3DWindow()
{
	Destroy();
}

// Create the window and begin initialisation routines
bool C_D3DWindow::AttachToWindow(HINSTANCE instance, HWND hWnd)
{
	m_hInstance = instance;
	m_hWnd = hWnd;

	bool result = CreateD3DAndD3DDevice();

	if (!result) 
	{
		// for some reason we couldn't initialise d3d, destroy window and exit
		DestroyD3DAndD3DDevice();
		return false;
	}


	return Initialise();
}

bool C_D3DWindow::Initialise()
{
	CreateFonts();

	F32 aspect=((F32)m_width / (F32)m_height);

	D3DXMATRIX projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, //Result Matrix
								D3DX_PI/4,          //Field of View, in radians.
								aspect,             //Aspect ratio
								0.01f,               //Near view plane
								50000.0f );           //Far view plane

	//Our Projection matrix won't change either, so we set it now and never touch
	//it again.
	m_pD3DDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

	m_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	return true;
}

void C_D3DWindow::Uninitialise()
{
	DestroyFonts();
}

bool C_D3DWindow::CreateFonts()
{
	if (m_pD3DDevice == 0)
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
    hr = D3DXCreateFont(m_pD3DDevice, 
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

	hr = D3DXCreateFont(m_pD3DDevice,          // D3D device
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
	hr = D3DXCreateFont(m_pD3DDevice, 
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

	hr = D3DXCreateSprite(m_pD3DDevice, &m_pD3DxSprite);

	return true;
}

void C_D3DWindow::DestroyFonts()
{
	for (S32 i = 0; i < C_D3DWINDOW_NUM_FONTS; i++)
	{
		if (m_fonts[i] != 0)
		{
			m_fonts[i]->Release();
			m_fonts[i] = 0;
		}
		if (m_pD3DxSprite != 0)
		{
			m_pD3DxSprite->Release();
			m_pD3DxSprite = 0;
		}
	}
}


// Destroy window and shut everything down
void C_D3DWindow::Destroy()
{
	HRESULT hr = D3D_OK;

	DestroyFonts();

	// Get rid of d3d bits and pieces
	DestroyD3DAndD3DDevice();
	m_pD3DDevice = 0;
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
HRESULT C_D3DWindow::Format(IDirect3D9 *d3d,bool fullscreen,UCHAR depth,D3DFORMAT *format)
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
HRESULT C_D3DWindow::HandleLostDevice()
{
	HRESULT hr;

	hr = m_pD3DDevice->TestCooperativeLevel();

	if(hr == D3DERR_DEVICELOST) 
	{ 
		//Device is lost and cannot be reset yet
		Sleep(100); 
		//Wait a bit so we don't burn through cycles for no reason
		for (U32 i = 0; i < C_D3DWINDOW_NUM_FONTS; i++)
		{
			hr = m_fonts[i]->OnLostDevice();
		}
	}
	else if(hr == D3DERR_DEVICENOTRESET)
	{ 
		m_pp.BackBufferWidth		= m_width;
		m_pp.BackBufferHeight		= m_height;

		for (U32 i = 0; i < C_D3DWINDOW_NUM_FONTS; i++)
		{
			if (m_fonts[i])
				hr = m_fonts[i]->OnResetDevice();
		}

		Uninitialise();
		//Lost but we can reset it now
		hr = m_pD3DDevice->Reset(&m_pp);
		Initialise();
	}

	return hr;
}

#define NV_PERF_HUD

bool C_D3DWindow::CreateD3DAndD3DDevice()
{
   if(((m_pD3D) = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
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

	m_pp = d3dPresentParams;

	HRESULT hr;

	#ifdef D3D_USE_REF_DEVICE
		D3DDEVTYPE deviceType = D3DDEVTYPE_REF;
		UINT flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
	#else
		D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;
		UINT flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
	#endif

	// Set default settings
	UINT adapterToUse = D3DADAPTER_DEFAULT;
	#ifdef  NV_PERF_HUD
		// Look for 'NVIDIA NVPerfHUD' adapter
		// If it is present, override default settings
		for (UINT adapter = 0; adapter < m_pD3D->GetAdapterCount(); adapter++)
		{
			D3DADAPTER_IDENTIFIER9 identifier;
			HRESULT Res=m_pD3D->GetAdapterIdentifier(adapter,0,&identifier);
			if (strcmp(identifier.Description,"NVIDIA NVPerfHUD")==0)
			{
				adapterToUse=adapter;
				deviceType=D3DDEVTYPE_REF;
				break;
			}
		}
	#endif

	//Create D3D device.
	hr = m_pD3D->CreateDevice( 
		adapterToUse, 
		deviceType,
		m_hWnd, 
		flags,
		&d3dPresentParams, 
		&m_pD3DDevice);

	if (FAILED(hr))
	{
		UINT flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED;
		if (deviceType==D3DDEVTYPE_REF)
			//There's nothing more we can do :(
			return false;

		//Create software D3D device.
		hr = m_pD3D->CreateDevice( 
			adapterToUse, 
			D3DDEVTYPE_REF,
			m_hWnd, 
			flags,
			&d3dPresentParams, 
			&m_pD3DDevice);
		if (FAILED(hr))
			//There's nothing more we can do :(
			return false;
	}

	return true;
}

void C_D3DWindow::DestroyD3DAndD3DDevice()
{
  
   if (m_pD3DDevice != 0)
   {
      m_pD3DDevice->Release();
      m_pD3DDevice = 0;
   }
   
   if (m_pD3D != 0)
   {
      m_pD3D->Release();
      m_pD3D = 0;
   }
  
}


// Get Direct3D S32erface
IDirect3D9 *C_D3DWindow::InterfacePtr()
{
	return m_pD3D;
}


// Get Direct3D device
IDirect3DDevice9 *C_D3DWindow::DevicePtr()
{
	return m_pD3DDevice;
}

void C_D3DWindow::Resize(U32 width, U32 height)
{
	if (!m_hWnd || !m_pD3DDevice)
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

	if (FAILED(m_pD3DDevice->Reset(&d3dPresentParams)))
		return;

	m_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	CreateFonts();
}

void C_D3DWindow::SetHInstance(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}

void C_D3DWindow::SetClassName(const std::string className)
{
	m_className = className;
}

void C_D3DWindow::SetHWnd(HWND hWnd)
{
	m_hWnd = hWnd;
}

void C_D3DWindow::SetHRC(HGLRC hRC)
{
	m_hRC = hRC;
}

void C_D3DWindow::SetVisible(bool flag)
{
	m_isVisibleFlag = flag;
}

HINSTANCE C_D3DWindow::HInstance() const
{
	return m_hInstance;
}

const std::string& C_D3DWindow::ClassName() const
{
	return m_className;
}

HWND C_D3DWindow::GetHWnd() const
{
	return m_hWnd;
}

HGLRC C_D3DWindow::GetHRC() const
{
	return m_hRC;
}

bool C_D3DWindow::IsVisible() const
{
	return m_isVisibleFlag;
}

void C_D3DWindow::SetWidth(U32 width)
{
	m_width = width;
}

void C_D3DWindow::SetHeight(U32 height)
{
	m_height = height;
}

void C_D3DWindow::SetBitsPerPixel(U32 bitsPerPixel)
{
	m_bitsPerPixel = bitsPerPixel;
}

void C_D3DWindow::SetDepth(U32 depth)
{
	m_depth = depth;
}

void C_D3DWindow::SetFullScreen(bool flag)
{
	m_fullScreenFlag = flag;
}

U32 C_D3DWindow::GetWidth() const
{
	return m_width;
}

U32 C_D3DWindow::GetHeight() const
{
	return m_height;
}

U32 C_D3DWindow::GetBitsPerPixel() const
{
	return m_bitsPerPixel;
}

U32 C_D3DWindow::GetDepth() const
{
	return m_depth;
}

bool C_D3DWindow::IsFullScreen() const
{
	return m_fullScreenFlag;
}

LPD3DXFONT	C_D3DWindow::GetFontPtr(S32 index)
{
	return m_fonts[index];
}

S32	C_D3DWindow::GetNumFonts() const
{
	return C_D3DWINDOW_NUM_FONTS;
}
