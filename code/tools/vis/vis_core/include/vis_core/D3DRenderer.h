#ifndef __VIS_D3DRENDERER_H__
#define __VIS_D3DRENDERER_H__

/*
 * This class contains code for the lightmapper to render stuff using the graphics card
 * and read back the result.
 *
 */

// information for starting the singleton class
struct SD3DRI
{
	int m_iWidth;
	int m_iHeight;
	int m_iAtlasSize;

	SD3DRI(int w, int h, int iAtlasSize) : m_iWidth(w), m_iHeight(h), m_iAtlasSize(iAtlasSize) {};
};

class CD3DSquare;
class Frustum;

// the main singleton class
class CD3DRenderer
{
public:
static IDirect3D9			*m_pD3D;
static IDirect3DDevice9		*m_pD3DDevice;
static CD3DRenderer			*m_pInterface;
static void					Initialize(SD3DRI &);
static void					Destroy();
static const int			m_iBackBufferSize = 4;

							CD3DRenderer(SD3DRI &);
							~CD3DRenderer();

	void					BeginRenderSceneToAtlas(int iAtlas);
	void					EndRenderScene();
	void					SetCamera(const float3 &vPos, const float3 &vDir, Frustum *pFrustum);
	LPVOID					GetFrameBuffer();
	void					GetFrustum(Frustum &frustum);

	void					SaveFrameToDisk(char *psz);
	bool					TestScreenGrabbing();

	///
	int						m_iWidth, m_iHeight;
	int						m_iAtlasSize;
	int						m_nVertices;
	int						m_nAtlasTextures;
	LPDIRECT3DSURFACE9		m_pAtlasTexture;	// Texture to which the atlas is rendered
	LPDIRECT3DSURFACE9		m_pOrgRenderTarget;

	HWND					m_hWnd;

private:
	HANDLE					MakeDib( HBITMAP hbitmap, UINT bits );
	HBITMAP					LoadBMPFromDC( HDC hdcScreen, int w, int h );
};

void CreateDummyD3DDevice();
void ReleaseDummyD3DDevice();

void SaveTexture(char *pszName, U32 *pAtlasTex, int w, int h, D3DFORMAT iFmt, bool bWhiteAlpha);

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

inline HMat44 D3DToNightmareMatrix44(const D3DMATRIX& rhs)
{
	HMat44 m;
	m.AX(HVec4(rhs.m[0][0], rhs.m[0][1], rhs.m[0][2], rhs.m[0][3]));
	m.AY(HVec4(rhs.m[1][0], rhs.m[1][1], rhs.m[1][2], rhs.m[1][3]));
	m.AZ(HVec4(rhs.m[2][0], rhs.m[2][1], rhs.m[2][2], rhs.m[2][3]));
	m.AW(HVec4(rhs.m[3][0], rhs.m[3][1], rhs.m[3][2], rhs.m[3][3]));

	return m;
}

#endif