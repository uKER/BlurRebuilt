// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provides platform-dependent drawing code to allow a mesh to draw itself

#include <bdGraphics/bdMesh.h>
#include <bdGraphics/bdGfx.h>
#include <bdGraphics/bdMesh.h>

#include <psptypes.h>
//#include <libemu.h>
#include <libgu.h>
#include <libgum.h>

#define BD_LOG_LEVEL "mesh-psp"

void bdMesh::draw() const
{
	bdInt primType;
	bdInt vertexCount;
	bdInt type;

	if (m_visible)
	{
		sceGumMatrixMode(SCEGU_MATRIX_WORLD);
		sceGumLoadIdentity();
		sceGumTranslate(reinterpret_cast<const ScePspFVector3*>(m_position.getData()));

		// Set the vertex type to vertex position with 32-bit color
//		type = SCEGU_VERTEX_FLOAT|SCEGU_COLOR_PF8888;
		type = SCEGU_VERTEX_FLOAT;

		// Determine whether or not to use indices
		type |= (m_numIndices != 0) ? SCEGU_INDEX_USHORT : SCEGU_INDEX_NONE;

		// Choose the correct value for the vertex count (depending on whether or not the mesh is indexed)
		vertexCount = (m_numIndices != 0) ? m_numIndices : m_numVertices;

		switch (m_meshType)
		{
		case BD_MESH_LINE_LIST:
			primType = SCEGU_PRIM_LINES;
			break;
		case BD_MESH_TRIANGLE_LIST:
			primType = SCEGU_PRIM_TRIANGLES;
			break;
		case BD_MESH_TRIANGLE_STRIP:
			primType = SCEGU_PRIM_TRIANGLE_STRIP;
			break;
		case BD_MESH_TRIANGLE_FAN:
			primType = SCEGU_PRIM_TRIANGLE_FAN;
			break;
		default:
			BD_ERR(BD_LOG_LEVEL, "m_meshType must be one of the bdMeshType enum values");

			// Initialize variable to stop non-initialized variable compiler warning
			primType = SCEGU_PRIM_LINES;
		}
					// aabbggrr
		unsigned int color = 0xff884400;
		color = m_color;

		sceGuColor(color);
		sceGumDrawArray(primType, type, vertexCount, m_indexBuffer, m_vertexBuffer);
	}
}
