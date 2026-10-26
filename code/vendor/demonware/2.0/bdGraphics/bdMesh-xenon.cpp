// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdGraphics/bdGfx.h>

#include <bdGraphics/bdMesh.h>
#include <bdGraphics/bdD3D.h>

#include <xtl.h>

#define BD_LOG_LEVEL "mesh-xenon"

void bdMesh::draw() const
{
	D3DXMATRIX matWorld;
	LPDIRECT3DDEVICE9 device = bdD3D::getInstance()->getDevice();
	LPDIRECT3DVERTEXBUFFER9 d3DVertexBuffer;
	LPDIRECT3DVERTEXBUFFER9 d3DVertexColorBuffer;
	LPDIRECT3DINDEXBUFFER9 d3DIndexBuffer;
	void *lockedBuffer;
	D3DPRIMITIVETYPE primType;
	bdUInt primCount;
	bdUInt vertexCount;

	if (m_visible)
	{
		D3DXMatrixTranslation(&matWorld, m_position.getX(), m_position.getY(), m_position.getZ());
		device->SetVertexShaderConstantF(0, matWorld, 4);

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
			primCount = vertexCount - 2;		// The first three indices define a triangle and every one index after
			break;							// that defines another
		case BD_MESH_TRIANGLE_FAN:
			primType = D3DPT_TRIANGLEFAN;
			primCount = vertexCount - 2;		// The first three indices define a triangle and every one index after
			break;							// that defines another
		default:
			BD_ERR(BD_LOG_LEVEL, "meshType must be one of the bdMeshType enum values");

			primCount = 0;
			primType = D3DPT_LINELIST;
		}

		device->CreateVertexBuffer(m_numVertices * sizeof(bdVertex), D3DUSAGE_WRITEONLY, 0, 0, &d3DVertexBuffer, NULL);
		device->CreateVertexBuffer(sizeof(D3DCOLOR), D3DUSAGE_WRITEONLY, 0, 0, &d3DVertexColorBuffer, NULL);

		d3DVertexBuffer->Lock(0, 0, &lockedBuffer, 0);
		bdMemcpy(lockedBuffer, m_vertexBuffer, m_numVertices * sizeof(bdVertex));
		d3DVertexBuffer->Unlock();

		d3DVertexColorBuffer->Lock(0, 0, &lockedBuffer, 0);
		reinterpret_cast<D3DCOLOR*>(lockedBuffer)[0] = bdGfxImpl::ConvertColor(m_color);
		d3DVertexColorBuffer->Unlock();

		LPDIRECT3DVERTEXBUFFER9 oldbuffer;
		UINT offset, stride;
		device->GetStreamSource(0, &oldbuffer, &offset, &stride);
		device->SetStreamSource(0, d3DVertexBuffer, 0, sizeof(bdVertex));
		if (oldbuffer)
		{
			oldbuffer->Release();
		}

		device->GetStreamSource(1, &oldbuffer, &offset, &stride);
		device->SetStreamSource(1, d3DVertexColorBuffer, 0, 0);
		if (oldbuffer)
		{
			oldbuffer->Release();
		}
		oldbuffer = NULL;


		// Draw the mesh differently depending on whether it is indexed or not
		if (m_numIndices != 0)
		{
			device->CreateIndexBuffer(m_numIndices * sizeof(bdUInt16), NULL, D3DFMT_INDEX16, NULL,
				&d3DIndexBuffer, NULL);

			d3DIndexBuffer->Lock(0, 0, &lockedBuffer, 0);
			bdMemcpy(lockedBuffer, m_indexBuffer, m_numIndices * sizeof(bdUInt16));
			d3DIndexBuffer->Unlock();

			device->SetIndices(d3DIndexBuffer);

			device->DrawIndexedPrimitive(primType, 0, 0, 0, 0, primCount);

			d3DIndexBuffer->Release();

			// Draw the mesh
//			device->DrawIndexedPrimitiveUP(	primType, 0, m_numVertices, primCount, m_indexBuffer,
//											D3DFMT_INDEX16, m_vertexBuffer, sizeof(m_vertexBuffer[0]));
		}
		else
		{
			device->SetIndices(NULL);
			device->DrawPrimitive(primType, 0, primCount);
//			device->DrawPrimitiveUP(primType, primCount, m_vertexBuffer, sizeof(m_vertexBuffer[0]));
		}
	}
}
