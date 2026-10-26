#include "stdafx.h"
#include "Render/Geometry/InfiniteGrid.h"
#include "Render/Shaders/Constants.h"
#include "Display/Buffer/VertexDeclarations.h"

namespace Render
{
namespace Geometry
{

Debug::Float	g_debugInfiniteGridScale	("Graphics.Render.Geometry.InfiniteGrid.Scale",		128.0f,		Debug::Range(0.0f, 128.0f));
Debug::Float	g_debugInfiniteGridFactor	("Graphics.Render.Geometry.InfiniteGrid.Factor",		1.0f,		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugInfiniteGridPower	("Graphics.Render.Geometry.InfiniteGrid.Power",		4.0f,		Debug::Range(0.0f, 16.0f));
Debug::Float	g_debugInfiniteGridForward	("Graphics.Render.Geometry.InfiniteGrid.Forward",	0.5f,		Debug::Range(0.0f, 1.0f));
Debug::Bool		g_debugInfiniteGridRender0	("Graphics.Render.Geometry.InfiniteGrid.Render0",	true);
Debug::Bool		g_debugInfiniteGridRender1	("Graphics.Render.Geometry.InfiniteGrid.Render1",	true);
Debug::Bool		g_debugInfiniteGridRender2	("Graphics.Render.Geometry.InfiniteGrid.Render2",	true);
Debug::Bool		g_debugInfiniteGridRender3	("Graphics.Render.Geometry.InfiniteGrid.Render3",	true);
Debug::Bool		g_debugInfiniteGridRender4	("Graphics.Render.Geometry.InfiniteGrid.Render4",	true);

C_InfiniteGrid::C_InfiniteGrid()
{
	m_iWidth = 64;
	m_iHeight = 64;
	m_fHeight = 0.0f;
}

C_InfiniteGrid::~C_InfiniteGrid()
{
}

bool C_InfiniteGrid::Create()
{
	m_VertexCount = (m_iWidth + 1) * (m_iHeight + 1);
	m_IndexCount = m_iHeight * (2 * m_iWidth + 4);

	if (!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XY), 0, &m_VertexBuffer0))
		return false;
	if (!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XY), 0, &m_VertexBuffer1))
		return false;
	if (!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XY), 0, &m_VertexBuffer2))
		return false;
	if (!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XY), 0, &m_VertexBuffer3))
		return false;
	if (!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(Display::Vertex_XY), 0, &m_VertexBuffer4))
		return false;

	{
		volatile Display::Vertex_XY * pVertices = (Display::Vertex_XY *) m_VertexBuffer0.Lock();
		if (pVertices)
		{
			const F32 fWidthRcp = 1.0f / (F32) m_iWidth;
			const F32 fHeightRcp = 1.0f / (F32) m_iHeight;
			for (U32 y = 0; y <= m_iHeight; ++y)
			{
				for (U32 x = 0; x <= m_iWidth; ++x)
				{
					pVertices->x = x * fWidthRcp - 0.5f;
					pVertices->y = y * fHeightRcp - 0.5f;
					pVertices++;
				}
			}
			m_VertexBuffer0.Unlock();
		}
	}
	{
		volatile Display::Vertex_XY * pVertices = (Display::Vertex_XY *) m_VertexBuffer1.Lock();
		if (pVertices)
		{
			const F32 fWidthRcp = 1.0f / (F32) m_iWidth;
			const F32 fHeightRcp = 1.0f / (F32) m_iHeight;
			for (U32 y = 0; y <= m_iHeight; ++y)
			{
				for (U32 x = 0; x <= m_iWidth; ++x)
				{
					F32 yPos = y * fHeightRcp * (1.0f + powf(1.0f + g_debugInfiniteGridFactor * y * fHeightRcp, g_debugInfiniteGridPower));

					pVertices->x = (x * fWidthRcp - 0.5f) * (1.0f + 2.0f * yPos);
					pVertices->y = 0.5f + yPos;
					pVertices++;
				}
			}
			m_VertexBuffer1.Unlock();
		}
	}
	{
		volatile Display::Vertex_XY * pVertices = (Display::Vertex_XY *) m_VertexBuffer2.Lock();
		if (pVertices)
		{
			const F32 fWidthRcp = 1.0f / (F32) m_iWidth;
			const F32 fHeightRcp = 1.0f / (F32) m_iHeight;
			for (U32 y = 0; y <= m_iHeight; ++y)
			{
				for (U32 x = 0; x <= m_iWidth; ++x)
				{
					F32 yPos = y * fHeightRcp * (1.0f + powf(1.0f + g_debugInfiniteGridFactor * y * fHeightRcp, g_debugInfiniteGridPower));

					pVertices->x = (x * fWidthRcp - 0.5f) * (1.0f + 2.0f * yPos);
					pVertices->y = -0.5f - yPos;
					pVertices++;
				}
			}
			m_VertexBuffer2.Unlock();
		}
	}
	{
		volatile Display::Vertex_XY * pVertices = (Display::Vertex_XY *) m_VertexBuffer3.Lock();
		if (pVertices)
		{
			const F32 fWidthRcp = 1.0f / (F32) m_iWidth;
			const F32 fHeightRcp = 1.0f / (F32) m_iHeight;
			for (U32 y = 0; y <= m_iHeight; ++y)
			{
				for (U32 x = 0; x <= m_iWidth; ++x)
				{
					F32 xPos = x * fWidthRcp * (1.0f + powf(1.0f + g_debugInfiniteGridFactor * x * fWidthRcp, g_debugInfiniteGridPower));

					pVertices->x = 0.5f + xPos;
					pVertices->y = (y * fHeightRcp - 0.5f) * (1.0f + 2.0f * xPos);
					pVertices++;
				}
			}
			m_VertexBuffer3.Unlock();
		}
	}
	{
		volatile Display::Vertex_XY * pVertices = (Display::Vertex_XY *) m_VertexBuffer4.Lock();
		if (pVertices)
		{
			const F32 fWidthRcp = 1.0f / (F32) m_iWidth;
			const F32 fHeightRcp = 1.0f / (F32) m_iHeight;
			for (U32 y = 0; y <= m_iHeight; ++y)
			{
				for (U32 x = 0; x <= m_iWidth; ++x)
				{
					F32 xPos = x * fWidthRcp * (1.0f + powf(1.0f + g_debugInfiniteGridFactor * x * fWidthRcp, g_debugInfiniteGridPower));

					pVertices->x = -0.5f - xPos;
					pVertices->y = (y * fHeightRcp - 0.5f) * (1.0f + 2.0f * xPos);
					pVertices++;
				}
			}
			m_VertexBuffer4.Unlock();
		}
	}

	if (!Display::Device()->CreateIndexBuffer(m_IndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;

	volatile U16 * pIndices = (U16 *) m_IndexBuffer.Lock();
	if (pIndices)
	{
		U16 baseNo = 0;
		for (U32 y = 0; y < m_iHeight; ++y)
		{
			for (U32 x = 0; x < m_iWidth; ++x)
			{
				*pIndices++ = baseNo++;
				*pIndices++ = baseNo + m_iWidth;
			}

			*pIndices++ = baseNo++;
			*pIndices++ = baseNo + m_iWidth;
			*pIndices++ = baseNo + m_iWidth;
			*pIndices++ = baseNo;
		}
		m_IndexBuffer.Unlock();
	}

	return true;
}

void C_InfiniteGrid::Destroy()
{
	Display::Device()->Destroy(&m_IndexBuffer);
	Display::Device()->Destroy(&m_VertexBuffer4);
	Display::Device()->Destroy(&m_VertexBuffer3);
	Display::Device()->Destroy(&m_VertexBuffer2);
	Display::Device()->Destroy(&m_VertexBuffer1);
	Display::Device()->Destroy(&m_VertexBuffer0);
}

void C_InfiniteGrid::Render()
{
	const Maths::HVec3 vViewForward = Display::Device()->GetViewSettings3D().GetCameraToWorld().AZ();
	const Maths::HVec3 vViewPosition = Display::Device()->GetViewSettings3D().GetCameraToWorld().AW();

	const F32 fCentreX = vViewPosition.X() + g_debugInfiniteGridScale * g_debugInfiniteGridForward * vViewForward.X();
	const F32 fCentreZ = vViewPosition.Z() + g_debugInfiniteGridScale * g_debugInfiniteGridForward * vViewForward.Z();

	const F32 fCellScale = g_debugInfiniteGridScale / (F32) m_iWidth;
	const F32 fRcpCellScale = 1.0f / fCellScale;

	const S32 nGridX = (S32) (fCentreX * fRcpCellScale);
	const S32 nGridZ = (S32) (fCentreZ * fRcpCellScale);
	const F32 fSnappedX = nGridX * fCellScale;
	const F32 fSnappedZ = nGridZ * fCellScale;

	Display::Device()->SetVertexShaderConstantF(CV_RENDER_GEOMETRY_SETTINGS, Display::C_VSConstant(fSnappedX, m_fHeight, fSnappedZ, g_debugInfiniteGridScale), 1);

	Display::Device()->SetIndexBuffer(&m_IndexBuffer);
	Display::Device()->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);

#ifndef CFG_SPEED
	if (g_debugInfiniteGridRender0)
#endif
	{
		Display::Device()->SetVertexBuffer(0, &m_VertexBuffer0, 0, sizeof(Display::Vertex_XY));
		Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount);
	}
#ifndef CFG_SPEED
	if (g_debugInfiniteGridRender1)
#endif
	{
		Display::Device()->SetVertexBuffer(0, &m_VertexBuffer1, 0, sizeof(Display::Vertex_XY));
		Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount);
	}
#ifndef CFG_SPEED
	if (g_debugInfiniteGridRender2)
#endif
	{
		Display::Device()->SetVertexBuffer(0, &m_VertexBuffer2, 0, sizeof(Display::Vertex_XY));
		Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount);
	}
#ifndef CFG_SPEED
	if (g_debugInfiniteGridRender3)
#endif
	{
		Display::Device()->SetVertexBuffer(0, &m_VertexBuffer3, 0, sizeof(Display::Vertex_XY));
		Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount);
	}
#ifndef CFG_SPEED
	if (g_debugInfiniteGridRender4)
#endif
	{
		Display::Device()->SetVertexBuffer(0, &m_VertexBuffer4, 0, sizeof(Display::Vertex_XY));
		Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount);
	}
}

}; // namespace Geometry
}; // namespace Render
