#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/VBWrapper.h>

using namespace NexusRender;

#define NEXUS_RENDER_VB_WRAPPER_MAX_VERTS_PER_VB 65532*8

VBWrapper::VBWrapper()
{
	m_vertexSize = 0;
	m_nVerts = 0;
	m_locked = false;
}

VBWrapper::~VBWrapper()
{
	Destroy();
}

Tu::Status VBWrapper::Create(
	U32 vertexSize,
	U32 nVerts,
	U32 nVertsPerPrim)
{
	ASSERT(vertexSize);
	ASSERT(nVerts);
	ASSERT(nVertsPerPrim);
	ASSERT(nVertsPerPrim <= nVerts);
	m_nVertsPerPrim = nVertsPerPrim;
	m_vertexSize = vertexSize;
	m_nVerts = nVerts;

	D3DCAPS9 caps;
	D3DManager::GetDevicePtr()->GetDeviceCaps(&caps);

	//Determine how many VBs will be needed.
	m_maxVBSize = caps.MaxPrimitiveCount*2;
	if (m_maxVBSize > NEXUS_RENDER_VB_WRAPPER_MAX_VERTS_PER_VB)
		m_maxVBSize = NEXUS_RENDER_VB_WRAPPER_MAX_VERTS_PER_VB;
	
	if (m_maxVBSize < nVertsPerPrim)
		//This should never happen.
		return Tu::Status::FAILURE;
	m_maxVBSize -= m_maxVBSize % nVertsPerPrim;

	//Allocate space for our VB pointers.
	U32 iVB = 0;
	while(nVerts > 0)
	{
		U32 vbSize = nVerts;
		//Clamp to out maximum allowable VB size.
		if (vbSize > m_maxVBSize)
			vbSize = m_maxVBSize;
		//Make sure we can fit an entire node into a single VB. We don't want straddling of VBs.
		vbSize -= vbSize % nVertsPerPrim;

		m_vertexBuffers.PushBack();
		//Create our vertex buffer of size vbSize.
		if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
			vbSize * m_vertexSize,
			D3DUSAGE_WRITEONLY, 
			0,
			D3DPOOL_MANAGED, 
			&m_vertexBuffers[iVB].pBuffer,
			NULL)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
		m_vertexBuffers[iVB].pContents = 0;
		//Set the number of vertices in the vertex buffer.
		m_vertexBuffers[iVB].size = vbSize;

		//Decrement the number of remaining vertices.
		nVerts -= vbSize;
		iVB++;
	}

	return Tu::Status::SUCCESS;
}

void VBWrapper::Destroy()
{
	//Destroy any vertex buffers that we might have.
	for (U32 iVB = 0; iVB < (U32)m_vertexBuffers.Size(); iVB++)
	{
		m_vertexBuffers[iVB].pBuffer->Release();
	}

	m_vertexBuffers.Clear();
}

void VBWrapper::SetVertex(U32 index, void* pVertex)
{
	ASSERT(m_locked);
	U32 currentVB = index / m_maxVBSize;
	index -= currentVB * m_maxVBSize;
	
	ASSERT(index < m_vertexBuffers[currentVB].size);
	D3DVertexPosCol* pVertexBufferContents = 
		(D3DVertexPosCol*)m_vertexBuffers[currentVB].pContents;
	memcpy(&(pVertexBufferContents[index]), pVertex, m_vertexSize);
}

void* VBWrapper::GetVertexPtrByVertexIndex(U32 index)
{
	void* pVertex;
	ASSERT(m_locked);
	U32 currentVB = index / m_maxVBSize;
	pVertex = (D3DVertexPosCol*)m_vertexBuffers[currentVB].pContents;
	
	index -= currentVB * m_maxVBSize;
	U8* temp = (U8*)pVertex;
	temp += index * m_vertexSize;
	pVertex = temp;
	return pVertex;
}

void* VBWrapper::GetVertexPtrByPrimitiveIndex(U32 index)
{
	return GetVertexPtrByVertexIndex(m_nVertsPerPrim*index);
}

U32 VBWrapper::GetNumVertices() const
{
	return m_nVerts;
}

U32 VBWrapper::GetVertexSize() const
{
	return m_vertexSize;
}

void VBWrapper::Draw(D3DPRIMITIVETYPE primitiveType)
{
	//Choose appropriate vertex declaration.
	for (U16 iVB = 0; iVB < (U32)m_vertexBuffers.Size(); iVB++)
	{
		U32 numPrimitives;
		switch (primitiveType)
		{
			case D3DPT_POINTLIST:
				numPrimitives = m_vertexBuffers[iVB].size;
				break;
			case D3DPT_LINELIST:
				numPrimitives = m_vertexBuffers[iVB].size / NEXUS_RENDER_PRIMITIVES_LINE_VERTS;
				break;
			case D3DPT_LINESTRIP:
				numPrimitives = m_vertexBuffers[iVB].size -1;
				break;
			case D3DPT_TRIANGLELIST:
				numPrimitives = m_vertexBuffers[iVB].size / 3;
				break;
			case D3DPT_TRIANGLESTRIP:
			case D3DPT_TRIANGLEFAN:
			default:
			{
				ASSERT(false);
				return;
			}
		}

		if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
			0, 
			m_vertexBuffers[iVB].pBuffer, 
			0, 
			m_vertexSize)))
			ASSERT(false);

		HRESULT hr;
		hr = D3DManager::GetDevicePtr()->DrawPrimitive(
			primitiveType, 
			0, 
			numPrimitives);
		if (FAILED(hr))
			ASSERT(false);
	}
}

void VBWrapper::Lock()
{
	if (m_locked)
		return;

	Tu::Status status;
	for (U32 iVB = 0; iVB < (U32)m_vertexBuffers.Size(); iVB++)
	{
		LockVertexBuffer(iVB, m_vertexBuffers[iVB].pContents);
	}

	m_locked = true;
}

void VBWrapper::Unlock()
{
	if (!m_locked)
		return;

	for (U32 iVB = 0; iVB < (U32)m_vertexBuffers.Size(); iVB++)
	{
		UnlockVertexBuffer(iVB);
		m_vertexBuffers[iVB].pContents = 0;
	}

	m_locked = false;
}

void VBWrapper::LockVertexBuffer(U32 index, void*& pVertexBufferContents)
{
	if (FAILED(m_vertexBuffers[index].pBuffer->Lock( 
		0, 
		m_vertexBuffers[index].size * m_vertexSize, 
		(void**)&pVertexBufferContents, 
		0)))
		ASSERT(false);
}

void VBWrapper::UnlockVertexBuffer(U32 index)
{
	if (FAILED(m_vertexBuffers[index].pBuffer->Unlock()))
		ASSERT(false);
}


