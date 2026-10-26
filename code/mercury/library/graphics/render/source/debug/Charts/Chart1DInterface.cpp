
#include "stdafx.h"

#include "Render/Debug/Charts/Chart1DInterface.h"
#include "Display/Buffer/VertexDeclarations.h"

using namespace Render::Chart;

void I_Chart1D::RenderGrid() const
{
	if (m_fMin >= m_fMax || m_fGridSpacing < (m_fMax - m_fMin) * 0.001f)
		return;

	F32 fFirstLine = ceil(m_fMin / m_fGridSpacing);
	F32 fLastLine = floor(m_fMax / m_fGridSpacing);
	S32 nLines = S32(fLastLine) - S32(fFirstLine) + 1;

	if (nLines < 1)
		return;

	Display::Vertex_XY_C* v = (Display::Vertex_XY_C*)Display::Device()->BeginVertices(
		Display::E_PrimitiveType_LineList, 
		nLines * 2,
		sizeof(Display::Vertex_XY_C),
		Display::Vertex_XY_C::m_pDeclaration,
		true); // This can fail

	if(v)
	{
		U32 white = 0xffffffff;

		F32 fValue = fFirstLine * m_fGridSpacing;
		for (S32 i=0; i < nLines; ++i, fValue += m_fGridSpacing)
		{
			F32 fY = m_vPosition.y + m_vSize.y - (GetValueRelative(fValue) * m_vSize.y);

			v->x = m_vPosition.x;
			v->y = fY;
			v->c = white;
			v++;

			v->x = m_vPosition.x + m_vSize.x;
			v->y = fY;
			v->c = white;
			v++;
		}

		Display::Device()->EndVertices(v);
	}
}
