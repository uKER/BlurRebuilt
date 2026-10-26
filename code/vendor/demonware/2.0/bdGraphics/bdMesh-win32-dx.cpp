// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provides platform-dependent drawing code to allow a mesh to draw itself

#include <bdCore/bdCore.h>
#include <bdGraphics/bdGfx.h>

#if defined BD_PLATFORM_WIN32 && defined BD_WIN32_DIRECTX

#include <bdGraphics/bdMesh.h>
#include <bdGraphics/bdD3D.h>

#include <d3d9.h>
#include <d3dx9.h>

#define BD_LOG_LEVEL "mesh-win32dx"

void bdMesh::draw() const
{
	LPDIRECT3DDEVICE9 device = bdD3D::getInstance()->getDevice();
	LPDIRECT3DVERTEXBUFFER9 d3DVertexBuffer;
	LPDIRECT3DVERTEXBUFFER9 d3DVertexColorBuffer;
	LPDIRECT3DINDEXBUFFER9 d3DIndexBuffer;
	void *lockedBuffer;
	D3DXMATRIX matWorld;
	D3DPRIMITIVETYPE primType;
	bdUInt primCount;
	bdUInt vertexCount;

	// Only draw if the mesh is visible
	if (m_visible)
	{
		D3DXMatrixTranslation(&matWorld, m_position.getX(), m_position.getY(), m_position.getZ());

		device->SetTransform(D3DTS_WORLD, &matWorld);

		// The vertex count is either the number of indices or vertices depending on whether or not the mesh is indexed
		vertexCount = (m_numIndices != 0) ? m_numIndices : m_numVertices;

		// Set up the primitive type and count depending on the mesh type
		switch (m_meshType)
		{
		case BD_MESH_LINE_LIST:
			primType = D3DPT_LINELIST;
			primCount = vertexCount / 2;	// Every two indices define a line
			break;
		case BD_MESH_TRIANGLE_LIST:
			primType = D3DPT_TRIANGLELIST;
			primCount = vertexCount / 3;	// Every three indices define a triangle
			break;
		case BD_MESH_TRIANGLE_STRIP:
			primType = D3DPT_TRIANGLESTRIP;
			primCount = vertexCount - 2;	// The first three indices define a triangle and every one index after
			break;							// that defines another
		case BD_MESH_TRIANGLE_FAN:
			primType = D3DPT_TRIANGLEFAN;
			primCount = vertexCount - 2;	// The first three indices define a triangle and every one index after
			break;							// that defines another
		default:
			BD_ERR(BD_LOG_LEVEL, "m_meshType must be one of the bdMeshType enum values");

			// Initialize variables to stop non-initialized variable compiler warning
			primCount = 0;
			primType = D3DPT_LINELIST;
		}

		// Create the vertex buffer for the per-vertex data (e.g. position, normal)
		device->CreateVertexBuffer(m_numVertices * sizeof(bdVertex), D3DUSAGE_WRITEONLY, NULL, D3DPOOL_DEFAULT,
			&d3DVertexBuffer, NULL);

		// Create the vertex buffer for the per-mesh data (e.g. color, material properties)
		device->CreateVertexBuffer(sizeof(D3DCOLOR), D3DUSAGE_WRITEONLY, NULL, D3DPOOL_DEFAULT,
			&d3DVertexColorBuffer, NULL);

		// Lock the D3D vertex buffer and copy our vertex buffer into it
		d3DVertexBuffer->Lock(0, 0, &lockedBuffer, 0);
		bdMemcpy(lockedBuffer, m_vertexBuffer, m_numVertices * sizeof(bdVertex));
		d3DVertexBuffer->Unlock();

		// Lock the second vertex buffer. This will contain the color for the vertices of this mesh.
		d3DVertexColorBuffer->Lock(0, 0, &lockedBuffer, 0);
		reinterpret_cast<D3DCOLOR*>(lockedBuffer)[0] = bdGfxImpl::ConvertColor(m_color);
		d3DVertexColorBuffer->Unlock();

		// Set the per-vertex data buffer as the source of stream 0.
		device->SetStreamSource(0, d3DVertexBuffer, 0, sizeof(bdVertex));

		// Set the per-mesh data buffer as the source of stream 1. Notice we use a stride (parameter 4) of 0. This
		// ensures that the same color value is used for each vertex, so only a small buffer is required.
		device->SetStreamSource(1, d3DVertexColorBuffer, 0, 0);

		// Draw the mesh differently depending on whether it is indexed or not
		if (m_numIndices != 0)
		{
			// Indexed mesh.
			// Create a d3D index buffer.
			device->CreateIndexBuffer(m_numIndices * sizeof(bdUInt16), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
				D3DPOOL_DEFAULT, &d3DIndexBuffer, NULL);

			// Lock the buffer and copy our vertex buffer over to it
			d3DIndexBuffer->Lock(0, 0, &lockedBuffer, 0);
			bdMemcpy(lockedBuffer, m_indexBuffer, m_numIndices * sizeof(bdUInt16));
			d3DIndexBuffer->Unlock();

			// Set the indices source for the mesh
			device->SetIndices(d3DIndexBuffer);

			// Draw the mesh
			device->DrawIndexedPrimitive(primType, 0, 0, m_numVertices, 0, primCount);

//			device->DrawIndexedPrimitiveUP(	primType, 0, m_numVertices, primCount, m_indexBuffer,
//											D3DFMT_INDEX16, m_vertexBuffer, sizeof(m_vertexBuffer[0]));

			// Release the index buffer.
			d3DIndexBuffer->Release();
		}
		else
		{
			// Non-indexed mesh.
			// Set the indices source to null.
			device->SetIndices(NULL);

			// Draw the mesh.
			device->DrawPrimitive(primType, 0, primCount);

//			device->DrawPrimitiveUP(primType, primCount, m_vertexBuffer, sizeof(m_vertexBuffer[0]));
		}

		// Release the two vertex buffers.
		d3DVertexBuffer->Release();
		d3DVertexColorBuffer->Release();
	}
}

#endif // BD_PLATFORM_WIN32 && BD_WIN32_DIRECTX
