#pragma once

class CLightMapperView;

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

	LPDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;

	void	Render(CLightMapperView *pView);
	void	Initialize(float fRadius = 1024.0f);
	int		InitializeGrid(int nCells, float nSpacing, int iColour, LPSVertex &pCurV);
	D3DD
};
