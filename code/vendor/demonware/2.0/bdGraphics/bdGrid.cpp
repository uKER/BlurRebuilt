// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdGrid.h>

bdGrid::bdGrid(bdUInt numColumns, bdUInt numRows,
			   bdFloat32 columnSpacing, bdFloat32 rowSpacing)
{
	m_color = bdColor(25, 25, 255);
	init(numColumns, numRows, columnSpacing, rowSpacing);
}

bdBool bdGrid::init(bdUInt numColumns, bdUInt numRows,
					bdFloat32 columnSpacing, bdFloat32 rowSpacing)
{
	// Free up any memory currently used by the mesh
	clear();

	// Fill in the member variables for the mesh
	m_meshType = BD_MESH_LINE_LIST;
	m_numVertices = ((numColumns + 1) * 2) + ((numRows + 1) * 2);
	m_numIndices = 0;

	// Create the new vertex buffer
	m_vertexBuffer = new bdVertex[m_numVertices];
	if (!m_vertexBuffer)
	{
		return false;
	}

	bdFloat32 x1 = columnSpacing * (static_cast<bdFloat32>(numColumns) / 2.0f) * -1.0f;
	bdFloat32 x2 = columnSpacing * (static_cast<bdFloat32>(numColumns) / 2.0f);
	bdFloat32 y1 = rowSpacing * (static_cast<bdFloat32>(numRows) / 2.0f);
	bdFloat32 y2 = rowSpacing * (static_cast<bdFloat32>(numRows) / 2.0f) * -1.0f;

	for (bdUInt row = 0; row < numRows + 1; row++)
	{
		bdFloat32 y = rowSpacing * ((static_cast<bdFloat32>(numRows) / 2.0f) - row);

//		m_vertexBuffer[row * 2].m_color = m_color;
		m_vertexBuffer[row * 2].m_x = x1;
		m_vertexBuffer[row * 2].m_y = y;
		m_vertexBuffer[row * 2].m_z = 0.0f;

//		m_vertexBuffer[(row * 2) + 1].m_color = m_color;
		m_vertexBuffer[(row * 2) + 1].m_x = x2;
		m_vertexBuffer[(row * 2) + 1].m_y = y;
		m_vertexBuffer[(row * 2) + 1].m_z = 0.0f;
	}

	for (bdUInt col = 0; col < numColumns + 1; col++)
	{
		bdFloat32 x = columnSpacing * ((static_cast<bdFloat32>(numColumns) / 2.0f) - col);

//		m_vertexBuffer[(col * 2) + ((numRows * 2) + 2)].m_color = m_color;
		m_vertexBuffer[(col * 2) + ((numRows * 2) + 2)].m_x = x;
		m_vertexBuffer[(col * 2) + ((numRows * 2) + 2)].m_y = y1;
		m_vertexBuffer[(col * 2) + ((numRows * 2) + 2)].m_z = 0.0f;

//		m_vertexBuffer[(col * 2) + ((numRows * 2) + 3)].m_color = m_color;
		m_vertexBuffer[(col * 2) + ((numRows * 2) + 3)].m_x = x;
		m_vertexBuffer[(col * 2) + ((numRows * 2) + 3)].m_y = y2;
		m_vertexBuffer[(col * 2) + ((numRows * 2) + 3)].m_z = 0.0f;
	}

	m_visible = true;

	return true;
}
