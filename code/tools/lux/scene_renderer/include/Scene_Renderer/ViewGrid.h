#pragma once

namespace SRD
{

class CLightMapperView;
class Camera;

class CViewGrid
{
public:
	CViewGrid(void) : m_pVertexBuffer(NULL), m_bInitialized(false) {};
	~CViewGrid(void);

	int		m_nVertices;
	bool	m_bInitialized;

	const static DWORD	m_FVF;
	const static float	m_matAxis[3][16];

	struct SVertex
	{
		float3 vPos;
		DWORD iColour;
	};
	typedef SVertex *LPSVertex;

	IDirect3DDevice9 *m_pD3DDevice;

	LPDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;

	IDirect3DDevice9		*GetD3DDevice() { return m_pD3DDevice; }

	void	Render(const Camera *pView);
	void	Initialize(IDirect3DDevice9 *pD3DDevice);
	int		InitializeGrid(int nCells, float nSpacing, int iColour, LPSVertex &pCurV);
};

}