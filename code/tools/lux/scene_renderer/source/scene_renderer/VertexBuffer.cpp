#include "stdafx.h"

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/VertexBuffer.h>

using namespace SRD;

VertexBufferResource::VertexBufferResource()
{
    m_pVertexBuffer = NULL;
}

VertexBufferResource::~VertexBufferResource()
{
    if(m_pVertexBuffer)
    {
        ////dbWarn( DBSTR "Vertex buffer not released");
    }
}

void VertexBufferResource::Init(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool)
{
    m_CreateInfo.m_Length = Length;
    m_CreateInfo.m_Usage = Usage;
    m_CreateInfo.m_FVF = FVF;
    m_CreateInfo.m_Pool = Pool;
}

S32 VertexBufferResource::IsDynamic()
{
    if(m_CreateInfo.m_Pool == D3DPOOL_DEFAULT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void VertexBufferResource::Create(D3dWindow* pD3dWindow)
{
    HRESULT hr;

	m_pD3dWindow = pD3dWindow;
        
	hr = m_pD3dWindow->GetD3dDevicePtr()->CreateVertexBuffer(
                m_CreateInfo.m_Length, 
                m_CreateInfo.m_Usage, 
                m_CreateInfo.m_FVF, 
                D3DPOOL_MANAGED, 
                &m_pVertexBuffer, 
                NULL);

    if(FAILED(hr))
    {
//        //dbError(DBSTR "Failed to create vertex buffer");
    }
}

void VertexBufferResource::Release()
{
    if(m_pVertexBuffer)
    {
        m_pVertexBuffer->Release();
    }

    m_pVertexBuffer = NULL;
}