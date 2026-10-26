#pragma once

namespace NexusRender
{
	class Camera;

	class ViewGrid
	{
	public:
		ViewGrid(void) : m_pVertexBuffer(NULL), m_bInitialized(false) {};
		~ViewGrid(void);

		int		m_nVertices;
		bool	m_bInitialized;

		const static DWORD	m_FVF;
		const static float	m_matAxis[3][16];

		struct SVertex
		{
			float vPos[3];
			DWORD iColour;
		};
		typedef SVertex *LPSVertex;

		LPDIRECT3DVERTEXBUFFER9	m_pVertexBuffer;

		void	Render(const Camera *pView);
		void	Initialize();
		int		InitializeGrid(
			int nCells,
			float nSpacing, 
			int iColour,
			LPSVertex &pCurV);
	};
}