// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provides platform-dependent drawing code to allow a mesh to draw itself

#include <bdCore/bdCore.h>
#include <bdGraphics/bdGfx.h>

#include <bdGraphics/bdMesh.h>
#include <bdGraphics/bdD3D.h>

#include <xtl.h>

#define BD_LOG_LEVEL "mesh-xbox"

void bdMesh::draw() const
{
	LPDIRECT3DDEVICE8 device;
	LPDIRECT3DVERTEXBUFFER8 d3DVertexBuffer;
	LPDIRECT3DVERTEXBUFFER8 d3DVertexColorBuffer;
	LPDIRECT3DINDEXBUFFER8 d3dIndexBuffer;
	BYTE *lockedBuffer;
	D3DXMATRIX matWorld;
	D3DPRIMITIVETYPE primType;
	bdUInt primCount;
	bdUInt vertexCount;

	device = bdD3D::getInstance()->getDevice();

	// Only draw if the mesh is visible
	if (m_visible)
	{
		D3DXMatrixTranslation(&matWorld, m_position.getX(), m_position.getY(), m_position.getZ());
		device->SetTransform(D3DTS_WORLD, &matWorld);

		vertexCount = (m_numIndices != 0) ? m_numIndices : m_numVertices;

		// Set up the primitive type and count depending on the mesh type
		switch (m_meshType)
		{
		case BD_MESH_LINE_LIST:
			primType = D3DPT_LINELIST;
			primCount = vertexCount / 2;		// Every two indices define a line
			break;
		case BD_MESH_TRIANGLE_LIST:
			primType = D3DPT_TRIANGLELIST;
			primCount = vertexCount / 3;		// Every three indices define a triangle
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

			primType = D3DPT_LINELIST;
			primCount = 0;
		}

		device->CreateVertexBuffer(m_numVertices * sizeof(bdVertex), 0, 0, 0, &d3DVertexBuffer);
		device->CreateVertexBuffer(sizeof(D3DCOLOR), 0, 0, 0, &d3DVertexColorBuffer);

		d3DVertexBuffer->Lock(0, 0, &lockedBuffer, 0);
		bdMemcpy(lockedBuffer, m_vertexBuffer, m_numVertices * sizeof(bdVertex));
		d3DVertexBuffer->Unlock();

		d3DVertexColorBuffer->Lock(0, 0, &lockedBuffer, 0);
		reinterpret_cast<D3DCOLOR*>(lockedBuffer)[0] = bdGfxImpl::ConvertColor(m_color);
		d3DVertexColorBuffer->Unlock();

		device->SetStreamSource(0, d3DVertexBuffer, sizeof(bdVertex));
		device->SetStreamSource(1, d3DVertexColorBuffer, 0);

		// Draw the mesh differently depending on whether it is indexed or not
		if (m_numIndices != 0)
		{
			device->CreateIndexBuffer(m_numIndices * sizeof(bdUInt16), 0, D3DFMT_INDEX16, 0, &d3dIndexBuffer);

			d3dIndexBuffer->Lock(0, 0, &lockedBuffer, 0);
			bdMemcpy(lockedBuffer, m_indexBuffer, m_numIndices * sizeof(bdUInt16));
			d3dIndexBuffer->Unlock();

			device->SetIndices(d3dIndexBuffer, 0);

			device->DrawIndexedPrimitive(primType, 0, 0, 0, primCount);

			d3dIndexBuffer->Release();
			// Draw the mesh
//			device->DrawIndexedPrimitiveUP(	primType, 0, m_numVertices, primCount, m_indexBuffer,
//											D3DFMT_INDEX16, m_vertexBuffer, sizeof(m_vertexBuffer[0]));
		}
		else
		{
			device->SetIndices(NULL, 0);
			device->DrawPrimitive(primType, 0, primCount);

//			device->DrawPrimitiveUP(primType, primCount, m_vertexBuffer, sizeof(m_vertexBuffer[0]));
		}

		d3DVertexBuffer->Release();
		d3DVertexColorBuffer->Release();
	}
}
