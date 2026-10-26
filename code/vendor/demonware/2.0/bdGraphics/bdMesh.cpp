// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON PS2 PSP-INFRA PSP-ADHOC PS3 WII OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdGfx.h>
#include <bdGraphics/bdMesh.h>

bdMesh::bdMesh()
:	m_color(255, 128, 0),	// default orange color
	m_position(0, 0, 0, 0)	// position object at the origin
{
	m_visible = false;
	m_meshDestroy = true;
	m_meshType = BD_MESH_TRIANGLE_STRIP;
	m_numVertices = 0;
	m_numIndices = 0;
	m_indexBuffer = BD_NULL;
	m_vertexBuffer = BD_NULL;

	bdGfx::getInstance()->addMesh(this);
}

bdMesh::~bdMesh()
{
	if (m_meshDestroy)
	{
		clear();
	}

	// Find the pointer to this object in the array
	bdGfx::getInstance()->removeMesh(this);
}

void bdMesh::setPosition(const bdVector4 &position)
{
	m_position = position;
}

void bdMesh::getPosition(bdVector4 &position) const
{
	position = m_position;
}

void bdMesh::setColor(const bdColor &color)
{
	m_color = color;

/*
	if (m_vertexBuffer)
	{
		for (bdUInt i = 0; i < m_numVertices; i++)
		{
			m_vertexBuffer[i].m_color = m_color;
		}
	}
*/
}

void bdMesh::getColor(bdColor &color) const
{
	color = m_color;
}

void bdMesh::setVisible(const bdBool visible)
{
	m_visible = visible;
}

bdBool bdMesh::isVisible() const
{
	return m_visible;
}

void bdMesh::clear()
{
	m_numIndices = 0;
	m_numVertices = 0;
	m_position = bdVector4(0, 0, 0);
	m_visible = false;

	if (m_vertexBuffer)
	{
		delete [] m_vertexBuffer;
		m_vertexBuffer = BD_NULL;
	}

	if (m_indexBuffer)
	{
		delete [] m_indexBuffer;
		m_indexBuffer = BD_NULL;
	}
}

bdBool bdMesh::init(bdMeshType type, bdUInt numIndices, bdUInt numVertices,
					const bdUInt16 *const indexBuffer, const bdVertex *const vertexBuffer)
{

	BD_ASSERT(vertexBuffer != BD_NULL, "vBuffer cannot be NULL");

	// See if the buffers have already been allocated. If so, free up that memory
	clear();

	// Store the mesh type
	m_meshType = type;

	// Store the lengths of the buffers
	m_numIndices = numIndices;
	m_numVertices = numVertices;

	// If m_numIndices > 0, we are using an indexed mesh, so set up the index buffer
	if (m_numIndices > 0)
	{
		// Allocate memory for the index buffer
		m_indexBuffer = new bdUInt16[m_numIndices];

		// Ensure the allocation was successful
		if (!m_indexBuffer)
		{
			return false;
		}

		// If m_numIndices is non-zero, then the input index buffer cannot be NULL
		BD_ASSERT(indexBuffer != BD_NULL, "The iBuffer cannot be NULL if nIndices is non-zero");
	}

	// Allocate memory for the m_vertexBuffer
	m_vertexBuffer = new bdVertex[m_numVertices];

	// Make sure the allocation succeeded
	if (!m_vertexBuffer)
	{
		// On failure, free the index buffer too (if needed)
		if (m_indexBuffer)
		{
			delete [] m_indexBuffer;
			m_indexBuffer = BD_NULL;
		}

		return false;
	}

	// Copy over the index buffer
	bdUInt i;
	for (i = 0; i < m_numIndices; i++)
	{
		m_indexBuffer[i] = indexBuffer[i];
	}

	// Copy over the vertex buffer
	for (i = 0; i < m_numVertices; i++)
	{
		m_vertexBuffer[i] = vertexBuffer[i];
	}

	// The mesh is initialized, so make it visible
	m_visible = true;

	return true;
}
