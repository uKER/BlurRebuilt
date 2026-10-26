#include "stdafx.h"

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/VertexBuffer.h>

#include <scene_renderer/VertexFormats.h>

#include <scene_renderer/Camera.h>

#include <scene_renderer/ObjectRenderingBatch.h>
#include <scene_renderer/ObjectRenderingFragment.h>

using namespace SRD;

ObjectRenderingFragment::ObjectRenderingFragment()
{
	Reset();
}

ObjectRenderingFragment::~ObjectRenderingFragment()
{
}

ObjectRenderingFragment::ObjectRenderingFragment(const ObjectRenderingFragment &rhs)
{
	*this = rhs;
}

const ObjectRenderingFragment& ObjectRenderingFragment::operator =(const ObjectRenderingFragment& rhs)
{
	m_pMaterialResource = rhs.m_pMaterialResource;
	m_pObjectRenderingBatch = rhs.m_pObjectRenderingBatch;
	m_vertexBufferIndices = rhs.m_vertexBufferIndices;
	m_pD3dWindow = rhs.m_pD3dWindow;

	return *this;
}

void ObjectRenderingFragment::Clear()
{
	Reset();
}

void ObjectRenderingFragment::Reset()
{
	m_pD3dWindow = 0;
	m_pMaterialResource = 0;
	m_pObjectRenderingBatch = 0;
	m_vertexBufferIndices.Clear();
}

void ObjectRenderingFragment::SetD3dWindowPtr(D3dWindow *pD3dWindow)
{
	m_pD3dWindow = pD3dWindow;
}

void ObjectRenderingFragment::SetMaterialResourcePtr(const Aftk::C_MaterialResource* pMaterialResource)
{
	m_pMaterialResource = pMaterialResource;
}

void ObjectRenderingFragment::SetObjectRenderingBatchPtr(ObjectRenderingBatch* pRenderingBatch)
{
	m_pObjectRenderingBatch = pRenderingBatch;
}

U32 ObjectRenderingFragment::AddVertexBufferIndex(U32 index)
{
	m_vertexBufferIndices.PushBack(index);
	return (U32)(m_vertexBufferIndices.Size()-1);
}

const Aftk::C_MaterialResource* ObjectRenderingFragment::GetMaterialResourcePtr() const
{
	return m_pMaterialResource;
}

const ObjectRenderingBatch* ObjectRenderingFragment::GetObjectRenderingBatchPtr() const
{
	return m_pObjectRenderingBatch;
}

U32 ObjectRenderingFragment::GetNumVertexBufferIndices() const
{
	return (U32)m_vertexBufferIndices.Size();
}

U32 ObjectRenderingFragment::GetVertexBufferIndex(U32 instance) const
{
	return (U32)m_vertexBufferIndices[instance];
}

void ObjectRenderingFragment::OnUpdateDevice()
{
	if (m_pD3dWindow == 0)
	{
		return;
	}

	// do rendering
	if (m_pObjectRenderingBatch == 0)
	{
		return;
	}

	for (U32 i = 0; i < (U32)m_vertexBufferIndices.Size(); i++)
	{
		HRESULT hr;
		U32 vertexBufferIndex = m_vertexBufferIndices[i];
		IDirect3DVertexBuffer9* pVertexBuffer = 
			m_pObjectRenderingBatch->GetVertexBufferPtr(vertexBufferIndex);

		if (pVertexBuffer != 0)
		{
			IDirect3DDevice9* pD3DDevice = m_pD3dWindow->GetD3dDevicePtr();
			hr = pD3DDevice->SetStreamSource(0, pVertexBuffer, 0, 
				sizeof(D3DVertexBasicTex));
			if (FAILED(hr)) 
			{
				return;
			}	
			hr = pD3DDevice->SetFVF(D3DFVF_BASIC_TEX);
			if (FAILED(hr)) 
			{
				return;
			}	
			U32 triCount = m_pObjectRenderingBatch->GetNumTrianglesInVertexBuffer(vertexBufferIndex);
			hr = pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, triCount);
			if (FAILED(hr)) 
			{
				return;
			}
		}
	}
}