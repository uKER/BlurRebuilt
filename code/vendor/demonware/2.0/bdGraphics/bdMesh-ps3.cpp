// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provides platform-dependent drawing code to allow a mesh to draw itself

#include <bdGraphics/bdMesh.h>
#include <bdGraphics/bdGfx.h>

#include <bdGraphics/bdMesh.h>

#include <PSGL/psgl.h>
#include <PSGL/psglu.h>
//#include <PSGL/psglfx.h>

#define BD_LOG_LEVEL "mesh-ps3"

void bdMesh::draw() const
{
	GLenum primType;
	GLsizei primCount;
	bdUInt vertexCount;

	// Only draw if the mesh is visible
	if (m_visible)
	{
		// Adjust the modelview matrix to position the mesh
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(m_position.getX(), m_position.getY(), m_position.getZ());

		// The vertex count is either the number of indices or vertices depending on whether or not the mesh is indexed
		vertexCount = (m_numIndices != 0) ? m_numIndices : m_numVertices;

		// Set up the primitive type and count depending on the mesh type
		switch (m_meshType)
		{
		case BD_MESH_LINE_LIST:
			primType = GL_LINES;
			primCount = vertexCount / 2;	// Every two indices define a line
			break;
		case BD_MESH_TRIANGLE_LIST:
			primType = GL_TRIANGLES;
			primCount = vertexCount / 3;	// Every three indices define a triangle
			break;
		case BD_MESH_TRIANGLE_STRIP:
			primType = GL_TRIANGLE_STRIP;
			primCount = vertexCount - 2;	// The first three indices define a triangle and
			break;							// every one index after that defines another
		case BD_MESH_TRIANGLE_FAN:
			primType = GL_TRIANGLE_FAN;
			primCount = vertexCount - 2;	// The first three indices define a triangle and
			break;							// every one index after that defines another
		default:
			BD_ERR(BD_LOG_LEVEL, "m_meshType must be one of the bdMeshType enum values");

			// Initialize variables to stop non-initialized variable compiler warning
			primCount = 0;
			primType = GL_LINES;
		}

		// Give OpenGL the pointer to the vertex position data in the vertex buffer
		glVertexPointer(3, GL_FLOAT, sizeof(bdVertex), m_vertexBuffer);

		// Give OpenGL the pointer to the color data in the vertex buffer
//		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(bdVertex), &(m_vertexBuffer[0].m_color));

		// Set the color of the mesh vertices.
		glColor4ub(m_color.getRed(), m_color.getGreen(), m_color.getBlue(), m_color.getAlpha());

		// Draw the mesh differently depending on whether it is indexed or not
		if (m_numIndices != 0)
		{
			//glDrawElements(primType, primCount, GL_UNSIGNED_SHORT, m_indexBuffer);
		}
		else
		{
//			glBegin(primType);
//			for (bdUInt i = 0; i < m_numVertices; i++)
//			{
//				glColor4ub(	bdColor::getRed(m_vertexBuffer[i].m_color),
//							bdColor::getGreen(m_vertexBuffer[i].m_color),
//							bdColor::getBlue(m_vertexBuffer[i].m_color),
//							bdColor::getAlpha(m_vertexBuffer[i].m_color));
//
//				glVertex3f(	m_vertexBuffer[i].m_x,
//							m_vertexBuffer[i].m_y,
//							m_vertexBuffer[i].m_z);
//			}
//			glEnd();

			glDrawArrays(primType, 0, m_numVertices);
		}
	}
}


