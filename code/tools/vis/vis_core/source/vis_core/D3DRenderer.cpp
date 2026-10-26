#include "stdafx.h"

#include "vis_core\PolyRef.h"
#include "vis_core\D3DRenderer.h"
#include "vis_core\D3DMesh.h"
#include "vis_core\Frustum.h"
#include <ddraw.h>
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <lm.h>
#include "vis_core\d3dmesh.h"

#define SHADOWMAP_SIZE 1024

//#define DEBUG_PS

IDirect3D9				*CD3DRenderer::m_pD3D			= NULL;
IDirect3DDevice9		*CD3DRenderer::m_pD3DDevice		= NULL;
CD3DRenderer			*CD3DRenderer::m_pInterface		= NULL;

// initialize singleton
void CD3DRenderer::Initialize(SD3DRI &ri)
{
	if (!m_pInterface)
		CD3DRenderer::m_pInterface = new CD3DRenderer(ri);
}
// destroy singleton
void CD3DRenderer::Destroy()
{
	if (m_pInterface)
	{
		delete m_pInterface;
		m_pInterface = NULL;
	}
}

static IDirect3D9 *gpD3D;
static IDirect3DDevice9 *gpD3DDevice;

bool CheckDummyD3DDevice()
{
	return gpD3DDevice != NULL;
}

void CreateDummyD3DDevice()
{
	// create D3D interface
	gpD3D = NULL;
	gpD3DDevice = NULL;

	gpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!gpD3D)
	{
		S_FATAL( g_Log, "FATAL ERROR: Could not initialize Direct3D. This might be because you are using an outdated version of DirectX.\n"
						"Please download the latest version of directX from http://www.microsoft.com/directx and try again.\n");
		exit(-1);
	}

	if (gpD3D)
	{
		// create D3D Device
		D3DPRESENT_PARAMETERS Pp;
		ZeroMemory(&Pp, sizeof(Pp));

		HWND Wnd = CreateWindow("BUTTON", "Hello", WS_OVERLAPPED, 100, 100, 32, 32, 0, 0, GetModuleHandle(0), 0);

		Pp.Windowed					= TRUE;
		Pp.hDeviceWindow			= Wnd;
		Pp.BackBufferWidth			= 32;
		Pp.BackBufferHeight			= 32;
		Pp.BackBufferFormat			= D3DFMT_A8R8G8B8;
		Pp.BackBufferCount			= 1;
		Pp.EnableAutoDepthStencil	= TRUE;
		Pp.AutoDepthStencilFormat	= D3DFMT_D24S8;
		Pp.Flags					= 0;
		Pp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
		HRESULT hr = gpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, &Pp, &gpD3DDevice);
		if (!gpD3DDevice)
		{
			S_ERROR( g_Log, "WARNING: couldn't create Direct3D Device. DDS Textures will not be written out.\n");

			switch (hr)
			{
			case D3DERR_DEVICELOST:		 S_ERROR( g_Log, "The device has been lost but cannot be reset at this time. Therefore, rendering is not possible."); break;
			case D3DERR_INVALIDCALL:	 S_ERROR( g_Log, "The method call is invalid. For example, a method's parameter may have an invalid value."); break;
			case D3DERR_NOTAVAILABLE:	 S_ERROR( g_Log, "This device does not support the queried technique."); break;
			case D3DERR_OUTOFVIDEOMEMORY:S_ERROR( g_Log, "Direct3D does not have enough display memory to perform the operation."); break;
			}
		}
	}
}
void ReleaseDummyD3DDevice()
{
	if (gpD3DDevice)
		gpD3DDevice->Release();
	if (gpD3D)
		gpD3D->Release();
	gpD3DDevice = NULL;
}

void SaveTexture(char *pszName, U32 *pAtlasTex, int w, int h, D3DFORMAT iFmt, bool bWhiteAlpha)
{
	LPDIRECT3DTEXTURE9 pTempTex, pTex;
	D3DLOCKED_RECT lr;

	if (!gpD3DDevice)
		return;
	D3DXCreateTexture(gpD3DDevice, w, h, 3, 0, iFmt, D3DPOOL_MANAGED, &pTex);

	pTex->LockRect(0,&lr,0,0);

	U8 *pOut = (U8*)lr.pBits;
	U8 *pIn = (U8*)pAtlasTex;

	switch (iFmt)
	{
	case D3DFMT_A8L8:
		// copy data into A8L8 texture
		for (int i=0; i<w*h; i++)
		{
			pOut[0] = (pAtlasTex[i] >> 8) & 0xff;
			pOut[1] = (pAtlasTex[i] >> 0) & 0xff;
			pOut+=2;
		}
		break;
	case D3DFMT_A8R8G8B8:
		// copy data into RGBA32 texture
		memcpy(pOut, pAtlasTex, w*h*4);

		// optionally white-out alpha
		if (bWhiteAlpha)
		{
			for (int i=3; i<w*h*4; i+=4)
				pOut[i] = 0xff;
		}
		break;
	case D3DFMT_R5G6B5:
		// copy data into RGB16 texture
		{
			struct Tex16
			{
				U16 blue : 5;
				U16 green : 6;
				U16 red : 5;
			};

			Tex16 *pOut16 = (Tex16*)pOut;

			for (int i=0; i<w*h; i++)
			{
				pOut16[i].red   = pIn[2] * 31 / 255;
				pOut16[i].green = pIn[1] * 63 / 255;
				pOut16[i].blue  = pIn[0] * 31 / 255;
				pIn+=4;
			}
		}
		break;
	case D3DFMT_UYVY:
		// copy data into YUV texture
		{
			struct Tex16
			{
				union { U8 U; U8 V; };
				U8 Y;
			};

			Tex16 *pOut16 = (Tex16*)pOut;

			for (int i=0; i<w*h; i++)
			{
				int R = pIn[2];
				int G = pIn[1];
				int B = pIn[0];

				int Y,U,V;

				Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
				U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
				V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

				switch (i&1)
				{
				case 0:
					pOut16[i].Y = Y;
					pOut16[i].U = U;
					break;
				case 1:
					pOut16[i].Y = Y;
					pOut16[i].V = V;
					break;
				}

				pIn+=4;
			}
		}
		break;
	default:
		LOGERROR( g_Log, "output texture format not yet supported\n");
		break;
	}

	pTex->UnlockRect(0);

	D3DXFilterTexture(pTex, NULL, 0, D3DX_FILTER_BOX);

	HRESULT hr;

	if (iFmt == D3DFMT_A8R8G8B8)
		hr = D3DXSaveTextureToFile(pszName,D3DXIFF_BMP,pTex,NULL);
	else
		hr = D3DXSaveTextureToFile(pszName,D3DXIFF_DDS,pTex,NULL);

	char buffer[512];
	if (hr != D3D_OK)
		sprintf(buffer, "*** WARNING: Couldn't save texture %s.\n", pszName );
	else
		sprintf(buffer, "Saved %s\n", pszName);

	LOGERROR( g_Log, buffer);

	pTex->Release();
}

// init class
void CD3DRenderer::SaveFrameToDisk(char *psz)
{
	LPVOID alpbi = GetFrameBuffer();

	// flip texture upside down
	U32 *pTex = new U32[m_iWidth * m_iHeight];
	for (int i=0; i<m_iHeight; i++)
		memcpy(pTex + m_iWidth *i, (int*)alpbi + m_iWidth * (m_iHeight-1-i), sizeof(int)*m_iWidth);

	// write texture to disk
	SaveTexture(psz, pTex, m_iWidth, m_iHeight, D3DFMT_A8R8G8B8, true);

	delete [] alpbi;
	delete pTex;
}

// create D3D and try doing a screengrab, check that it works
bool CD3DRenderer::TestScreenGrabbing()
{
	// render and grab a blank screen, just to check that it works
	BeginRenderSceneToAtlas(0);

	// draw something
	CD3DMesh d3dmesh;

	const U32 testId = 12345;
	FaceFlags flags;
	flags.id = testId;

	MeshNode meshnode;
	meshnode.AddPolygon(float3(-10,-10,0), float3(10,-10,0), float3(10,10,0), flags);
	d3dmesh.CreateFromMesh(&meshnode);
	SetCamera(float3(0,0,10), float3(0,0,-1), NULL);
	d3dmesh.Render();

	EndRenderScene();

	bool bFoundTestColour = false;
	bool bFoundEmpty = false;
	bool bFoundSomethingElse = false;
	int *pBMP = (int*)GetFrameBuffer();
	for (int i=0; i < m_iWidth * m_iHeight; i++)
	{
		int x = (pBMP[i] & FACE_ID_MASK);
		if (x == FACE_ID_MASK)
			bFoundEmpty = true;
		else if (x == testId)
			bFoundTestColour = true;
		else
			bFoundSomethingElse = true;
	}
	delete [] pBMP;

	if (sizeof(FaceFlags) != sizeof(U32))
	{
		assert(!"FaceFlags structure too big to pack into 32-bits!");
		return false;
	}

	if (!bFoundEmpty || !bFoundTestColour || bFoundSomethingElse)
	{
		printf("ERROR: Self test failed. Could not grab the screen properly. The program\n"
				"       will now exit.\n");
		return false;
	}

	return true;
}

bool MemberOfGroup(char *psz)
{
	char buffer[512];
	WCHAR wbuffer[512];
	WCHAR wpsz[512];
	DWORD c = 512;
	GetUserNameW(wbuffer,&c);

//MultiByteToWideChar( CP_ACP, 0, buffer, strlen(buffer)+1, wbuffer, sizeof(buffer[0])/sizeof(wbuffer[0]) );
	MultiByteToWideChar( CP_ACP, 0, psz, strlen(psz)+1, wpsz, 512 );

	LPGROUP_USERS_INFO_0 pBuf = NULL;
	DWORD dwLevel = 0;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	NET_API_STATUS nStatus;

	nStatus = NetUserGetGroups(NULL /* [Blur2PC] local machine */, wbuffer, dwLevel, (LPBYTE*)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries);

	// couldn't read groups, so can't determine whether we're part of a particular group
	if (nStatus != NERR_Success)
		return false;

	LPGROUP_USERS_INFO_0 pTmpBuf;
	DWORD i;
	DWORD dwTotalCount = 0;

	if ((pTmpBuf = pBuf) != NULL)
	{
		for (i = 0; i < dwEntriesRead; i++)
		{
			if (_wcsicmp(pTmpBuf[i].grui0_name,wpsz) == 0)
				return true;
		}
	}


	return false;
}

CD3DRenderer::CD3DRenderer(SD3DRI &ri)
{
	// get creation info
	m_iWidth		= 512;//320;
	m_iHeight		= 512;//240;

	m_iWidth		= 1024;//320;
	m_iHeight		= 1024;//240;

	m_pD3D = NULL;
	m_pD3DDevice = NULL;
	m_pAtlasTexture = NULL;

	if (MemberOfGroup("nopvs"))
	{
		printf("WARNING: This tool has been banned from rendering PVS.\n");
		return;
	}

	// create D3D interface
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	// create D3D Device
	D3DPRESENT_PARAMETERS Pp;
	ZeroMemory(&Pp, sizeof(Pp));

	m_hWnd = CreateWindow("BUTTON", "Hello", WS_OVERLAPPED, 100, 100, m_iWidth, m_iHeight, 0, 0, GetModuleHandle(0), 0);

	Pp.Windowed					= TRUE;
	Pp.hDeviceWindow			= m_hWnd;
	Pp.BackBufferWidth			= m_iWidth;
	Pp.BackBufferHeight			= m_iHeight;
	Pp.BackBufferFormat			= D3DFMT_X8R8G8B8;
	Pp.BackBufferCount			= 1;
	Pp.EnableAutoDepthStencil	= TRUE;
	Pp.AutoDepthStencilFormat	= D3DFMT_D24S8;
	Pp.Flags					= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	Pp.SwapEffect				= D3DSWAPEFFECT_DISCARD;

	if (!m_pD3D)
	{
		printf("ERROR:   Could not initialize Direct3D. This might be because you are using an \n"
			   "         outdated version of DirectX. Please download the latest version of\n"
			   "         directX from http://www.microsoft.com/directx and try again.\n");
		return;
	}

	// try to create a direct3d device
	int iRetries = 6;
	while (1)
	{
		m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Pp, &m_pD3DDevice);
		if (m_pD3DDevice)
			break;
		if (iRetries--<0)
		{
			printf("ERROR:   Could not create Direct3D Device. Possibly because some other\n"
				   "         program is running.\n");
			SAFE_RELEASE(m_pD3D);
			return;
		}
		printf("Failed to create D3D Device. Waiting to retry...\n");
		Sleep(1000);
	};

	m_pD3DDevice->CreateRenderTarget( m_iWidth, m_iHeight,
									D3DFMT_X8R8G8B8,
									D3DMULTISAMPLE_NONE,
									0,
									TRUE, // lockable
									&m_pAtlasTexture,
									NULL );

	m_pD3DDevice->SetRenderTarget(0, m_pAtlasTexture);
	if (!m_pAtlasTexture)
	{
		printf("ERROR:   Failed to create D3D render target. Your videocard probably sucks.\n");
		SAFE_RELEASE(m_pD3DDevice);
		SAFE_RELEASE(m_pD3D);
		return;
	}

	m_pD3DDevice->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);

	m_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	if (!TestScreenGrabbing())
	{
		SAFE_RELEASE(m_pAtlasTexture);
		SAFE_RELEASE(m_pD3DDevice);
		SAFE_RELEASE(m_pD3D);
	}
}

// free
CD3DRenderer::~CD3DRenderer()
{
	SAFE_RELEASE(m_pAtlasTexture);
	SAFE_RELEASE(m_pD3DDevice);
	SAFE_RELEASE(m_pD3D);
	DestroyWindow(m_hWnd);
}

// begin renderscene
void CD3DRenderer::BeginRenderSceneToAtlas(int iAtlas)
{
	// set rendertarget to the atlas we've specified
    m_pD3DDevice->SetRenderTarget( 0, m_pAtlasTexture );
	m_pD3DDevice->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

	m_pD3DDevice->BeginScene();

	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,	D3DTOP_SELECTARG2 );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );

	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );
}

// end renderscene
void CD3DRenderer::EndRenderScene()
{
	HRESULT hr;
	hr = m_pD3DDevice->EndScene();
}

// pass in a light direction and a bounding box - this function will figure out how to position the
// frustum
// out: frustum class
void CD3DRenderer::SetCamera(const float3 &vPos, const float3 &vDir, Frustum *pFrustum)
{
	D3DXMATRIXA16 mtx, mtx2;
	HMat44 mProj, mView;

	// setup the viewing matrix
	D3DXVECTOR3 vUp( 0.0f, 1.0f, 0.0f );
	D3DXMatrixTranslation( &mtx, vPos.x, vPos.y, vPos.z);
	D3DXMatrixRotationY( &mtx2, DEG2RAD(270));
	D3DXMatrixMultiply( &mtx, &mtx, &mtx2 );

	HVec3 vCross = CrossProduct(vDir, HVec3(0,1.f,0));
	if (vCross.LengthSquared() < 0.00001)
		vUp = D3DXVECTOR3(1.f, 0.f, 0.f);

	float3 vTgt = vPos + vDir;
	D3DXMatrixLookAtLH( &mtx, (D3DXVECTOR3*)&vPos, (D3DXVECTOR3*)&vTgt, &vUp);

	m_pD3DDevice->SetTransform(D3DTS_VIEW, &mtx);
	mView = D3DToNightmareMatrix44(mtx);

	// setup projection matrix
	// set near/far clip
	D3DXMatrixPerspectiveFovLH( &mtx, DEG2RAD(90.0f), (float)m_iWidth/(float)m_iHeight, 0.010, 10000);
	m_pD3DDevice->SetTransform(D3DTS_PROJECTION, &mtx);
	mProj = D3DToNightmareMatrix44(mtx);

	// setup world matrix
	D3DXMatrixIdentity( &mtx );
	m_pD3DDevice->SetTransform(D3DTS_WORLD, &mtx);

	if (pFrustum)
	{
		HMat44 mViewProjection = mView * mProj;
		pFrustum->ExtractPlanes(mViewProjection);
	}
}

HBITMAP CD3DRenderer::LoadBMPFromDC( HDC hdcScreen, int w, int h )
{
	HDC hdcCompatible = CreateCompatibleDC(hdcScreen); 
	HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, w, h ); 
	assert(hbmScreen);
	SelectObject(hdcCompatible, hbmScreen);
	BitBlt(hdcCompatible, 0,0, w, h, hdcScreen, 0, 0, SRCCOPY);
    DeleteDC( hdcCompatible );

    return( hbmScreen );
}

HANDLE CD3DRenderer::MakeDib( HBITMAP hbitmap, UINT bits )
{
	HANDLE              hdib ;
	HDC                 hdc ;
	BITMAP              bitmap ;
	UINT                wLineLen ;
	DWORD               dwSize ;
	DWORD               wColSize ;
	LPBITMAPINFOHEADER  lpbi ;
	LPBYTE              lpBits ;
	
	GetObject(hbitmap,sizeof(BITMAP),&bitmap) ;

	//
	// DWORD align the width of the DIB
	// Figure out the size of the colour table
	// Calculate the size of the DIB
	//
	wLineLen = (bitmap.bmWidth*bits+31)/32 * 4;
	wColSize = sizeof(RGBQUAD)*((bits <= 8) ? 1<<bits : 0);
	dwSize = sizeof(BITMAPINFOHEADER) + wColSize +
		(DWORD)(UINT)wLineLen*(DWORD)(UINT)bitmap.bmHeight;

	//
	// Allocate room for a DIB and set the LPBI fields
	//
	hdib = GlobalAlloc(GHND,dwSize);
	if (!hdib)
		return hdib ;

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib) ;

	lpbi->biSize = sizeof(BITMAPINFOHEADER) ;
	lpbi->biWidth = bitmap.bmWidth ;
	lpbi->biHeight = bitmap.bmHeight ;
	lpbi->biPlanes = 1 ;
	lpbi->biBitCount = (WORD) bits ;
	lpbi->biCompression = BI_RGB ;
	lpbi->biSizeImage = dwSize - sizeof(BITMAPINFOHEADER) - wColSize ;
	lpbi->biXPelsPerMeter = 0 ;
	lpbi->biYPelsPerMeter = 0 ;
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;
	lpbi->biClrImportant = 0 ;

	//
	// Get the bits from the bitmap and stuff them after the LPBI
	//
	lpBits = (LPBYTE)(lpbi+1)+wColSize ;

	hdc = CreateCompatibleDC(NULL) ;

	int result = GetDIBits(hdc,hbitmap,0,bitmap.bmHeight,lpBits,(LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

	// Fix this if GetDIBits messed it up....
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;

	DeleteDC(hdc) ;
	GlobalUnlock(hdib);

	return hdib ;
}
/*
// get framebuffer in the form of a bitmap
LPBITMAPINFOHEADER CD3DRenderer::GetFrameBuffer()
{
    // Get an image and stuff it into a bitmap.
    HBITMAP bmp;
	int width  = m_iWidth;
	int height = m_iHeight;
	HDC hDC;
	m_pAtlasTexture->GetDC(&hDC);
    bmp = LoadBMPFromDC(hDC, width, height );

	LPBITMAPINFOHEADER alpbi = (LPBITMAPINFOHEADER)GlobalLock(MakeDib(bmp, 32));
    DeleteObject( bmp );

	m_pAtlasTexture->ReleaseDC(hDC);

	return alpbi;	// caller has to do GlobalFreePtr(alpbi);
}
*/

LPVOID CD3DRenderer::GetFrameBuffer()
{
	LPDIRECT3DSURFACE9 lpDDS = m_pAtlasTexture;

    bool Success=false;
    HDC SurfDC=NULL;        // GDI-compatible device context for the surface
    HBITMAP OffscrBmp=NULL; // bitmap that is converted to a DIB
    HDC OffscrDC=NULL;      // offscreen DC that we can select OffscrBmp into
    LPBITMAPINFO lpbi=NULL; // bitmap format info; used by GetDIBits
    LPVOID lpvBits=NULL;    // pointer to bitmap bits array
    HANDLE BmpFile=INVALID_HANDLE_VALUE;    // destination .bmp file

    // Get dimensions of Surface:
    DDSURFACEDESC2 ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
	int Width = m_iWidth;
    int Height = m_iHeight;

    // Create a GDI-compatible device context for the surface:
    if (FAILED(lpDDS->GetDC(&SurfDC))) return NULL;

    // We need an HBITMAP to convert it to a DIB:
    if ((OffscrBmp = CreateCompatibleBitmap(SurfDC, Width, Height)) == NULL)
        return NULL;

    // The bitmap is empty, so let's copy the contents of the surface to it.
    // For that we need to select it into a device context. We create one.
    if ((OffscrDC = CreateCompatibleDC(SurfDC)) == NULL)
		return NULL;
    // Select OffscrBmp into OffscrDC:
    HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
    // Now we can copy the contents of the surface to the offscreen bitmap:
    BitBlt(OffscrDC, 0, 0, Width, Height, SurfDC, 0, 0, SRCCOPY);

    // We don't need SurfDC anymore. Free it:
    lpDDS->ReleaseDC(SurfDC); SurfDC = NULL;

    // GetDIBits requires format info about the bitmap. We can have GetDIBits
    // fill a structure with that info if we pass a NULL pointer for lpvBits:
    // Reserve memory for bitmap info (BITMAPINFOHEADER + largest possible
    // palette):
    if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) +
        256 * sizeof(RGBQUAD)])) == NULL)
		return NULL;
    ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    // Get info but first de-select OffscrBmp because GetDIBits requires it:
    SelectObject(OffscrDC, OldBmp);
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, Height, NULL, lpbi, DIB_RGB_COLORS))
        return NULL;

    // Reserve memory for bitmap bits:
    if ((lpvBits = new char[lpbi->bmiHeader.biSizeImage]) == NULL)
        return NULL;

    // Have GetDIBits convert OffscrBmp to a DIB (device-independent bitmap):
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, Height, lpvBits, lpbi,
        DIB_RGB_COLORS))
		return NULL;

	CloseHandle(BmpFile);
	if (SurfDC) lpDDS->ReleaseDC(SurfDC);
	if (OffscrDC) DeleteDC(OffscrDC);
	if (OffscrBmp) DeleteObject(OffscrBmp);
	if (lpbi) delete[] lpbi;

	return lpvBits;
}